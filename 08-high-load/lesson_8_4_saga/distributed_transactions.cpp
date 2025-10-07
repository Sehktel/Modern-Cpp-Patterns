/**
 * @file distributed_transactions.cpp
 * @brief Распределенные транзакции для Saga Pattern
 * 
 * Реализованы распределенные транзакции с поддержкой:
 * - Управление состоянием транзакций
 * - Компенсационные операции
 * - Обработка отказов
 * - Восстановление состояния
 */

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <chrono>
#include <functional>
#include <optional>
#include <unordered_map>
#include <queue>
#include <random>

// Состояние транзакции
enum class TransactionState {
    PENDING,     // Ожидает выполнения
    RUNNING,     // Выполняется
    COMPLETED,   // Успешно завершена
    FAILED,      // Провалена
    COMPENSATING,  // Выполняется компенсация
    COMPENSATED    // Компенсация завершена
};

std::string stateToString(TransactionState state) {
    switch (state) {
        case TransactionState::PENDING: return "PENDING";
        case TransactionState::RUNNING: return "RUNNING";
        case TransactionState::COMPLETED: return "COMPLETED";
        case TransactionState::FAILED: return "FAILED";
        case TransactionState::COMPENSATING: return "COMPENSATING";
        case TransactionState::COMPENSATED: return "COMPENSATED";
        default: return "UNKNOWN";
    }
}

// Шаг Saga (Transaction Step)
struct SagaStep {
    std::string name;
    std::function<bool()> action;           // Основное действие
    std::function<bool()> compensation;     // Компенсирующее действие
    TransactionState state;
    std::string error_message;
    
    SagaStep(const std::string& n, 
            std::function<bool()> act, 
            std::function<bool()> comp)
        : name(n), 
          action(std::move(act)), 
          compensation(std::move(comp)),
          state(TransactionState::PENDING) {}
};

// Saga Transaction
class SagaTransaction {
private:
    std::string transaction_id_;
    std::vector<std::shared_ptr<SagaStep>> steps_;
    TransactionState overall_state_;
    std::chrono::system_clock::time_point started_at_;
    std::chrono::system_clock::time_point completed_at_;
    mutable std::mutex mutex_;
    
public:
    explicit SagaTransaction(const std::string& id)
        : transaction_id_(id), 
          overall_state_(TransactionState::PENDING),
          started_at_(std::chrono::system_clock::now()) {
        std::cout << "Saga Transaction '" << transaction_id_ << "' создана" << std::endl;
    }
    
    // Добавление шага
    void addStep(const std::string& name,
                std::function<bool()> action,
                std::function<bool()> compensation) {
        auto step = std::make_shared<SagaStep>(name, std::move(action), std::move(compensation));
        steps_.push_back(step);
        
        std::cout << "[" << transaction_id_ << "] Добавлен шаг: " << name << std::endl;
    }
    
    // Выполнение Saga
    bool execute() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        std::cout << "\n[" << transaction_id_ << "] Начало выполнения Saga ("
                  << steps_.size() << " шагов)" << std::endl;
        
        overall_state_ = TransactionState::RUNNING;
        
        // Выполняем шаги последовательно
        for (size_t i = 0; i < steps_.size(); ++i) {
            auto& step = steps_[i];
            
            std::cout << "[" << transaction_id_ << "] Выполнение шага " << (i + 1) 
                      << "/" << steps_.size() << ": " << step->name << std::endl;
            
            step->state = TransactionState::RUNNING;
            
            try {
                bool success = step->action();
                
                if (success) {
                    step->state = TransactionState::COMPLETED;
                    std::cout << "[" << transaction_id_ << "] Шаг '" << step->name 
                              << "' успешно выполнен" << std::endl;
                } else {
                    step->state = TransactionState::FAILED;
                    step->error_message = "Action failed";
                    std::cout << "[" << transaction_id_ << "] Шаг '" << step->name 
                              << "' провален, начинается компенсация..." << std::endl;
                    
                    // Компенсируем все выполненные шаги
                    compensate(i);
                    overall_state_ = TransactionState::COMPENSATED;
                    return false;
                }
                
            } catch (const std::exception& e) {
                step->state = TransactionState::FAILED;
                step->error_message = e.what();
                std::cerr << "[" << transaction_id_ << "] Исключение в шаге '" 
                          << step->name << "': " << e.what() << std::endl;
                
                // Компенсируем все выполненные шаги
                compensate(i);
                overall_state_ = TransactionState::COMPENSATED;
                return false;
            }
        }
        
        overall_state_ = TransactionState::COMPLETED;
        completed_at_ = std::chrono::system_clock::now();
        
        std::cout << "[" << transaction_id_ << "] Saga успешно завершена!" << std::endl;
        return true;
    }
    
    // Компенсация выполненных шагов
    void compensate(size_t failed_step_index) {
        std::cout << "\n[" << transaction_id_ << "] Начинается компенсация..." << std::endl;
        
        overall_state_ = TransactionState::COMPENSATING;
        
        // Компенсируем шаги в обратном порядке
        for (int i = static_cast<int>(failed_step_index); i >= 0; --i) {
            auto& step = steps_[i];
            
            if (step->state != TransactionState::COMPLETED) {
                continue;  // Пропускаем не завершенные шаги
            }
            
            std::cout << "[" << transaction_id_ << "] Компенсация шага: " 
                      << step->name << std::endl;
            
            step->state = TransactionState::COMPENSATING;
            
            try {
                bool success = step->compensation();
                
                if (success) {
                    step->state = TransactionState::COMPENSATED;
                    std::cout << "[" << transaction_id_ << "] Компенсация '" 
                              << step->name << "' успешна" << std::endl;
                } else {
                    std::cerr << "[" << transaction_id_ << "] Ошибка компенсации '" 
                              << step->name << "'" << std::endl;
                }
                
            } catch (const std::exception& e) {
                std::cerr << "[" << transaction_id_ << "] Исключение при компенсации '" 
                          << step->name << "': " << e.what() << std::endl;
            }
        }
        
        std::cout << "[" << transaction_id_ << "] Компенсация завершена" << std::endl;
    }
    
    // Получение статистики
    void printStats() const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto duration = completed_at_ - started_at_;
        auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
        
        std::cout << "\n=== Saga Transaction '" << transaction_id_ << "' ===" << std::endl;
        std::cout << "Общее состояние: " << stateToString(overall_state_) << std::endl;
        std::cout << "Всего шагов: " << steps_.size() << std::endl;
        
        if (overall_state_ == TransactionState::COMPLETED) {
            std::cout << "Длительность: " << duration_ms.count() << " ms" << std::endl;
        }
        
        std::cout << "\nШаги:" << std::endl;
        for (size_t i = 0; i < steps_.size(); ++i) {
            const auto& step = steps_[i];
            std::cout << "  " << (i + 1) << ". " << step->name 
                      << " - " << stateToString(step->state);
            
            if (!step->error_message.empty()) {
                std::cout << " (Ошибка: " << step->error_message << ")";
            }
            
            std::cout << std::endl;
        }
        
        std::cout << "================================================" << std::endl;
    }
    
    TransactionState getState() const {
        return overall_state_;
    }
    
    const std::string& getId() const {
        return transaction_id_;
    }
};

// Менеджер Saga транзакций
class SagaOrchestrator {
private:
    std::unordered_map<std::string, std::shared_ptr<SagaTransaction>> transactions_;
    std::mutex mutex_;
    
    std::atomic<size_t> total_transactions_{0};
    std::atomic<size_t> completed_transactions_{0};
    std::atomic<size_t> failed_transactions_{0};
    std::atomic<size_t> compensated_transactions_{0};
    
public:
    SagaOrchestrator() {
        std::cout << "Saga Orchestrator создан" << std::endl;
    }
    
    // Создание новой Saga транзакции
    std::shared_ptr<SagaTransaction> createTransaction(const std::string& id) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto transaction = std::make_shared<SagaTransaction>(id);
        transactions_[id] = transaction;
        total_transactions_.fetch_add(1);
        
        return transaction;
    }
    
    // Выполнение транзакции
    bool executeTransaction(const std::string& id) {
        std::shared_ptr<SagaTransaction> transaction;
        
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = transactions_.find(id);
            if (it == transactions_.end()) {
                std::cerr << "Транзакция " << id << " не найдена" << std::endl;
                return false;
            }
            transaction = it->second;
        }
        
        bool success = transaction->execute();
        
        if (success) {
            completed_transactions_.fetch_add(1);
        } else {
            if (transaction->getState() == TransactionState::COMPENSATED) {
                compensated_transactions_.fetch_add(1);
            } else {
                failed_transactions_.fetch_add(1);
            }
        }
        
        return success;
    }
    
    // Получение транзакции
    std::shared_ptr<SagaTransaction> getTransaction(const std::string& id) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = transactions_.find(id);
        if (it != transactions_.end()) {
            return it->second;
        }
        
        return nullptr;
    }
    
    // Статистика
    void printStats() const {
        std::cout << "\n=== Saga Orchestrator Statistics ===" << std::endl;
        std::cout << "Всего транзакций: " << total_transactions_.load() << std::endl;
        std::cout << "Завершенных: " << completed_transactions_.load() << std::endl;
        std::cout << "Провалено: " << failed_transactions_.load() << std::endl;
        std::cout << "Компенсировано: " << compensated_transactions_.load() << std::endl;
        std::cout << "======================================" << std::endl;
    }
};

// Имитация сервисов
class OrderService {
public:
    bool createOrder(const std::string& order_id) {
        std::cout << "  [OrderService] Создание заказа " << order_id << "..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "  [OrderService] Заказ создан" << std::endl;
        return true;
    }
    
    bool cancelOrder(const std::string& order_id) {
        std::cout << "  [OrderService] Отмена заказа " << order_id << "..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        std::cout << "  [OrderService] Заказ отменен" << std::endl;
        return true;
    }
};

class PaymentService {
private:
    double failure_rate_;
    
public:
    PaymentService(double failure_rate = 0.0) : failure_rate_(failure_rate) {}
    
    bool processPayment(const std::string& order_id, double amount) {
        std::cout << "  [PaymentService] Обработка платежа для заказа " << order_id 
                  << " на сумму $" << amount << "..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
        
        // Симуляция ошибок
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_real_distribution<> dis(0.0, 1.0);
        
        if (dis(gen) < failure_rate_) {
            std::cout << "  [PaymentService] Платеж провален!" << std::endl;
            return false;
        }
        
        std::cout << "  [PaymentService] Платеж успешен" << std::endl;
        return true;
    }
    
    bool refundPayment(const std::string& order_id) {
        std::cout << "  [PaymentService] Возврат средств для заказа " << order_id << "..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "  [PaymentService] Средства возвращены" << std::endl;
        return true;
    }
};

class InventoryService {
public:
    bool reserveItems(const std::string& order_id) {
        std::cout << "  [InventoryService] Резервирование товаров для заказа " << order_id << "..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(120));
        std::cout << "  [InventoryService] Товары зарезервированы" << std::endl;
        return true;
    }
    
    bool releaseItems(const std::string& order_id) {
        std::cout << "  [InventoryService] Освобождение товаров для заказа " << order_id << "..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(80));
        std::cout << "  [InventoryService] Товары освобождены" << std::endl;
        return true;
    }
};

class ShippingService {
public:
    bool scheduleShipment(const std::string& order_id) {
        std::cout << "  [ShippingService] Планирование доставки для заказа " << order_id << "..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "  [ShippingService] Доставка запланирована" << std::endl;
        return true;
    }
    
    bool cancelShipment(const std::string& order_id) {
        std::cout << "  [ShippingService] Отмена доставки для заказа " << order_id << "..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(70));
        std::cout << "  [ShippingService] Доставка отменена" << std::endl;
        return true;
    }
};

// Демонстрация успешной Saga
void demonstrateSuccessfulSaga() {
    std::cout << "\n=== Демонстрация успешной Saga ===" << std::endl;
    
    SagaOrchestrator orchestrator;
    
    // Создаем сервисы
    OrderService order_service;
    PaymentService payment_service(0.0);  // 0% отказов
    InventoryService inventory_service;
    ShippingService shipping_service;
    
    // Создаем Saga транзакцию для обработки заказа
    auto saga = orchestrator.createTransaction("ORDER_12345");
    
    // Шаг 1: Создание заказа
    saga->addStep("CreateOrder",
        [&order_service]() {
            return order_service.createOrder("ORDER_12345");
        },
        [&order_service]() {
            return order_service.cancelOrder("ORDER_12345");
        }
    );
    
    // Шаг 2: Обработка платежа
    saga->addStep("ProcessPayment",
        [&payment_service]() {
            return payment_service.processPayment("ORDER_12345", 99.99);
        },
        [&payment_service]() {
            return payment_service.refundPayment("ORDER_12345");
        }
    );
    
    // Шаг 3: Резервирование товаров
    saga->addStep("ReserveInventory",
        [&inventory_service]() {
            return inventory_service.reserveItems("ORDER_12345");
        },
        [&inventory_service]() {
            return inventory_service.releaseItems("ORDER_12345");
        }
    );
    
    // Шаг 4: Планирование доставки
    saga->addStep("ScheduleShipping",
        [&shipping_service]() {
            return shipping_service.scheduleShipment("ORDER_12345");
        },
        [&shipping_service]() {
            return shipping_service.cancelShipment("ORDER_12345");
        }
    );
    
    // Выполняем Saga
    bool success = orchestrator.executeTransaction("ORDER_12345");
    
    saga->printStats();
    orchestrator.printStats();
}

// Демонстрация неудачной Saga с компенсацией
void demonstrateFailedSaga() {
    std::cout << "\n=== Демонстрация неудачной Saga с компенсацией ===" << std::endl;
    
    SagaOrchestrator orchestrator;
    
    // Создаем сервисы (платежи будут отказывать в 100% случаев)
    OrderService order_service;
    PaymentService payment_service(1.0);  // 100% отказов
    InventoryService inventory_service;
    ShippingService shipping_service;
    
    // Создаем Saga транзакцию
    auto saga = orchestrator.createTransaction("ORDER_67890");
    
    // Добавляем те же шаги
    saga->addStep("CreateOrder",
        [&order_service]() {
            return order_service.createOrder("ORDER_67890");
        },
        [&order_service]() {
            return order_service.cancelOrder("ORDER_67890");
        }
    );
    
    saga->addStep("ProcessPayment",
        [&payment_service]() {
            return payment_service.processPayment("ORDER_67890", 149.99);
        },
        [&payment_service]() {
            return payment_service.refundPayment("ORDER_67890");
        }
    );
    
    saga->addStep("ReserveInventory",
        [&inventory_service]() {
            return inventory_service.reserveItems("ORDER_67890");
        },
        [&inventory_service]() {
            return inventory_service.releaseItems("ORDER_67890");
        }
    );
    
    saga->addStep("ScheduleShipping",
        [&shipping_service]() {
            return shipping_service.scheduleShipment("ORDER_67890");
        },
        [&shipping_service]() {
            return shipping_service.cancelShipment("ORDER_67890");
        }
    );
    
    // Выполняем Saga (провалится на платеже)
    bool success = orchestrator.executeTransaction("ORDER_67890");
    
    saga->printStats();
    orchestrator.printStats();
}

// Демонстрация множественных Sagas
void demonstrateMultipleSagas() {
    std::cout << "\n=== Демонстрация множественных Sagas ===" << std::endl;
    
    SagaOrchestrator orchestrator;
    
    // Создаем сервисы с 30% отказов
    OrderService order_service;
    PaymentService payment_service(0.3);
    InventoryService inventory_service;
    ShippingService shipping_service;
    
    // Создаем несколько заказов
    for (int i = 0; i < 5; ++i) {
        std::string order_id = "ORDER_" + std::to_string(10000 + i);
        auto saga = orchestrator.createTransaction(order_id);
        
        saga->addStep("CreateOrder",
            [&order_service, order_id]() {
                return order_service.createOrder(order_id);
            },
            [&order_service, order_id]() {
                return order_service.cancelOrder(order_id);
            }
        );
        
        saga->addStep("ProcessPayment",
            [&payment_service, order_id]() {
                return payment_service.processPayment(order_id, 50.0 + i * 10);
            },
            [&payment_service, order_id]() {
                return payment_service.refundPayment(order_id);
            }
        );
        
        saga->addStep("ReserveInventory",
            [&inventory_service, order_id]() {
                return inventory_service.reserveItems(order_id);
            },
            [&inventory_service, order_id]() {
                return inventory_service.releaseItems(order_id);
            }
        );
        
        saga->addStep("ScheduleShipping",
            [&shipping_service, order_id]() {
                return shipping_service.scheduleShipment(order_id);
            },
            [&shipping_service, order_id]() {
                return shipping_service.cancelShipment(order_id);
            }
        );
        
        orchestrator.executeTransaction(order_id);
        
        std::cout << "\n" << std::endl;
    }
    
    orchestrator.printStats();
}

int main() {
    std::cout << "=== Saga Pattern: Distributed Transactions ===" << std::endl;
    
    try {
        demonstrateSuccessfulSaga();
        demonstrateFailedSaga();
        demonstrateMultipleSagas();
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\n=== Демонстрация завершена ===" << std::endl;
    return 0;
}
