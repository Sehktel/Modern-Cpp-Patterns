/**
 * @file batch_processing.cpp
 * @brief Батчинг для Command Queue Pattern
 * 
 * Реализован батчинг с поддержкой:
 * - Группировка команд
 * - Оптимизация обработки
 * - Приоритизация команд
 * - Мониторинг производительности
 */

#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <chrono>
#include <functional>
#include <unordered_map>
#include <algorithm>

// Приоритет команды
enum class CommandPriority {
    LOW = 0,
    NORMAL = 1,
    HIGH = 2,
    CRITICAL = 3
};

std::string priorityToString(CommandPriority priority) {
    switch (priority) {
        case CommandPriority::LOW: return "LOW";
        case CommandPriority::NORMAL: return "NORMAL";
        case CommandPriority::HIGH: return "HIGH";
        case CommandPriority::CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

// Базовая команда с приоритетом
class BatchableCommand {
protected:
    CommandPriority priority_;
    std::chrono::system_clock::time_point created_at_;
    
public:
    explicit BatchableCommand(CommandPriority priority = CommandPriority::NORMAL)
        : priority_(priority), created_at_(std::chrono::system_clock::now()) {}
    
    virtual ~BatchableCommand() = default;
    virtual void execute() = 0;
    virtual std::string getName() const = 0;
    virtual std::string getBatchKey() const = 0;
    
    CommandPriority getPriority() const { return priority_; }
    
    auto getCreatedTime() const { return created_at_; }
    
    // Для сортировки по приоритету
    bool operator<(const BatchableCommand& other) const {
        if (priority_ != other.priority_) {
            return priority_ < other.priority_;  // Обратный порядок
        }
        return created_at_ > other.created_at_;  // FIFO для одного приоритета
    }
};

// Команда записи в БД
class DatabaseWriteCommand : public BatchableCommand {
private:
    std::string table_;
    int record_id_;
    std::string data_;
    
public:
    DatabaseWriteCommand(const std::string& table, int id, const std::string& data,
                        CommandPriority priority = CommandPriority::NORMAL)
        : BatchableCommand(priority), table_(table), record_id_(id), data_(data) {}
    
    void execute() override {
        std::cout << "    [DB Write] Table: " << table_ 
                  << ", Record: " << record_id_ 
                  << ", Data: " << data_ << std::endl;
        
        // Имитация записи в БД
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    
    std::string getName() const override {
        return "DatabaseWrite";
    }
    
    std::string getBatchKey() const override {
        return "db_write_" + table_;
    }
    
    const std::string& getTable() const { return table_; }
    int getRecordId() const { return record_id_; }
};

// Команда отправки email
class SendEmailCommand : public BatchableCommand {
private:
    std::string recipient_;
    std::string subject_;
    
public:
    SendEmailCommand(const std::string& recipient, const std::string& subject,
                    CommandPriority priority = CommandPriority::NORMAL)
        : BatchableCommand(priority), recipient_(recipient), subject_(subject) {}
    
    void execute() override {
        std::cout << "    [Email] To: " << recipient_ 
                  << ", Subject: " << subject_ << std::endl;
        
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    std::string getName() const override {
        return "SendEmail";
    }
    
    std::string getBatchKey() const override {
        return "email";
    }
};

// Команда обработки изображения
class ProcessImageCommand : public BatchableCommand {
private:
    std::string image_path_;
    std::string operation_;
    
public:
    ProcessImageCommand(const std::string& path, const std::string& op,
                       CommandPriority priority = CommandPriority::NORMAL)
        : BatchableCommand(priority), image_path_(path), operation_(op) {}
    
    void execute() override {
        std::cout << "    [Image] Path: " << image_path_ 
                  << ", Operation: " << operation_ << std::endl;
        
        std::this_thread::sleep_for(std::chrono::milliseconds(15));
    }
    
    std::string getName() const override {
        return "ProcessImage";
    }
    
    std::string getBatchKey() const override {
        return "image_" + operation_;
    }
};

// Батч команд
struct CommandBatch {
    std::string batch_key;
    std::vector<std::shared_ptr<BatchableCommand>> commands;
    CommandPriority highest_priority;
    std::chrono::system_clock::time_point created_at;
    
    explicit CommandBatch(const std::string& key)
        : batch_key(key), 
          highest_priority(CommandPriority::LOW),
          created_at(std::chrono::system_clock::now()) {}
    
    void addCommand(std::shared_ptr<BatchableCommand> command) {
        if (command->getPriority() > highest_priority) {
            highest_priority = command->getPriority();
        }
        commands.push_back(std::move(command));
    }
    
    size_t size() const {
        return commands.size();
    }
    
    // Для сортировки батчей
    bool operator<(const CommandBatch& other) const {
        if (highest_priority != other.highest_priority) {
            return highest_priority < other.highest_priority;  // Обратный порядок
        }
        return created_at > other.created_at;
    }
};

// Процессор батчей
class BatchProcessor {
private:
    std::queue<std::shared_ptr<BatchableCommand>> command_queue_;
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    std::atomic<bool> running_{true};
    
    // Конфигурация батчинга
    size_t max_batch_size_;
    std::chrono::milliseconds flush_interval_;
    std::chrono::system_clock::time_point last_flush_time_;
    
    // Статистика
    std::atomic<size_t> commands_processed_{0};
    std::atomic<size_t> batches_executed_{0};
    std::atomic<size_t> commands_queued_{0};
    std::atomic<size_t> forced_flushes_{0};
    
public:
    BatchProcessor(size_t max_batch = 100,
                  std::chrono::milliseconds interval = std::chrono::milliseconds(100))
        : max_batch_size_(max_batch), flush_interval_(interval),
          last_flush_time_(std::chrono::system_clock::now()) {
        std::cout << "Batch Processor создан (max batch: " << max_batch_size_ 
                  << ", flush interval: " << flush_interval_.count() << " ms)" << std::endl;
    }
    
    // Добавление команды
    void submit(std::shared_ptr<BatchableCommand> command) {
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            command_queue_.push(std::move(command));
            commands_queued_.fetch_add(1);
        }
        condition_.notify_one();
    }
    
    // Обработка батча
    void processBatch() {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        
        // Ждем команды или таймаута
        condition_.wait_for(lock, flush_interval_, [this] {
            return !command_queue_.empty() || !running_.load();
        });
        
        if (!running_.load() && command_queue_.empty()) {
            return;
        }
        
        auto now = std::chrono::system_clock::now();
        auto time_since_flush = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - last_flush_time_);
        
        // Проверяем условия для флаша
        bool should_flush = false;
        
        if (command_queue_.size() >= max_batch_size_) {
            should_flush = true;
            std::cout << "[FLUSH] Размер очереди достиг " << command_queue_.size() << std::endl;
        } else if (time_since_flush >= flush_interval_ && !command_queue_.empty()) {
            should_flush = true;
            forced_flushes_.fetch_add(1);
            std::cout << "[FLUSH] Истек таймаут (" << time_since_flush.count() << " ms)" << std::endl;
        }
        
        if (!should_flush) {
            return;
        }
        
        // Собираем команды в батчи по ключам
        std::unordered_map<std::string, CommandBatch> batches;
        
        while (!command_queue_.empty()) {
            auto command = command_queue_.front();
            command_queue_.pop();
            
            std::string key = command->getBatchKey();
            
            auto it = batches.find(key);
            if (it == batches.end()) {
                batches.emplace(key, CommandBatch(key));
                it = batches.find(key);
            }
            
            it->second.addCommand(command);
        }
        
        lock.unlock();
        
        // Сортируем батчи по приоритету
        std::vector<CommandBatch> sorted_batches;
        for (auto& pair : batches) {
            sorted_batches.push_back(std::move(pair.second));
        }
        
        std::sort(sorted_batches.begin(), sorted_batches.end(),
                 [](const CommandBatch& a, const CommandBatch& b) {
                     return b < a;  // Обратный порядок для приоритета
                 });
        
        // Выполняем батчи
        for (auto& batch : sorted_batches) {
            executeBatch(batch);
        }
        
        last_flush_time_ = std::chrono::system_clock::now();
    }
    
    // Принудительный флаш
    void forceFlush() {
        std::cout << "\n[FORCE FLUSH]" << std::endl;
        
        std::lock_guard<std::mutex> lock(queue_mutex_);
        
        std::unordered_map<std::string, CommandBatch> batches;
        
        while (!command_queue_.empty()) {
            auto command = command_queue_.front();
            command_queue_.pop();
            
            std::string key = command->getBatchKey();
            
            auto it = batches.find(key);
            if (it == batches.end()) {
                batches.emplace(key, CommandBatch(key));
                it = batches.find(key);
            }
            
            it->second.addCommand(command);
        }
        
        for (auto& pair : batches) {
            executeBatch(pair.second);
        }
        
        forced_flushes_.fetch_add(1);
        last_flush_time_ = std::chrono::system_clock::now();
    }
    
    // Остановка процессора
    void stop() {
        running_.store(false);
        condition_.notify_all();
    }
    
    // Статистика
    void printStats() const {
        std::cout << "\n=== Batch Processor Statistics ===" << std::endl;
        std::cout << "Команд в очереди: " << commands_queued_.load() << std::endl;
        std::cout << "Команд обработано: " << commands_processed_.load() << std::endl;
        std::cout << "Батчей выполнено: " << batches_executed_.load() << std::endl;
        std::cout << "Принудительных флашей: " << forced_flushes_.load() << std::endl;
        
        if (batches_executed_.load() > 0) {
            double avg_batch_size = static_cast<double>(commands_processed_.load()) / 
                                   batches_executed_.load();
            std::cout << "Средний размер батча: " << avg_batch_size << std::endl;
        }
        
        std::cout << "===================================" << std::endl;
    }
    
private:
    void executeBatch(CommandBatch& batch) {
        std::cout << "\n[BATCH " << batches_executed_.load() + 1 << "] "
                  << "Key: '" << batch.batch_key << "', "
                  << "Size: " << batch.size() << ", "
                  << "Priority: " << priorityToString(batch.highest_priority) << std::endl;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        // Выполняем команды в батче
        for (auto& command : batch.commands) {
            command->execute();
            commands_processed_.fetch_add(1);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "  [BATCH] Выполнен за " << duration.count() << " ms" << std::endl;
        
        batches_executed_.fetch_add(1);
    }
};

// Сервис с батчингом
class BatchProcessingService {
private:
    std::shared_ptr<BatchProcessor> processor_;
    std::thread processing_thread_;
    std::atomic<bool> running_{false};
    
public:
    BatchProcessingService()
        : processor_(std::make_shared<BatchProcessor>(50, std::chrono::milliseconds(500))) {}
    
    ~BatchProcessingService() {
        stop();
    }
    
    void start() {
        running_.store(true);
        
        processing_thread_ = std::thread([this]() {
            std::cout << "Batch Processing Thread запущен" << std::endl;
            
            while (running_.load()) {
                processor_->processBatch();
            }
            
            // Финальный флаш
            processor_->forceFlush();
            
            std::cout << "Batch Processing Thread завершен" << std::endl;
        });
        
        std::cout << "Batch Processing Service запущен" << std::endl;
    }
    
    void stop() {
        if (!running_.load()) return;
        
        std::cout << "Останавливаем Batch Processing Service..." << std::endl;
        
        processor_->stop();
        running_.store(false);
        
        if (processing_thread_.joinable()) {
            processing_thread_.join();
        }
        
        std::cout << "Batch Processing Service остановлен" << std::endl;
        processor_->printStats();
    }
    
    void submitCommand(std::shared_ptr<BatchableCommand> command) {
        processor_->submit(std::move(command));
    }
};

// Демонстрация базового батчинга
void demonstrateBasicBatching() {
    std::cout << "\n=== Демонстрация базового батчинга ===" << std::endl;
    
    BatchProcessingService service;
    service.start();
    
    std::cout << "\n--- Отправка команд ---" << std::endl;
    
    // Записи в разные таблицы БД
    for (int i = 0; i < 15; ++i) {
        std::string table = (i % 3 == 0) ? "users" : (i % 3 == 1) ? "orders" : "products";
        service.submitCommand(
            std::make_shared<DatabaseWriteCommand>(table, i, "data_" + std::to_string(i))
        );
    }
    
    // Отправка email
    for (int i = 0; i < 8; ++i) {
        service.submitCommand(
            std::make_shared<SendEmailCommand>(
                "user" + std::to_string(i) + "@example.com",
                "Welcome!"
            )
        );
    }
    
    // Обработка изображений
    for (int i = 0; i < 10; ++i) {
        std::string op = (i % 2 == 0) ? "resize" : "compress";
        service.submitCommand(
            std::make_shared<ProcessImageCommand>(
                "image_" + std::to_string(i) + ".jpg",
                op
            )
        );
    }
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    service.stop();
}

// Демонстрация приоритетов
void demonstratePriorities() {
    std::cout << "\n=== Демонстрация приоритетов ===" << std::endl;
    
    BatchProcessingService service;
    service.start();
    
    std::cout << "\n--- Отправка команд с разными приоритетами ---" << std::endl;
    
    // Низкий приоритет
    for (int i = 0; i < 10; ++i) {
        service.submitCommand(
            std::make_shared<DatabaseWriteCommand>(
                "logs", i, "log_entry", CommandPriority::LOW
            )
        );
    }
    
    // Нормальный приоритет
    for (int i = 0; i < 10; ++i) {
        service.submitCommand(
            std::make_shared<DatabaseWriteCommand>(
                "users", i, "user_data", CommandPriority::NORMAL
            )
        );
    }
    
    // Высокий приоритет
    for (int i = 0; i < 5; ++i) {
        service.submitCommand(
            std::make_shared<SendEmailCommand>(
                "admin@example.com",
                "URGENT: Security Alert",
                CommandPriority::HIGH
            )
        );
    }
    
    // Критический приоритет
    service.submitCommand(
        std::make_shared<DatabaseWriteCommand>(
            "system", 999, "critical_event", CommandPriority::CRITICAL
        )
    );
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    service.stop();
}

// Демонстрация нагрузки
void demonstrateHighLoad() {
    std::cout << "\n=== Демонстрация высокой нагрузки ===" << std::endl;
    
    BatchProcessingService service;
    service.start();
    
    std::cout << "\n--- Отправка большого количества команд ---" << std::endl;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Генерируем много команд
    for (int i = 0; i < 500; ++i) {
        std::string table = (i % 5 == 0) ? "users" : 
                           (i % 5 == 1) ? "orders" : 
                           (i % 5 == 2) ? "products" : 
                           (i % 5 == 3) ? "inventory" : "logs";
        
        CommandPriority priority = (i % 10 == 0) ? CommandPriority::HIGH : CommandPriority::NORMAL;
        
        service.submitCommand(
            std::make_shared<DatabaseWriteCommand>(
                table, i, "data_" + std::to_string(i), priority
            )
        );
        
        // Имитация потока команд
        if (i % 50 == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
    
    std::this_thread::sleep_for(std::chrono::seconds(3));
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "\nВремя обработки: " << duration.count() << " ms" << std::endl;
    
    service.stop();
}

int main() {
    std::cout << "=== Batch Processing Pattern ===" << std::endl;
    
    try {
        demonstrateBasicBatching();
        demonstratePriorities();
        demonstrateHighLoad();
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\n=== Демонстрация завершена ===" << std::endl;
    return 0;
}
