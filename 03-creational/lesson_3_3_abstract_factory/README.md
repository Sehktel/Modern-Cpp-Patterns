# Урок 3.3: Abstract Factory Pattern (Абстрактная фабрика)

## 🎯 Цель урока
Изучить паттерн Abstract Factory - мощный креационный паттерн для создания семейств связанных или зависимых объектов без указания их конкретных классов. Понять применение паттерна в реальных системах, его преимущества и недостатки.

## 📚 Что изучаем

### 1. Паттерн Abstract Factory
- **Определение**: Предоставляет интерфейс для создания семейств связанных объектов
- **Назначение**: Изолирование создания объектов от их использования
- **Применение**: UI frameworks, cross-platform applications, database factories
- **Alias**: Kit паттерн (в некоторых источниках)

### 2. Структура паттерна
- **AbstractFactory**: Интерфейс для создания абстрактных продуктов
- **ConcreteFactory**: Реализация создания конкретных продуктов
- **AbstractProduct**: Интерфейс продукта
- **ConcreteProduct**: Конкретная реализация продукта
- **Client**: Использует только абстрактные интерфейсы

### 3. Отличия от других паттернов
- **vs Factory Method**: Создает семейства объектов vs один объект
- **vs Builder**: Создание за один вызов vs пошаговое построение
- **vs Prototype**: Создание через интерфейс vs клонирование

## 🔍 Ключевые концепции

### Классическая структура Abstract Factory
```cpp
// ============================================================================
// АБСТРАКТНЫЕ ПРОДУКТЫ
// ============================================================================

// Семейство продуктов: Кнопки
class AbstractButton {
public:
    virtual ~AbstractButton() = default;
    virtual void render() const = 0;
    virtual void onClick() = 0;
};

// Семейство продуктов: Чекбоксы
class AbstractCheckbox {
public:
    virtual ~AbstractCheckbox() = default;
    virtual void render() const = 0;
    virtual void onCheck() = 0;
};

// ============================================================================
// КОНКРЕТНЫЕ ПРОДУКТЫ: Windows стиль
// ============================================================================

class WindowsButton : public AbstractButton {
public:
    void render() const override {
        std::cout << "Рендерим Windows кнопку" << std::endl;
    }
    
    void onClick() override {
        std::cout << "Windows кнопка нажата" << std::endl;
    }
};

class WindowsCheckbox : public AbstractCheckbox {
public:
    void render() const override {
        std::cout << "Рендерим Windows чекбокс" << std::endl;
    }
    
    void onCheck() override {
        std::cout << "Windows чекбокс отмечен" << std::endl;
    }
};

// ============================================================================
// КОНКРЕТНЫЕ ПРОДУКТЫ: macOS стиль
// ============================================================================

class MacOSButton : public AbstractButton {
public:
    void render() const override {
        std::cout << "Рендерим macOS кнопку" << std::endl;
    }
    
    void onClick() override {
        std::cout << "macOS кнопка нажата" << std::endl;
    }
};

class MacOSCheckbox : public AbstractCheckbox {
public:
    void render() const override {
        std::cout << "Рендерим macOS чекбокс" << std::endl;
    }
    
    void onCheck() override {
        std::cout << "macOS чекбокс отмечен" << std::endl;
    }
};

// ============================================================================
// АБСТРАКТНАЯ ФАБРИКА
// ============================================================================

class AbstractUIFactory {
public:
    virtual ~AbstractUIFactory() = default;
    
    virtual std::unique_ptr<AbstractButton> createButton() const = 0;
    virtual std::unique_ptr<AbstractCheckbox> createCheckbox() const = 0;
};

// ============================================================================
// КОНКРЕТНЫЕ ФАБРИКИ
// ============================================================================

class WindowsFactory : public AbstractUIFactory {
public:
    std::unique_ptr<AbstractButton> createButton() const override {
        return std::make_unique<WindowsButton>();
    }
    
    std::unique_ptr<AbstractCheckbox> createCheckbox() const override {
        return std::make_unique<WindowsCheckbox>();
    }
};

class MacOSFactory : public AbstractUIFactory {
public:
    std::unique_ptr<AbstractButton> createButton() const override {
        return std::make_unique<MacOSButton>();
    }
    
    std::unique_ptr<AbstractCheckbox> createCheckbox() const override {
        return std::make_unique<MacOSCheckbox>();
    }
};

// ============================================================================
// КЛИЕНТ
// ============================================================================

class Application {
private:
    std::unique_ptr<AbstractButton> button_;
    std::unique_ptr<AbstractCheckbox> checkbox_;
    
public:
    explicit Application(const AbstractUIFactory& factory) {
        button_ = factory.createButton();
        checkbox_ = factory.createCheckbox();
    }
    
    void render() const {
        button_->render();
        checkbox_->render();
    }
};

// Использование
void demonstrateAbstractFactory() {
    // Выбираем фабрику в зависимости от платформы
    std::unique_ptr<AbstractUIFactory> factory;
    
    #ifdef _WIN32
        factory = std::make_unique<WindowsFactory>();
    #elif __APPLE__
        factory = std::make_unique<MacOSFactory>();
    #endif
    
    // Клиент работает с абстрактной фабрикой
    Application app(*factory);
    app.render();
}
```

**Теоретическая база**: Abstract Factory реализует принцип инверсии зависимостей (DIP) из SOLID. Высокоуровневый код (Application) зависит от абстракций (AbstractUIFactory), а не от конкретных реализаций. Это обеспечивает декаплинг и тестируемость.

### Современный подход с шаблонами
```cpp
// Шаблонная фабрика для type-safe создания
template<typename ProductFamily>
class ModernFactory {
public:
    template<typename Product>
    static auto create() {
        return std::make_unique<typename ProductFamily::template ProductType<Product>>();
    }
};

// Семейство продуктов Windows
struct WindowsFamily {
    template<typename T>
    struct ProductType;
};

template<>
struct WindowsFamily::ProductType<AbstractButton> {
    using type = WindowsButton;
};

// Использование с type safety
auto button = ModernFactory<WindowsFamily>::create<AbstractButton>();
```

**Теоретическая база**: Шаблонный подход использует compile-time полиморфизм вместо runtime, что устраняет overhead виртуальных вызовов. Сложность создания O(1), но увеличивается размер бинарника (code bloat).

## 🤔 Вопросы для размышления

### 1. Когда использовать Abstract Factory?
**✅ Хорошие случаи:**
- Система должна быть независима от способа создания продуктов
- Нужно создавать семейства связанных объектов
- Требуется обеспечить совместимость продуктов одного семейства
- Библиотека классов должна предоставлять только интерфейсы

**Примеры применения:**
- **GUI Frameworks**: Windows/macOS/Linux виджеты
- **Database Access**: MySQL/PostgreSQL/SQLite drivers
- **Game Development**: DirectX/OpenGL/Vulkan renderers
- **Document Processing**: PDF/Word/Excel exporters

**❌ Плохие случаи:**
- Нужно создать только один тип объекта (используйте Factory Method)
- Продукты не образуют семейства
- Редкое добавление новых типов продуктов (большой overhead)

### 2. Преимущества Abstract Factory
```cpp
// ✅ Изолирует конкретные классы
// Клиент не знает о конкретных классах
Application app(*factory);  // Работает с любой фабрикой

// ✅ Легко заменить семейство продуктов
factory = std::make_unique<LinuxFactory>();  // Меняем ВСЮ платформу

// ✅ Гарантирует совместимость
// Невозможно создать WindowsButton с MacOSCheckbox
auto button = windowsFactory->createButton();
auto checkbox = windowsFactory->createCheckbox();
// Гарантированно совместимы!

// ✅ Упрощает тестирование
class MockFactory : public AbstractUIFactory {
    // Mock-реализация для тестов
};
```

### 3. Недостатки Abstract Factory
```cpp
// ❌ Сложно добавить новый тип продукта
// Нужно изменить AbstractFactory и ВСЕ конкретные фабрики
class AbstractUIFactory {
public:
    virtual std::unique_ptr<AbstractButton> createButton() = 0;
    virtual std::unique_ptr<AbstractCheckbox> createCheckbox() = 0;
    // Добавление нового продукта требует изменения всех фабрик!
    virtual std::unique_ptr<AbstractScrollbar> createScrollbar() = 0; // НОВОЕ
};

// ❌ Много классов и интерфейсов
// Для N продуктов и M фабрик: N абстрактных + N*M конкретных классов

// ❌ Overhead от виртуальных вызовов
// Каждое создание объекта - виртуальный вызов
```

## 🛠️ Практические примеры

### Пример 1: Database Factory
```cpp
// Абстрактные продукты
class Connection {
public:
    virtual ~Connection() = default;
    virtual void connect(const std::string& connectionString) = 0;
    virtual void disconnect() = 0;
    virtual bool isConnected() const = 0;
};

class Command {
public:
    virtual ~Command() = default;
    virtual void execute(const std::string& query) = 0;
    virtual std::string getResult() const = 0;
};

class Transaction {
public:
    virtual ~Transaction() = default;
    virtual void begin() = 0;
    virtual void commit() = 0;
    virtual void rollback() = 0;
};

// Абстрактная фабрика
class AbstractDatabaseFactory {
public:
    virtual ~AbstractDatabaseFactory() = default;
    
    virtual std::unique_ptr<Connection> createConnection() const = 0;
    virtual std::unique_ptr<Command> createCommand(Connection& conn) const = 0;
    virtual std::unique_ptr<Transaction> createTransaction(Connection& conn) const = 0;
};

// Конкретные продукты: MySQL
class MySQLConnection : public Connection {
public:
    void connect(const std::string& connectionString) override {
        std::cout << "MySQL: Подключение к " << connectionString << std::endl;
    }
    
    void disconnect() override {
        std::cout << "MySQL: Отключение" << std::endl;
    }
    
    bool isConnected() const override {
        return true;
    }
};

class MySQLCommand : public Command {
private:
    Connection& connection_;
    std::string result_;
    
public:
    explicit MySQLCommand(Connection& conn) : connection_(conn) {}
    
    void execute(const std::string& query) override {
        std::cout << "MySQL: Выполнение запроса: " << query << std::endl;
        result_ = "MySQL result";
    }
    
    std::string getResult() const override {
        return result_;
    }
};

class MySQLTransaction : public Transaction {
public:
    void begin() override {
        std::cout << "MySQL: BEGIN TRANSACTION" << std::endl;
    }
    
    void commit() override {
        std::cout << "MySQL: COMMIT" << std::endl;
    }
    
    void rollback() override {
        std::cout << "MySQL: ROLLBACK" << std::endl;
    }
};

// Конкретная фабрика: MySQL
class MySQLFactory : public AbstractDatabaseFactory {
public:
    std::unique_ptr<Connection> createConnection() const override {
        return std::make_unique<MySQLConnection>();
    }
    
    std::unique_ptr<Command> createCommand(Connection& conn) const override {
        return std::make_unique<MySQLCommand>(conn);
    }
    
    std::unique_ptr<Transaction> createTransaction(Connection& conn) const override {
        return std::make_unique<MySQLTransaction>();
    }
};

// Аналогично для PostgreSQL
class PostgreSQLFactory : public AbstractDatabaseFactory {
    // ... реализация для PostgreSQL
};

// Data Access Layer
class DataAccessLayer {
private:
    std::unique_ptr<Connection> connection_;
    const AbstractDatabaseFactory& factory_;
    
public:
    explicit DataAccessLayer(const AbstractDatabaseFactory& factory)
        : factory_(factory) {
        connection_ = factory_.createConnection();
        connection_->connect("localhost:3306");
    }
    
    void executeQuery(const std::string& query) {
        auto command = factory_.createCommand(*connection_);
        command->execute(query);
        std::cout << "Результат: " << command->getResult() << std::endl;
    }
    
    void executeTransaction(const std::vector<std::string>& queries) {
        auto transaction = factory_.createTransaction(*connection_);
        
        try {
            transaction->begin();
            
            for (const auto& query : queries) {
                auto command = factory_.createCommand(*connection_);
                command->execute(query);
            }
            
            transaction->commit();
        } catch (...) {
            transaction->rollback();
            throw;
        }
    }
};

// Использование
void useDatabaseFactory() {
    // Выбираем БД через конфигурацию
    std::unique_ptr<AbstractDatabaseFactory> factory;
    
    std::string dbType = getConfig("database.type");
    if (dbType == "mysql") {
        factory = std::make_unique<MySQLFactory>();
    } else if (dbType == "postgresql") {
        factory = std::make_unique<PostgreSQLFactory>();
    }
    
    // DAL работает с любой БД
    DataAccessLayer dal(*factory);
    dal.executeQuery("SELECT * FROM users");
    dal.executeTransaction({"INSERT INTO users ...", "UPDATE stats ..."});
}
```

### Пример 2: Game Renderer Factory
```cpp
// Абстрактные продукты для рендеринга
class Texture {
public:
    virtual ~Texture() = default;
    virtual void load(const std::string& filename) = 0;
    virtual void bind() = 0;
};

class Shader {
public:
    virtual ~Shader() = default;
    virtual void compile(const std::string& source) = 0;
    virtual void use() = 0;
};

class Buffer {
public:
    virtual ~Buffer() = default;
    virtual void upload(const void* data, size_t size) = 0;
    virtual void bind() = 0;
};

// Абстрактная фабрика рендерера
class AbstractRendererFactory {
public:
    virtual ~AbstractRendererFactory() = default;
    
    virtual std::unique_ptr<Texture> createTexture() const = 0;
    virtual std::unique_ptr<Shader> createShader() const = 0;
    virtual std::unique_ptr<Buffer> createBuffer() const = 0;
    
    virtual void initialize() = 0;
    virtual void shutdown() = 0;
};

// OpenGL реализация
class OpenGLTexture : public Texture {
private:
    unsigned int textureId_;
    
public:
    void load(const std::string& filename) override {
        std::cout << "OpenGL: Загрузка текстуры " << filename << std::endl;
        // glGenTextures, glBindTexture, glTexImage2D...
    }
    
    void bind() override {
        std::cout << "OpenGL: Привязка текстуры" << std::endl;
        // glBindTexture(GL_TEXTURE_2D, textureId_);
    }
};

class OpenGLFactory : public AbstractRendererFactory {
public:
    void initialize() override {
        std::cout << "OpenGL: Инициализация" << std::endl;
        // glfwInit(), создание контекста...
    }
    
    void shutdown() override {
        std::cout << "OpenGL: Завершение работы" << std::endl;
        // Очистка ресурсов
    }
    
    std::unique_ptr<Texture> createTexture() const override {
        return std::make_unique<OpenGLTexture>();
    }
    
    std::unique_ptr<Shader> createShader() const override {
        return std::make_unique<OpenGLShader>();
    }
    
    std::unique_ptr<Buffer> createBuffer() const override {
        return std::make_unique<OpenGLBuffer>();
    }
};

// Vulkan реализация
class VulkanFactory : public AbstractRendererFactory {
    // Аналогичная реализация для Vulkan
};

// DirectX реализация
class DirectXFactory : public AbstractRendererFactory {
    // Аналогичная реализация для DirectX
};

// Движок игры
class GameEngine {
private:
    std::unique_ptr<AbstractRendererFactory> rendererFactory_;
    std::unique_ptr<Texture> texture_;
    std::unique_ptr<Shader> shader_;
    
public:
    explicit GameEngine(std::unique_ptr<AbstractRendererFactory> factory)
        : rendererFactory_(std::move(factory)) {
        rendererFactory_->initialize();
        
        // Создаем ресурсы через фабрику
        texture_ = rendererFactory_->createTexture();
        shader_ = rendererFactory_->createShader();
    }
    
    ~GameEngine() {
        rendererFactory_->shutdown();
    }
    
    void loadResources() {
        texture_->load("texture.png");
        shader_->compile("shader.glsl");
    }
    
    void render() {
        shader_->use();
        texture_->bind();
        // Рендеринг сцены...
    }
};

// Использование
void initializeGame() {
    std::unique_ptr<AbstractRendererFactory> factory;
    
    // Выбор API рендеринга
    std::string api = getConfig("graphics.api");
    if (api == "opengl") {
        factory = std::make_unique<OpenGLFactory>();
    } else if (api == "vulkan") {
        factory = std::make_unique<VulkanFactory>();
    } else if (api == "directx") {
        factory = std::make_unique<DirectXFactory>();
    }
    
    GameEngine engine(std::move(factory));
    engine.loadResources();
    engine.render();
}
```

### Пример 3: Document Export Factory
```cpp
// Абстрактные продукты документа
class DocumentWriter {
public:
    virtual ~DocumentWriter() = default;
    virtual void writeHeader(const std::string& title) = 0;
    virtual void writeParagraph(const std::string& text) = 0;
    virtual void writeTable(const std::vector<std::vector<std::string>>& data) = 0;
    virtual void save(const std::string& filename) = 0;
};

class ImageExporter {
public:
    virtual ~ImageExporter() = default;
    virtual void exportImage(const std::string& source, const std::string& dest) = 0;
};

// Абстрактная фабрика документов
class AbstractDocumentFactory {
public:
    virtual ~AbstractDocumentFactory() = default;
    
    virtual std::unique_ptr<DocumentWriter> createWriter() const = 0;
    virtual std::unique_ptr<ImageExporter> createImageExporter() const = 0;
};

// PDF реализация
class PDFWriter : public DocumentWriter {
private:
    std::ostringstream content_;
    
public:
    void writeHeader(const std::string& title) override {
        content_ << "PDF Header: " << title << "\n";
    }
    
    void writeParagraph(const std::string& text) override {
        content_ << "PDF Paragraph: " << text << "\n";
    }
    
    void writeTable(const std::vector<std::vector<std::string>>& data) override {
        content_ << "PDF Table with " << data.size() << " rows\n";
    }
    
    void save(const std::string& filename) override {
        std::cout << "Сохранение PDF в " << filename << std::endl;
        std::cout << content_.str() << std::endl;
    }
};

class PDFImageExporter : public ImageExporter {
public:
    void exportImage(const std::string& source, const std::string& dest) override {
        std::cout << "Экспорт изображения в PDF формат: " 
                  << source << " -> " << dest << std::endl;
    }
};

class PDFDocumentFactory : public AbstractDocumentFactory {
public:
    std::unique_ptr<DocumentWriter> createWriter() const override {
        return std::make_unique<PDFWriter>();
    }
    
    std::unique_ptr<ImageExporter> createImageExporter() const override {
        return std::make_unique<PDFImageExporter>();
    }
};

// Word реализация
class WordDocumentFactory : public AbstractDocumentFactory {
    // Аналогичная реализация для Word
};

// Markdown реализация
class MarkdownDocumentFactory : public AbstractDocumentFactory {
    // Аналогичная реализация для Markdown
};

// Система экспорта документов
class DocumentExportSystem {
private:
    const AbstractDocumentFactory& factory_;
    
public:
    explicit DocumentExportSystem(const AbstractDocumentFactory& factory)
        : factory_(factory) {}
    
    void exportDocument(const Document& doc, const std::string& filename) {
        auto writer = factory_.createWriter();
        auto imageExporter = factory_.createImageExporter();
        
        writer->writeHeader(doc.getTitle());
        
        for (const auto& paragraph : doc.getParagraphs()) {
            writer->writeParagraph(paragraph);
        }
        
        for (const auto& image : doc.getImages()) {
            imageExporter->exportImage(image, filename + "_img");
        }
        
        writer->save(filename);
    }
};
```

## 🎨 Современные альтернативы

### Dependency Injection вместо Factory
```cpp
// Вместо фабрики можно использовать DI контейнер
class DIContainer {
private:
    std::map<std::type_index, std::function<std::any()>> factories_;
    
public:
    template<typename Interface, typename Implementation>
    void registerType() {
        factories_[typeid(Interface)] = []() {
            return std::make_any<std::unique_ptr<Interface>>(
                std::make_unique<Implementation>()
            );
        };
    }
    
    template<typename Interface>
    std::unique_ptr<Interface> resolve() {
        return std::any_cast<std::unique_ptr<Interface>>(
            factories_[typeid(Interface)]()
        );
    }
};

// Использование
DIContainer container;
container.registerType<AbstractButton, WindowsButton>();
container.registerType<AbstractCheckbox, WindowsCheckbox>();

auto button = container.resolve<AbstractButton>();
```

### Functional approach
```cpp
// Функциональный подход с лямбдами
using ButtonFactory = std::function<std::unique_ptr<AbstractButton>()>;
using CheckboxFactory = std::function<std::unique_ptr<AbstractCheckbox>()>;

struct UIFactoryFunctions {
    ButtonFactory createButton;
    CheckboxFactory createCheckbox;
};

// Создание фабрики как набора функций
UIFactoryFunctions createWindowsFactory() {
    return {
        []() { return std::make_unique<WindowsButton>(); },
        []() { return std::make_unique<WindowsCheckbox>(); }
    };
}

// Использование
auto factory = createWindowsFactory();
auto button = factory.createButton();
```

## 🚀 Современный C++

### C++20 Concepts для type-safe factories
```cpp
#include <concepts>

// Concept для продукта
template<typename T>
concept Product = requires(T t) {
    { t.render() } -> std::same_as<void>;
};

// Concept для фабрики
template<typename F, typename P>
concept Factory = requires(F f) {
    { f.template create<P>() } -> std::same_as<std::unique_ptr<P>>;
};

// Type-safe фабрика с concepts
template<Factory F>
class SafeApplication {
    F factory_;
    
public:
    explicit SafeApplication(F factory) : factory_(factory) {}
    
    template<Product P>
    void useProduct() {
        auto product = factory_.template create<P>();
        product->render();
    }
};
```

### C++23 std::expected для обработки ошибок
```cpp
#include <expected>

class ModernFactory : public AbstractUIFactory {
public:
    std::expected<std::unique_ptr<AbstractButton>, std::string> 
    tryCreateButton() const {
        try {
            return std::make_unique<WindowsButton>();
        } catch (const std::exception& e) {
            return std::unexpected(e.what());
        }
    }
};

// Использование
auto result = factory.tryCreateButton();
if (result) {
    result->get()->render();
} else {
    std::cerr << "Ошибка: " << result.error() << std::endl;
}
```

## 🎯 Практические упражнения

### Упражнение 1: Cross-platform UI Factory
Создайте фабрику для создания UI элементов разных платформ.

### Упражнение 2: Plugin System
Реализуйте систему плагинов используя Abstract Factory.

### Упражнение 3: Configuration-driven Factory
Создайте фабрику, которая читает конфигурацию и создает соответствующие объекты.

### Упражнение 4: Testing with Mock Factory
Напишите тесты используя mock-фабрику.

## 📈 Связь с другими паттернами

### Abstract Factory + Singleton
```cpp
// Часто фабрика реализуется как Singleton
class UIFactorySingleton : public AbstractUIFactory {
private:
    UIFactorySingleton() = default;
    
public:
    static UIFactorySingleton& getInstance() {
        static UIFactorySingleton instance;
        return instance;
    }
    
    // Методы фабрики...
};
```

### Abstract Factory + Prototype
```cpp
// Фабрика может использовать прототипы для клонирования
class PrototypeFactory : public AbstractUIFactory {
private:
    std::unique_ptr<AbstractButton> buttonPrototype_;
    
public:
    std::unique_ptr<AbstractButton> createButton() const override {
        return buttonPrototype_->clone();
    }
};
```

## 📈 Следующие шаги
После изучения Abstract Factory вы будете готовы к:
- Уроку 3.4: Builder Pattern
- Пониманию других креационных паттернов
- Проектированию расширяемых систем
- Написанию тестируемого кода

## ⚠️ Важные предупреждения

1. **Не переусложняйте**: Если не нужны семейства объектов, используйте простой Factory Method
2. **Продумайте расширяемость**: Добавление нового продукта требует изменения всех фабрик
3. **Учитывайте производительность**: Виртуальные вызовы добавляют overhead
4. **Используйте smart pointers**: Избегайте сырых указателей для управления памятью

---

*"Abstract Factory provides an interface for creating families of related or dependent objects without specifying their concrete classes."* - Gang of Four

