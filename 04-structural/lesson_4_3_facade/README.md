# Урок 4.3: Facade Pattern (Фасад)

## 🎯 Цель урока
Изучить паттерн Facade - структурный паттерн для предоставления упрощенного интерфейса к сложной подсистеме. Понять, как скрыть сложность системы за простым и понятным API.

## 📚 Что изучаем

### 1. Паттерн Facade
- **Определение**: Предоставляет единый упрощенный интерфейс к сложной подсистеме
- **Назначение**: Снижение сложности взаимодействия с подсистемой
- **Применение**: Библиотеки, фреймворки, legacy системы, complex APIs
- **Принцип**: "Простота использования важнее гибкости"

### 2. Структура паттерна
- **Facade**: Упрощенный интерфейс к подсистеме
- **Subsystem Classes**: Компоненты сложной подсистемы
- **Client**: Использует Facade вместо прямого обращения к подсистеме

### 3. Отличия от других паттернов
- **vs Adapter**: Facade упрощает интерфейс, Adapter адаптирует
- **vs Mediator**: Facade односторонний, Mediator координирует взаимодействие
- **vs Decorator**: Facade не добавляет функциональность, только упрощает доступ

## 🔍 Ключевые концепции

### Проблема: Сложная подсистема
```cpp
// ❌ ПРОБЛЕМА: Клиент должен знать о всех деталях подсистемы

// Сложная подсистема с множеством классов
class CPU {
public:
    void freeze() { std::cout << "CPU: freeze" << std::endl; }
    void jump(long position) { std::cout << "CPU: jump to " << position << std::endl; }
    void execute() { std::cout << "CPU: execute" << std::endl; }
};

class Memory {
public:
    void load(long position, const std::string& data) {
        std::cout << "Memory: load at " << position << ": " << data << std::endl;
    }
};

class HardDrive {
public:
    std::string read(long lba, int size) {
        std::cout << "HDD: read " << size << " bytes from " << lba << std::endl;
        return "boot_data";
    }
};

// Клиент вынужден знать все детали загрузки компьютера
void bootComputerWithoutFacade() {
    CPU cpu;
    Memory memory;
    HardDrive hd;
    
    // Сложная последовательность операций
    cpu.freeze();
    memory.load(0x00, hd.read(0x7C00, 512));
    cpu.jump(0x00);
    cpu.execute();
    
    // Проблемы:
    // - Клиент должен знать порядок операций
    // - Нужно помнить магические числа (0x7C00, 512)
    // - Сложно переиспользовать
    // - Трудно изменить логику загрузки
}
```

**Теоретическая база**: Проблема high coupling - клиент тесно связан с подсистемой. Каждое изменение в подсистеме требует изменения клиента. Количество зависимостей: O(N×M) где N - классов в подсистеме, M - клиентов.

### Решение: Facade Pattern
```cpp
// ============================================================================
// СЛОЖНАЯ ПОДСИСТЕМА (без изменений)
// ============================================================================

class CPU {
public:
    void freeze() { std::cout << "CPU: freeze" << std::endl; }
    void jump(long position) { std::cout << "CPU: jump to " << position << std::endl; }
    void execute() { std::cout << "CPU: execute" << std::endl; }
};

class Memory {
public:
    void load(long position, const std::string& data) {
        std::cout << "Memory: load at " << position << ": " << data << std::endl;
    }
};

class HardDrive {
public:
    std::string read(long lba, int size) {
        std::cout << "HDD: read " << size << " bytes from " << lba << std::endl;
        return "boot_data";
    }
};

// ============================================================================
// FACADE - упрощенный интерфейс
// ============================================================================

class ComputerFacade {
private:
    CPU cpu_;
    Memory memory_;
    HardDrive hardDrive_;
    
    // Константы скрыты внутри фасада
    static constexpr long BOOT_ADDRESS = 0x7C00;
    static constexpr int BOOT_SECTOR = 512;
    static constexpr long BOOT_POSITION = 0x00;
    
public:
    // Простой метод вместо сложной последовательности
    void boot() {
        std::cout << "=== Загрузка компьютера ===" << std::endl;
        
        cpu_.freeze();
        memory_.load(BOOT_POSITION, 
                     hardDrive_.read(BOOT_ADDRESS, BOOT_SECTOR));
        cpu_.jump(BOOT_POSITION);
        cpu_.execute();
        
        std::cout << "=== Компьютер загружен ===" << std::endl;
    }
    
    void shutdown() {
        std::cout << "=== Выключение компьютера ===" << std::endl;
        // Сложная логика выключения скрыта
    }
    
    void restart() {
        shutdown();
        boot();
    }
};

// ============================================================================
// ИСПОЛЬЗОВАНИЕ
// ============================================================================

void bootComputerWithFacade() {
    ComputerFacade computer;
    
    // Просто и понятно!
    computer.boot();
    
    // Больше не нужно знать детали
    // Все сложности скрыты за фасадом
}
```

**Теоретическая база**: Facade реализует принцип низкого coupling (low coupling) и высокой cohesion (high cohesion). Количество зависимостей уменьшается до O(M) где M - клиентов, так как все зависимости от подсистемы инкапсулированы в Facade.

### Facade для сложных библиотек
```cpp
// Пример: Сложная библиотека обработки видео
// (как FFmpeg, OpenCV и т.д.)

// ============================================================================
// СЛОЖНАЯ ПОДСИСТЕМА
// ============================================================================

class VideoDecoder {
public:
    void setCodec(const std::string& codec) {
        std::cout << "Decoder: codec set to " << codec << std::endl;
    }
    
    void decode(const std::string& data) {
        std::cout << "Decoder: decoding..." << std::endl;
    }
};

class AudioDecoder {
public:
    void setFormat(const std::string& format) {
        std::cout << "Audio: format set to " << format << std::endl;
    }
    
    void decode(const std::string& data) {
        std::cout << "Audio: decoding..." << std::endl;
    }
};

class VideoRenderer {
public:
    void initialize(int width, int height) {
        std::cout << "Renderer: " << width << "x" << height << std::endl;
    }
    
    void render(const std::string& frame) {
        std::cout << "Renderer: rendering frame" << std::endl;
    }
};

class AudioMixer {
public:
    void setVolume(float volume) {
        std::cout << "Mixer: volume " << volume << std::endl;
    }
    
    void mix(const std::string& audio) {
        std::cout << "Mixer: mixing audio" << std::endl;
    }
};

class FileReader {
public:
    void open(const std::string& filename) {
        std::cout << "File: opened " << filename << std::endl;
    }
    
    std::string read() {
        return "video_data";
    }
    
    void close() {
        std::cout << "File: closed" << std::endl;
    }
};

// ============================================================================
// FACADE - простой интерфейс
// ============================================================================

class VideoPlayerFacade {
private:
    VideoDecoder videoDecoder_;
    AudioDecoder audioDecoder_;
    VideoRenderer renderer_;
    AudioMixer mixer_;
    FileReader reader_;
    
    bool isPlaying_;
    std::string currentFile_;
    
public:
    VideoPlayerFacade() : isPlaying_(false) {}
    
    // Простой метод открытия и воспроизведения
    void play(const std::string& filename) {
        std::cout << "\n=== Воспроизведение " << filename << " ===" << std::endl;
        
        // Все сложности скрыты
        reader_.open(filename);
        
        // Автоопределение формата (упрощено)
        videoDecoder_.setCodec("H264");
        audioDecoder_.setFormat("AAC");
        
        // Настройка рендерера
        renderer_.initialize(1920, 1080);
        mixer_.setVolume(0.8f);
        
        // Декодирование и воспроизведение
        auto data = reader_.read();
        videoDecoder_.decode(data);
        audioDecoder_.decode(data);
        
        isPlaying_ = true;
        currentFile_ = filename;
        
        std::cout << "=== Воспроизведение началось ===" << std::endl;
    }
    
    void pause() {
        if (isPlaying_) {
            std::cout << "=== Пауза ===" << std::endl;
            isPlaying_ = false;
        }
    }
    
    void resume() {
        if (!isPlaying_ && !currentFile_.empty()) {
            std::cout << "=== Продолжение ===" << std::endl;
            isPlaying_ = true;
        }
    }
    
    void stop() {
        std::cout << "=== Остановка ===" << std::endl;
        isPlaying_ = false;
        reader_.close();
        currentFile_.clear();
    }
    
    void setVolume(float volume) {
        mixer_.setVolume(volume);
    }
};

// Использование
void watchVideo() {
    VideoPlayerFacade player;
    
    // Просто и интуитивно!
    player.play("movie.mp4");
    player.setVolume(0.5f);
    player.pause();
    player.resume();
    player.stop();
    
    // Вся сложность обработки видео скрыта
}
```

## 🤔 Вопросы для размышления

### 1. Когда использовать Facade?
**✅ Хорошие случаи:**
- Подсистема очень сложна
- Есть множество зависимостей между классами подсистемы
- Нужно упростить использование библиотеки
- Нужно разделить систему на слои
- Работа с legacy кодом

**Примеры применения:**
```cpp
// 1. Database Access Facade
class DatabaseFacade {
public:
    void saveUser(const User& user);
    User getUser(int id);
    void deleteUser(int id);
    // Скрывает SQL, транзакции, пулы соединений
};

// 2. Compiler Facade
class CompilerFacade {
public:
    bool compile(const std::string& sourceFile);
    // Скрывает lexer, parser, optimizer, code generator
};

// 3. Game Engine Facade
class GameEngineFacade {
public:
    void initialize();
    void render();
    void update(float deltaTime);
    // Скрывает graphics, physics, audio, input системы
};

// 4. Network Facade
class HttpClientFacade {
public:
    std::string get(const std::string& url);
    std::string post(const std::string& url, const std::string& data);
    // Скрывает sockets, SSL, buffers, parsing
};
```

**❌ Плохие случаи:**
- Подсистема проста
- Нужен доступ к деталям подсистемы
- Facade становится god object
- Facade нарушает SRP (делает слишком много)

### 2. Преимущества Facade
```cpp
// ✅ Упрощение использования
// Вместо:
Connection conn = pool.getConnection();
Statement stmt = conn.createStatement();
ResultSet rs = stmt.executeQuery("SELECT * FROM users");
while (rs.next()) { /* ... */ }
rs.close();
stmt.close();
conn.close();

// Просто:
DatabaseFacade db;
auto users = db.getAllUsers();

// ✅ Изоляция от изменений
// Если подсистема изменится, клиенты не пострадают
// Только Facade нужно обновить

// ✅ Слоистая архитектура
// Application Layer
//   ↓ (использует Facade)
// Business Logic Layer
//   ↓ (использует Facade)
// Data Access Layer

// ✅ Легче тестировать
// Можно создать mock Facade
class MockDatabaseFacade : public DatabaseFacade {
    // Простой mock вместо реальной БД
};
```

### 3. Недостатки Facade
```cpp
// ❌ Facade может стать God Object
// Слишком много ответственности
class MegaFacade {
public:
    void doEverything();  // ПЛОХО!
    void handleAllCases();
    // ... 100 методов
};

// ❌ Ограничение гибкости
// Если Facade не предоставляет нужный метод,
// приходится обращаться к подсистеме напрямую
class LimitedFacade {
public:
    void basicOperation();
    // Но что если нужна расширенная операция?
};

// ❌ Дополнительный уровень абстракции
// Overhead: еще один класс, еще один слой
// Если подсистема проста, Facade избыточен
```

## 🛠️ Практические примеры

### Пример 1: Email System Facade
```cpp
// Сложная подсистема email
class SMTPConnection {
public:
    void connect(const std::string& host, int port) {
        std::cout << "SMTP: Connecting to " << host << ":" << port << std::endl;
    }
    
    void authenticate(const std::string& user, const std::string& password) {
        std::cout << "SMTP: Authenticating " << user << std::endl;
    }
    
    void send(const std::string& data) {
        std::cout << "SMTP: Sending data" << std::endl;
    }
    
    void disconnect() {
        std::cout << "SMTP: Disconnecting" << std::endl;
    }
};

class MIMEBuilder {
public:
    void setFrom(const std::string& from) {
        content_ += "From: " + from + "\r\n";
    }
    
    void setTo(const std::string& to) {
        content_ += "To: " + to + "\r\n";
    }
    
    void setSubject(const std::string& subject) {
        content_ += "Subject: " + subject + "\r\n";
    }
    
    void setBody(const std::string& body) {
        content_ += "\r\n" + body;
    }
    
    void addAttachment(const std::string& filename) {
        content_ += "\r\nAttachment: " + filename;
    }
    
    std::string build() const {
        return content_;
    }
    
private:
    std::string content_;
};

class EmailValidator {
public:
    bool isValidEmail(const std::string& email) {
        return email.find('@') != std::string::npos;
    }
};

// Facade - простой интерфейс для отправки email
class EmailFacade {
private:
    SMTPConnection smtp_;
    MIMEBuilder builder_;
    EmailValidator validator_;
    
    std::string smtpHost_;
    int smtpPort_;
    std::string username_;
    std::string password_;
    
public:
    EmailFacade(const std::string& host, int port,
                const std::string& user, const std::string& pass)
        : smtpHost_(host), smtpPort_(port), username_(user), password_(pass) {}
    
    bool sendEmail(const std::string& to,
                   const std::string& subject,
                   const std::string& body,
                   const std::vector<std::string>& attachments = {}) {
        std::cout << "\n=== Отправка email ===" << std::endl;
        
        // Валидация
        if (!validator_.isValidEmail(to)) {
            std::cerr << "Ошибка: неверный email адрес" << std::endl;
            return false;
        }
        
        try {
            // Подключение
            smtp_.connect(smtpHost_, smtpPort_);
            smtp_.authenticate(username_, password_);
            
            // Построение сообщения
            builder_.setFrom(username_);
            builder_.setTo(to);
            builder_.setSubject(subject);
            builder_.setBody(body);
            
            for (const auto& attachment : attachments) {
                builder_.addAttachment(attachment);
            }
            
            // Отправка
            smtp_.send(builder_.build());
            smtp_.disconnect();
            
            std::cout << "=== Email отправлен ===" << std::endl;
            return true;
            
        } catch (const std::exception& e) {
            std::cerr << "Ошибка отправки: " << e.what() << std::endl;
            smtp_.disconnect();
            return false;
        }
    }
};

// Использование
void sendBusinessEmail() {
    EmailFacade email("smtp.gmail.com", 587, "user@gmail.com", "password");
    
    // Просто и понятно!
    email.sendEmail(
        "client@example.com",
        "Quarterly Report",
        "Please find the attached report.",
        {"report.pdf", "summary.xlsx"}
    );
    
    // Вся сложность SMTP, MIME, валидации скрыта
}
```

### Пример 2: E-commerce Order Processing Facade
```cpp
// Сложная подсистема обработки заказов
class Inventory {
public:
    bool checkAvailability(const std::string& productId, int quantity) {
        std::cout << "Inventory: Checking " << productId << std::endl;
        return true;  // Упрощено
    }
    
    void reserve(const std::string& productId, int quantity) {
        std::cout << "Inventory: Reserved " << quantity << " x " << productId << std::endl;
    }
    
    void release(const std::string& productId, int quantity) {
        std::cout << "Inventory: Released " << quantity << " x " << productId << std::endl;
    }
};

class PaymentProcessor {
public:
    std::string processPayment(const std::string& cardNumber, double amount) {
        std::cout << "Payment: Processing $" << amount << std::endl;
        return "TXN_" + std::to_string(rand());
    }
    
    void refund(const std::string& transactionId) {
        std::cout << "Payment: Refunding " << transactionId << std::endl;
    }
};

class ShippingService {
public:
    std::string createShipment(const std::string& address, double weight) {
        std::cout << "Shipping: Creating shipment to " << address << std::endl;
        return "SHIP_" + std::to_string(rand());
    }
    
    void trackShipment(const std::string& shipmentId) {
        std::cout << "Shipping: Tracking " << shipmentId << std::endl;
    }
};

class NotificationService {
public:
    void sendOrderConfirmation(const std::string& email, const std::string& orderId) {
        std::cout << "Notification: Order " << orderId << " confirmed to " << email << std::endl;
    }
    
    void sendShippingNotification(const std::string& email, const std::string& trackingNumber) {
        std::cout << "Notification: Tracking " << trackingNumber << " sent to " << email << std::endl;
    }
};

class OrderDatabase {
public:
    void saveOrder(const std::string& orderId, const std::string& status) {
        std::cout << "Database: Order " << orderId << " saved with status " << status << std::endl;
    }
    
    void updateOrderStatus(const std::string& orderId, const std::string& status) {
        std::cout << "Database: Order " << orderId << " updated to " << status << std::endl;
    }
};

// Order data
struct OrderItem {
    std::string productId;
    int quantity;
    double price;
};

struct Order {
    std::string orderId;
    std::string customerEmail;
    std::string shippingAddress;
    std::vector<OrderItem> items;
    std::string cardNumber;
};

// Facade - упрощенный интерфейс обработки заказов
class OrderProcessingFacade {
private:
    Inventory inventory_;
    PaymentProcessor payment_;
    ShippingService shipping_;
    NotificationService notification_;
    OrderDatabase database_;
    
public:
    bool processOrder(const Order& order) {
        std::cout << "\n=== Обработка заказа " << order.orderId << " ===" << std::endl;
        
        try {
            // Шаг 1: Проверка наличия
            for (const auto& item : order.items) {
                if (!inventory_.checkAvailability(item.productId, item.quantity)) {
                    throw std::runtime_error("Product not available: " + item.productId);
                }
            }
            
            // Шаг 2: Резервирование товаров
            for (const auto& item : order.items) {
                inventory_.reserve(item.productId, item.quantity);
            }
            
            // Шаг 3: Обработка платежа
            double total = 0;
            for (const auto& item : order.items) {
                total += item.price * item.quantity;
            }
            
            std::string transactionId = payment_.processPayment(order.cardNumber, total);
            
            // Шаг 4: Создание доставки
            double totalWeight = order.items.size() * 1.0;  // Упрощено
            std::string shipmentId = shipping_.createShipment(
                order.shippingAddress, 
                totalWeight
            );
            
            // Шаг 5: Сохранение заказа
            database_.saveOrder(order.orderId, "PROCESSING");
            database_.updateOrderStatus(order.orderId, "CONFIRMED");
            
            // Шаг 6: Уведомления
            notification_.sendOrderConfirmation(order.customerEmail, order.orderId);
            notification_.sendShippingNotification(order.customerEmail, shipmentId);
            
            std::cout << "=== Заказ успешно обработан ===" << std::endl;
            return true;
            
        } catch (const std::exception& e) {
            std::cerr << "Ошибка обработки заказа: " << e.what() << std::endl;
            
            // Откат изменений
            for (const auto& item : order.items) {
                inventory_.release(item.productId, item.quantity);
            }
            
            database_.updateOrderStatus(order.orderId, "FAILED");
            return false;
        }
    }
    
    void trackOrder(const std::string& orderId) {
        // Упрощенная версия отслеживания
        std::cout << "Отслеживание заказа " << orderId << std::endl;
    }
};

// Использование
void placeOrder() {
    OrderProcessingFacade orderSystem;
    
    Order order;
    order.orderId = "ORD_12345";
    order.customerEmail = "customer@example.com";
    order.shippingAddress = "123 Main St, City";
    order.cardNumber = "1234-5678-9012-3456";
    order.items = {
        {"PROD_001", 2, 29.99},
        {"PROD_002", 1, 49.99}
    };
    
    // Одна простая операция вместо сложной последовательности
    if (orderSystem.processOrder(order)) {
        orderSystem.trackOrder(order.orderId);
    }
}
```

### Пример 3: Home Theater Facade
```cpp
// Классический пример из Gang of Four
class Amplifier {
public:
    void on() { std::cout << "Amplifier: ON" << std::endl; }
    void off() { std::cout << "Amplifier: OFF" << std::endl; }
    void setVolume(int level) { std::cout << "Amplifier: Volume " << level << std::endl; }
};

class DVDPlayer {
public:
    void on() { std::cout << "DVD Player: ON" << std::endl; }
    void off() { std::cout << "DVD Player: OFF" << std::endl; }
    void play(const std::string& movie) { std::cout << "DVD: Playing " << movie << std::endl; }
    void stop() { std::cout << "DVD: Stopped" << std::endl; }
};

class Projector {
public:
    void on() { std::cout << "Projector: ON" << std::endl; }
    void off() { std::cout << "Projector: OFF" << std::endl; }
    void setInput(const std::string& input) { std::cout << "Projector: Input " << input << std::endl; }
    void wideScreenMode() { std::cout << "Projector: Widescreen mode" << std::endl; }
};

class Screen {
public:
    void down() { std::cout << "Screen: Down" << std::endl; }
    void up() { std::cout << "Screen: Up" << std::endl; }
};

class TheaterLights {
public:
    void dim(int level) { std::cout << "Lights: Dimmed to " << level << "%" << std::endl; }
    void on() { std::cout << "Lights: ON" << std::endl; }
};

class PopcornPopper {
public:
    void on() { std::cout << "Popcorn: ON" << std::endl; }
    void off() { std::cout << "Popcorn: OFF" << std::endl; }
    void pop() { std::cout << "Popcorn: Popping!" << std::endl; }
};

// Facade для домашнего кинотеатра
class HomeTheaterFacade {
private:
    Amplifier& amp_;
    DVDPlayer& dvd_;
    Projector& projector_;
    Screen& screen_;
    TheaterLights& lights_;
    PopcornPopper& popper_;
    
public:
    HomeTheaterFacade(Amplifier& amp, DVDPlayer& dvd, Projector& proj,
                      Screen& screen, TheaterLights& lights, PopcornPopper& popper)
        : amp_(amp), dvd_(dvd), projector_(proj), 
          screen_(screen), lights_(lights), popper_(popper) {}
    
    void watchMovie(const std::string& movie) {
        std::cout << "\n=== Подготовка к просмотру фильма ===" << std::endl;
        
        popper_.on();
        popper_.pop();
        
        lights_.dim(10);
        
        screen_.down();
        
        projector_.on();
        projector_.wideScreenMode();
        projector_.setInput("DVD");
        
        amp_.on();
        amp_.setVolume(5);
        
        dvd_.on();
        dvd_.play(movie);
        
        std::cout << "=== Приятного просмотра! ===" << std::endl;
    }
    
    void endMovie() {
        std::cout << "\n=== Завершение просмотра ===" << std::endl;
        
        popper_.off();
        lights_.on();
        screen_.up();
        projector_.off();
        amp_.off();
        dvd_.stop();
        dvd_.off();
        
        std::cout << "=== Кинотеатр выключен ===" << std::endl;
    }
};

// Использование
void enjoyMovie() {
    // Создание компонентов
    Amplifier amp;
    DVDPlayer dvd;
    Projector projector;
    Screen screen;
    TheaterLights lights;
    PopcornPopper popper;
    
    // Создание фасада
    HomeTheaterFacade theater(amp, dvd, projector, screen, lights, popper);
    
    // Просто и понятно!
    theater.watchMovie("The Matrix");
    
    // ... после просмотра ...
    theater.endMovie();
    
    // Без фасада пришлось бы вызывать 15+ методов вручную!
}
```

## 🎨 Современные подходы

### Fluent Facade
```cpp
// Facade с fluent interface
class FluentDatabaseFacade {
private:
    std::string table_;
    std::vector<std::string> columns_;
    std::string whereClause_;
    
public:
    FluentDatabaseFacade& select(const std::vector<std::string>& cols) {
        columns_ = cols;
        return *this;
    }
    
    FluentDatabaseFacade& from(const std::string& table) {
        table_ = table;
        return *this;
    }
    
    FluentDatabaseFacade& where(const std::string& condition) {
        whereClause_ = condition;
        return *this;
    }
    
    std::vector<std::string> execute() {
        // Выполнение запроса
        std::cout << "SELECT ";
        for (const auto& col : columns_) std::cout << col << " ";
        std::cout << "FROM " << table_;
        if (!whereClause_.empty()) {
            std::cout << " WHERE " << whereClause_;
        }
        std::cout << std::endl;
        
        return {};  // Results
    }
};

// Использование
auto results = FluentDatabaseFacade()
                  .select({"name", "age"})
                  .from("users")
                  .where("age > 18")
                  .execute();
```

### Template-based Facade
```cpp
// Шаблонный фасад для разных подсистем
template<typename SubsystemType>
class GenericFacade {
private:
    SubsystemType subsystem_;
    
public:
    void simpleOperation() {
        subsystem_.complexOperation1();
        subsystem_.complexOperation2();
        subsystem_.complexOperation3();
    }
};
```

## 🚀 Современный C++

### std::optional для опциональных операций
```cpp
#include <optional>

class ModernFacade {
public:
    std::optional<User> findUser(int id) {
        // Может вернуть пользователя или nullopt
        if (id > 0) {
            return User{id, "John"};
        }
        return std::nullopt;
    }
};

// Использование
auto user = facade.findUser(42);
if (user) {
    std::cout << "Found: " << user->name << std::endl;
}
```

### std::expected для обработки ошибок (C++23)
```cpp
#include <expected>

class SafeFacade {
public:
    std::expected<Result, Error> operation() {
        try {
            // Сложная операция
            return Result{};
        } catch (const std::exception& e) {
            return std::unexpected(Error{e.what()});
        }
    }
};
```

## 🎯 Практические упражнения

### Упражнение 1: Graphics Library Facade
Создайте Facade для сложной графической библиотеки (OpenGL/DirectX).

### Упражнение 2: Build System Facade
Реализуйте Facade для системы сборки (compiler, linker, packager).

### Упражнение 3: API Client Facade
Создайте Facade для REST API клиента с множеством endpoints.

### Упражнение 4: Game Subsystem Facade
Реализуйте Facade для игрового движка (graphics, physics, audio, input).

## 📈 Связь с другими паттернами

### Facade + Singleton
```cpp
// Facade часто реализуется как Singleton
class SystemFacade {
public:
    static SystemFacade& getInstance() {
        static SystemFacade instance;
        return instance;
    }
    
private:
    SystemFacade() = default;
};
```

### Facade + Abstract Factory
```cpp
// Facade может использовать фабрику
class FacadeWithFactory {
private:
    std::unique_ptr<AbstractFactory> factory_;
public:
    void operation() {
        auto product = factory_->createProduct();
        // использование
    }
};
```

## 📈 Следующие шаги
После изучения Facade вы будете готовы к:
- Уроку 4.4: Proxy Pattern
- Пониманию архитектурных паттернов
- Проектированию API и библиотек
- Работе с legacy системами

## ⚠️ Важные предупреждения

1. **Не превращайте Facade в God Object**: Разделяйте ответственности
2. **Facade - это не слой**: Не делайте слой за слоем Facades
3. **Сохраняйте доступ к подсистеме**: Facade не должен полностью закрывать подсистему
4. **Документируйте**: Объясните что скрывает Facade

---

*"The Facade pattern provides a unified interface to a set of interfaces in a subsystem."* - Gang of Four

