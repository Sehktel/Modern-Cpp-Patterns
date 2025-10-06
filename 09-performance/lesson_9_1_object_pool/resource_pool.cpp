/**
 * @file resource_pool.cpp
 * @brief Расширенный пул ресурсов для Object Pool Pattern
 * 
 * Реализован расширенный пул ресурсов с поддержкой:
 * - Пул соединений с БД
 * - Пул сетевых сокетов
 * - Пул буферов
 * - Мониторинг и статистика
 */

#include <iostream>
#include <queue>
#include <vector>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <atomic>
#include <unordered_map>
#include <functional>
#include <random>

// Базовый интерфейс для ресурсов
class Resource {
public:
    virtual ~Resource() = default;
    virtual bool isValid() const = 0;
    virtual void reset() = 0;
    virtual std::string getType() const = 0;
    virtual std::string getId() const = 0;
};

// Соединение с базой данных
class DatabaseConnection : public Resource {
private:
    std::string connection_id_;
    std::string database_url_;
    bool connected_;
    std::chrono::steady_clock::time_point created_at_;
    std::chrono::steady_clock::time_point last_used_;
    
public:
    DatabaseConnection(const std::string& id, const std::string& url)
        : connection_id_(id), database_url_(url), connected_(false) {
        created_at_ = std::chrono::steady_clock::now();
        last_used_ = created_at_;
        std::cout << "Создано соединение с БД: " << connection_id_ << std::endl;
    }
    
    ~DatabaseConnection() {
        if (connected_) {
            disconnect();
        }
    }
    
    bool connect() {
        if (connected_) return true;
        
        std::cout << "Подключаемся к БД: " << database_url_ << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Симуляция подключения
        
        connected_ = true;
        last_used_ = std::chrono::steady_clock::now();
        std::cout << "Соединение с БД установлено: " << connection_id_ << std::endl;
        return true;
    }
    
    void disconnect() {
        if (!connected_) return;
        
        std::cout << "Отключаемся от БД: " << connection_id_ << std::endl;
        connected_ = false;
    }
    
    bool executeQuery(const std::string& query) {
        if (!connected_) return false;
        
        std::cout << "Выполняем запрос в " << connection_id_ << ": " << query << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Симуляция выполнения
        last_used_ = std::chrono::steady_clock::now();
        return true;
    }
    
    bool isValid() const override {
        return connected_;
    }
    
    void reset() override {
        if (connected_) {
            disconnect();
        }
        connect();
    }
    
    std::string getType() const override {
        return "DatabaseConnection";
    }
    
    std::string getId() const override {
        return connection_id_;
    }
    
    std::chrono::steady_clock::time_point getLastUsed() const {
        return last_used_;
    }
    
    auto getAge() const {
        return std::chrono::steady_clock::now() - created_at_;
    }
};

// Сетевой сокет
class NetworkSocket : public Resource {
private:
    std::string socket_id_;
    std::string host_;
    int port_;
    bool connected_;
    std::chrono::steady_clock::time_point created_at_;
    std::chrono::steady_clock::time_point last_used_;
    
public:
    NetworkSocket(const std::string& id, const std::string& host, int port)
        : socket_id_(id), host_(host), port_(port), connected_(false) {
        created_at_ = std::chrono::steady_clock::now();
        last_used_ = created_at_;
        std::cout << "Создан сокет: " << socket_id_ << " (" << host_ << ":" << port_ << ")" << std::endl;
    }
    
    ~NetworkSocket() {
        if (connected_) {
            disconnect();
        }
    }
    
    bool connect() {
        if (connected_) return true;
        
        std::cout << "Подключаемся к " << host_ << ":" << port_ << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(200)); // Симуляция подключения
        
        connected_ = true;
        last_used_ = std::chrono::steady_clock::now();
        std::cout << "Сокет подключен: " << socket_id_ << std::endl;
        return true;
    }
    
    void disconnect() {
        if (!connected_) return;
        
        std::cout << "Отключаем сокет: " << socket_id_ << std::endl;
        connected_ = false;
    }
    
    bool sendData(const std::string& data) {
        if (!connected_) return false;
        
        std::cout << "Отправляем данные через " << socket_id_ << ": " << data << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(30)); // Симуляция отправки
        last_used_ = std::chrono::steady_clock::now();
        return true;
    }
    
    std::string receiveData() {
        if (!connected_) return "";
        
        std::cout << "Получаем данные через " << socket_id_ << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(20)); // Симуляция получения
        last_used_ = std::chrono::steady_clock::now();
        return "received_data_from_" + socket_id_;
    }
    
    bool isValid() const override {
        return connected_;
    }
    
    void reset() override {
        if (connected_) {
            disconnect();
        }
        connect();
    }
    
    std::string getType() const override {
        return "NetworkSocket";
    }
    
    std::string getId() const override {
        return socket_id_;
    }
    
    std::chrono::steady_clock::time_point getLastUsed() const {
        return last_used_;
    }
    
    auto getAge() const {
        return std::chrono::steady_clock::now() - created_at_;
    }
};

// Буфер данных
class DataBuffer : public Resource {
private:
    std::string buffer_id_;
    size_t size_;
    std::vector<char> data_;
    bool in_use_;
    std::chrono::steady_clock::time_point created_at_;
    std::chrono::steady_clock::time_point last_used_;
    
public:
    DataBuffer(const std::string& id, size_t buffer_size)
        : buffer_id_(id), size_(buffer_size), in_use_(false) {
        data_.resize(size_);
        created_at_ = std::chrono::steady_clock::now();
        last_used_ = created_at_;
        std::cout << "Создан буфер: " << buffer_id_ << " размером " << size_ << " байт" << std::endl;
    }
    
    void writeData(const std::string& data) {
        if (data.length() > size_) {
            std::cout << "Ошибка: данные превышают размер буфера" << std::endl;
            return;
        }
        
        std::copy(data.begin(), data.end(), data_.begin());
        last_used_ = std::chrono::steady_clock::now();
        std::cout << "Записаны данные в буфер " << buffer_id_ << ": " << data << std::endl;
    }
    
    std::string readData() {
        std::string result(data_.begin(), data_.end());
        last_used_ = std::chrono::steady_clock::now();
        std::cout << "Прочитаны данные из буфера " << buffer_id_ << std::endl;
        return result;
    }
    
    void clear() {
        std::fill(data_.begin(), data_.end(), 0);
        last_used_ = std::chrono::steady_clock::now();
        std::cout << "Буфер " << buffer_id_ << " очищен" << std::endl;
    }
    
    bool isValid() const override {
        return true; // Буфер всегда валиден
    }
    
    void reset() override {
        clear();
    }
    
    std::string getType() const override {
        return "DataBuffer";
    }
    
    std::string getId() const override {
        return buffer_id_;
    }
    
    size_t getSize() const {
        return size_;
    }
    
    bool isInUse() const {
        return in_use_;
    }
    
    void setInUse(bool use) {
        in_use_ = use;
    }
    
    std::chrono::steady_clock::time_point getLastUsed() const {
        return last_used_;
    }
    
    auto getAge() const {
        return std::chrono::steady_clock::now() - created_at_;
    }
};

// Статистика пула ресурсов
struct PoolStats {
    std::atomic<size_t> total_created_{0};
    std::atomic<size_t> total_destroyed_{0};
    std::atomic<size_t> current_active_{0};
    std::atomic<size_t> current_idle_{0};
    std::atomic<size_t> total_requests_{0};
    std::atomic<size_t> successful_requests_{0};
    std::atomic<size_t> failed_requests_{0};
    
    void print() const {
        std::cout << "\n=== Pool Statistics ===" << std::endl;
        std::cout << "Всего создано: " << total_created_.load() << std::endl;
        std::cout << "Всего уничтожено: " << total_destroyed_.load() << std::endl;
        std::cout << "Активных: " << current_active_.load() << std::endl;
        std::cout << "Свободных: " << current_idle_.load() << std::endl;
        std::cout << "Всего запросов: " << total_requests_.load() << std::endl;
        std::cout << "Успешных: " << successful_requests_.load() << std::endl;
        std::cout << "Неудачных: " << failed_requests_.load() << std::endl;
        
        if (total_requests_.load() > 0) {
            double success_rate = (double)successful_requests_.load() / total_requests_.load() * 100;
            std::cout << "Процент успеха: " << success_rate << "%" << std::endl;
        }
        std::cout << "=====================" << std::endl;
    }
};

// Универсальный пул ресурсов
template<typename T>
class ResourcePool {
private:
    std::queue<std::shared_ptr<T>> available_resources_;
    std::unordered_map<std::string, std::shared_ptr<T>> active_resources_;
    std::mutex pool_mutex_;
    std::condition_variable pool_condition_;
    
    size_t min_size_;
    size_t max_size_;
    std::chrono::milliseconds max_idle_time_;
    
    PoolStats stats_;
    std::atomic<bool> shutdown_{false};
    
    std::function<std::shared_ptr<T>()> resource_factory_;
    
public:
    ResourcePool(size_t min_size, size_t max_size, 
                 std::chrono::milliseconds max_idle_time,
                 std::function<std::shared_ptr<T>()> factory)
        : min_size_(min_size), max_size_(max_size), max_idle_time_(max_idle_time), resource_factory_(factory) {
        
        std::cout << "Создан пул ресурсов: min=" << min_size_ << ", max=" << max_size_ << std::endl;
        
        // Создаем минимальное количество ресурсов
        for (size_t i = 0; i < min_size_; ++i) {
            auto resource = resource_factory_();
            available_resources_.push(resource);
            stats_.total_created_.fetch_add(1);
            stats_.current_idle_.fetch_add(1);
        }
        
        // Запускаем фоновую задачу для очистки неиспользуемых ресурсов
        std::thread cleanup_thread([this]() { cleanupIdleResources(); });
        cleanup_thread.detach();
    }
    
    ~ResourcePool() {
        shutdown_.store(true);
        pool_condition_.notify_all();
        
        // Очищаем все ресурсы
        std::lock_guard<std::mutex> lock(pool_mutex_);
        while (!available_resources_.empty()) {
            available_resources_.pop();
        }
        active_resources_.clear();
    }
    
    std::shared_ptr<T> acquire(std::chrono::milliseconds timeout = std::chrono::milliseconds(5000)) {
        stats_.total_requests_.fetch_add(1);
        
        std::unique_lock<std::mutex> lock(pool_mutex_);
        
        // Ждем доступный ресурс или таймаут
        if (!pool_condition_.wait_for(lock, timeout, [this]() {
            return !available_resources_.empty() || shutdown_.load();
        })) {
            stats_.failed_requests_.fetch_add(1);
            std::cout << "Таймаут при получении ресурса" << std::endl;
            return nullptr;
        }
        
        if (shutdown_.load()) {
            stats_.failed_requests_.fetch_add(1);
            return nullptr;
        }
        
        // Получаем ресурс из пула
        auto resource = available_resources_.front();
        available_resources_.pop();
        
        // Перемещаем в активные
        active_resources_[resource->getId()] = resource;
        
        stats_.current_idle_.fetch_sub(1);
        stats_.current_active_.fetch_add(1);
        stats_.successful_requests_.fetch_add(1);
        
        std::cout << "Получен ресурс: " << resource->getId() << std::endl;
        return resource;
    }
    
    void release(std::shared_ptr<T> resource) {
        if (!resource) return;
        
        std::lock_guard<std::mutex> lock(pool_mutex_);
        
        // Удаляем из активных
        auto it = active_resources_.find(resource->getId());
        if (it != active_resources_.end()) {
            active_resources_.erase(it);
        }
        
        // Сбрасываем состояние ресурса
        resource->reset();
        
        // Возвращаем в пул
        available_resources_.push(resource);
        
        stats_.current_active_.fetch_sub(1);
        stats_.current_idle_.fetch_add(1);
        
        std::cout << "Освобожден ресурс: " << resource->getId() << std::endl;
        pool_condition_.notify_one();
    }
    
    void printStats() const {
        stats_.print();
    }
    
    size_t getAvailableCount() const {
        std::lock_guard<std::mutex> lock(pool_mutex_);
        return available_resources_.size();
    }
    
    size_t getActiveCount() const {
        std::lock_guard<std::mutex> lock(pool_mutex_);
        return active_resources_.size();
    }
    
private:
    void cleanupIdleResources() {
        while (!shutdown_.load()) {
            std::this_thread::sleep_for(std::chrono::seconds(10));
            
            std::lock_guard<std::mutex> lock(pool_mutex_);
            
            // Проверяем неиспользуемые ресурсы
            std::queue<std::shared_ptr<T>> temp_queue;
            while (!available_resources_.empty()) {
                auto resource = available_resources_.front();
                available_resources_.pop();
                
                // Проверяем время неиспользования
                auto idle_time = std::chrono::steady_clock::now() - resource->getLastUsed();
                if (idle_time > max_idle_time_ && available_resources_.size() > min_size_) {
                    // Удаляем старый ресурс
                    stats_.total_destroyed_.fetch_add(1);
                    stats_.current_idle_.fetch_sub(1);
                    std::cout << "Удален неиспользуемый ресурс: " << resource->getId() << std::endl;
                } else {
                    temp_queue.push(resource);
                }
            }
            
            available_resources_ = std::move(temp_queue);
        }
    }
};

// Демонстрация пула соединений с БД
void demonstrateDatabasePool() {
    std::cout << "\n=== Демонстрация пула соединений с БД ===" << std::endl;
    
    int connection_counter = 0;
    auto db_factory = [&connection_counter]() -> std::shared_ptr<DatabaseConnection> {
        return std::make_shared<DatabaseConnection>(
            "db_conn_" + std::to_string(++connection_counter),
            "localhost:5432/mydb"
        );
    };
    
    ResourcePool<DatabaseConnection> db_pool(2, 5, std::chrono::minutes(5), db_factory);
    
    // Получаем несколько соединений
    std::vector<std::shared_ptr<DatabaseConnection>> connections;
    
    for (int i = 0; i < 3; ++i) {
        auto conn = db_pool.acquire();
        if (conn) {
            conn->connect();
            conn->executeQuery("SELECT * FROM users");
            connections.push_back(conn);
        }
    }
    
    std::cout << "Активных соединений: " << db_pool.getActiveCount() << std::endl;
    std::cout << "Доступных соединений: " << db_pool.getAvailableCount() << std::endl;
    
    // Освобождаем соединения
    for (auto& conn : connections) {
        db_pool.release(conn);
    }
    
    db_pool.printStats();
}

// Демонстрация пула сетевых сокетов
void demonstrateSocketPool() {
    std::cout << "\n=== Демонстрация пула сетевых сокетов ===" << std::endl;
    
    int socket_counter = 0;
    auto socket_factory = [&socket_counter]() -> std::shared_ptr<NetworkSocket> {
        return std::make_shared<NetworkSocket>(
            "socket_" + std::to_string(++socket_counter),
            "example.com",
            8080
        );
    };
    
    ResourcePool<NetworkSocket> socket_pool(1, 3, std::chrono::minutes(2), socket_factory);
    
    // Получаем сокеты и выполняем операции
    std::vector<std::shared_ptr<NetworkSocket>> sockets;
    
    for (int i = 0; i < 2; ++i) {
        auto socket = socket_pool.acquire();
        if (socket) {
            socket->connect();
            socket->sendData("Hello from client " + std::to_string(i));
            socket->receiveData();
            sockets.push_back(socket);
        }
    }
    
    std::cout << "Активных сокетов: " << socket_pool.getActiveCount() << std::endl;
    std::cout << "Доступных сокетов: " << socket_pool.getAvailableCount() << std::endl;
    
    // Освобождаем сокеты
    for (auto& socket : sockets) {
        socket_pool.release(socket);
    }
    
    socket_pool.printStats();
}

// Демонстрация пула буферов
void demonstrateBufferPool() {
    std::cout << "\n=== Демонстрация пула буферов ===" << std::endl;
    
    int buffer_counter = 0;
    auto buffer_factory = [&buffer_counter]() -> std::shared_ptr<DataBuffer> {
        return std::make_shared<DataBuffer>(
            "buffer_" + std::to_string(++buffer_counter),
            1024
        );
    };
    
    ResourcePool<DataBuffer> buffer_pool(3, 10, std::chrono::minutes(1), buffer_factory);
    
    // Получаем буферы и работаем с данными
    std::vector<std::shared_ptr<DataBuffer>> buffers;
    
    for (int i = 0; i < 4; ++i) {
        auto buffer = buffer_pool.acquire();
        if (buffer) {
            buffer->writeData("Data for buffer " + std::to_string(i));
            std::string data = buffer->readData();
            buffers.push_back(buffer);
        }
    }
    
    std::cout << "Активных буферов: " << buffer_pool.getActiveCount() << std::endl;
    std::cout << "Доступных буферов: " << buffer_pool.getAvailableCount() << std::endl;
    
    // Освобождаем буферы
    for (auto& buffer : buffers) {
        buffer_pool.release(buffer);
    }
    
    buffer_pool.printStats();
}

// Демонстрация производительности пула
void demonstratePoolPerformance() {
    std::cout << "\n=== Демонстрация производительности пула ===" << std::endl;
    
    int resource_counter = 0;
    auto resource_factory = [&resource_counter]() -> std::shared_ptr<DatabaseConnection> {
        return std::make_shared<DatabaseConnection>(
            "perf_conn_" + std::to_string(++resource_counter),
            "localhost:5432/testdb"
        );
    };
    
    ResourcePool<DatabaseConnection> pool(5, 20, std::chrono::minutes(5), resource_factory);
    
    // Тестируем производительность
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::shared_ptr<DatabaseConnection>> resources;
    
    // Получаем ресурсы
    for (int i = 0; i < 100; ++i) {
        auto resource = pool.acquire();
        if (resource) {
            resource->connect();
            resource->executeQuery("SELECT * FROM test_table");
            resources.push_back(resource);
        }
    }
    
    // Освобождаем ресурсы
    for (auto& resource : resources) {
        pool.release(resource);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Выполнено 100 операций за " << duration.count() << " микросекунд" << std::endl;
    pool.printStats();
}

int main() {
    std::cout << "=== Resource Pool Pattern ===" << std::endl;
    
    try {
        demonstrateDatabasePool();
        demonstrateSocketPool();
        demonstrateBufferPool();
        demonstratePoolPerformance();
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\n=== Демонстрация завершена ===" << std::endl;
    return 0;
}
