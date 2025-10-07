# 🏋️ Упражнения: Object Pool Pattern

## 📋 Задание 1: Generic Object Pool

### Описание
Реализуйте обобщенный Object Pool с поддержкой любых типов объектов и RAII.

### Требования
1. ✅ Template-based реализация
2. ✅ Factory function для создания объектов
3. ✅ Reset function для очистки состояния
4. ✅ RAII wrapper для автоматического возврата
5. ✅ Метрики: available, in-use, created, destroyed

### Шаблон для реализации
```cpp
#include <iostream>
#include <queue>
#include <mutex>
#include <memory>
#include <functional>
#include <atomic>
#include <chrono>

template<typename T>
class ObjectPool {
private:
    std::queue<std::unique_ptr<T>> available_;
    std::mutex mutex_;
    size_t capacity_;
    
    std::function<std::unique_ptr<T>()> factory_;
    std::function<void(T*)> reset_func_;
    
    // Метрики
    std::atomic<size_t> total_created_{0};
    std::atomic<size_t> in_use_count_{0};
    std::atomic<size_t> acquire_count_{0};
    
public:
    // TODO: Реализуйте конструктор
    ObjectPool(size_t capacity,
              std::function<std::unique_ptr<T>()> factory,
              std::function<void(T*)> reset = nullptr)
        : capacity_(capacity), 
          factory_(std::move(factory)),
          reset_func_(std::move(reset)) {
        
        // TODO: Предварительное создание объектов
    }
    
    // TODO: Реализуйте acquire с unique_ptr custom deleter
    std::unique_ptr<T, std::function<void(T*)>> acquire() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // TODO: Получите объект из пула
        // TODO: Верните unique_ptr с custom deleter
        // Custom deleter должен возвращать объект в пул
    }
    
    // TODO: Реализуйте acquire с timeout
    std::unique_ptr<T, std::function<void(T*)>> 
    acquire(std::chrono::milliseconds timeout) {
        // Ваш код здесь
    }
    
    void printStats() const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        std::cout << "Object Pool Stats:" << std::endl;
        std::cout << "  Capacity: " << capacity_ << std::endl;
        std::cout << "  Available: " << available_.size() << std::endl;
        std::cout << "  In-use: " << in_use_count_.load() << std::endl;
        std::cout << "  Total created: " << total_created_.load() << std::endl;
        std::cout << "  Total acquires: " << acquire_count_.load() << std::endl;
    }
    
private:
    void release(T* obj) {
        // TODO: Сброс состояния
        // TODO: Возврат в пул
    }
};
```

### Тесты
```cpp
class DatabaseConnection {
private:
    std::string conn_string_;
    bool is_connected_;
    
public:
    DatabaseConnection(const std::string& conn_str)
        : conn_string_(conn_str), is_connected_(true) {
        std::cout << "DB Connection created" << std::endl;
    }
    
    ~DatabaseConnection() {
        std::cout << "DB Connection destroyed" << std::endl;
    }
    
    void query(const std::string& sql) {
        if (!is_connected_) {
            throw std::runtime_error("Not connected");
        }
        std::cout << "Executing: " << sql << std::endl;
    }
    
    void reset() {
        std::cout << "DB Connection reset" << std::endl;
        is_connected_ = true;
    }
};

void testObjectPool() {
    ObjectPool<DatabaseConnection> pool(
        5,  // capacity
        []() { return std::make_unique<DatabaseConnection>("localhost:5432"); },
        [](auto* conn) { conn->reset(); }
    );
    
    // Используем соединения
    {
        auto conn1 = pool.acquire();
        auto conn2 = pool.acquire();
        
        conn1->query("SELECT * FROM users");
        conn2->query("SELECT * FROM orders");
        
        // Автоматически возвращаются в пул при выходе из scope
    }
    
    pool.printStats();
    std::cout << "Object Pool test PASSED ✅" << std::endl;
}
```

---

## 📋 Задание 2: Dynamic Object Pool

### Описание
Реализуйте Object Pool с динамическим расширением и сокращением в зависимости от нагрузки.

### Требования
1. ✅ Начальный размер + максимальный размер
2. ✅ Автоматическое расширение при exhaustion
3. ✅ Автоматическое сокращение при низкой утилизации
4. ✅ Idle timeout для объектов
5. ✅ Метрики: расширения, сокращения, пиковый размер

### Шаблон
```cpp
template<typename T>
class DynamicObjectPool {
private:
    size_t min_size_;
    size_t max_size_;
    std::chrono::seconds idle_timeout_;
    
    std::queue<std::unique_ptr<T>> available_;
    std::unordered_map<T*, std::chrono::system_clock::time_point> last_used_;
    
public:
    // TODO: Реализуйте dynamic pool
    
    void scaleUp(size_t count) {
        // TODO: Добавить объекты
    }
    
    void scaleDown(size_t count) {
        // TODO: Удалить idle объекты
    }
    
private:
    void monitorLoop() {
        // TODO: Мониторинг утилизации
        // TODO: Автоматическое scale up/down
    }
};
```

---

## 📋 Задание 3: Object Pool с валидацией

### Описание
Реализуйте Object Pool с валидацией объектов перед использованием и after return.

### Требования
1. ✅ Validator function для проверки
2. ✅ Автоматическая пересоздание invalid objects
3. ✅ Health check при acquire
4. ✅ Quarantine для broken objects
5. ✅ Метрики: validation failures, recreations

### Шаблон
```cpp
template<typename T>
class ValidatingObjectPool {
private:
    std::function<bool(const T*)> validator_;
    std::queue<std::unique_ptr<T>> quarantine_;
    
public:
    void setValidator(std::function<bool(const T*)> validator) {
        validator_ = std::move(validator);
    }
    
    std::unique_ptr<T, std::function<void(T*)>> acquire() {
        // TODO: 1. Получить объект
        // TODO: 2. Валидировать
        // TODO: 3. Если invalid - пересоздать
        // TODO: 4. Вернуть валидный объект
    }
    
private:
    void release(T* obj) {
        // TODO: Валидировать при возврате
        // TODO: Если invalid - в quarantine
        // TODO: Если valid - обратно в пул
    }
};
```

---

## 📋 Задание 4: Connection Pool с keep-alive

### Описание
Реализуйте Connection Pool для БД/HTTP с поддержкой keep-alive и reconnect.

### Требования
1. ✅ Периодический ping для keep-alive
2. ✅ Автоматический reconnect при обрыве
3. ✅ Connection timeout и idle timeout
4. ✅ Max connection lifetime
5. ✅ Health monitoring

---

## 📋 Задание 5: Benchmarking Object Pool

### Описание
Создайте benchmark сравнивающий производительность с/без Object Pool.

### Требования
1. ✅ Бенчмарк времени создания объектов
2. ✅ Бенчмарк memory footprint
3. ✅ Сравнение latency
4. ✅ Анализ fragmentation
5. ✅ Рекомендации по использованию

### Тесты
```cpp
void benchmarkObjectPool() {
    const size_t NUM_ITERATIONS = 10000;
    
    // Без пула
    auto start = std::chrono::high_resolution_clock::now();
    
    for (size_t i = 0; i < NUM_ITERATIONS; ++i) {
        auto conn = std::make_unique<DatabaseConnection>("localhost");
        conn->query("SELECT 1");
        // Уничтожается
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto without_pool = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // С пулом
    ObjectPool<DatabaseConnection> pool(10, /* ... */);
    
    start = std::chrono::high_resolution_clock::now();
    
    for (size_t i = 0; i < NUM_ITERATIONS; ++i) {
        auto conn = pool.acquire();
        conn->query("SELECT 1");
        // Возвращается в пул
    }
    
    end = std::chrono::high_resolution_clock::now();
    auto with_pool = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Without pool: " << without_pool.count() << " ms" << std::endl;
    std::cout << "With pool: " << with_pool.count() << " ms" << std::endl;
    std::cout << "Speedup: " << (without_pool.count() / (double)with_pool.count()) 
              << "x" << std::endl;
}
```

---

**Автор**: Senior C++ Developer  
**Дата**: 2025-10-07  
**Сложность**: ⭐⭐⭐⭐ (Продвинутый уровень)
