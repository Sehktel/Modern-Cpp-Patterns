# Урок 9.3: Command Queue Pattern (Очередь команд)

## 🎯 Цель урока
Изучить паттерн Command Queue - оптимизацию производительности через асинхронное выполнение и батчинг команд. Понять, как сгладить пиковые нагрузки и оптимизировать групповые операции.

## 📚 Что изучаем

### 1. Паттерн Command Queue
- **Определение**: Очередь команд для асинхронного выполнения с возможностью батчинга
- **Назначение**: Разделение производителей и потребителей, оптимизация групповых операций
- **Применение**: Игровые движки, batch processing, logging системы, DB операции

### 2. Ключевые компоненты
- **Command**: Инкапсулированная операция
- **Command Queue**: Очередь для хранения команд
- **Batch Processor**: Группировка и выполнение батчей
- **Flush Strategy**: Политика выполнения (size/time/priority)

### 3. Преимущества батчинга
- **Амортизация overhead**: Один overhead на батч
- **Оптимизация I/O**: Групповые операции эффективнее
- **Throughput**: Увеличение пропускной способности
- **Resource utilization**: Лучшая утилизация GPU/DB/Network

## 🔍 Ключевые концепции

### Базовая команда

```cpp
#include <iostream>
#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <memory>
#include <unordered_map>

class Command {
public:
    virtual ~Command() = default;
    virtual void execute() = 0;
    virtual std::string getName() const = 0;
    virtual std::string getBatchKey() const = 0;
};

class DatabaseWriteCommand : public Command {
private:
    std::string table_;
    int record_id_;
    std::string data_;
    
public:
    DatabaseWriteCommand(const std::string& table, int id, const std::string& data)
        : table_(table), record_id_(id), data_(data) {}
    
    void execute() override {
        std::cout << "DB Write: " << table_ << "[" << record_id_ << "]" << std::endl;
    }
    
    std::string getName() const override {
        return "DatabaseWrite";
    }
    
    std::string getBatchKey() const override {
        return "db_" + table_;  // Группировка по таблице
    }
};
```

### Batch Processor

```cpp
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
    
    void submit(std::shared_ptr<Command> cmd) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push(std::move(cmd));
        }
        cv_.notify_one();
    }
    
    void processBatch() {
        std::unique_lock<std::mutex> lock(mutex_);
        
        // Ждем с timeout
        cv_.wait_for(lock, flush_interval_, [this] {
            return !queue_.empty() || !running_.load();
        });
        
        auto now = std::chrono::system_clock::now();
        auto elapsed = now - last_flush_;
        
        // Проверяем условия flush
        bool should_flush = 
            queue_.size() >= max_batch_size_ ||
            (elapsed >= flush_interval_ && !queue_.empty());
        
        if (!should_flush) return;
        
        // Группируем по batch key
        std::unordered_map<std::string, 
                          std::vector<std::shared_ptr<Command>>> batches;
        
        while (!queue_.empty()) {
            auto cmd = queue_.front();
            queue_.pop();
            
            std::string key = cmd->getBatchKey();
            batches[key].push_back(cmd);
        }
        
        lock.unlock();
        
        // Выполняем батчи
        for (auto& [key, commands] : batches) {
            std::cout << "[BATCH] " << key << ": " 
                      << commands.size() << " commands" << std::endl;
            
            for (auto& cmd : commands) {
                cmd->execute();
                commands_processed_.fetch_add(1);
            }
            
            batches_executed_.fetch_add(1);
        }
        
        last_flush_ = now;
    }
};
```

### Flush Strategies

```cpp
// 1. Size-based flush
if (batch.size() >= MAX_BATCH_SIZE) {
    flush();
}

// 2. Time-based flush
if (time_since_last_flush > FLUSH_INTERVAL) {
    flush();
}

// 3. Hybrid flush (BEST PRACTICE)
if (batch.size() >= MAX_SIZE || time_since_last_flush > INTERVAL) {
    flush();
}

// 4. Priority flush
if (has_high_priority_command()) {
    flush();  // Немедленно
}
```

### Game Engine пример

```cpp
class GameEngine {
private:
    CommandBuffer command_buffer_;
    
public:
    void gameLoop() {
        while (running_) {
            auto frame_start = std::chrono::high_resolution_clock::now();
            
            // 1. Input
            processInput();
            
            // 2. Update
            updateGameLogic(dt);
            
            // 3. Submit render commands (instant!)
            for (auto& sprite : sprites_) {
                command_buffer_.submit(
                    std::make_shared<DrawSpriteCommand>(sprite)
                );
            }
            
            // 4. Render (батчинг!)
            command_buffer_.flip();  // Swap buffers + batch execute
            
            // 5. Cap at 60 FPS
            auto frame_time = std::chrono::high_resolution_clock::now() - frame_start;
            if (frame_time < std::chrono::milliseconds(16)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(16) - frame_time);
            }
        }
    }
};
```

## 🎓 Best Practices

### ✅ DO (Рекомендуется)

1. **Используйте hybrid flush strategy**
   - Size-based И time-based
   - Баланс throughput и latency

2. **Группируйте команды по типу**
   - DB writes по таблицам
   - Render commands по текстурам
   - Network requests по endpoints

3. **Bounded queue**
   - Защита от OOM
   - Backpressure на producers

4. **Мониторьте метрики**
   - Batch size, latency, throughput
   - Tune параметры по метрикам

5. **Делайте команды идемпотентными**
   - Безопасно повторять
   - Упрощает error handling

### ❌ DON'T (Не рекомендуется)

1. **НЕ делайте слишком большой flush interval**
   - > 1 second → высокая latency

2. **НЕ используйте unbounded queue**
   - Может привести к OOM

3. **НЕ батчуйте несовместимые команды**
   - Разные типы операций отдельно

4. **НЕ игнорируйте ordering**
   - Критичные зависимости

5. **НЕ забывайте про error handling**
   - Один failed command ≠ весь батч

## 📊 Performance Gains

### Database Batching
```
БЕЗ батчинга:
  1000 × INSERT (overhead 5ms + work 1ms) = 6,000ms

С батчингом (batch size 100):
  10 × batch INSERT (overhead 5ms + work 100×1ms) = 1,050ms
  
Ускорение: 6x
```

### Render Commands
```
БЕЗ батчинга:
  1000 draw calls (overhead 0.5ms each) = 500ms
  
С батчингом по текстурам:
  10 batches (overhead 0.5ms + 100 draws) = 5ms + 100ms = 105ms
  
Ускорение: 4.7x, можно достичь 60 FPS!
```

## 📁 Файлы урока

- `command_queue_pattern.cpp` - Command Queue для игрового движка
- `batch_processing.cpp` - Батчинг с приоритетами, flush стратегии
- `command_vulnerabilities.cpp` - Уязвимости
- `secure_command_alternatives.cpp` - Безопасные альтернативы
- `SECURITY_ANALYSIS.md` - Анализ безопасности

## 🔗 Связанные паттерны

- **Command**: Инкапсуляция операций
- **Producer-Consumer**: Производство и потребление команд
- **Thread Pool**: Workers для выполнения
- **Event Sourcing**: Commands как events

## 🚀 Практическое применение

### Используется в
- **Unity/Unreal Engine**: Render command buffers
- **Kafka**: Message batching
- **Databases**: Batch inserts, bulk operations
- **Logging**: Async log batching

### Когда использовать
✅ High-throughput системы  
✅ Операции с high overhead (network, I/O)  
✅ Групповые операции эффективнее  
✅ Сглаживание пиковых нагрузок

### Когда НЕ использовать
❌ Real-time strict latency  
❌ Немедленный ответ требуется  
❌ Low-overhead операции  
❌ Строгие ordering constraints

---

**Автор**: Senior C++ Developer  
**Дата**: 2025-10-07  
**Версия**: 1.0
