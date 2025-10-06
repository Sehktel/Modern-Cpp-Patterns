/**
 * @file saga_pattern.cpp
 * @brief Демонстрация Saga Pattern
 * 
 * Реализован Saga Pattern с поддержкой:
 * - Choreography Saga
 * - Orchestration Saga
 * - Компенсационные транзакции
 * - Управление состоянием
 */

#include <iostream>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>
#include <queue>
#include <mutex>
#include <thread>
#include <chrono>
#include <atomic>
#include <string>
#include <exception>

// Состояния Saga
enum class SagaState {
    PENDING,
    RUNNING,
    COMPLETED,
    FAILED,
    COMPENSATING,
    COMPENSATED
};

// Результат выполнения шага
enum class StepResult {
    SUCCESS,
    FAILURE,
    RETRYABLE_FAILURE
};

// Базовый класс для шага Saga
class SagaStep {
public:
    virtual ~SagaStep() = default;
    virtual std::string getName() const = 0;
    virtual StepResult execute() = 0;
    virtual StepResult compensate() = 0;
    virtual bool isCompensatable() const = 0;
};

// Базовый класс для Saga
class Saga {
protected:
    std::string saga_id_;
    SagaState state_;
    std::vector<std::unique_ptr<SagaStep>> steps_;
    std::mutex state_mutex_;
    std::atomic<int> current_step_{0};
    
public:
    Saga(const std::string& id) : saga_id_(id), state_(SagaState::PENDING) {
        std::cout << "Saga " << saga_id_ << " создана" << std::endl;
    }
    
    virtual ~Saga() = default;
    
    void addStep(std::unique_ptr<SagaStep> step) {
        steps_.push_back(std::move(step));
    }
    
    SagaState getState() const {
        std::lock_guard<std::mutex> lock(state_mutex_);
        return state_;
    }
    
    void setState(SagaState new_state) {
        std::lock_guard<std::mutex> lock(state_mutex_);
        state_ = new_state;
        std::cout << "Saga " << saga_id_ << " перешла в состояние " 
                  << static_cast<int>(new_state) << std::endl;
    }
    
    virtual bool execute() = 0;
    virtual bool compensate() = 0;
    
    void printStatus() const {
        std::cout << "Saga " << saga_id_ << ": состояние=" 
                  << static_cast<int>(state_) 
                  << ", шаг=" << current_step_.load() 
                  << "/" << steps_.size() << std::endl;
    }
};

// Orchestration Saga - централизованное управление
class OrchestrationSaga : public Saga {
private:
    std::atomic<bool> should_compensate_{false};
    
public:
    OrchestrationSaga(const std::string& id) : Saga(id) {
        std::cout << "Orchestration Saga " << saga_id_ << " создана" << std::endl;
    }
    
    bool execute() override {
        setState(SagaState::RUNNING);
        
        for (size_t i = 0; i < steps_.size(); ++i) {
            current_step_.store(i);
            
            std::cout << "Выполняем шаг " << i << ": " << steps_[i]->getName() << std::endl;
            
            StepResult result = steps_[i]->execute();
            
            switch (result) {
                case StepResult::SUCCESS:
                    std::cout << "Шаг " << i << " выполнен успешно" << std::endl;
                    break;
                    
                case StepResult::FAILURE:
                    std::cout << "Шаг " << i << " завершился с ошибкой" << std::endl;
                    setState(SagaState::FAILED);
                    return false;
                    
                case StepResult::RETRYABLE_FAILURE:
                    std::cout << "Шаг " << i << " завершился с ошибкой, но можно повторить" << std::endl;
                    // Простая логика повтора - повторяем до 3 раз
                    for (int retry = 0; retry < 3; ++retry) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                        result = steps_[i]->execute();
                        if (result == StepResult::SUCCESS) {
                            std::cout << "Шаг " << i << " выполнен успешно после " << (retry + 1) << " попытки" << std::endl;
                            break;
                        }
                    }
                    
                    if (result != StepResult::SUCCESS) {
                        std::cout << "Шаг " << i << " не удалось выполнить после всех попыток" << std::endl;
                        setState(SagaState::FAILED);
                        return false;
                    }
                    break;
            }
        }
        
        setState(SagaState::COMPLETED);
        std::cout << "Saga " << saga_id_ << " выполнена успешно" << std::endl;
        return true;
    }
    
    bool compensate() override {
        setState(SagaState::COMPENSATING);
        
        // Выполняем компенсацию в обратном порядке
        for (int i = current_step_.load(); i >= 0; --i) {
            if (steps_[i]->isCompensatable()) {
                std::cout << "Компенсируем шаг " << i << ": " << steps_[i]->getName() << std::endl;
                
                StepResult result = steps_[i]->compensate();
                if (result != StepResult::SUCCESS) {
                    std::cout << "Ошибка компенсации шага " << i << std::endl;
                    setState(SagaState::FAILED);
                    return false;
                }
            }
        }
        
        setState(SagaState::COMPENSATED);
        std::cout << "Saga " << saga_id_ << " компенсирована" << std::endl;
        return true;
    }
};

// Choreography Saga - децентрализованное управление
class ChoreographySaga : public Saga {
private:
    std::unordered_map<std::string, std::function<void()>> event_handlers_;
    std::queue<std::string> event_queue_;
    std::mutex event_mutex_;
    std::atomic<bool> saga_completed_{false};
    
public:
    ChoreographySaga(const std::string& id) : Saga(id) {
        std::cout << "Choreography Saga " << saga_id_ << " создана" << std::endl;
    }
    
    void registerEventHandler(const std::string& event, std::function<void()> handler) {
        event_handlers_[event] = handler;
    }
    
    void publishEvent(const std::string& event) {
        std::lock_guard<std::mutex> lock(event_mutex_);
        event_queue_.push(event);
        std::cout << "Опубликовано событие: " << event << std::endl;
    }
    
    void processEvents() {
        while (!saga_completed_.load()) {
            std::string event;
            
            {
                std::lock_guard<std::mutex> lock(event_mutex_);
                if (event_queue_.empty()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    continue;
                }
                event = event_queue_.front();
                event_queue_.pop();
            }
            
            auto it = event_handlers_.find(event);
            if (it != event_handlers_.end()) {
                it->second();
            }
        }
    }
    
    bool execute() override {
        setState(SagaState::RUNNING);
        
        // Запускаем обработку событий в отдельном потоке
        std::thread event_thread([this]() { processEvents(); });
        
        // Публикуем начальное событие
        publishEvent("saga_started");
        
        // Ждем завершения
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        saga_completed_.store(true);
        event_thread.join();
        
        setState(SagaState::COMPLETED);
        return true;
    }
    
    bool compensate() override {
        setState(SagaState::COMPENSATING);
        publishEvent("saga_compensation_started");
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        setState(SagaState::COMPENSATED);
        return true;
    }
};

// Конкретные шаги Saga для демонстрации

// Шаг резервирования товара
class ReserveInventoryStep : public SagaStep {
private:
    std::string product_id_;
    int quantity_;
    bool reserved_;
    
public:
    ReserveInventoryStep(const std::string& product_id, int quantity)
        : product_id_(product_id), quantity_(quantity), reserved_(false) {}
    
    std::string getName() const override {
        return "ReserveInventory(" + product_id_ + ", " + std::to_string(quantity_) + ")";
    }
    
    StepResult execute() override {
        std::cout << "Резервируем товар " << product_id_ << " в количестве " << quantity_ << std::endl;
        
        // Симуляция резервирования
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        
        // Случайная ошибка для демонстрации
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> dis(1, 10);
        
        if (dis(gen) <= 2) { // 20% вероятность ошибки
            std::cout << "Ошибка резервирования товара " << product_id_ << std::endl;
            return StepResult::RETRYABLE_FAILURE;
        }
        
        reserved_ = true;
        std::cout << "Товар " << product_id_ << " успешно зарезервирован" << std::endl;
        return StepResult::SUCCESS;
    }
    
    StepResult compensate() override {
        if (reserved_) {
            std::cout << "Отменяем резервирование товара " << product_id_ << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            reserved_ = false;
            std::cout << "Резервирование товара " << product_id_ << " отменено" << std::endl;
        }
        return StepResult::SUCCESS;
    }
    
    bool isCompensatable() const override {
        return reserved_;
    }
};

// Шаг обработки платежа
class ProcessPaymentStep : public SagaStep {
private:
    std::string customer_id_;
    double amount_;
    bool payment_processed_;
    
public:
    ProcessPaymentStep(const std::string& customer_id, double amount)
        : customer_id_(customer_id), amount_(amount), payment_processed_(false) {}
    
    std::string getName() const override {
        return "ProcessPayment(" + customer_id_ + ", " + std::to_string(amount_) + ")";
    }
    
    StepResult execute() override {
        std::cout << "Обрабатываем платеж от клиента " << customer_id_ 
                  << " на сумму " << amount_ << std::endl;
        
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        
        // Симуляция обработки платежа
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> dis(1, 10);
        
        if (dis(gen) <= 1) { // 10% вероятность ошибки
            std::cout << "Ошибка обработки платежа от клиента " << customer_id_ << std::endl;
            return StepResult::FAILURE;
        }
        
        payment_processed_ = true;
        std::cout << "Платеж от клиента " << customer_id_ << " обработан успешно" << std::endl;
        return StepResult::SUCCESS;
    }
    
    StepResult compensate() override {
        if (payment_processed_) {
            std::cout << "Возвращаем платеж клиенту " << customer_id_ 
                      << " на сумму " << amount_ << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            payment_processed_ = false;
            std::cout << "Платеж клиенту " << customer_id_ << " возвращен" << std::endl;
        }
        return StepResult::SUCCESS;
    }
    
    bool isCompensatable() const override {
        return payment_processed_;
    }
};

// Шаг отправки заказа
class ShipOrderStep : public SagaStep {
private:
    std::string order_id_;
    std::string address_;
    bool shipped_;
    
public:
    ShipOrderStep(const std::string& order_id, const std::string& address)
        : order_id_(order_id), address_(address), shipped_(false) {}
    
    std::string getName() const override {
        return "ShipOrder(" + order_id_ + ", " + address_ + ")";
    }
    
    StepResult execute() override {
        std::cout << "Отправляем заказ " << order_id_ << " по адресу " << address_ << std::endl;
        
        std::this_thread::sleep_for(std::chrono::milliseconds(400));
        
        shipped_ = true;
        std::cout << "Заказ " << order_id_ << " отправлен" << std::endl;
        return StepResult::SUCCESS;
    }
    
    StepResult compensate() override {
        if (shipped_) {
            std::cout << "Отзываем заказ " << order_id_ << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            shipped_ = false;
            std::cout << "Заказ " << order_id_ << " отозван" << std::endl;
        }
        return StepResult::SUCCESS;
    }
    
    bool isCompensatable() const override {
        return shipped_;
    }
};

// Демонстрация Orchestration Saga
void demonstrateOrchestrationSaga() {
    std::cout << "\n=== Демонстрация Orchestration Saga ===" << std::endl;
    
    auto saga = std::make_unique<OrchestrationSaga>("order_processing_001");
    
    // Добавляем шаги
    saga->addStep(std::make_unique<ReserveInventoryStep>("product_123", 2));
    saga->addStep(std::make_unique<ProcessPaymentStep>("customer_456", 99.99));
    saga->addStep(std::make_unique<ShipOrderStep>("order_789", "123 Main St"));
    
    // Выполняем Saga
    bool success = saga->execute();
    
    if (!success) {
        std::cout << "Saga завершилась с ошибкой, выполняем компенсацию..." << std::endl;
        saga->compensate();
    }
    
    saga->printStatus();
}

// Демонстрация Choreography Saga
void demonstrateChoreographySaga() {
    std::cout << "\n=== Демонстрация Choreography Saga ===" << std::endl;
    
    auto saga = std::make_unique<ChoreographySaga>("order_processing_002");
    
    // Регистрируем обработчики событий
    saga->registerEventHandler("saga_started", [&saga]() {
        std::cout << "Обработчик: Saga началась" << std::endl;
        saga->publishEvent("inventory_reserved");
    });
    
    saga->registerEventHandler("inventory_reserved", [&saga]() {
        std::cout << "Обработчик: Товар зарезервирован" << std::endl;
        saga->publishEvent("payment_processed");
    });
    
    saga->registerEventHandler("payment_processed", [&saga]() {
        std::cout << "Обработчик: Платеж обработан" << std::endl;
        saga->publishEvent("order_shipped");
    });
    
    saga->registerEventHandler("order_shipped", [&saga]() {
        std::cout << "Обработчик: Заказ отправлен" << std::endl;
        saga->publishEvent("saga_completed");
    });
    
    saga->registerEventHandler("saga_completed", [&saga]() {
        std::cout << "Обработчик: Saga завершена" << std::endl;
    });
    
    // Выполняем Saga
    bool success = saga->execute();
    
    saga->printStatus();
}

// Демонстрация компенсации при ошибке
void demonstrateSagaCompensation() {
    std::cout << "\n=== Демонстрация компенсации Saga ===" << std::endl;
    
    auto saga = std::make_unique<OrchestrationSaga>("order_processing_003");
    
    // Добавляем шаги
    saga->addStep(std::make_unique<ReserveInventoryStep>("product_456", 1));
    saga->addStep(std::make_unique<ProcessPaymentStep>("customer_789", 199.99));
    saga->addStep(std::make_unique<ShipOrderStep>("order_101", "456 Oak Ave"));
    
    // Выполняем Saga
    bool success = saga->execute();
    
    if (!success) {
        std::cout << "Saga завершилась с ошибкой, выполняем компенсацию..." << std::endl;
        saga->compensate();
    }
    
    saga->printStatus();
}

// Демонстрация множественных Saga
void demonstrateMultipleSagas() {
    std::cout << "\n=== Демонстрация множественных Saga ===" << std::endl;
    
    std::vector<std::unique_ptr<OrchestrationSaga>> sagas;
    
    // Создаем несколько Saga
    for (int i = 0; i < 3; ++i) {
        auto saga = std::make_unique<OrchestrationSaga>("order_" + std::to_string(i + 1));
        
        saga->addStep(std::make_unique<ReserveInventoryStep>("product_" + std::to_string(i + 1), 1));
        saga->addStep(std::make_unique<ProcessPaymentStep>("customer_" + std::to_string(i + 1), 50.0 + i * 10));
        saga->addStep(std::make_unique<ShipOrderStep>("order_" + std::to_string(i + 1), "Address " + std::to_string(i + 1)));
        
        sagas.push_back(std::move(saga));
    }
    
    // Выполняем все Saga параллельно
    std::vector<std::thread> threads;
    
    for (auto& saga : sagas) {
        threads.emplace_back([&saga]() {
            bool success = saga->execute();
            if (!success) {
                saga->compensate();
            }
        });
    }
    
    // Ждем завершения всех потоков
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Выводим статус всех Saga
    for (const auto& saga : sagas) {
        saga->printStatus();
    }
}

int main() {
    std::cout << "=== Saga Pattern ===" << std::endl;
    
    try {
        demonstrateOrchestrationSaga();
        demonstrateChoreographySaga();
        demonstrateSagaCompensation();
        demonstrateMultipleSagas();
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\n=== Демонстрация завершена ===" << std::endl;
    return 0;
}
