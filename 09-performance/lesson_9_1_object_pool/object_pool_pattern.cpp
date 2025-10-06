#include <iostream>
#include <queue>
#include <mutex>
#include <memory>
#include <functional>
#include <atomic>
#include <thread>
#include <vector>
#include <chrono>
#include <random>
#include <unordered_set>

/**
 * @file object_pool_pattern.cpp
 * @brief Демонстрация Object Pool Pattern
 * 
 * Этот файл показывает полную реализацию Object Pool с различными
 * примерами использования для оптимизации производительности.
 */

// ============================================================================
// БАЗОВАЯ РЕАЛИЗАЦИЯ OBJECT POOL
// ============================================================================

/**
 * @brief Универсальный Object Pool
 */
template<typename T>
class ObjectPool {
private:
    std::queue<std::unique_ptr<T>> pool_;
    std::mutex mutex_;
    std::function<std::unique_ptr<T>()> factory_;
    std::atomic<size_t> maxSize_;
    std::atomic<size_t> currentSize_{0};
    std::atomic<size_t> createdCount_{0};
    std::atomic<size_t> borrowedCount_{0};
    std::atomic<size_t> returnedCount_{0};
    
    // Отслеживание выданных объектов для отладки
    std::unordered_set<T*> borrowedObjects_;
    std::mutex borrowedMutex_;
    
public:
    explicit ObjectPool(size_t maxSize = 100, 
                       std::function<std::unique_ptr<T>()> factory = []() { 
                           return std::make_unique<T>(); 
                       })
        : maxSize_(maxSize), factory_(factory) {
        
        std::cout << "🏊 ObjectPool создан: maxSize=" << maxSize_ << std::endl;
        
        // Предварительно создаем половину объектов
        size_t initialSize = maxSize_ / 2;
        for (size_t i = 0; i < initialSize; ++i) {
            pool_.push(factory_());
            currentSize_.fetch_add(1);
            createdCount_.fetch_add(1);
        }
        
        std::cout << "🏊 Предварительно создано " << initialSize << " объектов" << std::endl;
    }
    
    // Получение объекта из пула
    std::unique_ptr<T> acquire() {
        std::unique_lock<std::mutex> lock(mutex_);
        
        if (!pool_.empty()) {
            auto obj = std::move(pool_.front());
            pool_.pop();
            
            // Отслеживаем выданный объект
            {
                std::lock_guard<std::mutex> borrowedLock(borrowedMutex_);
                borrowedObjects_.insert(obj.get());
            }
            
            borrowedCount_.fetch_add(1);
            
            std::cout << "🏊 Выдан объект из пула (доступно: " << pool_.size() 
                      << ", всего: " << currentSize_.load() << ")" << std::endl;
            
            return obj;
        }
        
        // Если пул пуст, создаем новый объект (если не превышен лимит)
        if (currentSize_.load() < maxSize_.load()) {
            currentSize_.fetch_add(1);
            createdCount_.fetch_add(1);
            
            auto obj = factory_();
            
            // Отслеживаем выданный объект
            {
                std::lock_guard<std::mutex> borrowedLock(borrowedMutex_);
                borrowedObjects_.insert(obj.get());
            }
            
            borrowedCount_.fetch_add(1);
            
            std::cout << "🏊 Создан новый объект (доступно: " << pool_.size() 
                      << ", всего: " << currentSize_.load() << ")" << std::endl;
            
            return obj;
        }
        
        std::cout << "🏊 Пул переполнен, объект не выдан" << std::endl;
        return nullptr;
    }
    
    // Возврат объекта в пул
    void release(std::unique_ptr<T> obj) {
        if (!obj) return;
        
        // Сбрасываем состояние объекта
        if constexpr (std::is_base_of_v<Resettable, T>) {
            obj->reset();
        }
        
        std::lock_guard<std::mutex> lock(mutex_);
        
        // Удаляем из отслеживания
        {
            std::lock_guard<std::mutex> borrowedLock(borrowedMutex_);
            borrowedObjects_.erase(obj.get());
        }
        
        pool_.push(std::move(obj));
        returnedCount_.fetch_add(1);
        
        std::cout << "🏊 Объект возвращен в пул (доступно: " << pool_.size() 
                  << ", всего: " << currentSize_.load() << ")" << std::endl;
    }
    
    // Статистика
    struct Statistics {
        size_t maxSize;
        size_t currentSize;
        size_t available;
        size_t borrowed;
        size_t createdCount;
        size_t borrowedCount;
        size_t returnedCount;
        double utilizationRate;
    };
    
    Statistics getStatistics() const {
        Statistics stats;
        stats.maxSize = maxSize_.load();
        stats.currentSize = currentSize_.load();
        
        {
            std::lock_guard<std::mutex> lock(mutex_);
            stats.available = pool_.size();
        }
        
        {
            std::lock_guard<std::mutex> lock(borrowedMutex_);
            stats.borrowed = borrowedObjects_.size();
        }
        
        stats.createdCount = createdCount_.load();
        stats.borrowedCount = borrowedCount_.load();
        stats.returnedCount = returnedCount_.load();
        
        if (stats.createdCount > 0) {
            stats.utilizationRate = static_cast<double>(stats.borrowedCount) / stats.createdCount;
        } else {
            stats.utilizationRate = 0.0;
        }
        
        return stats;
    }
    
    void printStatistics() const {
        auto stats = getStatistics();
        std::cout << "\n=== СТАТИСТИКА OBJECT POOL ===" << std::endl;
        std::cout << "Максимальный размер: " << stats.maxSize << std::endl;
        std::cout << "Текущий размер: " << stats.currentSize << std::endl;
        std::cout << "Доступно: " << stats.available << std::endl;
        std::cout << "Выдано: " << stats.borrowed << std::endl;
        std::cout << "Создано всего: " << stats.createdCount << std::endl;
        std::cout << "Выдано всего: " << stats.borrowedCount << std::endl;
        std::cout << "Возвращено всего: " << stats.returnedCount << std::endl;
        std::cout << "Коэффициент использования: " << (stats.utilizationRate * 100) << "%" << std::endl;
        std::cout << "===============================" << std::endl;
    }
    
    // Проверка состояния пула
    size_t available() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return pool_.size();
    }
    
    size_t total() const {
        return currentSize_.load();
    }
    
    size_t borrowed() const {
        std::lock_guard<std::mutex> lock(borrowedMutex_);
        return borrowedObjects_.size();
    }
    
    bool isEmpty() const {
        return available() == 0;
    }
    
    bool isFull() const {
        return currentSize_.load() >= maxSize_.load();
    }
};

// ============================================================================
// RAII ОБЕРТКА ДЛЯ АВТОМАТИЧЕСКОГО ВОЗВРАТА
// ============================================================================

/**
 * @brief RAII обертка для автоматического возврата объекта в пул
 */
template<typename T>
class PooledObject {
private:
    ObjectPool<T>& pool_;
    std::unique_ptr<T> object_;
    
public:
    explicit PooledObject(ObjectPool<T>& pool) : pool_(pool), object_(pool.acquire()) {
        if (!object_) {
            throw std::runtime_error("Не удалось получить объект из пула");
        }
    }
    
    ~PooledObject() {
        if (object_) {
            pool_.release(std::move(object_));
        }
    }
    
    // Запрещаем копирование
    PooledObject(const PooledObject&) = delete;
    PooledObject& operator=(const PooledObject&) = delete;
    
    // Разрешаем перемещение
    PooledObject(PooledObject&& other) noexcept 
        : pool_(other.pool_), object_(std::move(other.object_)) {}
    
    PooledObject& operator=(PooledObject&& other) noexcept {
        if (this != &other) {
            if (object_) {
                pool_.release(std::move(object_));
            }
            object_ = std::move(other.object_);
        }
        return *this;
    }
    
    T* get() const { return object_.get(); }
    T* operator->() const { return object_.get(); }
    T& operator*() const { return *object_; }
    
    explicit operator bool() const { return object_ != nullptr; }
};

// ============================================================================
// ИНТЕРФЕЙС ДЛЯ СБРОСА СОСТОЯНИЯ
// ============================================================================

/**
 * @brief Интерфейс для объектов, которые могут сбрасывать свое состояние
 */
class Resettable {
public:
    virtual ~Resettable() = default;
    virtual void reset() = 0;
};

// ============================================================================
// ПРИМЕРЫ ОБЪЕКТОВ ДЛЯ ПУЛА
// ============================================================================

/**
 * @brief Соединение с базой данных
 */
class DatabaseConnection : public Resettable {
private:
    std::string connectionString_;
    bool isConnected_;
    std::chrono::system_clock::time_point createdAt_;
    
public:
    explicit DatabaseConnection(const std::string& connStr = "localhost:5432") 
        : connectionString_(connStr), isConnected_(false),
          createdAt_(std::chrono::system_clock::now()) {
        connect();
    }
    
    ~DatabaseConnection() {
        disconnect();
    }
    
    void reset() override {
        if (isConnected_) {
            disconnect();
        }
        isConnected_ = false;
        std::cout << "🔄 DatabaseConnection сброшен" << std::endl;
    }
    
    bool executeQuery(const std::string& query) {
        if (!isConnected_) {
            connect();
        }
        
        // Имитация выполнения запроса
        std::this_thread::sleep_for(std::chrono::milliseconds(10 + (rand() % 50)));
        
        std::cout << "🗄️ Выполнен запрос: " << query << std::endl;
        return true;
    }
    
    std::string getConnectionInfo() const {
        return "DB: " + connectionString_ + " (создано: " + 
               std::to_string(std::chrono::duration_cast<std::chrono::seconds>(
                   std::chrono::system_clock::now() - createdAt_).count()) + "s назад)";
    }
    
private:
    void connect() {
        std::cout << "🔗 Подключение к БД: " << connectionString_ << std::endl;
        isConnected_ = true;
    }
    
    void disconnect() {
        if (isConnected_) {
            std::cout << "🔌 Отключение от БД: " << connectionString_ << std::endl;
            isConnected_ = false;
        }
    }
};

/**
 * @brief Игровой объект
 */
class GameObject : public Resettable {
private:
    int id_;
    float x_, y_, z_;
    bool active_;
    std::chrono::system_clock::time_point createdAt_;
    
public:
    GameObject() : id_(-1), x_(0), y_(0), z_(0), active_(false),
                   createdAt_(std::chrono::system_clock::now()) {}
    
    void reset() override {
        id_ = -1;
        x_ = y_ = z_ = 0.0f;
        active_ = false;
        std::cout << "🔄 GameObject сброшен" << std::endl;
    }
    
    void initialize(int id, float x, float y, float z) {
        id_ = id;
        x_ = x;
        y_ = y;
        z_ = z;
        active_ = true;
        
        std::cout << "🎮 GameObject " << id_ << " инициализирован в позиции (" 
                  << x_ << ", " << y_ << ", " << z_ << ")" << std::endl;
    }
    
    void update(float deltaTime) {
        if (!active_) return;
        
        // Имитация обновления логики
        x_ += deltaTime * 10.0f;
        y_ += deltaTime * 5.0f;
        
        std::cout << "🎮 Обновление GameObject " << id_ << " -> (" 
                  << x_ << ", " << y_ << ", " << z_ << ")" << std::endl;
    }
    
    void render() {
        if (!active_) return;
        
        std::cout << "🎨 Отрисовка GameObject " << id_ << " в позиции (" 
                  << x_ << ", " << y_ << ", " << z_ << ")" << std::endl;
    }
    
    bool isActive() const { return active_; }
    int getId() const { return id_; }
    
    std::string getInfo() const {
        return "GameObject{id=" + std::to_string(id_) + 
               ", pos=(" + std::to_string(x_) + "," + std::to_string(y_) + "," + std::to_string(z_) + 
               "), active=" + (active_ ? "true" : "false") + "}";
    }
};

/**
 * @brief Сетевой сокет
 */
class NetworkSocket : public Resettable {
private:
    std::string address_;
    int port_;
    bool isConnected_;
    std::chrono::system_clock::time_point createdAt_;
    
public:
    NetworkSocket(const std::string& addr = "127.0.0.1", int p = 8080)
        : address_(addr), port_(p), isConnected_(false),
          createdAt_(std::chrono::system_clock::now()) {}
    
    void reset() override {
        if (isConnected_) {
            disconnect();
        }
        isConnected_ = false;
        std::cout << "🔄 NetworkSocket сброшен" << std::endl;
    }
    
    bool connect() {
        if (!isConnected_) {
            std::cout << "🌐 Подключение к " << address_ << ":" << port_ << std::endl;
            isConnected_ = true;
        }
        return isConnected_;
    }
    
    bool sendData(const std::string& data) {
        if (!isConnected_) {
            return false;
        }
        
        // Имитация отправки данных
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        std::cout << "📤 Отправка данных: " << data << " -> " << address_ << ":" << port_ << std::endl;
        return true;
    }
    
    std::string receiveData() {
        if (!isConnected_) {
            return "";
        }
        
        // Имитация получения данных
        std::this_thread::sleep_for(std::chrono::milliseconds(3));
        return "Ответ от " + address_ + ":" + std::to_string(port_);
    }
    
    void disconnect() {
        if (isConnected_) {
            std::cout << "🔌 Отключение от " << address_ << ":" << port_ << std::endl;
            isConnected_ = false;
        }
    }
    
    std::string getConnectionInfo() const {
        return "Socket: " + address_ + ":" + std::to_string(port_) + 
               " (создан: " + std::to_string(std::chrono::duration_cast<std::chrono::seconds>(
                   std::chrono::system_clock::now() - createdAt_).count()) + "s назад)";
    }
};

// ============================================================================
// ДЕМОНСТРАЦИОННЫЕ ФУНКЦИИ
// ============================================================================

/**
 * @brief Демонстрация базового Object Pool
 */
void demonstrateBasicObjectPool() {
    std::cout << "\n=== БАЗОВЫЙ OBJECT POOL ===" << std::endl;
    
    ObjectPool<DatabaseConnection> dbPool(5, []() {
        return std::make_unique<DatabaseConnection>("pool_db:5432");
    });
    
    std::cout << "Начальное состояние пула:" << std::endl;
    dbPool.printStatistics();
    
    // Получаем объекты из пула
    std::vector<std::unique_ptr<DatabaseConnection>> connections;
    
    for (int i = 0; i < 7; ++i) {
        auto conn = dbPool.acquire();
        if (conn) {
            conn->executeQuery("SELECT * FROM users WHERE id = " + std::to_string(i));
            connections.push_back(std::move(conn));
        } else {
            std::cout << "❌ Не удалось получить соединение " << i << std::endl;
        }
    }
    
    std::cout << "\nСостояние пула после получения объектов:" << std::endl;
    dbPool.printStatistics();
    
    // Возвращаем объекты в пул
    for (auto& conn : connections) {
        dbPool.release(std::move(conn));
    }
    connections.clear();
    
    std::cout << "\nСостояние пула после возврата объектов:" << std::endl;
    dbPool.printStatistics();
}

/**
 * @brief Демонстрация RAII обертки
 */
void demonstrateRAIIWrapper() {
    std::cout << "\n=== RAII ОБЕРТКА ===" << std::endl;
    
    ObjectPool<GameObject> gameObjectPool(10);
    
    std::cout << "Начальное состояние пула:" << std::endl;
    gameObjectPool.printStatistics();
    
    // Используем RAII обертку
    {
        std::cout << "\n--- Блок 1 ---" << std::endl;
        PooledObject<GameObject> obj1(gameObjectPool);
        PooledObject<GameObject> obj2(gameObjectPool);
        PooledObject<GameObject> obj3(gameObjectPool);
        
        obj1->initialize(1, 10.0f, 20.0f, 30.0f);
        obj2->initialize(2, 40.0f, 50.0f, 60.0f);
        obj3->initialize(3, 70.0f, 80.0f, 90.0f);
        
        obj1->update(1.0f);
        obj2->render();
        obj3->update(0.5f);
        
        std::cout << "Состояние пула в блоке:" << std::endl;
        gameObjectPool.printStatistics();
        
        std::cout << "Объекты автоматически вернутся в пул при выходе из блока" << std::endl;
    }
    
    std::cout << "\nСостояние пула после выхода из блока:" << std::endl;
    gameObjectPool.printStatistics();
}

/**
 * @brief Демонстрация производительности
 */
void demonstratePerformance() {
    std::cout << "\n=== ТЕСТ ПРОИЗВОДИТЕЛЬНОСТИ ===" << std::endl;
    
    const int NUM_OPERATIONS = 10000;
    
    // Тест с Object Pool
    {
        std::cout << "Тестирую Object Pool..." << std::endl;
        
        ObjectPool<NetworkSocket> socketPool(100);
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < NUM_OPERATIONS; ++i) {
            PooledObject<NetworkSocket> socket(socketPool);
            socket->connect();
            socket->sendData("Test data " + std::to_string(i));
            socket->receiveData();
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "Object Pool: " << NUM_OPERATIONS << " операций за " 
                  << duration.count() << " мс" << std::endl;
        std::cout << "Производительность: " 
                  << (NUM_OPERATIONS * 1000.0 / duration.count()) << " операций/сек" << std::endl;
        
        socketPool.printStatistics();
    }
    
    // Тест без Object Pool (создание/уничтожение объектов)
    {
        std::cout << "\nТестирую без Object Pool..." << std::endl;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < NUM_OPERATIONS; ++i) {
            auto socket = std::make_unique<NetworkSocket>();
            socket->connect();
            socket->sendData("Test data " + std::to_string(i));
            socket->receiveData();
            socket->disconnect();
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "Без Object Pool: " << NUM_OPERATIONS << " операций за " 
                  << duration.count() << " мс" << std::endl;
        std::cout << "Производительность: " 
                  << (NUM_OPERATIONS * 1000.0 / duration.count()) << " операций/сек" << std::endl;
    }
}

/**
 * @brief Демонстрация многопоточности
 */
void demonstrateMultithreading() {
    std::cout << "\n=== МНОГОПОТОЧНОСТЬ ===" << std::endl;
    
    ObjectPool<DatabaseConnection> dbPool(20);
    
    const int NUM_THREADS = 5;
    const int OPERATIONS_PER_THREAD = 100;
    
    std::vector<std::thread> threads;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Запускаем потоки
    for (int t = 0; t < NUM_THREADS; ++t) {
        threads.emplace_back([&dbPool, t, OPERATIONS_PER_THREAD]() {
            for (int i = 0; i < OPERATIONS_PER_THREAD; ++i) {
                PooledObject<DatabaseConnection> conn(dbPool);
                conn->executeQuery("Thread " + std::to_string(t) + 
                                 " Operation " + std::to_string(i));
                
                // Небольшая задержка для имитации работы
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            
            std::cout << "Поток " << t << " завершен" << std::endl;
        });
    }
    
    // Ждем завершения всех потоков
    for (auto& thread : threads) {
        thread.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Многопоточный тест завершен за " << duration.count() << " мс" << std::endl;
    
    dbPool.printStatistics();
}

// ============================================================================
// ОСНОВНАЯ ФУНКЦИЯ
// ============================================================================

int main() {
    std::cout << "🏊 Демонстрация Object Pool Pattern" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    
    try {
        demonstrateBasicObjectPool();
        demonstrateRAIIWrapper();
        demonstratePerformance();
        demonstrateMultithreading();
        
        std::cout << "\n✅ Все демонстрации завершены успешно!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Ошибка: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\n🎯 Рекомендации:" << std::endl;
    std::cout << "1. Используйте Object Pool для дорогих объектов" << std::endl;
    std::cout << "2. Реализуйте интерфейс reset() для сброса состояния" << std::endl;
    std::cout << "3. Используйте RAII обертки для автоматического возврата" << std::endl;
    std::cout << "4. Мониторьте использование и производительность пула" << std::endl;
    std::cout << "5. Настройте размер пула под ваши потребности" << std::endl;
    
    return 0;
}
