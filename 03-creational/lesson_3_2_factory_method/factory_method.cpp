#include <iostream>
#include <memory>
#include <string>
#include <map>
#include <functional>
#include <vector>
#include <stdexcept>

/**
 * @file factory_method.cpp
 * @brief Демонстрация паттерна Factory Method
 * 
 * Этот файл показывает различные реализации Factory Method паттерна,
 * от простых до продвинутых с использованием современных возможностей C++.
 */

// ============================================================================
// БАЗОВЫЕ ИНТЕРФЕЙСЫ
// ============================================================================

/**
 * @brief Базовый класс для документов
 */
class Document {
public:
    virtual ~Document() = default;
    
    virtual void open() = 0;
    virtual void close() = 0;
    virtual void save() = 0;
    virtual void print() = 0;
    virtual std::string getType() const = 0;
};

/**
 * @brief Базовый интерфейс для фабрики документов
 */
class DocumentFactory {
public:
    virtual ~DocumentFactory() = default;
    virtual std::unique_ptr<Document> createDocument(const std::string& type) = 0;
    virtual std::vector<std::string> getSupportedTypes() const = 0;
};

// ============================================================================
// КОНКРЕТНЫЕ РЕАЛИЗАЦИИ ДОКУМЕНТОВ
// ============================================================================

/**
 * @brief PDF документ
 */
class PDFDocument : public Document {
private:
    std::string filename_;
    bool isOpen_;
    
public:
    explicit PDFDocument(const std::string& filename = "document.pdf") 
        : filename_(filename), isOpen_(false) {}
    
    void open() override {
        isOpen_ = true;
        std::cout << "PDF: Открываем документ " << filename_ << std::endl;
    }
    
    void close() override {
        if (isOpen_) {
            isOpen_ = false;
            std::cout << "PDF: Закрываем документ " << filename_ << std::endl;
        }
    }
    
    void save() override {
        if (isOpen_) {
            std::cout << "PDF: Сохраняем документ " << filename_ << std::endl;
        } else {
            throw std::runtime_error("PDF: Документ не открыт!");
        }
    }
    
    void print() override {
        if (isOpen_) {
            std::cout << "PDF: Печатаем документ " << filename_ << std::endl;
        } else {
            throw std::runtime_error("PDF: Документ не открыт!");
        }
    }
    
    std::string getType() const override {
        return "PDF";
    }
};

/**
 * @brief Word документ
 */
class WordDocument : public Document {
private:
    std::string filename_;
    bool isOpen_;
    
public:
    explicit WordDocument(const std::string& filename = "document.docx") 
        : filename_(filename), isOpen_(false) {}
    
    void open() override {
        isOpen_ = true;
        std::cout << "Word: Открываем документ " << filename_ << std::endl;
    }
    
    void close() override {
        if (isOpen_) {
            isOpen_ = false;
            std::cout << "Word: Закрываем документ " << filename_ << std::endl;
        }
    }
    
    void save() override {
        if (isOpen_) {
            std::cout << "Word: Сохраняем документ " << filename_ << std::endl;
        } else {
            throw std::runtime_error("Word: Документ не открыт!");
        }
    }
    
    void print() override {
        if (isOpen_) {
            std::cout << "Word: Печатаем документ " << filename_ << std::endl;
        } else {
            throw std::runtime_error("Word: Документ не открыт!");
        }
    }
    
    std::string getType() const override {
        return "Word";
    }
};

/**
 * @brief Excel документ
 */
class ExcelDocument : public Document {
private:
    std::string filename_;
    bool isOpen_;
    
public:
    explicit ExcelDocument(const std::string& filename = "document.xlsx") 
        : filename_(filename), isOpen_(false) {}
    
    void open() override {
        isOpen_ = true;
        std::cout << "Excel: Открываем документ " << filename_ << std::endl;
    }
    
    void close() override {
        if (isOpen_) {
            isOpen_ = false;
            std::cout << "Excel: Закрываем документ " << filename_ << std::endl;
        }
    }
    
    void save() override {
        if (isOpen_) {
            std::cout << "Excel: Сохраняем документ " << filename_ << std::endl;
        } else {
            throw std::runtime_error("Excel: Документ не открыт!");
        }
    }
    
    void print() override {
        if (isOpen_) {
            std::cout << "Excel: Печатаем документ " << filename_ << std::endl;
        } else {
            throw std::runtime_error("Excel: Документ не открыт!");
        }
    }
    
    std::string getType() const override {
        return "Excel";
    }
};

// ============================================================================
// КОНКРЕТНЫЕ ФАБРИКИ
// ============================================================================

/**
 * @brief Стандартная фабрика документов
 */
class StandardDocumentFactory : public DocumentFactory {
public:
    std::unique_ptr<Document> createDocument(const std::string& type) override {
        if (type == "PDF" || type == "pdf") {
            return std::make_unique<PDFDocument>();
        } else if (type == "Word" || type == "word" || type == "docx") {
            return std::make_unique<WordDocument>();
        } else if (type == "Excel" || type == "excel" || type == "xlsx") {
            return std::make_unique<ExcelDocument>();
        }
        
        throw std::invalid_argument("Неподдерживаемый тип документа: " + type);
    }
    
    std::vector<std::string> getSupportedTypes() const override {
        return {"PDF", "Word", "Excel"};
    }
};

/**
 * @brief Расширенная фабрика документов с настройками
 */
class AdvancedDocumentFactory : public DocumentFactory {
private:
    std::string defaultPath_;
    
public:
    explicit AdvancedDocumentFactory(const std::string& defaultPath = "/documents/")
        : defaultPath_(defaultPath) {}
    
    std::unique_ptr<Document> createDocument(const std::string& type) override {
        std::string filename = defaultPath_ + "document";
        
        if (type == "PDF" || type == "pdf") {
            return std::make_unique<PDFDocument>(filename + ".pdf");
        } else if (type == "Word" || type == "word" || type == "docx") {
            return std::make_unique<WordDocument>(filename + ".docx");
        } else if (type == "Excel" || type == "excel" || type == "xlsx") {
            return std::make_unique<ExcelDocument>(filename + ".xlsx");
        }
        
        throw std::invalid_argument("Неподдерживаемый тип документа: " + type);
    }
    
    std::vector<std::string> getSupportedTypes() const override {
        return {"PDF", "Word", "Excel"};
    }
};

// ============================================================================
// СОВРЕМЕННЫЕ ПОДХОДЫ: STATIC FACTORY METHODS
// ============================================================================

/**
 * @brief Демонстрация Static Factory Methods
 */
class Point {
private:
    double x_, y_;
    
    Point(double x, double y) : x_(x), y_(y) {}
    
public:
    // Static factory methods
    static Point fromCartesian(double x, double y) {
        std::cout << "Создаем точку из декартовых координат: (" << x << ", " << y << ")" << std::endl;
        return Point(x, y);
    }
    
    static Point fromPolar(double radius, double angle) {
        double x = radius * std::cos(angle);
        double y = radius * std::sin(angle);
        std::cout << "Создаем точку из полярных координат: радиус=" << radius 
                  << ", угол=" << angle << " -> (" << x << ", " << y << ")" << std::endl;
        return Point(x, y);
    }
    
    static Point origin() {
        std::cout << "Создаем точку в начале координат" << std::endl;
        return Point(0, 0);
    }
    
    static Point fromString(const std::string& coords) {
        // Простой парсинг "x,y"
        size_t commaPos = coords.find(',');
        if (commaPos == std::string::npos) {
            throw std::invalid_argument("Неверный формат координат: " + coords);
        }
        
        double x = std::stod(coords.substr(0, commaPos));
        double y = std::stod(coords.substr(commaPos + 1));
        
        std::cout << "Создаем точку из строки '" << coords << "' -> (" << x << ", " << y << ")" << std::endl;
        return Point(x, y);
    }
    
    double getX() const { return x_; }
    double getY() const { return y_; }
    
    void print() const {
        std::cout << "Point(" << x_ << ", " << y_ << ")" << std::endl;
    }
};

// ============================================================================
// REGISTRY-BASED FACTORY
// ============================================================================

/**
 * @brief Фабрика на основе реестра для динамической регистрации типов
 */
template<typename BaseType>
class FactoryRegistry {
private:
    std::map<std::string, std::function<std::unique_ptr<BaseType>()>> creators_;
    
public:
    template<typename DerivedType>
    void registerType(const std::string& name) {
        creators_[name] = []() {
            return std::make_unique<DerivedType>();
        };
        std::cout << "Зарегистрирован тип: " << name << std::endl;
    }
    
    template<typename DerivedType, typename... Args>
    void registerType(const std::string& name, Args&&... args) {
        creators_[name] = [args...]() {
            return std::make_unique<DerivedType>(args...);
        };
        std::cout << "Зарегистрирован тип с параметрами: " << name << std::endl;
    }
    
    std::unique_ptr<BaseType> create(const std::string& name) {
        auto it = creators_.find(name);
        if (it != creators_.end()) {
            std::cout << "Создаем объект типа: " << name << std::endl;
            return it->second();
        }
        throw std::invalid_argument("Неизвестный тип: " + name);
    }
    
    std::vector<std::string> getRegisteredTypes() const {
        std::vector<std::string> types;
        for (const auto& pair : creators_) {
            types.push_back(pair.first);
        }
        return types;
    }
    
    bool isRegistered(const std::string& name) const {
        return creators_.find(name) != creators_.end();
    }
};

// ============================================================================
// ДЕМОНСТРАЦИОННЫЕ ФУНКЦИИ
// ============================================================================

/**
 * @brief Демонстрация базового Factory Method
 */
void demonstrateBasicFactoryMethod() {
    std::cout << "\n=== Базовый Factory Method ===" << std::endl;
    
    auto factory = std::make_unique<StandardDocumentFactory>();
    
    // Создаем различные типы документов
    auto pdfDoc = factory->createDocument("PDF");
    auto wordDoc = factory->createDocument("Word");
    auto excelDoc = factory->createDocument("Excel");
    
    // Работаем с документами
    std::vector<std::unique_ptr<Document>> documents;
    documents.push_back(std::move(pdfDoc));
    documents.push_back(std::move(wordDoc));
    documents.push_back(std::move(excelDoc));
    
    for (auto& doc : documents) {
        std::cout << "\nРабота с " << doc->getType() << " документом:" << std::endl;
        doc->open();
        doc->save();
        doc->print();
        doc->close();
    }
    
    // Показываем поддерживаемые типы
    auto supportedTypes = factory->getSupportedTypes();
    std::cout << "\nПоддерживаемые типы: ";
    for (const auto& type : supportedTypes) {
        std::cout << type << " ";
    }
    std::cout << std::endl;
}

/**
 * @brief Демонстрация расширенной фабрики
 */
void demonstrateAdvancedFactory() {
    std::cout << "\n=== Расширенная фабрика ===" << std::endl;
    
    auto factory = std::make_unique<AdvancedDocumentFactory>("/projects/docs/");
    
    auto doc = factory->createDocument("PDF");
    doc->open();
    doc->save();
    doc->close();
}

/**
 * @brief Демонстрация Static Factory Methods
 */
void demonstrateStaticFactoryMethods() {
    std::cout << "\n=== Static Factory Methods ===" << std::endl;
    
    // Различные способы создания точек
    auto point1 = Point::fromCartesian(3.0, 4.0);
    auto point2 = Point::fromPolar(5.0, 0.6435); // 5, ~37 градусов
    auto point3 = Point::origin();
    auto point4 = Point::fromString("2.5,3.7");
    
    std::cout << "\nСозданные точки:" << std::endl;
    point1.print();
    point2.print();
    point3.print();
    point4.print();
}

/**
 * @brief Демонстрация Registry-based Factory
 */
void demonstrateRegistryFactory() {
    std::cout << "\n=== Registry-based Factory ===" << std::endl;
    
    FactoryRegistry<Document> registry;
    
    // Регистрируем типы
    registry.registerType<PDFDocument>("pdf");
    registry.registerType<WordDocument>("word");
    registry.registerType<ExcelDocument>("excel");
    
    // Регистрируем с параметрами
    registry.registerType<PDFDocument>("custom_pdf", "custom.pdf");
    
    // Показываем зарегистрированные типы
    auto types = registry.getRegisteredTypes();
    std::cout << "\nЗарегистрированные типы: ";
    for (const auto& type : types) {
        std::cout << type << " ";
    }
    std::cout << std::endl;
    
    // Создаем объекты
    auto doc1 = registry.create("pdf");
    auto doc2 = registry.create("word");
    auto doc3 = registry.create("custom_pdf");
    
    doc1->open();
    doc2->open();
    doc3->open();
    
    doc1->close();
    doc2->close();
    doc3->close();
}

/**
 * @brief Демонстрация обработки ошибок
 */
void demonstrateErrorHandling() {
    std::cout << "\n=== Обработка ошибок ===" << std::endl;
    
    auto factory = std::make_unique<StandardDocumentFactory>();
    
    try {
        auto doc = factory->createDocument("UnknownType");
        doc->open();
    } catch (const std::invalid_argument& e) {
        std::cout << "Ошибка создания: " << e.what() << std::endl;
    }
    
    try {
        auto doc = factory->createDocument("PDF");
        // Не открываем документ
        doc->save(); // Это должно вызвать исключение
    } catch (const std::runtime_error& e) {
        std::cout << "Ошибка операции: " << e.what() << std::endl;
    }
}

// ============================================================================
// ОСНОВНАЯ ФУНКЦИЯ
// ============================================================================

int main() {
    std::cout << "🏭 Демонстрация паттерна Factory Method" << std::endl;
    std::cout << std::string(50, '=') << std::endl;
    
    demonstrateBasicFactoryMethod();
    demonstrateAdvancedFactory();
    demonstrateStaticFactoryMethods();
    demonstrateRegistryFactory();
    demonstrateErrorHandling();
    
    std::cout << "\n✅ Демонстрация Factory Method завершена!" << std::endl;
    std::cout << "\n🎯 Ключевые выводы:" << std::endl;
    std::cout << "• Factory Method инкапсулирует создание объектов" << std::endl;
    std::cout << "• Используйте полиморфизм для создания объектов" << std::endl;
    std::cout << "• Static Factory Methods для простых случаев" << std::endl;
    std::cout << "• Registry-based Factory для динамической регистрации" << std::endl;
    std::cout << "• Всегда обрабатывайте ошибки создания объектов" << std::endl;
    
    return 0;
}
