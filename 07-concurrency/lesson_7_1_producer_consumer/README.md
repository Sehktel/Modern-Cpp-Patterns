# Урок 7.1: Producer-Consumer Pattern (Производитель-Потребитель)

## 🎯 Цель урока
Изучить паттерн Producer-Consumer - один из фундаментальных паттернов многопоточного программирования. Понять, как организовать асинхронную обработку данных между потоками с использованием очередей и синхронизации.

## 📚 Что изучаем

### 1. Паттерн Producer-Consumer
- **Определение**: Разделение производства и потребления данных между разными потоками
- **Назначение**: Асинхронная обработка данных, буферизация, балансировка нагрузки
- **Применение**: Очереди сообщений, обработка файлов, стриминг данных, web-серверы

### 2. Ключевые компоненты
- **Producer**: Поток, производящий данные
- **Consumer**: Поток, потребляющий данные
- **Buffer/Queue**: Общий буфер для передачи данных
- **Synchronization**: Механизмы синхронизации (мьютексы, условные переменные)

### 3. Проблемы и решения
- **Race Conditions**: Состояния гонки при доступе к общим данным
- **Deadlock**: Взаимные блокировки потоков
- **Buffer Overflow**: Переполнение буфера
- **Buffer Underflow**: Недостаток данных в буфере

## 🔍 Ключевые концепции

### Базовая реализация
```cpp
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>

template<typename T>
class ProducerConsumerQueue {
private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable condition_;
    bool finished_ = false;
    
public:
    void push(T item) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(item);
        condition_.notify_one();
    }
    
    bool pop(T& item) {
        std::unique_lock<std::mutex> lock(mutex_);
        condition_.wait(lock, [this] { return !queue_.empty() || finished_; });
        
        if (queue_.empty()) return false;
        
        item = queue_.front();
        queue_.pop();
        return true;
    }
    
    void finish() {
        std::lock_guard<std::mutex> lock(mutex_);
        finished_ = true;
        condition_.notify_all();
    }
};
```

### Producer (Производитель)
```cpp
class DataProducer {
private:
    ProducerConsumerQueue<int>& queue_;
    int maxItems_;
    
public:
    DataProducer(ProducerConsumerQueue<int>& queue, int maxItems) 
        : queue_(queue), maxItems_(maxItems) {}
    
    void produce() {
        for (int i = 0; i < maxItems_; ++i) {
            // Имитация производства данных
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            queue_.push(i);
            std::cout << "Produced: " << i << std::endl;
        }
        
        queue_.finish();
        std::cout << "Producer finished" << std::endl;
    }
};
```

### Consumer (Потребитель)
```cpp
class DataConsumer {
private:
    ProducerConsumerQueue<int>& queue_;
    
public:
    DataConsumer(ProducerConsumerQueue<int>& queue) : queue_(queue) {}
    
    void consume() {
        int item;
        while (queue_.pop(item)) {
            // Имитация обработки данных
            std::this_thread::sleep_for(std::chrono::milliseconds(150));
            
            std::cout << "Consumed: " << item << std::endl;
        }
        
        std::cout << "Consumer finished" << std::endl;
    }
};
```

## 🤔 Вопросы для размышления

### 1. Когда использовать Producer-Consumer?
**✅ Хорошие случаи:**
- Асинхронная обработка данных
- Балансировка нагрузки между потоками
- Буферизация данных
- Разделение I/O и вычислений
- Обработка потоков данных

**❌ Плохие случаи:**
- Синхронная обработка данных
- Простые вычисления
- Когда порядок обработки критичен
- Ограниченные ресурсы

### 2. Преимущества паттерна
- **Разделение ответственности**: Producer и Consumer независимы
- **Буферизация**: Сглаживание различий в скорости производства/потребления
- **Масштабируемость**: Можно иметь несколько Producer/Consumer
- **Гибкость**: Легко добавлять новые обработчики

### 3. Недостатки паттерна
- **Сложность**: Требует синхронизации
- **Память**: Нужен буфер для хранения данных
- **Отладка**: Сложнее отлаживать многопоточный код
- **Производительность**: Накладные расходы на синхронизацию

## 🛠️ Практические примеры

### Обработка файлов
```cpp
class FileProcessor {
private:
    ProducerConsumerQueue<std::string>& queue_;
    
public:
    void processFiles() {
        std::string filename;
        while (queue_.pop(filename)) {
            std::ifstream file(filename);
            if (file.is_open()) {
                std::string line;
                while (std::getline(file, line)) {
                    // Обработка строки файла
                    processLine(line);
                }
            }
        }
    }
    
private:
    void processLine(const std::string& line) {
        // Логика обработки строки
    }
};
```

### Очередь сообщений
```cpp
struct Message {
    std::string sender;
    std::string content;
    std::chrono::system_clock::time_point timestamp;
};

class MessageQueue {
private:
    ProducerConsumerQueue<Message> queue_;
    
public:
    void sendMessage(const Message& msg) {
        queue_.push(msg);
    }
    
    void processMessages() {
        Message msg;
        while (queue_.pop(msg)) {
            handleMessage(msg);
        }
    }
    
private:
    void handleMessage(const Message& msg) {
        std::cout << "From: " << msg.sender 
                  << ", Content: " << msg.content << std::endl;
    }
};
```

## 🎨 Современные подходы в C++

### Lock-free очередь
```cpp
#include <atomic>

template<typename T>
class LockFreeQueue {
private:
    struct Node {
        std::atomic<T*> data;
        std::atomic<Node*> next;
    };
    
    std::atomic<Node*> head_;
    std::atomic<Node*> tail_;
    
public:
    LockFreeQueue() {
        Node* dummy = new Node;
        dummy->data = nullptr;
        head_ = tail_ = dummy;
    }
    
    void push(T item) {
        Node* new_node = new Node;
        new_node->data = new T(item);
        new_node->next = nullptr;
        
        Node* prev_tail = tail_.exchange(new_node);
        prev_tail->next = new_node;
    }
    
    bool pop(T& item) {
        Node* head = head_.load();
        Node* next = head->next.load();
        
        if (next == nullptr) return false;
        
        item = *next->data;
        head_ = next;
        delete head->data.load();
        delete head;
        return true;
    }
};
```

### Async/await подход
```cpp
#include <future>
#include <async>

class AsyncProducerConsumer {
private:
    std::queue<std::promise<int>> promises_;
    std::mutex mutex_;
    
public:
    std::future<int> asyncConsume() {
        std::promise<int> promise;
        auto future = promise.get_future();
        
        std::lock_guard<std::mutex> lock(mutex_);
        promises_.push(std::move(promise));
        
        return future;
    }
    
    void asyncProduce(int value) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!promises_.empty()) {
            auto promise = std::move(promises_.front());
            promises_.pop();
            promise.set_value(value);
        }
    }
};
```

## 🧪 Тестирование Producer-Consumer

### Тест производительности
```cpp
#include <chrono>

void testPerformance() {
    const int NUM_ITEMS = 10000;
    ProducerConsumerQueue<int> queue;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Запускаем producer и consumer
    std::thread producer([&]() {
        for (int i = 0; i < NUM_ITEMS; ++i) {
            queue.push(i);
        }
        queue.finish();
    });
    
    std::thread consumer([&]() {
        int item;
        while (queue.pop(item)) {
            // Обработка элемента
        }
    });
    
    producer.join();
    consumer.join();
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Processed " << NUM_ITEMS << " items in " 
              << duration.count() << " ms" << std::endl;
}
```

## 🎯 Практические упражнения

### Упражнение 1: Многопоточная обработка файлов
Создайте систему для параллельной обработки файлов с использованием Producer-Consumer.

### Упражнение 2: Очередь задач
Реализуйте систему очереди задач для web-сервера с приоритетами.

### Упражнение 3: Стриминг данных
Создайте систему для обработки потоков данных в реальном времени.

### Упражнение 4: Lock-free реализация
Реализуйте lock-free версию Producer-Consumer очереди.

## 📈 Связь с другими паттернами

### Producer-Consumer vs Observer
- **Producer-Consumer**: Асинхронная передача данных
- **Observer**: Синхронные уведомления

### Producer-Consumer vs Command Queue
- **Producer-Consumer**: Обработка данных
- **Command Queue**: Выполнение команд

### Producer-Consumer vs Pipeline
- **Producer-Consumer**: Двухэтапная обработка
- **Pipeline**: Многоэтапная обработка

## 🆕 Сравнение C++17/20 vs C++23

В этом уроке доступны **два файла сравнения** для разных целей:

### 📄 Файлы в директории:
1. **`producer_consumer_pattern.cpp`** - базовая реализация паттерна
2. **`modern_producer_consumer.cpp`** - продвинутая реализация с C++17/20
3. **`producer_consumer_cpp23_quick_comparison.cpp`** ⚡ - краткое сравнение (110 строк, 5 минут)
4. **`producer_consumer_cpp23_full_comparison.cpp`** 📚 - полное сравнение (820 строк, 30-60 минут)
5. **`CPP23_COMPARISON_GUIDE.md`** - подробное руководство по файлам сравнения

### Быстрый старт:

#### Для быстрого ознакомления (5 минут):
```bash
# Читайте producer_consumer_cpp23_quick_comparison.cpp
# Основные отличия C++23:
# - std::jthread (auto-join + stop_token)
# - std::expected (элегантная обработка ошибок)
```

#### Для глубокого изучения (30-60 минут):
```bash
# Читайте producer_consumer_cpp23_full_comparison.cpp
# Полная реализация с:
# - 4 стратегии Producer (LINEAR, RANDOM, FIBONACCI, PRIME)
# - 4 стратегии Consumer (SIMPLE, STATISTICS, FILTER, TRANSFORM)
# - Детальная статистика и мониторинг
# - Side-by-side сравнение C++17/20 vs C++23
```

### 📖 Подробная информация:
Смотрите **`CPP23_COMPARISON_GUIDE.md`** для:
- Детального сравнения файлов
- Рекомендуемого пути обучения
- Примеров компиляции
- FAQ и советов

## 📈 Следующие шаги
После изучения Producer-Consumer вы будете готовы к:
- Уроку 7.2: Thread Pool Pattern
- Пониманию многопоточного программирования
- Созданию масштабируемых систем
- Оптимизации производительности

## 💡 Важные принципы

1. **Используйте правильную синхронизацию**: Мьютексы и условные переменные
2. **Обрабатывайте завершение**: Корректно завершайте потоки
3. **Мониторьте производительность**: Измеряйте пропускную способность
4. **Тестируйте многопоточность**: Проверяйте race conditions
5. **Рассмотрите lock-free варианты**: Для критичных по производительности участков
