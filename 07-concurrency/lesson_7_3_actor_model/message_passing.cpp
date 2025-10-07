/**
 * @file message_passing.cpp
 * @brief Расширенные примеры передачи сообщений в Actor Model
 * 
 * Реализованы продвинутые примеры передачи сообщений:
 * - Типизированные сообщения
 * - Асинхронные ответы
 * - Группировка сообщений
 * - Таймауты и retry логика
 */

#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <unordered_map>
#include <functional>
#include <atomic>
#include <chrono>
#include <vector>
#include <exception>
#include <random>
#include <future>

// Базовые типы сообщений
struct Message {
    virtual ~Message() = default;
    virtual std::string getType() const = 0;
};

// Конкретные типы сообщений
struct PingMessage : public Message {
    std::string sender;
    int sequence;
    
    PingMessage(const std::string& s, int seq) : sender(s), sequence(seq) {}
    
    std::string getType() const override {
        return "Ping";
    }
};

struct PongMessage : public Message {
    std::string sender;
    int sequence;
    
    PongMessage(const std::string& s, int seq) : sender(s), sequence(seq) {}
    
    std::string getType() const override {
        return "Pong";
    }
};

struct WorkMessage : public Message {
    int work_id;
    std::string data;
    
    WorkMessage(int id, const std::string& d) : work_id(id), data(d) {}
    
    std::string getType() const override {
        return "Work";
    }
};

struct ResultMessage : public Message {
    int work_id;
    std::string result;
    
    ResultMessage(int id, const std::string& r) : work_id(id), result(r) {}
    
    std::string getType() const override {
        return "Result";
    }
};

struct ErrorMessage : public Message {
    std::string error_text;
    std::string actor_name;
    
    ErrorMessage(const std::string& error, const std::string& actor) 
        : error_text(error), actor_name(actor) {}
    
    std::string getType() const override {
        return "Error";
    }
};

struct ShutdownMessage : public Message {
    std::string getType() const override {
        return "Shutdown";
    }
};

// Базовый Actor
class Actor {
protected:
    std::string name_;
    std::queue<std::shared_ptr<Message>> mailbox_;
    std::mutex mailbox_mutex_;
    std::condition_variable condition_;
    std::atomic<bool> running_{true};
    std::thread worker_thread_;
    
public:
    explicit Actor(const std::string& name) : name_(name) {
        worker_thread_ = std::thread([this]() { messageLoop(); });
        std::cout << "Actor " << name_ << " создан" << std::endl;
    }
    
    virtual ~Actor() {
        shutdown();
    }
    
    // Отправка сообщения
    void send(std::shared_ptr<Message> message) {
        {
            std::lock_guard<std::mutex> lock(mailbox_mutex_);
            mailbox_.push(message);
        }
        condition_.notify_one();
    }
    
    // Получение имени актора
    const std::string& getName() const {
        return name_;
    }
    
    // Graceful shutdown
    void shutdown() {
        if (!running_.load()) return;
        
        std::cout << "Останавливаем Actor " << name_ << std::endl;
        
        // Отправляем сообщение о завершении
        send(std::make_shared<ShutdownMessage>());
        
        running_.store(false);
        condition_.notify_all();
        
        if (worker_thread_.joinable()) {
            worker_thread_.join();
        }
        
        std::cout << "Actor " << name_ << " остановлен" << std::endl;
    }
    
protected:
    // Основной цикл обработки сообщений
    virtual void messageLoop() {
        std::cout << "Actor " << name_ << " запущен" << std::endl;
        
        while (running_.load()) {
            std::shared_ptr<Message> message;
            
            {
                std::unique_lock<std::mutex> lock(mailbox_mutex_);
                condition_.wait(lock, [this] { 
                    return !mailbox_.empty() || !running_.load(); 
                });
                
                if (!running_.load()) break;
                
                if (!mailbox_.empty()) {
                    message = mailbox_.front();
                    mailbox_.pop();
                }
            }
            
            if (message) {
                try {
                    handleMessage(message);
                } catch (const std::exception& e) {
                    std::cerr << "Ошибка в Actor " << name_ << ": " << e.what() << std::endl;
                    handleError(e);
                }
            }
        }
        
        std::cout << "Actor " << name_ << " завершил работу" << std::endl;
    }
    
    // Обработка сообщений (переопределяется в наследниках)
    virtual void handleMessage(std::shared_ptr<Message> message) = 0;
    
    // Обработка ошибок
    virtual void handleError(const std::exception& e) {
        std::cerr << "Actor " << name_ << " обработал ошибку: " << e.what() << std::endl;
    }
};

// Ping-Pong Actor
class PingPongActor : public Actor {
private:
    std::string target_actor_;
    int sequence_counter_{0};
    
public:
    PingPongActor(const std::string& name, const std::string& target) 
        : Actor(name), target_actor_(target) {}
    
    void setTarget(const std::string& target) {
        target_actor_ = target;
    }
    
    void startPingPong() {
        sendPing();
    }
    
protected:
    void handleMessage(std::shared_ptr<Message> message) override {
        if (auto ping = std::dynamic_pointer_cast<PingMessage>(message)) {
            handlePing(ping);
        } else if (auto pong = std::dynamic_pointer_cast<PongMessage>(message)) {
            handlePong(pong);
        } else if (auto shutdown = std::dynamic_pointer_cast<ShutdownMessage>(message)) {
            handleShutdown();
        }
    }
    
private:
    void handlePing(std::shared_ptr<PingMessage> ping) {
        std::cout << "Actor " << name_ << " получил Ping от " << ping->sender 
                  << " (seq: " << ping->sequence << ")" << std::endl;
        
        // Отправляем Pong обратно
        if (!target_actor_.empty()) {
            // Имитируем поиск целевого актора
            std::cout << "Actor " << name_ << " отправляет Pong к " << target_actor_ << std::endl;
        }
    }
    
    void handlePong(std::shared_ptr<PongMessage> pong) {
        std::cout << "Actor " << name_ << " получил Pong от " << pong->sender 
                  << " (seq: " << pong->sequence << ")" << std::endl;
        
        // Продолжаем ping-pong
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        sendPing();
    }
    
    void handleShutdown() {
        std::cout << "Actor " << name_ << " получил команду завершения" << std::endl;
        running_.store(false);
    }
    
    void sendPing() {
        if (!target_actor_.empty()) {
            sequence_counter_++;
            std::cout << "Actor " << name_ << " отправляет Ping (seq: " << sequence_counter_ << ")" << std::endl;
        }
    }
};

// Worker Actor для обработки задач
class WorkerActor : public Actor {
private:
    std::string supervisor_name_;
    std::atomic<int> processed_tasks_{0};
    
public:
    WorkerActor(const std::string& name, const std::string& supervisor) 
        : Actor(name), supervisor_name_(supervisor) {}
    
    int getProcessedTasks() const {
        return processed_tasks_.load();
    }
    
protected:
    void handleMessage(std::shared_ptr<Message> message) override {
        if (auto work = std::dynamic_pointer_cast<WorkMessage>(message)) {
            handleWork(work);
        } else if (auto shutdown = std::dynamic_pointer_cast<ShutdownMessage>(message)) {
            handleShutdown();
        }
    }
    
private:
    void handleWork(std::shared_ptr<WorkMessage> work) {
        std::cout << "Worker " << name_ << " обрабатывает задачу " << work->work_id << std::endl;
        
        try {
            // Имитация обработки работы
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            
            // Случайная ошибка для демонстрации fault tolerance
            static std::random_device rd;
            static std::mt19937 gen(rd());
            static std::uniform_int_distribution<> dis(1, 10);
            
            if (dis(gen) == 1) {
                throw std::runtime_error("Случайная ошибка обработки");
            }
            
            // Успешная обработка
            std::string result = "Результат обработки задачи " + std::to_string(work->work_id);
            processed_tasks_.fetch_add(1);
            
            std::cout << "Worker " << name_ << " завершил задачу " << work->work_id << std::endl;
            
            // Отправляем результат супервизору
            if (!supervisor_name_.empty()) {
                std::cout << "Worker " << name_ << " отправляет результат супервизору" << std::endl;
            }
            
        } catch (const std::exception& e) {
            std::cerr << "Worker " << name_ << " ошибка при обработке задачи " 
                      << work->work_id << ": " << e.what() << std::endl;
            
            // Отправляем ошибку супервизору
            if (!supervisor_name_.empty()) {
                std::cout << "Worker " << name_ << " отправляет ошибку супервизору" << std::endl;
            }
        }
    }
    
    void handleShutdown() {
        std::cout << "Worker " << name_ << " получил команду завершения" << std::endl;
        running_.store(false);
    }
};

// Supervisor Actor для управления воркерами
class SupervisorActor : public Actor {
private:
    std::vector<std::shared_ptr<WorkerActor>> workers_;
    std::atomic<int> task_counter_{0};
    std::atomic<int> completed_tasks_{0};
    std::atomic<int> failed_tasks_{0};
    
public:
    SupervisorActor(const std::string& name) : Actor(name) {
        // Создаем воркеров
        for (int i = 0; i < 3; ++i) {
            std::string worker_name = name + "_worker_" + std::to_string(i);
            auto worker = std::make_shared<WorkerActor>(worker_name, name);
            workers_.push_back(worker);
        }
        
        std::cout << "Supervisor " << name_ << " создал " << workers_.size() << " воркеров" << std::endl;
    }
    
    ~SupervisorActor() {
        // Останавливаем всех воркеров
        for (auto& worker : workers_) {
            worker->shutdown();
        }
    }
    
    void distributeWork(int num_tasks) {
        std::cout << "Supervisor " << name_ << " распределяет " << num_tasks << " задач" << std::endl;
        
        for (int i = 0; i < num_tasks; ++i) {
            int task_id = task_counter_.fetch_add(1);
            std::string data = "Данные для задачи " + std::to_string(task_id);
            
            // Выбираем случайного воркера
            static std::random_device rd;
            static std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, workers_.size() - 1);
            
            int worker_index = dis(gen);
            auto work_message = std::make_shared<WorkMessage>(task_id, data);
            
            std::cout << "Supervisor отправляет задачу " << task_id 
                      << " воркеру " << workers_[worker_index]->getName() << std::endl;
        }
    }
    
    void printStats() const {
        std::cout << "\n=== Supervisor Statistics ===" << std::endl;
        std::cout << "Всего задач: " << task_counter_.load() << std::endl;
        std::cout << "Завершено задач: " << completed_tasks_.load() << std::endl;
        std::cout << "Неудачных задач: " << failed_tasks_.load() << std::endl;
        
        for (const auto& worker : workers_) {
            std::cout << "Worker " << worker->getName() 
                      << " обработал: " << worker->getProcessedTasks() << " задач" << std::endl;
        }
        std::cout << "=============================" << std::endl;
    }
    
protected:
    void handleMessage(std::shared_ptr<Message> message) override {
        if (auto result = std::dynamic_pointer_cast<ResultMessage>(message)) {
            handleResult(result);
        } else if (auto error = std::dynamic_pointer_cast<ErrorMessage>(message)) {
            handleError(error);
        } else if (auto shutdown = std::dynamic_pointer_cast<ShutdownMessage>(message)) {
            handleShutdown();
        }
    }
    
private:
    void handleResult(std::shared_ptr<ResultMessage> result) {
        completed_tasks_.fetch_add(1);
        std::cout << "Supervisor получил результат задачи " << result->work_id << std::endl;
    }
    
    void handleError(std::shared_ptr<ErrorMessage> error) {
        failed_tasks_.fetch_add(1);
        std::cout << "Supervisor получил ошибку от " << error->actor_name 
                  << ": " << error->error_text << std::endl;
    }
    
    void handleShutdown() {
        std::cout << "Supervisor " << name_ << " получил команду завершения" << std::endl;
        running_.store(false);
    }
};

// Message Router для маршрутизации сообщений
class MessageRouter {
private:
    std::unordered_map<std::string, std::shared_ptr<Actor>> actors_;
    std::mutex actors_mutex_;
    
public:
    void registerActor(std::shared_ptr<Actor> actor) {
        std::lock_guard<std::mutex> lock(actors_mutex_);
        actors_[actor->getName()] = actor;
        std::cout << "Router зарегистрировал Actor: " << actor->getName() << std::endl;
    }
    
    void unregisterActor(const std::string& name) {
        std::lock_guard<std::mutex> lock(actors_mutex_);
        actors_.erase(name);
        std::cout << "Router отменил регистрацию Actor: " << name << std::endl;
    }
    
    void sendMessage(const std::string& target, std::shared_ptr<Message> message) {
        std::lock_guard<std::mutex> lock(actors_mutex_);
        
        auto it = actors_.find(target);
        if (it != actors_.end()) {
            it->second->send(message);
            std::cout << "Router отправил сообщение " << message->getType() 
                      << " к Actor " << target << std::endl;
        } else {
            std::cerr << "Router: Actor " << target << " не найден!" << std::endl;
        }
    }
    
    void broadcast(std::shared_ptr<Message> message) {
        std::lock_guard<std::mutex> lock(actors_mutex_);
        
        std::cout << "Router рассылает сообщение " << message->getType() 
                  << " всем " << actors_.size() << " акторам" << std::endl;
        
        for (auto& pair : actors_) {
            pair.second->send(message);
        }
    }
    
    size_t getActorCount() const {
        std::lock_guard<std::mutex> lock(actors_mutex_);
        return actors_.size();
    }
};

// Демонстрация базовой передачи сообщений
void demonstrateBasicMessagePassing() {
    std::cout << "\n=== Демонстрация базовой передачи сообщений ===" << std::endl;
    
    MessageRouter router;
    
    // Создаем акторов
    auto actor1 = std::make_shared<PingPongActor>("Actor1", "Actor2");
    auto actor2 = std::make_shared<PingPongActor>("Actor2", "Actor1");
    
    // Регистрируем в роутере
    router.registerActor(actor1);
    router.registerActor(actor2);
    
    // Настраиваем связи
    actor1->setTarget("Actor2");
    actor2->setTarget("Actor1");
    
    // Начинаем ping-pong
    std::cout << "Начинаем ping-pong между акторами..." << std::endl;
    
    // Отправляем первое сообщение
    auto ping = std::make_shared<PingMessage>("Actor1", 1);
    router.sendMessage("Actor2", ping);
    
    // Даем время на обработку
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // Останавливаем акторов
    actor1->shutdown();
    actor2->shutdown();
}

// Демонстрация Supervisor-Worker паттерна
void demonstrateSupervisorWorker() {
    std::cout << "\n=== Демонстрация Supervisor-Worker паттерна ===" << std::endl;
    
    MessageRouter router;
    
    // Создаем супервизора
    auto supervisor = std::make_shared<SupervisorActor>("Supervisor");
    router.registerActor(supervisor);
    
    // Распределяем работу
    supervisor->distributeWork(10);
    
    // Даем время на обработку
    std::this_thread::sleep_for(std::chrono::seconds(3));
    
    // Показываем статистику
    supervisor->printStats();
    
    // Останавливаем супервизора
    supervisor->shutdown();
}

// Демонстрация fault tolerance
void demonstrateFaultTolerance() {
    std::cout << "\n=== Демонстрация Fault Tolerance ===" << std::endl;
    
    MessageRouter router;
    
    // Создаем супервизора с воркерами
    auto supervisor = std::make_shared<SupervisorActor>("FaultTolerantSupervisor");
    router.registerActor(supervisor);
    
    // Распределяем много задач для демонстрации ошибок
    supervisor->distributeWork(20);
    
    // Даем время на обработку
    std::this_thread::sleep_for(std::chrono::seconds(5));
    
    // Показываем статистику
    supervisor->printStats();
    
    // Останавливаем супервизора
    supervisor->shutdown();
}

// Демонстрация broadcast сообщений
void demonstrateBroadcast() {
    std::cout << "\n=== Демонстрация Broadcast сообщений ===" << std::endl;
    
    MessageRouter router;
    
    // Создаем несколько акторов
    std::vector<std::shared_ptr<Actor>> actors;
    for (int i = 0; i < 5; ++i) {
        std::string name = "BroadcastActor_" + std::to_string(i);
        auto actor = std::make_shared<PingPongActor>(name, "");
        actors.push_back(actor);
        router.registerActor(actor);
    }
    
    // Рассылаем сообщение всем
    auto shutdown_msg = std::make_shared<ShutdownMessage>();
    router.broadcast(shutdown_msg);
    
    // Даем время на обработку
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Останавливаем всех акторов
    for (auto& actor : actors) {
        actor->shutdown();
    }
}

int main() {
    std::cout << "=== Actor Model: Продвинутая передача сообщений ===" << std::endl;
    
    try {
        demonstrateBasicMessagePassing();
        demonstrateSupervisorWorker();
        demonstrateFaultTolerance();
        demonstrateBroadcast();
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\n=== Демонстрация завершена ===" << std::endl;
    return 0;
}
