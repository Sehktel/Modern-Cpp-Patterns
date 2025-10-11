# Урок 4.4: Proxy Pattern (Заместитель)

## 🎯 Цель урока
Изучить паттерн Proxy - структурный паттерн для контроля доступа к объекту через объект-заместитель. Понять различные типы прокси и их применение для ленивой инициализации, контроля доступа, кэширования и удаленных вызовов.

## 📚 Что изучаем

### 1. Паттерн Proxy
- **Определение**: Предоставляет суррогат или заместитель другого объекта для контроля доступа к нему
- **Назначение**: Контроль доступа, ленивая инициализация, логирование, кэширование
- **Применение**: Remote proxy, Virtual proxy, Protection proxy, Smart reference
- **Принцип**: Прозрачный контроль доступа к объекту

### 2. Типы Proxy
- **Virtual Proxy**: Ленивая инициализация дорогих объектов
- **Remote Proxy**: Представитель объекта в другом адресном пространстве
- **Protection Proxy**: Контроль прав доступа
- **Smart Reference**: Дополнительные действия при доступе к объекту
- **Cache Proxy**: Кэширование результатов
- **Logging Proxy**: Логирование операций

### 3. Отличия от других паттернов
- **vs Decorator**: Proxy контролирует доступ, Decorator добавляет функциональность
- **vs Adapter**: Proxy сохраняет интерфейс, Adapter изменяет
- **vs Facade**: Proxy работает с одним объектом, Facade с подсистемой

## 🔍 Ключевые концепции

### Базовая структура Proxy
```cpp
// ============================================================================
// SUBJECT INTERFACE
// ============================================================================

// Общий интерфейс для реального объекта и прокси
class Image {
public:
    virtual ~Image() = default;
    virtual void display() = 0;
    virtual int getWidth() const = 0;
    virtual int getHeight() const = 0;
};

// ============================================================================
// REAL SUBJECT
// ============================================================================

// Реальный объект - дорогая операция загрузки
class RealImage : public Image {
private:
    std::string filename_;
    int width_;
    int height_;
    std::vector<char> imageData_;
    
    void loadFromDisk() {
        std::cout << "Загрузка изображения " << filename_ << " с диска..." << std::endl;
        
        // Симуляция долгой загрузки
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        width_ = 1920;
        height_ = 1080;
        imageData_.resize(width_ * height_ * 4);  // RGBA
        
        std::cout << "Изображение загружено: " << width_ << "x" << height_ << std::endl;
    }
    
public:
    explicit RealImage(const std::string& filename) : filename_(filename) {
        loadFromDisk();  // Загрузка сразу при создании
    }
    
    void display() override {
        std::cout << "Отображение изображения " << filename_ << std::endl;
    }
    
    int getWidth() const override { return width_; }
    int getHeight() const override { return height_; }
};

// ============================================================================
// PROXY
// ============================================================================

// Virtual Proxy - ленивая загрузка изображения
class ImageProxy : public Image {
private:
    std::string filename_;
    mutable std::unique_ptr<RealImage> realImage_;  // mutable для lazy loading в const методах
    
    // Ленивая инициализация
    void loadImage() const {
        if (!realImage_) {
            realImage_ = std::make_unique<RealImage>(filename_);
        }
    }
    
public:
    explicit ImageProxy(const std::string& filename) : filename_(filename) {
        std::cout << "ImageProxy создан для " << filename_ << " (изображение НЕ загружено)" << std::endl;
    }
    
    void display() override {
        loadImage();  // Загружаем только когда нужно
        realImage_->display();
    }
    
    int getWidth() const override {
        loadImage();
        return realImage_->getWidth();
    }
    
    int getHeight() const override {
        loadImage();
        return realImage_->getHeight();
    }
};

// ============================================================================
// ИСПОЛЬЗОВАНИЕ
// ============================================================================

void demonstrateVirtualProxy() {
    std::cout << "=== Без Proxy ===" << std::endl;
    auto start1 = std::chrono::high_resolution_clock::now();
    
    RealImage image1("photo1.jpg");  // Загрузка сразу (2 секунды)
    RealImage image2("photo2.jpg");  // Загрузка сразу (2 секунды)
    RealImage image3("photo3.jpg");  // Загрузка сразу (2 секунды)
    // Всего 6 секунд, даже если изображения не используются!
    
    auto end1 = std::chrono::high_resolution_clock::now();
    auto duration1 = std::chrono::duration_cast<std::chrono::seconds>(end1 - start1);
    std::cout << "Время: " << duration1.count() << "s\n" << std::endl;
    
    std::cout << "=== С Proxy ===" << std::endl;
    auto start2 = std::chrono::high_resolution_clock::now();
    
    ImageProxy proxy1("photo1.jpg");  // Мгновенно
    ImageProxy proxy2("photo2.jpg");  // Мгновенно
    ImageProxy proxy3("photo3.jpg");  // Мгновенно
    
    auto end2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start2);
    std::cout << "Время создания прокси: " << duration2.count() << "ms\n" << std::endl;
    
    std::cout << "Отображаем только первое изображение:" << std::endl;
    proxy1.display();  // Только здесь происходит загрузка (2 секунды)
    
    // proxy2 и proxy3 остаются незагруженными - экономия памяти и времени!
}
```

**Теоретическая база**: Virtual Proxy реализует паттерн "lazy evaluation" из функционального программирования. Вычисление откладывается до момента первого использования: eval(λx.e) выполняется только при первом обращении к x. Сложность: O(1) для создания прокси, O(n) только при первом реальном обращении.

### Protection Proxy - контроль доступа
```cpp
// Интерфейс файловой системы
class FileSystem {
public:
    virtual ~FileSystem() = default;
    virtual std::string readFile(const std::string& filename) = 0;
    virtual void writeFile(const std::string& filename, const std::string& content) = 0;
    virtual void deleteFile(const std::string& filename) = 0;
};

// Реальная файловая система
class RealFileSystem : public FileSystem {
public:
    std::string readFile(const std::string& filename) override {
        std::cout << "Чтение файла: " << filename << std::endl;
        return "file content";
    }
    
    void writeFile(const std::string& filename, const std::string& content) override {
        std::cout << "Запись в файл: " << filename << std::endl;
    }
    
    void deleteFile(const std::string& filename) override {
        std::cout << "Удаление файла: " << filename << std::endl;
    }
};

// Protection Proxy с проверкой прав доступа
class ProtectedFileSystem : public FileSystem {
private:
    std::unique_ptr<FileSystem> realFS_;
    std::string currentUser_;
    std::map<std::string, std::set<std::string>> permissions_;  // file -> users with access
    
    bool hasPermission(const std::string& filename) const {
        auto it = permissions_.find(filename);
        if (it == permissions_.end()) {
            return false;  // Файл не найден
        }
        return it->second.count(currentUser_) > 0;
    }
    
public:
    explicit ProtectedFileSystem(const std::string& user)
        : realFS_(std::make_unique<RealFileSystem>()),
          currentUser_(user) {
        // Инициализация прав доступа
        permissions_["public.txt"] = {"user1", "user2", "admin"};
        permissions_["secret.txt"] = {"admin"};
        permissions_["config.txt"] = {"admin"};
    }
    
    std::string readFile(const std::string& filename) override {
        if (!hasPermission(filename)) {
            throw std::runtime_error("Access denied: " + currentUser_ + 
                                     " cannot read " + filename);
        }
        return realFS_->readFile(filename);
    }
    
    void writeFile(const std::string& filename, const std::string& content) override {
        if (!hasPermission(filename)) {
            throw std::runtime_error("Access denied: " + currentUser_ + 
                                     " cannot write " + filename);
        }
        realFS_->writeFile(filename, content);
    }
    
    void deleteFile(const std::string& filename) override {
        if (currentUser_ != "admin") {
            throw std::runtime_error("Access denied: only admin can delete files");
        }
        realFS_->deleteFile(filename);
    }
};

// Использование
void demonstrateProtectionProxy() {
    std::cout << "\n=== Protection Proxy ===" << std::endl;
    
    // Обычный пользователь
    ProtectedFileSystem userFS("user1");
    
    try {
        userFS.readFile("public.txt");   // OK
        userFS.readFile("secret.txt");   // Ошибка доступа
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
    }
    
    // Администратор
    ProtectedFileSystem adminFS("admin");
    
    adminFS.readFile("secret.txt");   // OK
    adminFS.deleteFile("config.txt"); // OK
}
```

### Cache Proxy - кэширование результатов
```cpp
// Интерфейс для работы с БД
class Database {
public:
    virtual ~Database() = default;
    virtual std::string query(const std::string& sql) = 0;
};

// Реальная база данных
class RealDatabase : public Database {
public:
    std::string query(const std::string& sql) override {
        std::cout << "Выполнение SQL запроса: " << sql << std::endl;
        
        // Симуляция долгого запроса
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        return "query_result_for: " + sql;
    }
};

// Cache Proxy
class CachedDatabase : public Database {
private:
    std::unique_ptr<Database> realDB_;
    mutable std::map<std::string, std::string> cache_;
    mutable int cacheHits_;
    mutable int cacheMisses_;
    
public:
    CachedDatabase() 
        : realDB_(std::make_unique<RealDatabase>()),
          cacheHits_(0),
          cacheMisses_(0) {}
    
    std::string query(const std::string& sql) override {
        // Проверяем кэш
        auto it = cache_.find(sql);
        
        if (it != cache_.end()) {
            std::cout << "Кэш: HIT для запроса: " << sql << std::endl;
            cacheHits_++;
            return it->second;
        }
        
        std::cout << "Кэш: MISS для запроса: " << sql << std::endl;
        cacheMisses_++;
        
        // Выполняем реальный запрос
        std::string result = realDB_->query(sql);
        
        // Сохраняем в кэш
        cache_[sql] = result;
        
        return result;
    }
    
    void printStatistics() const {
        std::cout << "\nСтатистика кэша:" << std::endl;
        std::cout << "  Hits: " << cacheHits_ << std::endl;
        std::cout << "  Misses: " << cacheMisses_ << std::endl;
        std::cout << "  Hit rate: " << (100.0 * cacheHits_ / (cacheHits_ + cacheMisses_)) << "%" << std::endl;
    }
    
    void clearCache() {
        cache_.clear();
        std::cout << "Кэш очищен" << std::endl;
    }
};

// Использование
void demonstrateCacheProxy() {
    std::cout << "\n=== Cache Proxy ===" << std::endl;
    
    CachedDatabase db;
    
    // Первый запрос - долгий (cache miss)
    auto start = std::chrono::high_resolution_clock::now();
    db.query("SELECT * FROM users WHERE age > 18");
    auto end = std::chrono::high_resolution_clock::now();
    auto duration1 = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Время: " << duration1.count() << "ms\n" << std::endl;
    
    // Второй запрос - мгновенный (cache hit)
    start = std::chrono::high_resolution_clock::now();
    db.query("SELECT * FROM users WHERE age > 18");  // Тот же запрос
    end = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Время: " << duration2.count() << "ms\n" << std::endl;
    
    // Новый запрос - долгий
    db.query("SELECT * FROM products");
    
    // Повтор - быстрый
    db.query("SELECT * FROM products");
    
    db.printStatistics();
}
```

## 🤔 Вопросы для размышления

### 1. Когда использовать Proxy?
**✅ Хорошие случаи:**

**Virtual Proxy:**
```cpp
// Ленивая загрузка дорогих объектов
class HeavyDocument {
    // Большой объем данных
    std::vector<char> data_;  // Гигабайты
    
public:
    HeavyDocument() {
        // Долгая загрузка
    }
};

// Прокси создается быстро, реальный объект - только при первом использовании
class DocumentProxy {
    mutable std::unique_ptr<HeavyDocument> doc_;
    
    void ensureLoaded() const {
        if (!doc_) {
            doc_ = std::make_unique<HeavyDocument>();
        }
    }
};
```

**Protection Proxy:**
```cpp
// Контроль доступа к ресурсам
class SecureResource {
    // Проверка прав перед каждой операцией
    bool checkPermission(const User& user, const std::string& operation);
};
```

**Remote Proxy:**
```cpp
// Представитель удаленного объекта
class RemoteService {
    // Скрывает детали сетевого взаимодействия
    std::string sendRequest(const std::string& data);
};
```

**❌ Плохие случаи:**
- Объект дешевый в создании
- Не нужен контроль доступа
- Прокси добавляет ненужную сложность

### 2. Преимущества Proxy
```cpp
// ✅ Ленивая инициализация
// Экономия памяти и времени
ImageProxy proxy("huge_image.jpg");  // Мгновенно
// Реальная загрузка только когда нужно

// ✅ Контроль доступа
// Централизованная проверка прав
ProtectedResource resource(user);
resource.operation();  // Автоматическая проверка

// ✅ Кэширование
// Автоматическая оптимизация
CachedService service;
auto result = service.expensiveOperation();  // Кэшируется

// ✅ Логирование
// Прозрачное логирование операций
LoggingProxy proxy(realObject);
proxy.method();  // Автоматически логируется

// ✅ Отделение concerns
// Бизнес-логика отделена от:
// - Проверки прав
// - Кэширования
// - Логирования
// - Сетевого взаимодействия
```

### 3. Недостатки Proxy
```cpp
// ❌ Дополнительная сложность
// Еще один уровень косвенности

// ❌ Overhead
// Виртуальный вызов на каждую операцию
// Проверки в прокси

// ❌ Может быть неочевидным
// Клиент не знает работает с прокси или реальным объектом
```

## 🛠️ Практические примеры

### Пример 1: Smart Reference Proxy
```cpp
// Smart Reference - подсчет ссылок и автоудаление
template<typename T>
class SmartReferenceProxy {
private:
    struct ControlBlock {
        T* ptr;
        int refCount;
        
        ControlBlock(T* p) : ptr(p), refCount(1) {}
        
        ~ControlBlock() {
            delete ptr;
        }
    };
    
    ControlBlock* control_;
    
public:
    explicit SmartReferenceProxy(T* ptr) : control_(new ControlBlock(ptr)) {
        std::cout << "SmartReference создан, refCount = 1" << std::endl;
    }
    
    // Copy constructor
    SmartReferenceProxy(const SmartReferenceProxy& other) : control_(other.control_) {
        control_->refCount++;
        std::cout << "SmartReference скопирован, refCount = " << control_->refCount << std::endl;
    }
    
    // Destructor
    ~SmartReferenceProxy() {
        control_->refCount--;
        std::cout << "SmartReference удален, refCount = " << control_->refCount << std::endl;
        
        if (control_->refCount == 0) {
            std::cout << "Удаление реального объекта" << std::endl;
            delete control_;
        }
    }
    
    T* operator->() {
        return control_->ptr;
    }
    
    T& operator*() {
        return *control_->ptr;
    }
    
    int getRefCount() const {
        return control_->refCount;
    }
};

// Использование
void demonstrateSmartReference() {
    std::cout << "\n=== Smart Reference Proxy ===" << std::endl;
    
    {
        SmartReferenceProxy<std::string> ref1(new std::string("Hello"));
        std::cout << "Значение: " << *ref1 << std::endl;
        
        {
            SmartReferenceProxy<std::string> ref2 = ref1;  // Copy
            std::cout << "RefCount: " << ref2.getRefCount() << std::endl;
            
            *ref2 = "World";
            std::cout << "Значение через ref2: " << *ref2 << std::endl;
            std::cout << "Значение через ref1: " << *ref1 << std::endl;
        }  // ref2 удаляется, refCount = 1
        
        std::cout << "RefCount после удаления ref2: " << ref1.getRefCount() << std::endl;
    }  // ref1 удаляется, refCount = 0, объект удаляется
}
```

### Пример 2: Logging Proxy
```cpp
// Интерфейс сервиса
class Service {
public:
    virtual ~Service() = default;
    virtual void operation1(int param) = 0;
    virtual std::string operation2() = 0;
    virtual void operation3(const std::string& data) = 0;
};

// Реальный сервис
class RealService : public Service {
public:
    void operation1(int param) override {
        std::cout << "RealService::operation1(" << param << ")" << std::endl;
    }
    
    std::string operation2() override {
        std::cout << "RealService::operation2()" << std::endl;
        return "result";
    }
    
    void operation3(const std::string& data) override {
        std::cout << "RealService::operation3(\"" << data << "\")" << std::endl;
    }
};

// Logging Proxy
class LoggingServiceProxy : public Service {
private:
    std::unique_ptr<Service> realService_;
    mutable std::ofstream logFile_;
    
    void log(const std::string& message) const {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        
        std::string timestamp = std::ctime(&time);
        timestamp.pop_back();  // Убираем \n
        
        std::cout << "[LOG] " << timestamp << ": " << message << std::endl;
        
        if (logFile_.is_open()) {
            logFile_ << timestamp << ": " << message << std::endl;
            logFile_.flush();
        }
    }
    
public:
    explicit LoggingServiceProxy(std::unique_ptr<Service> service, 
                                  const std::string& logFilename = "service.log")
        : realService_(std::move(service)) {
        logFile_.open(logFilename, std::ios::app);
    }
    
    ~LoggingServiceProxy() {
        logFile_.close();
    }
    
    void operation1(int param) override {
        log("operation1 вызван с параметром: " + std::to_string(param));
        
        auto start = std::chrono::high_resolution_clock::now();
        realService_->operation1(param);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        log("operation1 завершен за " + std::to_string(duration.count()) + "ms");
    }
    
    std::string operation2() override {
        log("operation2 вызван");
        
        auto result = realService_->operation2();
        
        log("operation2 вернул: " + result);
        return result;
    }
    
    void operation3(const std::string& data) override {
        log("operation3 вызван с данными: " + data);
        realService_->operation3(data);
        log("operation3 завершен");
    }
};

// Использование
void demonstrateLoggingProxy() {
    std::cout << "\n=== Logging Proxy ===" << std::endl;
    
    auto service = std::make_unique<LoggingServiceProxy>(
        std::make_unique<RealService>()
    );
    
    service->operation1(42);
    auto result = service->operation2();
    service->operation3("test data");
}
```

### Пример 3: Remote Proxy (RPC)
```cpp
// Интерфейс удаленного сервиса
class RemoteCalculator {
public:
    virtual ~RemoteCalculator() = default;
    virtual int add(int a, int b) = 0;
    virtual int multiply(int a, int b) = 0;
    virtual double divide(int a, int b) = 0;
};

// Реальный сервис (на сервере)
class RealCalculator : public RemoteCalculator {
public:
    int add(int a, int b) override {
        std::cout << "Server: вычисление " << a << " + " << b << std::endl;
        return a + b;
    }
    
    int multiply(int a, int b) override {
        std::cout << "Server: вычисление " << a << " * " << b << std::endl;
        return a * b;
    }
    
    double divide(int a, int b) override {
        std::cout << "Server: вычисление " << a << " / " << b << std::endl;
        if (b == 0) {
            throw std::runtime_error("Division by zero");
        }
        return static_cast<double>(a) / b;
    }
};

// Remote Proxy (на клиенте)
class RemoteCalculatorProxy : public RemoteCalculator {
private:
    std::string serverAddress_;
    int serverPort_;
    
    // Симуляция сетевого вызова
    template<typename Result, typename... Args>
    Result remoteCall(const std::string& method, Args... args) {
        std::cout << "Proxy: отправка запроса на " << serverAddress_ 
                  << ":" << serverPort_ << std::endl;
        std::cout << "Proxy: вызов метода " << method << std::endl;
        
        // Симуляция сетевой задержки
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // В реальности: сериализация, отправка по сети, получение ответа
        // Здесь упрощенная версия - вызываем локальный объект
        static RealCalculator server;
        
        // В реальном RPC здесь был бы код десериализации и вызова
        if constexpr (std::is_same_v<Result, int>) {
            // Упрощенная диспетчеризация
            if (method == "add") {
                return server.add(args...);
            } else if (method == "multiply") {
                return server.multiply(args...);
            }
        } else if constexpr (std::is_same_v<Result, double>) {
            if (method == "divide") {
                return server.divide(args...);
            }
        }
        
        throw std::runtime_error("Unknown method");
    }
    
public:
    RemoteCalculatorProxy(const std::string& address, int port)
        : serverAddress_(address), serverPort_(port) {
        std::cout << "Proxy: подключение к серверу " << address << ":" << port << std::endl;
    }
    
    int add(int a, int b) override {
        return remoteCall<int>("add", a, b);
    }
    
    int multiply(int a, int b) override {
        return remoteCall<int>("multiply", a, b);
    }
    
    double divide(int a, int b) override {
        return remoteCall<double>("divide", a, b);
    }
};

// Использование
void demonstrateRemoteProxy() {
    std::cout << "\n=== Remote Proxy ===" << std::endl;
    
    RemoteCalculatorProxy calculator("192.168.1.100", 8080);
    
    // Клиент работает с прокси как с локальным объектом
    // Но все вызовы идут по сети
    int sum = calculator.add(5, 3);
    std::cout << "Результат: " << sum << "\n" << std::endl;
    
    int product = calculator.multiply(4, 7);
    std::cout << "Результат: " << product << "\n" << std::endl;
    
    double quotient = calculator.divide(10, 2);
    std::cout << "Результат: " << quotient << std::endl;
}
```

## 🎨 Современные подходы

### Thread-Safe Proxy
```cpp
// Потокобезопасный прокси
template<typename T>
class ThreadSafeProxy {
private:
    std::unique_ptr<T> realObject_;
    mutable std::mutex mutex_;
    
public:
    explicit ThreadSafeProxy(std::unique_ptr<T> obj)
        : realObject_(std::move(obj)) {}
    
    template<typename Func>
    auto execute(Func&& func) {
        std::lock_guard<std::mutex> lock(mutex_);
        return func(*realObject_);
    }
};

// Использование
ThreadSafeProxy<MyClass> proxy(std::make_unique<MyClass>());

// Потокобезопасный доступ
auto result = proxy.execute([](MyClass& obj) {
    return obj.method();
});
```

### Generic Proxy с шаблонами
```cpp
// Универсальный прокси
template<typename T, typename Policy>
class GenericProxy : public T {
private:
    std::unique_ptr<T> realObject_;
    Policy policy_;
    
public:
    template<typename... Args>
    explicit GenericProxy(Args&&... args)
        : realObject_(std::make_unique<T>(std::forward<Args>(args)...)) {}
    
    template<typename Func, typename... Args>
    auto callWithPolicy(Func func, Args&&... args) {
        policy_.before();
        auto result = (realObject_.get()->*func)(std::forward<Args>(args)...);
        policy_.after();
        return result;
    }
};

// Политика логирования
struct LoggingPolicy {
    void before() { std::cout << "Вызов метода" << std::endl; }
    void after() { std::cout << "Метод завершен" << std::endl; }
};
```

## 🚀 Современный C++

### std::shared_ptr как Smart Proxy
```cpp
// std::shared_ptr - это по сути Smart Reference Proxy
std::shared_ptr<MyClass> ptr1 = std::make_shared<MyClass>();
std::shared_ptr<MyClass> ptr2 = ptr1;  // Подсчет ссылок

std::cout << "RefCount: " << ptr1.use_count() << std::endl;
```

### std::optional как Null Proxy
```cpp
#include <optional>

// optional можно рассматривать как прокси для nullable объектов
std::optional<MyClass> maybeObject;

if (maybeObject) {
    maybeObject->method();  // Безопасный доступ
}
```

## 🎯 Практические упражнения

### Упражнение 1: Connection Pool Proxy
Создайте прокси для управления пулом соединений к БД.

### Упражнение 2: Rate Limiting Proxy
Реализуйте прокси с ограничением частоты вызовов.

### Упражнение 3: Transactional Proxy
Создайте прокси для автоматического управления транзакциями.

### Упражнение 4: Retry Proxy
Реализуйте прокси с автоматическим повтором при ошибках.

## 📈 Связь с другими паттернами

### Proxy + Decorator
```cpp
// Можно комбинировать прокси и декораторы
auto service = std::make_unique<RealService>();
service = std::make_unique<CacheProxy>(std::move(service));
service = std::make_unique<LoggingDecorator>(std::move(service));
service = std::make_unique<ProtectionProxy>(std::move(service));
```

### Proxy + Singleton
```cpp
// Прокси к единственному экземпляру
class SingletonProxy {
public:
    static RealObject& getInstance() {
        static RealObject instance;
        return instance;
    }
};
```

## 📈 Следующие шаги
После изучения Proxy вы будете готовы к:
- Модулю 5: Поведенческие паттерны
- Пониманию RPC и распределенных систем
- Проектированию защищенных систем
- Оптимизации производительности через кэширование

## ⚠️ Важные предупреждения

1. **Прозрачность**: Прокси должен быть прозрачен для клиента
2. **Производительность**: Учитывайте overhead от проксирования
3. **Thread-safety**: Прокси должен быть потокобезопасным если нужно
4. **Документируйте**: Укажите когда используется прокси

---

*"The Proxy pattern provides a surrogate or placeholder for another object to control access to it."* - Gang of Four

