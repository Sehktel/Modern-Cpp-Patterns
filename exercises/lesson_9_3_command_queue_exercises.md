# 🏋️ Упражнения: Command Queue Pattern

## 📋 Задание 1: Базовый Command Queue с батчингом

### Описание
Реализуйте Command Queue с автоматическим батчингом похожих команд для оптимизации производительности.

### Требования
1. ✅ Очередь команд с thread-safe доступом
2. ✅ Группировка команд по batch key
3. ✅ Size-based и time-based flush
4. ✅ Batch execution
5. ✅ Метрики: размер батчей, throughput

### Шаблон для реализации
```cpp
#include <iostream>
#include <queue>
#include <vector>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <chrono>
#include <memory>

class Command {
public:
    virtual ~Command() = default;
    virtual void execute() = 0;
    virtual std::string getName() const = 0;
    virtual std::string getBatchKey() const = 0;
};

class BatchProcessor {
private:
    std::queue<std::shared_ptr<Command>> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
    
    size_t max_batch_size_;
    std::chrono::milliseconds flush_interval_;
    std::chrono::system_clock::time_point last_flush_;
    
    std::atomic<bool> running_{true};
    std::atomic<size_t> commands_processed_{0};
    std::atomic<size_t> batches_executed_{0};
    
public:
    BatchProcessor(size_t max_batch = 100,
                  std::chrono::milliseconds interval = std::chrono::milliseconds(100))
        : max_batch_size_(max_batch), flush_interval_(interval),
          last_flush_(std::chrono::system_clock::now()) {}
    
    // TODO: Реализуйте submit
    void submit(std::shared_ptr<Command> cmd) {
        // Ваш код здесь
    }
    
    // TODO: Реализуйте processBatch
    void processBatch() {
        std::unique_lock<std::mutex> lock(mutex_);
        
        // TODO: Ждите с timeout
        
        auto now = std::chrono::system_clock::now();
        auto elapsed = now - last_flush_;
        
        bool should_flush = 
            queue_.size() >= max_batch_size_ ||
            (elapsed >= flush_interval_ && !queue_.empty());
        
        if (!should_flush) return;
        
        // TODO: Группировка по batch key
        std::unordered_map<std::string, std::vector<std::shared_ptr<Command>>> batches;
        
        while (!queue_.empty()) {
            auto cmd = queue_.front();
            queue_.pop();
            
            std::string key = cmd->getBatchKey();
            batches[key].push_back(cmd);
        }
        
        lock.unlock();
        
        // TODO: Выполнение батчей
        for (auto& [key, commands] : batches) {
            executeBatch(key, commands);
        }
        
        last_flush_ = now;
    }
    
private:
    void executeBatch(const std::string& key, 
                     const std::vector<std::shared_ptr<Command>>& commands) {
        std::cout << "[BATCH] " << key << ": " << commands.size() 
                  << " commands" << std::endl;
        
        for (auto& cmd : commands) {
            cmd->execute();
            commands_processed_.fetch_add(1);
        }
        
        batches_executed_.fetch_add(1);
    }
};
```

### Тесты
```cpp
class DatabaseWriteCommand : public Command {
private:
    std::string table_;
    int record_id_;
    std::string data_;
    
public:
    DatabaseWriteCommand(const std::string& table, int id, const std::string& data)
        : table_(table), record_id_(id), data_(data) {}
    
    void execute() override {
        std::cout << "  DB Write: " << table_ << "[" << record_id_ << "]" << std::endl;
    }
    
    std::string getName() const override { return "DatabaseWrite"; }
    std::string getBatchKey() const override { return "db_" + table_; }
};

void testBatching() {
    BatchProcessor processor(10, std::chrono::milliseconds(100));
    
    // Запускаем processor в отдельном потоке
    std::thread worker([&processor]() {
        while (true) {
            processor.processBatch();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });
    
    // Отправляем команды
    for (int i = 0; i < 50; ++i) {
        std::string table = (i % 3 == 0) ? "users" : "orders";
        processor.submit(
            std::make_shared<DatabaseWriteCommand>(table, i, "data")
        );
    }
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // TODO: Проверка что команды сбатчились
    std::cout << "Batching test PASSED ✅" << std::endl;
}
```

---

## 📋 Задание 2: Priority Command Queue

### Описание
Реализуйте Command Queue с поддержкой приоритетов и fair scheduling.

### Требования
1. ✅ Приоритетная очередь команд
2. ✅ Fair scheduling (не starve низкоприоритетные)
3. ✅ Age-based priority boost
4. ✅ Preemption для критичных команд
5. ✅ Метрики: latency по приоритетам

### Шаблон
```cpp
enum class CommandPriority {
    LOW = 0,
    NORMAL = 1,
    HIGH = 2,
    CRITICAL = 3
};

struct PrioritizedCommand {
    std::shared_ptr<Command> command;
    CommandPriority priority;
    std::chrono::system_clock::time_point created_at;
    
    // TODO: Компаратор учитывающий приоритет и age
    bool operator<(const PrioritizedCommand& other) const {
        // TODO: 1. Сравните приоритеты
        // TODO: 2. Для одного приоритета - FIFO (по created_at)
        // TODO: 3. Age boost: старые LOW → NORMAL после N секунд
        return false;
    }
};

class PriorityCommandQueue {
private:
    std::priority_queue<PrioritizedCommand> queue_;
    
public:
    // TODO: Реализуйте priority queue
    
    void submit(std::shared_ptr<Command> cmd, CommandPriority priority) {
        // TODO: Добавить в priority queue
    }
    
    std::shared_ptr<Command> dequeue() {
        // TODO: Получить команду с highest priority
    }
};
```

---

## 📋 Задание 3: Game Engine Command Buffer

### Описание
Реализуйте Command Buffer для игрового движка с double-buffering и frame-based execution.

### Требования
1. ✅ Double buffering (front/back buffers)
2. ✅ Frame-based батчинг
3. ✅ Render commands группировка
4. ✅ GPU state optimization
5. ✅ 60 FPS performance

### Шаблон
```cpp
class RenderCommand : public Command {
protected:
    int sprite_id_;
    int x_, y_;
    
public:
    RenderCommand(int id, int x, int y) 
        : sprite_id_(id), x_(x), y_(y) {}
};

class DrawSpriteCommand : public RenderCommand {
private:
    std::string texture_name_;
    
public:
    DrawSpriteCommand(int id, int x, int y, const std::string& texture)
        : RenderCommand(id, x, y), texture_name_(texture) {}
    
    void execute() override {
        // TODO: Draw sprite
    }
    
    std::string getBatchKey() const override {
        return "sprite_" + texture_name_;
    }
};

class CommandBuffer {
private:
    std::vector<std::shared_ptr<Command>> front_buffer_;
    std::vector<std::shared_ptr<Command>> back_buffer_;
    std::mutex mutex_;
    
public:
    // TODO: Добавление команды в back buffer
    void submit(std::shared_ptr<Command> cmd) {
        std::lock_guard<std::mutex> lock(mutex_);
        back_buffer_.push_back(std::move(cmd));
    }
    
    // TODO: Swap buffers и выполнение front buffer
    void flip() {
        // TODO: 1. Swap front/back
        // TODO: 2. Batch commands by type
        // TODO: 3. Execute batches
        // TODO: 4. Clear front buffer
    }
};

class GameEngine {
private:
    CommandBuffer command_buffer_;
    std::atomic<bool> running_{true};
    
public:
    void gameLoop() {
        while (running_) {
            auto frame_start = std::chrono::high_resolution_clock::now();
            
            // TODO: Process input, update logic
            
            // Submit render commands
            for (auto& sprite : sprites_) {
                command_buffer_.submit(
                    std::make_shared<DrawSpriteCommand>(/* ... */)
                );
            }
            
            // Execute render commands
            command_buffer_.flip();
            
            // TODO: Cap at 60 FPS
            auto frame_time = std::chrono::high_resolution_clock::now() - frame_start;
            if (frame_time < std::chrono::milliseconds(16)) {
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(16) - frame_time
                );
            }
        }
    }
};
```

---

## 📋 Задание 4: Async Command Queue с Futures

### Описание
Реализуйте асинхронный Command Queue с возвратом результатов через `std::future`.

### Требования
1. ✅ Команды возвращают результаты
2. ✅ std::future для асинхронного получения
3. ✅ Exception propagation через future
4. ✅ Cancellation support
5. ✅ Timeout для команд

### Шаблон
```cpp
template<typename R>
class AsyncCommand {
public:
    virtual ~AsyncCommand() = default;
    virtual R execute() = 0;
    virtual std::string getBatchKey() const = 0;
};

template<typename R>
class AsyncCommandQueue {
private:
    std::queue<std::packaged_task<R()>> tasks_;
    
public:
    std::future<R> submit(std::function<R()> func) {
        std::packaged_task<R()> task(std::move(func));
        std::future<R> future = task.get_future();
        
        // TODO: Добавить в очередь
        
        return future;
    }
};
```

---

## 📋 Задание 5: Benchmarking Batching Efficiency

### Описание
Создайте benchmark демонстрирующий преимущества батчинга для различных типов операций.

### Требования
1. ✅ Бенчмарк DB inserts (с/без батчинга)
2. ✅ Бенчмарк render commands (с/без батчинга)
3. ✅ Бенчмарк network requests (с/без батчинга)
4. ✅ Графики результатов
5. ✅ Анализ trade-offs (throughput vs latency)

### Пример
```cpp
void benchmarkDatabaseBatching() {
    const size_t NUM_INSERTS = 10000;
    
    // БЕЗ батчинга
    auto start = std::chrono::high_resolution_clock::now();
    
    for (size_t i = 0; i < NUM_INSERTS; ++i) {
        // Каждый INSERT отдельно (overhead 5ms)
        db.execute("INSERT INTO users VALUES (...)");
    }
    
    auto without = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now() - start);
    
    // С батчингом
    BatchProcessor processor(100, std::chrono::milliseconds(50));
    
    start = std::chrono::high_resolution_clock::now();
    
    for (size_t i = 0; i < NUM_INSERTS; ++i) {
        processor.submit(std::make_shared<DBInsertCommand>(/* ... */));
    }
    
    // Ждем обработки
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    auto with = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now() - start);
    
    std::cout << "Without batching: " << without.count() << " ms" << std::endl;
    std::cout << "With batching: " << with.count() << " ms" << std::endl;
    std::cout << "Speedup: " << (without.count() / (double)with.count()) << "x" << std::endl;
}
```

---

**Автор**: Senior C++ Developer  
**Дата**: 2025-10-07  
**Сложность**: ⭐⭐⭐⭐ (Продвинутый уровень)
