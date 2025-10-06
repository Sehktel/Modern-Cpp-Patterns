# 🏭 Упражнения: Producer-Consumer Pattern

## 📋 Задание 1: Многопоточная обработка файлов

### Описание
Создайте систему для параллельной обработки файлов с использованием Producer-Consumer паттерна.

### Требования
1. **FileProcessor**: Producer, который читает файлы и помещает их содержимое в очередь
2. **TextAnalyzer**: Consumer, который анализирует текст файлов
3. **StatisticsCollector**: Consumer, который собирает статистику по файлам
4. **Thread-safe очередь**: Для передачи данных между потоками

### Архитектура
```cpp
#include <fstream>
#include <string>
#include <vector>
#include <map>

// Структура для передачи данных о файле
struct FileData {
    std::string filename;
    std::string content;
    size_t fileSize;
    std::chrono::system_clock::time_point timestamp;
};

// Интерфейс для анализа файлов
class FileAnalyzer {
public:
    virtual ~FileAnalyzer() = default;
    virtual void analyze(const FileData& fileData) = 0;
    virtual std::string getResults() const = 0;
};

// Анализатор текста
class TextAnalyzer : public FileAnalyzer {
private:
    std::map<std::string, int> wordCount_;
    std::atomic<size_t> totalWords_{0};
    std::atomic<size_t> totalFiles_{0};
    
public:
    void analyze(const FileData& fileData) override {
        // Подсчет слов, символов, строк
        // Анализ частоты слов
        // Сохранение статистики
    }
    
    std::string getResults() const override {
        // Возврат результатов анализа
    }
};

// Сборщик статистики
class StatisticsCollector : public FileAnalyzer {
private:
    std::atomic<size_t> totalFiles_{0};
    std::atomic<size_t> totalBytes_{0};
    std::atomic<size_t> totalLines_{0};
    
public:
    void analyze(const FileData& fileData) override {
        // Сбор общей статистики по файлам
    }
    
    std::string getResults() const override {
        // Возврат общей статистики
    }
};
```

### Реализация
```cpp
class FileProcessor {
private:
    ProducerConsumerQueue<FileData>& queue_;
    std::vector<std::string> fileList_;
    
public:
    FileProcessor(ProducerConsumerQueue<FileData>& queue, 
                  const std::vector<std::string>& files)
        : queue_(queue), fileList_(files) {}
    
    void processFiles() {
        std::cout << "[FileProcessor] Начинаю обработку " 
                  << fileList_.size() << " файлов..." << std::endl;
        
        for (const auto& filename : fileList_) {
            try {
                FileData fileData = readFile(filename);
                if (!queue_.push(fileData)) {
                    std::cout << "[FileProcessor] Не удалось добавить файл: " 
                              << filename << std::endl;
                    break;
                }
                
                std::cout << "[FileProcessor] Обработал файл: " << filename << std::endl;
                
            } catch (const std::exception& e) {
                std::cerr << "[FileProcessor] Ошибка чтения файла " << filename 
                          << ": " << e.what() << std::endl;
            }
        }
        
        queue_.finish();
        std::cout << "[FileProcessor] Завершил обработку файлов" << std::endl;
    }
    
private:
    FileData readFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Не удалось открыть файл: " + filename);
        }
        
        FileData fileData;
        fileData.filename = filename;
        
        // Читаем содержимое файла
        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        fileData.content = content;
        fileData.fileSize = content.size();
        fileData.timestamp = std::chrono::system_clock::now();
        
        return fileData;
    }
};
```

### Демонстрация
```cpp
void demonstrateFileProcessing() {
    std::cout << "=== Многопоточная обработка файлов ===" << std::endl;
    
    // Создаем очередь
    ProducerConsumerQueue<FileData> queue(50);
    
    // Список файлов для обработки
    std::vector<std::string> files = {
        "file1.txt", "file2.txt", "file3.txt", "file4.txt", "file5.txt"
    };
    
    // Создаем analyzer'ы
    TextAnalyzer textAnalyzer;
    StatisticsCollector statsCollector;
    
    // Запускаем FileProcessor
    FileProcessor processor(queue, files);
    std::thread processorThread(&FileProcessor::processFiles, &processor);
    
    // Запускаем TextAnalyzer
    std::thread textThread([&]() {
        FileData fileData;
        while (queue.pop(fileData)) {
            textAnalyzer.analyze(fileData);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        std::cout << "[TextAnalyzer] Завершен" << std::endl;
    });
    
    // Запускаем StatisticsCollector
    std::thread statsThread([&]() {
        FileData fileData;
        while (queue.pop(fileData)) {
            statsCollector.analyze(fileData);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        std::cout << "[StatisticsCollector] Завершен" << std::endl;
    });
    
    // Ждем завершения всех потоков
    processorThread.join();
    textThread.join();
    statsThread.join();
    
    // Выводим результаты
    std::cout << "\n=== РЕЗУЛЬТАТЫ АНАЛИЗА ===" << std::endl;
    std::cout << textAnalyzer.getResults() << std::endl;
    std::cout << statsCollector.getResults() << std::endl;
}
```

---

## 📋 Задание 2: Система очереди задач с приоритетами

### Описание
Реализуйте систему очереди задач для web-сервера с поддержкой приоритетов и балансировки нагрузки.

### Требования
1. **Task**: Структура задачи с приоритетом и данными
2. **PriorityQueue**: Очередь с поддержкой приоритетов
3. **TaskProcessor**: Consumer для обработки задач
4. **LoadBalancer**: Балансировщик нагрузки между processor'ами

### Архитектура
```cpp
// Приоритеты задач
enum class TaskPriority {
    LOW = 1,
    NORMAL = 2,
    HIGH = 3,
    CRITICAL = 4
};

// Структура задачи
struct Task {
    int id;
    TaskPriority priority;
    std::string type;
    std::string data;
    std::chrono::system_clock::time_point created;
    std::chrono::milliseconds estimatedDuration;
    
    // Для сортировки по приоритету
    bool operator<(const Task& other) const {
        return priority < other.priority;
    }
};

// Приоритетная очередь
template<typename T>
class PriorityProducerConsumerQueue {
private:
    std::priority_queue<T> queue_;
    mutable std::mutex mutex_;
    std::condition_variable condition_;
    bool finished_ = false;
    
public:
    bool push(T item) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (finished_) return false;
        
        queue_.push(std::move(item));
        condition_.notify_one();
        return true;
    }
    
    bool pop(T& item) {
        std::unique_lock<std::mutex> lock(mutex_);
        condition_.wait(lock, [this] { return !queue_.empty() || finished_; });
        
        if (queue_.empty()) return false;
        
        item = std::move(const_cast<T&>(queue_.top()));
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

### Реализация TaskProcessor
```cpp
class TaskProcessor {
private:
    PriorityProducerConsumerQueue<Task>& queue_;
    std::string name_;
    std::atomic<size_t> processedTasks_{0};
    std::atomic<size_t> totalProcessingTime_{0};
    
public:
    TaskProcessor(PriorityProducerConsumerQueue<Task>& queue, const std::string& name)
        : queue_(queue), name_(name) {}
    
    void processTasks() {
        std::cout << "[" << name_ << "] Запуск обработки задач..." << std::endl;
        
        Task task;
        while (queue_.pop(task)) {
            auto start = std::chrono::high_resolution_clock::now();
            
            // Обрабатываем задачу
            processTask(task);
            
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            
            totalProcessingTime_.fetch_add(duration.count());
            processedTasks_.fetch_add(1);
            
            std::cout << "[" << name_ << "] Обработал задачу " << task.id 
                      << " (приоритет: " << static_cast<int>(task.priority) 
                      << ") за " << duration.count() << " мс" << std::endl;
        }
        
        std::cout << "[" << name_ << "] Завершил обработку. "
                  << "Обработано задач: " << processedTasks_.load() 
                  << ", Общее время: " << totalProcessingTime_.load() << " мс" << std::endl;
    }
    
private:
    void processTask(const Task& task) {
        // Имитация обработки задачи
        std::this_thread::sleep_for(task.estimatedDuration);
        
        // Логика обработки в зависимости от типа задачи
        if (task.type == "HTTP_REQUEST") {
            processHttpRequest(task);
        } else if (task.type == "DATABASE_QUERY") {
            processDatabaseQuery(task);
        } else if (task.type == "FILE_OPERATION") {
            processFileOperation(task);
        } else {
            std::cout << "[" << name_ << "] Неизвестный тип задачи: " << task.type << std::endl;
        }
    }
    
    void processHttpRequest(const Task& task) {
        // Имитация обработки HTTP запроса
        std::cout << "[" << name_ << "] Обрабатываю HTTP запрос: " << task.data << std::endl;
    }
    
    void processDatabaseQuery(const Task& task) {
        // Имитация выполнения SQL запроса
        std::cout << "[" << name_ << "] Выполняю SQL запрос: " << task.data << std::endl;
    }
    
    void processFileOperation(const Task& task) {
        // Имитация файловой операции
        std::cout << "[" << name_ << "] Выполняю файловую операцию: " << task.data << std::endl;
    }
};
```

### Демонстрация
```cpp
void demonstrateTaskQueue() {
    std::cout << "=== Система очереди задач с приоритетами ===" << std::endl;
    
    PriorityProducerConsumerQueue<Task> taskQueue;
    
    // Создаем несколько processor'ов
    std::vector<std::unique_ptr<TaskProcessor>> processors;
    for (int i = 1; i <= 3; ++i) {
        processors.push_back(std::make_unique<TaskProcessor>(taskQueue, "Processor" + std::to_string(i)));
    }
    
    // Запускаем processor'ы
    std::vector<std::thread> processorThreads;
    for (auto& processor : processors) {
        processorThreads.emplace_back(&TaskProcessor::processTasks, processor.get());
    }
    
    // Генерируем задачи с разными приоритетами
    std::thread taskGenerator([&]() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> priorityDist(1, 4);
        std::uniform_int_distribution<> durationDist(50, 500);
        
        std::vector<std::string> taskTypes = {"HTTP_REQUEST", "DATABASE_QUERY", "FILE_OPERATION"};
        std::uniform_int_distribution<> typeDist(0, taskTypes.size() - 1);
        
        for (int i = 1; i <= 20; ++i) {
            Task task;
            task.id = i;
            task.priority = static_cast<TaskPriority>(priorityDist(gen));
            task.type = taskTypes[typeDist(gen)];
            task.data = "Task data " + std::to_string(i);
            task.created = std::chrono::system_clock::now();
            task.estimatedDuration = std::chrono::milliseconds(durationDist(gen));
            
            taskQueue.push(task);
            std::cout << "[TaskGenerator] Создал задачу " << i 
                      << " (приоритет: " << static_cast<int>(task.priority) << ")" << std::endl;
            
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        taskQueue.finish();
        std::cout << "[TaskGenerator] Завершил создание задач" << std::endl;
    });
    
    // Ждем завершения всех потоков
    taskGenerator.join();
    for (auto& thread : processorThreads) {
        thread.join();
    }
    
    std::cout << "\n=== СТАТИСТИКА ОБРАБОТКИ ===" << std::endl;
    for (size_t i = 0; i < processors.size(); ++i) {
        std::cout << "Processor" << (i + 1) << ": "
                  << processors[i]->getProcessedTasks() << " задач, "
                  << processors[i]->getTotalProcessingTime() << " мс" << std::endl;
    }
}
```

---

## 📋 Задание 3: Система стриминга данных в реальном времени

### Описание
Создайте систему для обработки потоков данных в реальном времени с использованием Producer-Consumer.

### Требования
1. **DataStream**: Поток данных с метаданными
2. **StreamProcessor**: Обработчик потока данных
3. **DataAggregator**: Агрегатор данных
4. **Real-time monitoring**: Мониторинг в реальном времени

### Архитектура
```cpp
// Структура потока данных
struct DataStream {
    std::string streamId;
    std::vector<double> values;
    std::chrono::system_clock::time_point timestamp;
    std::string source;
    double sampleRate;
};

// Интерфейс для обработки потоков
class StreamProcessor {
public:
    virtual ~StreamProcessor() = default;
    virtual void process(const DataStream& stream) = 0;
    virtual std::string getResults() const = 0;
};

// Обработчик статистики
class StatisticsProcessor : public StreamProcessor {
private:
    std::map<std::string, std::vector<double>> streamHistory_;
    std::map<std::string, double> streamAverages_;
    std::map<std::string, double> streamVariances_;
    
public:
    void process(const DataStream& stream) override {
        const std::string& id = stream.streamId;
        
        // Сохраняем историю
        streamHistory_[id].insert(streamHistory_[id].end(), 
                                 stream.values.begin(), stream.values.end());
        
        // Ограничиваем размер истории
        if (streamHistory_[id].size() > 1000) {
            streamHistory_[id].erase(streamHistory_[id].begin(), 
                                   streamHistory_[id].begin() + 100);
        }
        
        // Вычисляем статистику
        updateStatistics(id);
    }
    
    std::string getResults() const override {
        std::ostringstream oss;
        oss << "=== СТАТИСТИКА ПОТОКОВ ===" << std::endl;
        
        for (const auto& pair : streamAverages_) {
            const std::string& id = pair.first;
            double avg = pair.second;
            double var = streamVariances_.at(id);
            
            oss << "Stream " << id << ": "
                << "Avg=" << std::fixed << std::setprecision(2) << avg
                << ", Var=" << var
                << ", Samples=" << streamHistory_.at(id).size() << std::endl;
        }
        
        return oss.str();
    }
    
private:
    void updateStatistics(const std::string& streamId) {
        const auto& values = streamHistory_[streamId];
        if (values.empty()) return;
        
        // Вычисляем среднее
        double sum = std::accumulate(values.begin(), values.end(), 0.0);
        double average = sum / values.size();
        streamAverages_[streamId] = average;
        
        // Вычисляем дисперсию
        double variance = 0.0;
        for (double value : values) {
            variance += (value - average) * (value - average);
        }
        variance /= values.size();
        streamVariances_[streamId] = variance;
    }
};

// Детектор аномалий
class AnomalyDetector : public StreamProcessor {
private:
    std::map<std::string, double> thresholds_;
    std::vector<std::string> anomalies_;
    
public:
    void process(const DataStream& stream) override {
        const std::string& id = stream.streamId;
        
        // Устанавливаем порог для нового потока
        if (thresholds_.find(id) == thresholds_.end()) {
            thresholds_[id] = calculateThreshold(stream.values);
        }
        
        // Проверяем на аномалии
        for (double value : stream.values) {
            if (std::abs(value) > thresholds_[id]) {
                std::ostringstream oss;
                oss << "Аномалия в потоке " << id 
                    << ": значение " << value 
                    << " превышает порог " << thresholds_[id]
                    << " в " << std::chrono::duration_cast<std::chrono::seconds>(
                        std::chrono::system_clock::now().time_since_epoch()).count();
                
                anomalies_.push_back(oss.str());
                std::cout << "[AnomalyDetector] " << oss.str() << std::endl;
            }
        }
    }
    
    std::string getResults() const override {
        std::ostringstream oss;
        oss << "=== ДЕТЕКТОР АНОМАЛИЙ ===" << std::endl;
        oss << "Обнаружено аномалий: " << anomalies_.size() << std::endl;
        
        for (const auto& anomaly : anomalies_) {
            oss << anomaly << std::endl;
        }
        
        return oss.str();
    }
    
private:
    double calculateThreshold(const std::vector<double>& values) {
        if (values.empty()) return 0.0;
        
        double mean = std::accumulate(values.begin(), values.end(), 0.0) / values.size();
        double variance = 0.0;
        
        for (double value : values) {
            variance += (value - mean) * (value - mean);
        }
        variance /= values.size();
        
        // Порог = среднее + 2 * стандартное отклонение
        return mean + 2.0 * std::sqrt(variance);
    }
};
```

### Демонстрация
```cpp
void demonstrateDataStreaming() {
    std::cout << "=== Система стриминга данных в реальном времени ===" << std::endl;
    
    ProducerConsumerQueue<DataStream> streamQueue(100);
    
    // Создаем processor'ы
    StatisticsProcessor statsProcessor;
    AnomalyDetector anomalyDetector;
    
    // Запускаем processor'ы
    std::thread statsThread([&]() {
        DataStream stream;
        while (streamQueue.pop(stream)) {
            statsProcessor.process(stream);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        std::cout << "[StatisticsProcessor] Завершен" << std::endl;
    });
    
    std::thread anomalyThread([&]() {
        DataStream stream;
        while (streamQueue.pop(stream)) {
            anomalyDetector.process(stream);
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
        }
        std::cout << "[AnomalyDetector] Завершен" << std::endl;
    });
    
    // Генерируем потоки данных
    std::thread dataGenerator([&]() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> normalDist(0.0, 10.0);
        std::uniform_real_distribution<> anomalyDist(20.0, 50.0);
        
        std::vector<std::string> streamIds = {"sensor1", "sensor2", "sensor3", "sensor4"};
        
        for (int i = 0; i < 100; ++i) {
            DataStream stream;
            stream.streamId = streamIds[i % streamIds.size()];
            stream.timestamp = std::chrono::system_clock::now();
            stream.source = "sensor_" + std::to_string(i % 4);
            stream.sampleRate = 100.0;
            
            // Генерируем данные (обычно нормальные, иногда аномальные)
            for (int j = 0; j < 10; ++j) {
                if (i % 20 == 0 && j == 5) {
                    // Иногда добавляем аномалию
                    stream.values.push_back(anomalyDist(gen));
                } else {
                    stream.values.push_back(normalDist(gen));
                }
            }
            
            streamQueue.push(stream);
            std::cout << "[DataGenerator] Создал поток " << stream.streamId 
                      << " с " << stream.values.size() << " значениями" << std::endl;
            
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        streamQueue.finish();
        std::cout << "[DataGenerator] Завершил генерацию данных" << std::endl;
    });
    
    // Ждем завершения всех потоков
    dataGenerator.join();
    statsThread.join();
    anomalyThread.join();
    
    // Выводим результаты
    std::cout << "\n" << statsProcessor.getResults() << std::endl;
    std::cout << anomalyDetector.getResults() << std::endl;
}
```

---

## 📋 Задание 4: Lock-free реализация Producer-Consumer

### Описание
Реализуйте lock-free версию Producer-Consumer очереди с использованием атомарных операций.

### Требования
1. **Lock-free queue**: Очередь без блокировок
2. **Memory ordering**: Правильный порядок операций с памятью
3. **ABA problem**: Решение проблемы ABA
4. **Performance testing**: Сравнение производительности

### Архитектура
```cpp
template<typename T>
class LockFreeQueue {
private:
    struct Node {
        std::atomic<T*> data;
        std::atomic<Node*> next;
        
        Node() : data(nullptr), next(nullptr) {}
        
        explicit Node(T* data) : data(data), next(nullptr) {}
    };
    
    std::atomic<Node*> head_;
    std::atomic<Node*> tail_;
    std::atomic<size_t> size_;
    
public:
    LockFreeQueue() {
        Node* dummy = new Node();
        head_.store(dummy);
        tail_.store(dummy);
        size_.store(0);
    }
    
    ~LockFreeQueue() {
        // Очищаем все узлы
        while (Node* node = head_.load()) {
            Node* next = node->next.load();
            delete node->data.load();
            delete node;
            head_.store(next);
        }
    }
    
    bool push(T item) {
        Node* new_node = new Node(new T(std::move(item)));
        
        Node* prev_tail = tail_.exchange(new_node, std::memory_order_acq_rel);
        prev_tail->next.store(new_node, std::memory_order_release);
        
        size_.fetch_add(1, std::memory_order_relaxed);
        return true;
    }
    
    bool pop(T& item) {
        Node* head = head_.load(std::memory_order_acquire);
        Node* next = head->next.load(std::memory_order_acquire);
        
        if (next == nullptr) {
            return false; // Очередь пустая
        }
        
        T* data = next->data.load(std::memory_order_acquire);
        if (data == nullptr) {
            return false; // Данные уже извлечены другим потоком
        }
        
        // Пытаемся атомарно извлечь данные
        if (next->data.compare_exchange_strong(data, nullptr, 
                                              std::memory_order_acq_rel)) {
            item = std::move(*data);
            delete data;
            
            // Обновляем голову очереди
            head_.store(next, std::memory_order_release);
            delete head;
            
            size_.fetch_sub(1, std::memory_order_relaxed);
            return true;
        }
        
        return false; // Конкуренция с другим потоком
    }
    
    size_t size() const {
        return size_.load(std::memory_order_relaxed);
    }
    
    bool empty() const {
        return size() == 0;
    }
};
```

### Тестирование производительности
```cpp
void performanceTest() {
    std::cout << "=== Тест производительности ===" << std::endl;
    
    const int NUM_OPERATIONS = 1000000;
    const int NUM_THREADS = 4;
    
    // Тест с обычной очередью
    {
        std::cout << "\nТестируем обычную очередь..." << std::endl;
        ProducerConsumerQueue<int> regularQueue;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        std::vector<std::thread> threads;
        
        // Producer потоки
        for (int i = 0; i < NUM_THREADS / 2; ++i) {
            threads.emplace_back([&, i]() {
                for (int j = 0; j < NUM_OPERATIONS / NUM_THREADS; ++j) {
                    regularQueue.push(i * 1000 + j);
                }
            });
        }
        
        // Consumer потоки
        for (int i = 0; i < NUM_THREADS / 2; ++i) {
            threads.emplace_back([&]() {
                int item;
                int count = 0;
                while (regularQueue.pop(item) && count < NUM_OPERATIONS / NUM_THREADS) {
                    count++;
                }
            });
        }
        
        // Ждем завершения producer'ов
        for (int i = 0; i < NUM_THREADS / 2; ++i) {
            threads[i].join();
        }
        
        regularQueue.finish();
        
        // Ждем завершения consumer'ов
        for (int i = NUM_THREADS / 2; i < NUM_THREADS; ++i) {
            threads[i].join();
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "Обычная очередь: " << duration.count() << " мс" << std::endl;
    }
    
    // Тест с lock-free очередью
    {
        std::cout << "\nТестируем lock-free очередь..." << std::endl;
        LockFreeQueue<int> lockFreeQueue;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        std::vector<std::thread> threads;
        
        // Producer потоки
        for (int i = 0; i < NUM_THREADS / 2; ++i) {
            threads.emplace_back([&, i]() {
                for (int j = 0; j < NUM_OPERATIONS / NUM_THREADS; ++j) {
                    lockFreeQueue.push(i * 1000 + j);
                }
            });
        }
        
        // Consumer потоки
        for (int i = 0; i < NUM_THREADS / 2; ++i) {
            threads.emplace_back([&]() {
                int item;
                int count = 0;
                while (lockFreeQueue.pop(item) && count < NUM_OPERATIONS / NUM_THREADS) {
                    count++;
                }
            });
        }
        
        // Ждем завершения всех потоков
        for (auto& thread : threads) {
            thread.join();
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "Lock-free очередь: " << duration.count() << " мс" << std::endl;
    }
}
```

---

## 🎯 Критерии оценки

### Отлично (5 баллов)
- ✅ Все требования выполнены
- ✅ Современные C++17/20 возможности
- ✅ Thread-safe реализация
- ✅ Хорошая производительность
- ✅ Полная документация и тесты
- ✅ Анализ производительности

### Хорошо (4 балла)
- ✅ Большинство требований выполнено
- ✅ Современные возможности использованы
- ✅ Код работает корректно
- ✅ Есть тесты

### Удовлетворительно (3 балла)
- ✅ Основные требования выполнены
- ✅ Код компилируется и работает
- ❌ Не все современные возможности использованы
- ❌ Возможны проблемы с thread-safety

### Неудовлетворительно (2 балла)
- ❌ Код не компилируется или работает неправильно
- ❌ Не выполнены основные требования
- ❌ Проблемы с управлением памятью

---

## 💡 Подсказки

1. **Thread Safety**: Используйте `std::atomic` для lock-free операций
2. **Memory Ordering**: Изучите различные модели памяти C++11
3. **ABA Problem**: Используйте hazard pointers или epoch-based reclamation
4. **Performance**: Профилируйте критические участки кода
5. **Testing**: Тестируйте многопоточность с различными сценариями нагрузки

---

## 🚨 Важные замечания

### Когда НЕ использовать Producer-Consumer:
- ❌ Когда нужна синхронная обработка данных
- ❌ Когда порядок обработки критичен
- ❌ Когда overhead превышает пользу
- ❌ В системах с ограниченными ресурсами

### Когда использовать Producer-Consumer:
- ✅ Для асинхронной обработки данных
- ✅ Для балансировки нагрузки между потоками
- ✅ Для буферизации данных
- ✅ Для разделения I/O и вычислений

---

*Помните: Producer-Consumer - это мощный паттерн для многопоточного программирования, но используйте его с пониманием trade-offs и тщательным тестированием!* 🚀
