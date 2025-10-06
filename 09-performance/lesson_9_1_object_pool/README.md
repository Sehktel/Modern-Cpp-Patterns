# Урок 9.1: Object Pool Pattern (Пул объектов)

## 🎯 Цель урока
Изучить паттерн Object Pool - один из ключевых паттернов для оптимизации производительности. Понять, как переиспользовать дорогие объекты вместо их постоянного создания и уничтожения.

## 📚 Что изучаем

### 1. Паттерн Object Pool
- **Определение**: Предварительно созданный пул объектов для переиспользования
- **Назначение**: Избежание накладных расходов на создание/уничтожение объектов
- **Применение**: Database connections, network sockets, game objects, GUI elements

### 2. Ключевые компоненты
- **Pool**: Контейнер для хранения объектов
- **Object Factory**: Фабрика для создания объектов
- **Borrowing**: Взятие объекта из пула
- **Returning**: Возврат объекта в пул

### 3. Преимущества
- **Производительность**: Нет накладных расходов на создание объектов
- **Контроль ресурсов**: Ограниченное количество объектов
- **Память**: Предсказуемое использование памяти
- **Стабильность**: Избежание фрагментации памяти

## 🔍 Ключевые концепции

### Базовая реализация Object Pool
```cpp
#include <queue>
#include <mutex>
#include <memory>
#include <functional>
#include <atomic>

template<typename T>
class ObjectPool {
private:
    std::queue<std::unique_ptr<T>> pool_;
    std::mutex mutex_;
    std::function<std::unique_ptr<T>()> factory_;
    std::atomic<size_t> maxSize_;
    std::atomic<size_t> currentSize_{0};
    
public:
    explicit ObjectPool(size_t maxSize = 100, 
                       std::function<std::unique_ptr<T>()> factory = []() { 
                           return std::make_unique<T>(); 
                       })
        : maxSize_(maxSize), factory_(factory) {
        
        // Предварительно создаем объекты
        for (size_t i = 0; i < maxSize / 2; ++i) {
            pool_.push(factory_());
            currentSize_.fetch_add(1);
        }
    }
    
    // Получение объекта из пула
    std::unique_ptr<T> acquire() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (!pool_.empty()) {
            auto obj = std::move(pool_.front());
            pool_.pop();
            return obj;
        }
        
        // Если пул пуст, создаем новый объект (если не превышен лимит)
        if (currentSize_.load() < maxSize_.load()) {
            currentSize_.fetch_add(1);
            return factory_();
        }
        
        return nullptr; // Пул переполнен
    }
    
    // Возврат объекта в пул
    void release(std::unique_ptr<T> obj) {
        if (!obj) return;
        
        // Сбрасываем состояние объекта
        obj->reset();
        
        std::lock_guard<std::mutex> lock(mutex_);
        pool_.push(std::move(obj));
    }
    
    // Статистика
    size_t available() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return pool_.size();
    }
    
    size_t total() const {
        return currentSize_.load();
    }
    
    size_t maxSize() const {
        return maxSize_.load();
    }
};

// RAII обертка для автоматического возврата объекта
template<typename T>
class PooledObject {
private:
    ObjectPool<T>& pool_;
    std::unique_ptr<T> object_;
    
public:
    PooledObject(ObjectPool<T>& pool) : pool_(pool), object_(pool.acquire()) {}
    
    ~PooledObject() {
        if (object_) {
            pool_.release(std::move(object_));
        }
    }
    
    T* get() const { return object_.get(); }
    T* operator->() const { return object_.get(); }
    T& operator*() const { return *object_; }
    
    explicit operator bool() const { return object_ != nullptr; }
};
```

## 🛠️ Практические примеры

### Пул соединений с базой данных
```cpp
class DatabaseConnection {
private:
    std::string connectionString_;
    bool isConnected_;
    
public:
    explicit DatabaseConnection(const std::string& connStr) 
        : connectionString_(connStr), isConnected_(false) {
        connect();
    }
    
    ~DatabaseConnection() {
        disconnect();
    }
    
    void reset() {
        if (isConnected_) {
            disconnect();
        }
        isConnected_ = false;
    }
    
    bool executeQuery(const std::string& query) {
        if (!isConnected_) {
            connect();
        }
        
        std::cout << "Выполняю запрос: " << query << std::endl;
        // Имитация выполнения запроса
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        return true;
    }
    
private:
    void connect() {
        std::cout << "Подключаюсь к базе данных: " << connectionString_ << std::endl;
        isConnected_ = true;
    }
    
    void disconnect() {
        std::cout << "Отключаюсь от базы данных" << std::endl;
        isConnected_ = false;
    }
};

class DatabaseConnectionPool {
private:
    ObjectPool<DatabaseConnection> pool_;
    
public:
    DatabaseConnectionPool(size_t maxConnections = 10) 
        : pool_(maxConnections, []() {
            return std::make_unique<DatabaseConnection>("localhost:5432");
        }) {}
    
    void executeQuery(const std::string& query) {
        PooledObject<DatabaseConnection> conn(pool_);
        
        if (conn) {
            conn->executeQuery(query);
        } else {
            throw std::runtime_error("Не удалось получить соединение из пула");
        }
        
        // Соединение автоматически возвращается в пул
    }
    
    void printStats() const {
        std::cout << "Пул соединений: доступно=" << pool_.available() 
                  << ", всего=" << pool_.total() 
                  << ", максимум=" << pool_.maxSize() << std::endl;
    }
};
```

### Пул игровых объектов
```cpp
class GameObject {
private:
    int id_;
    float x_, y_, z_;
    bool active_;
    
public:
    GameObject() : id_(-1), x_(0), y_(0), z_(0), active_(false) {}
    
    void reset() {
        id_ = -1;
        x_ = y_ = z_ = 0.0f;
        active_ = false;
    }
    
    void initialize(int id, float x, float y, float z) {
        id_ = id;
        x_ = x;
        y_ = y;
        z_ = z;
        active_ = true;
        
        std::cout << "Игровой объект " << id_ << " инициализирован в позиции (" 
                  << x_ << ", " << y_ << ", " << z_ << ")" << std::endl;
    }
    
    void update(float deltaTime) {
        if (!active_) return;
        
        // Имитация обновления логики
        x_ += deltaTime * 10.0f;
        y_ += deltaTime * 5.0f;
    }
    
    void render() {
        if (!active_) return;
        
        std::cout << "Отрисовка объекта " << id_ << " в позиции (" 
                  << x_ << ", " << y_ << ", " << z_ << ")" << std::endl;
    }
    
    bool isActive() const { return active_; }
    int getId() const { return id_; }
};

class GameObjectPool {
private:
    ObjectPool<GameObject> pool_;
    std::atomic<int> nextId_{1};
    
public:
    GameObjectPool(size_t maxObjects = 1000) 
        : pool_(maxObjects, []() { return std::make_unique<GameObject>(); }) {}
    
    PooledObject<GameObject> createObject(float x, float y, float z) {
        PooledObject<GameObject> obj(pool_);
        
        if (obj) {
            obj->initialize(nextId_.fetch_add(1), x, y, z);
        }
        
        return obj;
    }
    
    void updateAll(float deltaTime) {
        // В реальной игре здесь была бы итерация по активным объектам
        std::cout << "Обновление всех игровых объектов (deltaTime: " << deltaTime << ")" << std::endl;
    }
    
    void renderAll() {
        // В реальной игре здесь была бы отрисовка всех объектов
        std::cout << "Отрисовка всех игровых объектов" << std::endl;
    }
    
    void printStats() const {
        std::cout << "Пул игровых объектов: доступно=" << pool_.available() 
                  << ", всего=" << pool_.total() 
                  << ", максимум=" << pool_.maxSize() << std::endl;
    }
};
```

## 🎯 Практические упражнения

### Упражнение 1: Пул сетевых соединений
Создайте пул для управления TCP соединениями.

### Упражнение 2: Пул буферов
Реализуйте пул буферов для работы с данными.

### Упражнение 3: Пул потоков
Создайте пул потоков с переиспользованием.

### Упражнение 4: Пул с приоритетами
Реализуйте пул объектов с приоритетным распределением.

## 💡 Важные принципы

1. **Правильный размер пула**: Баланс между памятью и производительностью
2. **Сброс состояния**: Всегда сбрасывайте состояние объекта при возврате
3. **RAII**: Используйте RAII для автоматического возврата объектов
4. **Мониторинг**: Отслеживайте использование пула
5. **Thread Safety**: Обеспечивайте потокобезопасность для многопоточных приложений
