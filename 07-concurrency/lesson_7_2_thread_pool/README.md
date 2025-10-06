# Урок 7.2: Thread Pool Pattern (Пул потоков)

## 🎯 Цель урока
Изучить паттерн Thread Pool - один из самых эффективных способов управления многопоточностью. Понять, как создать пул рабочих потоков для выполнения задач без постоянного создания и уничтожения потоков.

## 📚 Что изучаем

### 1. Паттерн Thread Pool
- **Определение**: Предварительно созданный пул потоков для выполнения задач
- **Назначение**: Избежание накладных расходов на создание/уничтожение потоков
- **Применение**: Web-серверы, обработка запросов, параллельные вычисления

### 2. Ключевые компоненты
- **Worker Threads**: Рабочие потоки в пуле
- **Task Queue**: Очередь задач для выполнения
- **Task Interface**: Интерфейс для задач
- **Thread Manager**: Управление потоками

### 3. Преимущества
- **Производительность**: Нет накладных расходов на создание потоков
- **Контроль ресурсов**: Ограниченное количество потоков
- **Масштабируемость**: Легко настраивать размер пула
- **Простота использования**: Простой API для добавления задач

## 🔍 Ключевые концепции

### Базовая реализация Thread Pool
```cpp
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <functional>
#include <future>

class ThreadPool {
private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex queueMutex_;
    std::condition_variable condition_;
    bool stop_;
    
public:
    explicit ThreadPool(size_t numThreads) : stop_(false) {
        // Создаем рабочие потоки
        for (size_t i = 0; i < numThreads; ++i) {
            workers_.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    
                    {
                        std::unique_lock<std::mutex> lock(queueMutex_);
                        condition_.wait(lock, [this] { return stop_ || !tasks_.empty(); });
                        
                        if (stop_ && tasks_.empty()) {
                            return;
                        }
                        
                        task = std::move(tasks_.front());
                        tasks_.pop();
                    }
                    
                    task();
                }
            });
        }
    }
    
    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            stop_ = true;
        }
        
        condition_.notify_all();
        
        for (std::thread& worker : workers_) {
            worker.join();
        }
    }
    
    template<typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args) 
        -> std::future<typename std::result_of<F(Args...)>::type> {
        
        using return_type = typename std::result_of<F(Args...)>::type;
        
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        
        std::future<return_type> result = task->get_future();
        
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            
            if (stop_) {
                throw std::runtime_error("enqueue on stopped ThreadPool");
            }
            
            tasks_.emplace([task]() { (*task)(); });
        }
        
        condition_.notify_one();
        return result;
    }
};
```

## 🛠️ Практические примеры

### Web-сервер с Thread Pool
```cpp
class WebServer {
private:
    ThreadPool threadPool_;
    std::atomic<int> requestCount_{0};
    
public:
    WebServer(size_t numThreads = 4) : threadPool_(numThreads) {}
    
    void handleRequest(int requestId) {
        // Имитация обработки HTTP запроса
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        std::cout << "Обработан запрос " << requestId 
                  << " потоком " << std::this_thread::get_id() << std::endl;
        
        requestCount_.fetch_add(1);
    }
    
    void startServer() {
        std::cout << "Web-сервер запущен с " << threadPool_.size() << " потоками" << std::endl;
        
        // Имитация входящих запросов
        for (int i = 1; i <= 20; ++i) {
            threadPool_.enqueue([this, i]() {
                handleRequest(i);
            });
            
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        
        // Ждем завершения всех задач
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        std::cout << "Всего обработано запросов: " << requestCount_.load() << std::endl;
    }
};
```

### Параллельные вычисления
```cpp
class ParallelCalculator {
private:
    ThreadPool threadPool_;
    
public:
    ParallelCalculator(size_t numThreads = std::thread::hardware_concurrency()) 
        : threadPool_(numThreads) {}
    
    std::vector<double> calculateSquares(const std::vector<int>& numbers) {
        std::vector<std::future<double>> futures;
        std::vector<double> results(numbers.size());
        
        for (size_t i = 0; i < numbers.size(); ++i) {
            futures.push_back(threadPool_.enqueue([&numbers, i]() -> double {
                // Имитация сложных вычислений
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                return numbers[i] * numbers[i];
            }));
        }
        
        // Собираем результаты
        for (size_t i = 0; i < futures.size(); ++i) {
            results[i] = futures[i].get();
        }
        
        return results;
    }
    
    double calculateSum(const std::vector<int>& numbers) {
        const size_t chunkSize = numbers.size() / threadPool_.size();
        std::vector<std::future<double>> futures;
        
        for (size_t i = 0; i < numbers.size(); i += chunkSize) {
            size_t end = std::min(i + chunkSize, numbers.size());
            
            futures.push_back(threadPool_.enqueue([&numbers, i, end]() -> double {
                double sum = 0.0;
                for (size_t j = i; j < end; ++j) {
                    sum += numbers[j];
                }
                return sum;
            }));
        }
        
        // Суммируем результаты
        double totalSum = 0.0;
        for (auto& future : futures) {
            totalSum += future.get();
        }
        
        return totalSum;
    }
};
```

## 🎯 Практические упражнения

### Упражнение 1: Асинхронная обработка файлов
Создайте систему для параллельной обработки файлов с использованием Thread Pool.

### Упражнение 2: Web-сервер с балансировкой
Реализуйте простой web-сервер с Thread Pool и статистикой.

### Упражнение 3: Параллельная сортировка
Создайте алгоритм параллельной сортировки с использованием Thread Pool.

### Упражнение 4: Система мониторинга
Реализуйте систему мониторинга производительности Thread Pool.

## 💡 Важные принципы

1. **Правильный размер пула**: Обычно равен количеству CPU ядер
2. **Управление задачами**: Используйте std::future для получения результатов
3. **Graceful shutdown**: Корректно завершайте все потоки
4. **Мониторинг**: Отслеживайте производительность и загрузку
5. **Обработка ошибок**: Правильно обрабатывайте исключения в задачах
