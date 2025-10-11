# Урок 4.2: Decorator Pattern (Декоратор)

## 🎯 Цель урока
Изучить паттерн Decorator - структурный паттерн для динамического добавления функциональности объектам. Понять, как расширять поведение объектов без изменения их кода и без использования наследования.

## 📚 Что изучаем

### 1. Паттерн Decorator
- **Определение**: Динамически добавляет объекту новые обязанности
- **Назначение**: Гибкая альтернатива наследованию для расширения функциональности
- **Применение**: I/O streams, GUI компоненты, middleware, logging, caching
- **Alias**: Wrapper (обертка)

### 2. Структура паттерна
- **Component**: Базовый интерфейс для объектов
- **ConcreteComponent**: Конкретный объект
- **Decorator**: Абстрактный декоратор
- **ConcreteDecorator**: Конкретный декоратор с дополнительной функциональностью

### 3. Отличия от других паттернов
- **vs Adapter**: Decorator сохраняет интерфейс, Adapter изменяет
- **vs Proxy**: Decorator добавляет функциональность, Proxy контролирует доступ
- **vs Composite**: Decorator не группирует объекты

## 🔍 Ключевые концепции

### Проблема: Комбинаторный взрыв при наследовании
```cpp
// ❌ ПРОБЛЕМА: Слишком много классов при комбинациях
class Coffee { };
class CoffeeWithMilk : public Coffee { };
class CoffeeWithSugar : public Coffee { };
class CoffeeWithMilkAndSugar : public Coffee { };
class CoffeeWithMilkAndSugarAndChocolate : public Coffee { };
// И так далее... 2^N классов для N добавок!

// Невозможно добавить новую добавку без создания множества новых классов
```

**Теоретическая база**: Проблема комбинаторного взрыва: для N независимых характеристик через наследование требуется 2^N классов. Decorator решает это через композицию: O(N) классов-декораторов для любых комбинаций.

### Решение: Decorator Pattern
```cpp
// ============================================================================
// БАЗОВЫЙ КОМПОНЕНТ
// ============================================================================

// Абстрактный интерфейс напитка
class Coffee {
public:
    virtual ~Coffee() = default;
    virtual std::string getDescription() const = 0;
    virtual double getCost() const = 0;
};

// ============================================================================
// КОНКРЕТНЫЙ КОМПОНЕНТ
// ============================================================================

// Простой кофе
class SimpleCoffee : public Coffee {
public:
    std::string getDescription() const override {
        return "Простой кофе";
    }
    
    double getCost() const override {
        return 2.0;
    }
};

// ============================================================================
// БАЗОВЫЙ ДЕКОРАТОР
// ============================================================================

// Абстрактный декоратор
class CoffeeDecorator : public Coffee {
protected:
    std::unique_ptr<Coffee> coffee_;
    
public:
    explicit CoffeeDecorator(std::unique_ptr<Coffee> coffee)
        : coffee_(std::move(coffee)) {}
    
    std::string getDescription() const override {
        return coffee_->getDescription();
    }
    
    double getCost() const override {
        return coffee_->getCost();
    }
};

// ============================================================================
// КОНКРЕТНЫЕ ДЕКОРАТОРЫ
// ============================================================================

// Декоратор: молоко
class MilkDecorator : public CoffeeDecorator {
public:
    explicit MilkDecorator(std::unique_ptr<Coffee> coffee)
        : CoffeeDecorator(std::move(coffee)) {}
    
    std::string getDescription() const override {
        return coffee_->getDescription() + " + молоко";
    }
    
    double getCost() const override {
        return coffee_->getCost() + 0.5;
    }
};

// Декоратор: сахар
class SugarDecorator : public CoffeeDecorator {
public:
    explicit SugarDecorator(std::unique_ptr<Coffee> coffee)
        : CoffeeDecorator(std::move(coffee)) {}
    
    std::string getDescription() const override {
        return coffee_->getDescription() + " + сахар";
    }
    
    double getCost() const override {
        return coffee_->getCost() + 0.2;
    }
};

// Декоратор: шоколад
class ChocolateDecorator : public CoffeeDecorator {
public:
    explicit ChocolateDecorator(std::unique_ptr<Coffee> coffee)
        : CoffeeDecorator(std::move(coffee)) {}
    
    std::string getDescription() const override {
        return coffee_->getDescription() + " + шоколад";
    }
    
    double getCost() const override {
        return coffee_->getCost() + 0.7;
    }
};

// ============================================================================
// ИСПОЛЬЗОВАНИЕ
// ============================================================================

void orderCoffee() {
    // Простой кофе
    std::unique_ptr<Coffee> coffee = std::make_unique<SimpleCoffee>();
    std::cout << coffee->getDescription() << " - $" << coffee->getCost() << std::endl;
    // Вывод: Простой кофе - $2
    
    // Кофе с молоком
    coffee = std::make_unique<MilkDecorator>(std::move(coffee));
    std::cout << coffee->getDescription() << " - $" << coffee->getCost() << std::endl;
    // Вывод: Простой кофе + молоко - $2.5
    
    // Кофе с молоком и сахаром
    coffee = std::make_unique<SugarDecorator>(std::move(coffee));
    std::cout << coffee->getDescription() << " - $" << coffee->getCost() << std::endl;
    // Вывод: Простой кофе + молоко + сахар - $2.7
    
    // Кофе с молоком, сахаром и шоколадом
    coffee = std::make_unique<ChocolateDecorator>(std::move(coffee));
    std::cout << coffee->getDescription() << " - $" << coffee->getCost() << std::endl;
    // Вывод: Простой кофе + молоко + сахар + шоколад - $3.4
}

// Или строим всё сразу
void orderComplexCoffee() {
    auto coffee = std::make_unique<ChocolateDecorator>(
                     std::make_unique<SugarDecorator>(
                        std::make_unique<MilkDecorator>(
                           std::make_unique<SimpleCoffee>()
                        )
                     )
                  );
    
    std::cout << coffee->getDescription() << " - $" << coffee->getCost() << std::endl;
}
```

**Теоретическая база**: Decorator реализует принцип Open/Closed (OCP) из SOLID: класс открыт для расширения (через декораторы), но закрыт для модификации. Каждый декоратор - это монада из функционального программирования, оборачивающая значение и добавляющая поведение.

### Современный подход с умными указателями
```cpp
// Builder-style Decorator для более читаемого кода
class CoffeeBuilder {
private:
    std::unique_ptr<Coffee> coffee_;
    
public:
    CoffeeBuilder() : coffee_(std::make_unique<SimpleCoffee>()) {}
    
    CoffeeBuilder& withMilk() {
        coffee_ = std::make_unique<MilkDecorator>(std::move(coffee_));
        return *this;
    }
    
    CoffeeBuilder& withSugar() {
        coffee_ = std::make_unique<SugarDecorator>(std::move(coffee_));
        return *this;
    }
    
    CoffeeBuilder& withChocolate() {
        coffee_ = std::make_unique<ChocolateDecorator>(std::move(coffee_));
        return *this;
    }
    
    std::unique_ptr<Coffee> build() {
        return std::move(coffee_);
    }
};

// Использование
void orderWithBuilder() {
    auto coffee = CoffeeBuilder()
                     .withMilk()
                     .withSugar()
                     .withChocolate()
                     .build();
    
    std::cout << coffee->getDescription() << " - $" << coffee->getCost() << std::endl;
}
```

## 🤔 Вопросы для размышления

### 1. Когда использовать Decorator?
**✅ Хорошие случаи:**
- Динамическое добавление функциональности
- Много комбинаций характеристик
- Расширение функциональности без изменения класса
- Возможность убрать обязанность (unwrap)
- Наследование невозможно или нежелательно

**Примеры применения:**
```cpp
// 1. I/O Streams
std::ifstream file("data.txt");
BufferedInputStream buffered(file);
CompressedInputStream compressed(buffered);
EncryptedInputStream encrypted(compressed);

// 2. GUI компоненты
auto button = std::make_unique<Button>();
button = std::make_unique<BorderDecorator>(std::move(button));
button = std::make_unique<ScrollDecorator>(std::move(button));

// 3. Logging
auto logger = std::make_unique<FileLogger>();
logger = std::make_unique<TimestampDecorator>(std::move(logger));
logger = std::make_unique<LevelFilterDecorator>(std::move(logger));

// 4. Caching
auto service = std::make_unique<DatabaseService>();
service = std::make_unique<CachingDecorator>(std::move(service));
service = std::make_unique<RetryDecorator>(std::move(service));
```

**❌ Плохие случаи:**
- Фиксированный набор характеристик (используйте наследование)
- Порядок декораторов критичен (может быть запутанным)
- Нужен доступ к конкретному типу декоратора

### 2. Преимущества Decorator
```cpp
// ✅ Гибкость
// Можем комбинировать декораторы в любом порядке
auto coffee1 = CoffeeBuilder().withMilk().withSugar().build();
auto coffee2 = CoffeeBuilder().withSugar().withMilk().build();  // Другой порядок

// ✅ Open/Closed Principle
// Добавляем новый декоратор без изменения существующего кода
class VanillaDecorator : public CoffeeDecorator {
    // Новая функциональность без изменения других классов
};

// ✅ Single Responsibility
// Каждый декоратор отвечает за одну характеристику
class TimestampDecorator : public Logger {
    // Только добавление timestamp
};

// ✅ Динамичность
// Можем добавлять/убирать декораторы в runtime
if (config.needsEncryption) {
    stream = std::make_unique<EncryptedDecorator>(std::move(stream));
}
```

### 3. Недостатки Decorator
```cpp
// ❌ Много мелких классов
// Каждый декоратор - отдельный класс

// ❌ Сложность отладки
// Длинная цепочка декораторов сложна для понимания
auto complex = D1(D2(D3(D4(D5(Base())))));  // Что здесь происходит?

// ❌ Проблемы с идентичностью
// Декорированный объект не равен оригиналу
SimpleCoffee* original = new SimpleCoffee();
Coffee* decorated = new MilkDecorator(original);
// decorated != original (разные типы)

// ❌ Порядок декораторов может быть важен
auto stream1 = Compressed(Encrypted(file));  // Сжимаем зашифрованное
auto stream2 = Encrypted(Compressed(file));  // Шифруем сжатое
// Результат разный!
```

## 🛠️ Практические примеры

### Пример 1: Stream Decorators (как в Java InputStream)
```cpp
// Базовый интерфейс потока
class InputStream {
public:
    virtual ~InputStream() = default;
    virtual int read() = 0;
    virtual std::vector<char> readAll() = 0;
    virtual void close() = 0;
};

// Конкретный компонент: файловый поток
class FileInputStream : public InputStream {
private:
    std::ifstream file_;
    
public:
    explicit FileInputStream(const std::string& filename) {
        file_.open(filename, std::ios::binary);
        if (!file_) {
            throw std::runtime_error("Cannot open file: " + filename);
        }
    }
    
    int read() override {
        return file_.get();
    }
    
    std::vector<char> readAll() override {
        return std::vector<char>(
            std::istreambuf_iterator<char>(file_),
            std::istreambuf_iterator<char>()
        );
    }
    
    void close() override {
        file_.close();
    }
};

// Абстрактный декоратор
class InputStreamDecorator : public InputStream {
protected:
    std::unique_ptr<InputStream> stream_;
    
public:
    explicit InputStreamDecorator(std::unique_ptr<InputStream> stream)
        : stream_(std::move(stream)) {}
    
    int read() override {
        return stream_->read();
    }
    
    std::vector<char> readAll() override {
        return stream_->readAll();
    }
    
    void close() override {
        stream_->close();
    }
};

// Декоратор: буферизация
class BufferedInputStream : public InputStreamDecorator {
private:
    static const size_t BUFFER_SIZE = 4096;
    std::vector<char> buffer_;
    size_t bufferPos_;
    size_t bufferSize_;
    
public:
    explicit BufferedInputStream(std::unique_ptr<InputStream> stream)
        : InputStreamDecorator(std::move(stream)),
          buffer_(BUFFER_SIZE),
          bufferPos_(0),
          bufferSize_(0) {}
    
    int read() override {
        if (bufferPos_ >= bufferSize_) {
            fillBuffer();
            if (bufferSize_ == 0) {
                return -1;  // EOF
            }
        }
        return buffer_[bufferPos_++];
    }
    
private:
    void fillBuffer() {
        bufferPos_ = 0;
        bufferSize_ = 0;
        
        for (size_t i = 0; i < BUFFER_SIZE; ++i) {
            int byte = stream_->read();
            if (byte == -1) break;
            buffer_[i] = static_cast<char>(byte);
            bufferSize_++;
        }
        
        std::cout << "Буфер заполнен: " << bufferSize_ << " байт" << std::endl;
    }
};

// Декоратор: сжатие (упрощенная версия)
class CompressedInputStream : public InputStreamDecorator {
public:
    explicit CompressedInputStream(std::unique_ptr<InputStream> stream)
        : InputStreamDecorator(std::move(stream)) {}
    
    std::vector<char> readAll() override {
        auto compressed = stream_->readAll();
        std::cout << "Распаковка " << compressed.size() << " байт..." << std::endl;
        
        // Реальная распаковка (например, zlib)
        // Здесь упрощенная версия
        return compressed;
    }
};

// Декоратор: шифрование
class EncryptedInputStream : public InputStreamDecorator {
private:
    int key_;
    
public:
    EncryptedInputStream(std::unique_ptr<InputStream> stream, int key)
        : InputStreamDecorator(std::move(stream)), key_(key) {}
    
    int read() override {
        int byte = stream_->read();
        if (byte == -1) return -1;
        
        // Простое XOR шифрование (в реальности используйте AES)
        return byte ^ key_;
    }
};

// Использование
void readFile() {
    // Создаем цепочку декораторов
    auto stream = std::make_unique<FileInputStream>("data.txt");
    stream = std::make_unique<BufferedInputStream>(std::move(stream));
    stream = std::make_unique<CompressedInputStream>(std::move(stream));
    stream = std::make_unique<EncryptedInputStream>(std::move(stream), 42);
    
    // Читаем данные через все декораторы
    int byte;
    while ((byte = stream->read()) != -1) {
        std::cout << static_cast<char>(byte);
    }
    
    stream->close();
}
```

### Пример 2: Logging Decorators
```cpp
// Базовый интерфейс логгера
class Logger {
public:
    virtual ~Logger() = default;
    virtual void log(const std::string& message) = 0;
};

// Конкретный логгер: консоль
class ConsoleLogger : public Logger {
public:
    void log(const std::string& message) override {
        std::cout << message << std::endl;
    }
};

// Конкретный логгер: файл
class FileLogger : public Logger {
private:
    std::ofstream file_;
    
public:
    explicit FileLogger(const std::string& filename) {
        file_.open(filename, std::ios::app);
    }
    
    void log(const std::string& message) override {
        if (file_.is_open()) {
            file_ << message << std::endl;
            file_.flush();
        }
    }
    
    ~FileLogger() {
        file_.close();
    }
};

// Абстрактный декоратор
class LoggerDecorator : public Logger {
protected:
    std::unique_ptr<Logger> logger_;
    
public:
    explicit LoggerDecorator(std::unique_ptr<Logger> logger)
        : logger_(std::move(logger)) {}
    
    void log(const std::string& message) override {
        logger_->log(message);
    }
};

// Декоратор: timestamp
class TimestampDecorator : public LoggerDecorator {
public:
    explicit TimestampDecorator(std::unique_ptr<Logger> logger)
        : LoggerDecorator(std::move(logger)) {}
    
    void log(const std::string& message) override {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        
        std::ostringstream oss;
        oss << "[" << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << "] " 
            << message;
        
        logger_->log(oss.str());
    }
};

// Декоратор: уровень логирования
class LevelDecorator : public LoggerDecorator {
public:
    enum class Level { DEBUG, INFO, WARNING, ERROR };
    
private:
    Level level_;
    
    static std::string levelToString(Level level) {
        switch (level) {
            case Level::DEBUG: return "DEBUG";
            case Level::INFO: return "INFO";
            case Level::WARNING: return "WARNING";
            case Level::ERROR: return "ERROR";
            default: return "UNKNOWN";
        }
    }
    
public:
    LevelDecorator(std::unique_ptr<Logger> logger, Level level)
        : LoggerDecorator(std::move(logger)), level_(level) {}
    
    void log(const std::string& message) override {
        std::ostringstream oss;
        oss << "[" << levelToString(level_) << "] " << message;
        logger_->log(oss.str());
    }
};

// Декоратор: фильтр по уровню
class LevelFilterDecorator : public LoggerDecorator {
public:
    using Level = LevelDecorator::Level;
    
private:
    Level minLevel_;
    
public:
    LevelFilterDecorator(std::unique_ptr<Logger> logger, Level minLevel)
        : LoggerDecorator(std::move(logger)), minLevel_(minLevel) {}
    
    void log(const std::string& message, Level level) {
        if (level >= minLevel_) {
            logger_->log(message);
        }
    }
};

// Декоратор: цвета для консоли
class ColorDecorator : public LoggerDecorator {
private:
    std::string color_;
    
public:
    ColorDecorator(std::unique_ptr<Logger> logger, const std::string& color)
        : LoggerDecorator(std::move(logger)), color_(color) {}
    
    void log(const std::string& message) override {
        logger_->log(color_ + message + "\033[0m");  // ANSI escape codes
    }
};

// Использование
void setupLogging() {
    // Простой логгер
    auto logger = std::make_unique<ConsoleLogger>();
    logger->log("Simple message");
    
    // Логгер с timestamp
    logger = std::make_unique<TimestampDecorator>(std::move(logger));
    logger->log("Message with timestamp");
    
    // Логгер с timestamp и level
    logger = std::make_unique<LevelDecorator>(std::move(logger), 
                                               LevelDecorator::Level::ERROR);
    logger->log("Error message");
    
    // Полная цепочка
    auto fullLogger = std::make_unique<ColorDecorator>(
                         std::make_unique<LevelDecorator>(
                            std::make_unique<TimestampDecorator>(
                               std::make_unique<FileLogger>("app.log")
                            ),
                            LevelDecorator::Level::INFO
                         ),
                         "\033[32m"  // Green color
                      );
    
    fullLogger->log("Fully decorated message");
}
```

### Пример 3: Middleware Pattern (Web Framework)
```cpp
// HTTP запрос и ответ
struct HttpRequest {
    std::string method;
    std::string path;
    std::map<std::string, std::string> headers;
    std::string body;
};

struct HttpResponse {
    int statusCode;
    std::map<std::string, std::string> headers;
    std::string body;
};

// Базовый обработчик
class Handler {
public:
    virtual ~Handler() = default;
    virtual HttpResponse handle(const HttpRequest& request) = 0;
};

// Конкретный обработчик
class ApplicationHandler : public Handler {
public:
    HttpResponse handle(const HttpRequest& request) override {
        HttpResponse response;
        response.statusCode = 200;
        response.body = "Hello, World!";
        return response;
    }
};

// Middleware декоратор
class Middleware : public Handler {
protected:
    std::unique_ptr<Handler> next_;
    
public:
    explicit Middleware(std::unique_ptr<Handler> next)
        : next_(std::move(next)) {}
    
    HttpResponse handle(const HttpRequest& request) override {
        return next_->handle(request);
    }
};

// Middleware: логирование
class LoggingMiddleware : public Middleware {
public:
    explicit LoggingMiddleware(std::unique_ptr<Handler> next)
        : Middleware(std::move(next)) {}
    
    HttpResponse handle(const HttpRequest& request) override {
        std::cout << "Запрос: " << request.method << " " << request.path << std::endl;
        
        auto response = next_->handle(request);
        
        std::cout << "Ответ: " << response.statusCode << std::endl;
        return response;
    }
};

// Middleware: аутентификация
class AuthMiddleware : public Middleware {
public:
    explicit AuthMiddleware(std::unique_ptr<Handler> next)
        : Middleware(std::move(next)) {}
    
    HttpResponse handle(const HttpRequest& request) override {
        auto it = request.headers.find("Authorization");
        
        if (it == request.headers.end() || it->second != "Bearer valid_token") {
            HttpResponse response;
            response.statusCode = 401;
            response.body = "Unauthorized";
            return response;
        }
        
        return next_->handle(request);
    }
};

// Middleware: CORS
class CorsMiddleware : public Middleware {
public:
    explicit CorsMiddleware(std::unique_ptr<Handler> next)
        : Middleware(std::move(next)) {}
    
    HttpResponse handle(const HttpRequest& request) override {
        auto response = next_->handle(request);
        
        response.headers["Access-Control-Allow-Origin"] = "*";
        response.headers["Access-Control-Allow-Methods"] = "GET, POST, PUT, DELETE";
        
        return response;
    }
};

// Middleware: кэширование
class CacheMiddleware : public Middleware {
private:
    std::map<std::string, HttpResponse> cache_;
    
public:
    explicit CacheMiddleware(std::unique_ptr<Handler> next)
        : Middleware(std::move(next)) {}
    
    HttpResponse handle(const HttpRequest& request) override {
        std::string key = request.method + ":" + request.path;
        
        auto it = cache_.find(key);
        if (it != cache_.end()) {
            std::cout << "Возврат из кэша" << std::endl;
            return it->second;
        }
        
        auto response = next_->handle(request);
        cache_[key] = response;
        
        return response;
    }
};

// Использование
void setupWebServer() {
    // Строим цепочку middleware
    auto handler = std::make_unique<ApplicationHandler>();
    handler = std::make_unique<CacheMiddleware>(std::move(handler));
    handler = std::make_unique<CorsMiddleware>(std::move(handler));
    handler = std::make_unique<AuthMiddleware>(std::move(handler));
    handler = std::make_unique<LoggingMiddleware>(std::move(handler));
    
    // Обрабатываем запрос
    HttpRequest request;
    request.method = "GET";
    request.path = "/api/users";
    request.headers["Authorization"] = "Bearer valid_token";
    
    auto response = handler->handle(request);
    std::cout << "Статус: " << response.statusCode << std::endl;
    std::cout << "Тело: " << response.body << std::endl;
}
```

## 🎨 Современные альтернативы

### Functional Decorator (Лямбды)
```cpp
// Функциональный подход с лямбдами
using Operation = std::function<void()>;

Operation withLogging(Operation op) {
    return [op]() {
        std::cout << "Начало операции" << std::endl;
        op();
        std::cout << "Конец операции" << std::endl;
    };
}

Operation withTiming(Operation op) {
    return [op]() {
        auto start = std::chrono::high_resolution_clock::now();
        op();
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Время: " << duration.count() << "ms" << std::endl;
    };
}

// Использование
void functionalDecorators() {
    Operation op = []() {
        std::cout << "Выполнение операции" << std::endl;
    };
    
    op = withLogging(withTiming(op));
    op();
}
```

### Template-based Decorator
```cpp
// Шаблонный декоратор
template<typename Component>
class TimingDecorator : public Component {
public:
    using Component::Component;
    
    void operation() {
        auto start = std::chrono::high_resolution_clock::now();
        Component::operation();
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Время: " << duration.count() << "ms" << std::endl;
    }
};

// Использование
class MyComponent {
public:
    void operation() {
        std::cout << "Операция" << std::endl;
    }
};

TimingDecorator<MyComponent> decorated;
decorated.operation();
```

## 🚀 Современный C++

### Decorator с Concepts (C++20)
```cpp
#include <concepts>

// Concept для декорируемого компонента
template<typename T>
concept Decoratable = requires(T t) {
    { t.operation() } -> std::same_as<void>;
};

// Generic декоратор
template<Decoratable Component>
class GenericDecorator : public Component {
public:
    void operation() {
        std::cout << "До операции" << std::endl;
        Component::operation();
        std::cout << "После операции" << std::endl;
    }
};
```

### std::variant для type-safe декораторов
```cpp
#include <variant>

using Coffee = std::variant<
    SimpleCoffee,
    MilkDecorator,
    SugarDecorator
>;

// Pattern matching с std::visit
std::visit([](auto&& coffee) {
    std::cout << coffee.getDescription() << std::endl;
}, coffeeVariant);
```

## 🎯 Практические упражнения

### Упражнение 1: Text Decorators
Создайте декораторы для текста: uppercase, lowercase, trim, encrypt.

### Упражнение 2: Network Decorators
Реализуйте декораторы для сетевых соединений: retry, timeout, compression.

### Упражнение 3: GUI Decorators
Создайте декораторы для GUI компонентов: border, scroll, shadow.

### Упражнение 4: Performance Decorators
Реализуйте декораторы для измерения производительности: timing, memory, profiling.

## 📈 Связь с другими паттернами

### Decorator + Strategy
```cpp
// Decorator может использовать Strategy
class StrategyDecorator : public Handler {
private:
    std::unique_ptr<Strategy> strategy_;
public:
    void setStrategy(std::unique_ptr<Strategy> strategy) {
        strategy_ = std::move(strategy);
    }
};
```

### Decorator + Composite
```cpp
// Декораторы часто применяются к Composite структурам
class CompositeDecorator : public Component {
    std::vector<std::unique_ptr<Component>> children_;
};
```

## 📈 Следующие шаги
После изучения Decorator вы будете готовы к:
- Уроку 4.3: Facade Pattern
- Пониманию структурных паттернов
- Проектированию расширяемых систем
- Написанию middleware и plugins

## ⚠️ Важные предупреждения

1. **Порядок декораторов важен**: Разный порядок может дать разный результат
2. **Не злоупотребляйте**: Слишком много декораторов усложняет отладку
3. **Учитывайте производительность**: Каждый декоратор добавляет виртуальный вызов
4. **Используйте smart pointers**: Правильно управляйте временем жизни

---

*"The Decorator pattern attaches additional responsibilities to an object dynamically."* - Gang of Four

