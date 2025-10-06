#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <sstream>

/**
 * @file adapter_pattern.cpp
 * @brief Демонстрация паттерна Adapter
 * 
 * Этот файл показывает различные реализации паттерна Adapter,
 * от базовых до продвинутых с использованием современных возможностей C++.
 */

// ============================================================================
// БАЗОВЫЙ ПРИМЕР: ADAPTER PATTERN
// ============================================================================

/**
 * @brief Целевой интерфейс - то, что ожидает клиент
 */
class Target {
public:
    virtual ~Target() = default;
    virtual void request() = 0;
};

/**
 * @brief Адаптируемый класс - существующий класс с несовместимым интерфейсом
 */
class Adaptee {
public:
    void specificRequest() {
        std::cout << "Adaptee: Выполняем специфический запрос" << std::endl;
    }
    
    void anotherSpecificMethod(const std::string& data) {
        std::cout << "Adaptee: Обрабатываем данные: " << data << std::endl;
    }
};

/**
 * @brief Object Adapter - использует композицию
 */
class ObjectAdapter : public Target {
private:
    std::unique_ptr<Adaptee> adaptee_;
    
public:
    ObjectAdapter() : adaptee_(std::make_unique<Adaptee>()) {}
    
    void request() override {
        std::cout << "ObjectAdapter: Адаптируем запрос" << std::endl;
        adaptee_->specificRequest();
    }
    
    void adaptedMethod(const std::string& data) {
        std::cout << "ObjectAdapter: Адаптируем метод с данными" << std::endl;
        adaptee_->anotherSpecificMethod(data);
    }
};

// ============================================================================
// ПРАКТИЧЕСКИЙ ПРИМЕР: АДАПТАЦИЯ БАЗЫ ДАННЫХ
// ============================================================================

/**
 * @brief Интерфейс для работы с базой данных
 */
class Database {
public:
    virtual ~Database() = default;
    virtual void connect() = 0;
    virtual void executeQuery(const std::string& query) = 0;
    virtual void disconnect() = 0;
};

/**
 * @brief Legacy MySQL драйвер с другим интерфейсом
 */
class MySQLDriver {
public:
    void openConnection() {
        std::cout << "MySQL: Открываем соединение с базой данных" << std::endl;
    }
    
    void runQuery(const std::string& sql) {
        std::cout << "MySQL: Выполняем SQL запрос: " << sql << std::endl;
    }
    
    void closeConnection() {
        std::cout << "MySQL: Закрываем соединение с базой данных" << std::endl;
    }
    
    void setConnectionString(const std::string& connectionString) {
        std::cout << "MySQL: Устанавливаем строку подключения: " << connectionString << std::endl;
    }
};

/**
 * @brief Адаптер для MySQL драйвера
 */
class MySQLAdapter : public Database {
private:
    std::unique_ptr<MySQLDriver> driver_;
    std::string connectionString_;
    
public:
    explicit MySQLAdapter(const std::string& connectionString = "localhost:3306")
        : driver_(std::make_unique<MySQLDriver>()), connectionString_(connectionString) {
        driver_->setConnectionString(connectionString_);
    }
    
    void connect() override {
        driver_->openConnection();
    }
    
    void executeQuery(const std::string& query) override {
        driver_->runQuery(query);
    }
    
    void disconnect() override {
        driver_->closeConnection();
    }
};

/**
 * @brief PostgreSQL драйвер (еще один legacy драйвер)
 */
class PostgreSQLDriver {
public:
    void establishConnection() {
        std::cout << "PostgreSQL: Устанавливаем соединение" << std::endl;
    }
    
    void executeSQL(const std::string& sql) {
        std::cout << "PostgreSQL: Выполняем SQL: " << sql << std::endl;
    }
    
    void terminateConnection() {
        std::cout << "PostgreSQL: Завершаем соединение" << std::endl;
    }
    
    void configureConnection(const std::map<std::string, std::string>& config) {
        std::cout << "PostgreSQL: Настраиваем соединение" << std::endl;
        for (const auto& pair : config) {
            std::cout << "  " << pair.first << " = " << pair.second << std::endl;
        }
    }
};

/**
 * @brief Адаптер для PostgreSQL драйвера
 */
class PostgreSQLAdapter : public Database {
private:
    std::unique_ptr<PostgreSQLDriver> driver_;
    std::map<std::string, std::string> config_;
    
public:
    explicit PostgreSQLAdapter(const std::map<std::string, std::string>& config = {}) 
        : driver_(std::make_unique<PostgreSQLDriver>()) {
        
        // Устанавливаем конфигурацию по умолчанию
        config_ = config;
        if (config_.find("host") == config_.end()) {
            config_["host"] = "localhost";
        }
        if (config_.find("port") == config_.end()) {
            config_["port"] = "5432";
        }
        
        driver_->configureConnection(config_);
    }
    
    void connect() override {
        driver_->establishConnection();
    }
    
    void executeQuery(const std::string& query) override {
        driver_->executeSQL(query);
    }
    
    void disconnect() override {
        driver_->terminateConnection();
    }
};

// ============================================================================
// АДАПТАЦИЯ ФОРМАТОВ ДАННЫХ
// ============================================================================

/**
 * @brief Новый формат данных
 */
struct NewDataFormat {
    std::vector<std::string> fields;
    std::map<std::string, std::string> metadata;
    
    void print() const {
        std::cout << "NewDataFormat:" << std::endl;
        std::cout << "  Fields: ";
        for (const auto& field : fields) {
            std::cout << field << " ";
        }
        std::cout << std::endl;
        std::cout << "  Metadata:" << std::endl;
        for (const auto& pair : metadata) {
            std::cout << "    " << pair.first << " = " << pair.second << std::endl;
        }
    }
};

/**
 * @brief Старый формат данных
 */
struct OldDataFormat {
    std::string data;
    int type;
    bool isValid;
    
    void print() const {
        std::cout << "OldDataFormat:" << std::endl;
        std::cout << "  Data: " << data << std::endl;
        std::cout << "  Type: " << type << std::endl;
        std::cout << "  Valid: " << (isValid ? "true" : "false") << std::endl;
    }
};

/**
 * @brief Legacy система обработки данных
 */
class LegacyDataProcessor {
public:
    void processData(const OldDataFormat& data) {
        if (!data.isValid) {
            std::cout << "LegacyProcessor: Данные невалидны, пропускаем обработку" << std::endl;
            return;
        }
        
        std::cout << "LegacyProcessor: Обрабатываем данные типа " << data.type << std::endl;
        std::cout << "LegacyProcessor: Содержимое: " << data.data << std::endl;
    }
};

/**
 * @brief Адаптер для преобразования форматов данных
 */
class DataFormatAdapter {
private:
    LegacyDataProcessor& processor_;
    
public:
    explicit DataFormatAdapter(LegacyDataProcessor& processor) : processor_(processor) {}
    
    void processData(const NewDataFormat& newData) {
        std::cout << "DataFormatAdapter: Преобразуем новые данные в старый формат" << std::endl;
        
        // Преобразуем новые данные в старый формат
        OldDataFormat oldData = convertToOldFormat(newData);
        
        // Передаем в legacy систему
        processor_.processData(oldData);
    }
    
private:
    OldDataFormat convertToOldFormat(const NewDataFormat& newData) {
        OldDataFormat oldData;
        
        // Объединяем поля в строку
        std::ostringstream oss;
        for (size_t i = 0; i < newData.fields.size(); ++i) {
            if (i > 0) oss << "|";
            oss << newData.fields[i];
        }
        oldData.data = oss.str();
        
        // Определяем тип из метаданных
        auto typeIt = newData.metadata.find("type");
        oldData.type = (typeIt != newData.metadata.end()) ? std::stoi(typeIt->second) : 0;
        
        // Проверяем валидность
        auto validIt = newData.metadata.find("valid");
        oldData.isValid = (validIt != newData.metadata.end()) ? (validIt->second == "true") : true;
        
        return oldData;
    }
};

// ============================================================================
// СОВРЕМЕННЫЙ ПОДХОД: TEMPLATE ADAPTER
// ============================================================================

/**
 * @brief Шаблонный адаптер для различных типов
 */
template<typename AdapteeType>
class TemplateAdapter {
private:
    AdapteeType adaptee_;
    
public:
    template<typename... Args>
    explicit TemplateAdapter(Args&&... args) 
        : adaptee_(std::forward<Args>(args)...) {}
    
    template<typename... Args>
    void adaptedMethod(Args&&... args) {
        std::cout << "TemplateAdapter: Адаптируем метод" << std::endl;
        adaptee_.originalMethod(std::forward<Args>(args)...);
    }
    
    // Адаптация с преобразованием параметров
    template<typename T>
    void adaptedMethodWithConversion(const T& data) {
        std::cout << "TemplateAdapter: Адаптируем метод с преобразованием" << std::endl;
        auto converted = convertData(data);
        adaptee_.originalMethod(converted);
    }
    
private:
    template<typename T>
    auto convertData(const T& data) -> decltype(adaptee_.originalMethod(data)) {
        // Простое преобразование - можно переопределить для конкретных типов
        return data;
    }
};

/**
 * @brief Пример класса для адаптации
 */
class LegacyClass {
public:
    void originalMethod(int value) {
        std::cout << "LegacyClass: Обрабатываем значение: " << value << std::endl;
    }
    
    void originalMethod(const std::string& data) {
        std::cout << "LegacyClass: Обрабатываем строку: " << data << std::endl;
    }
};

// ============================================================================
// ДЕМОНСТРАЦИОННЫЕ ФУНКЦИИ
// ============================================================================

/**
 * @brief Демонстрация базового Adapter паттерна
 */
void demonstrateBasicAdapter() {
    std::cout << "\n=== Базовый Adapter Pattern ===" << std::endl;
    
    // Создаем адаптер
    auto adapter = std::make_unique<ObjectAdapter>();
    
    // Используем через целевой интерфейс
    adapter->request();
    
    // Используем адаптированный метод
    adapter->adaptedMethod("Тестовые данные");
}

/**
 * @brief Демонстрация адаптеров для баз данных
 */
void demonstrateDatabaseAdapters() {
    std::cout << "\n=== Адаптеры для баз данных ===" << std::endl;
    
    // Создаем адаптеры для разных БД
    auto mysqlAdapter = std::make_unique<MySQLAdapter>("mysql://localhost:3306/mydb");
    auto postgresAdapter = std::make_unique<PostgreSQLAdapter>(
        std::map<std::string, std::string>{
            {"host", "localhost"},
            {"port", "5432"},
            {"database", "mydb"}
        });
    
    // Используем единый интерфейс для разных БД
    std::vector<std::unique_ptr<Database>> databases;
    databases.push_back(std::move(mysqlAdapter));
    databases.push_back(std::move(postgresAdapter));
    
    for (auto& db : databases) {
        std::cout << "\n--- Работа с базой данных ---" << std::endl;
        db->connect();
        db->executeQuery("SELECT * FROM users");
        db->executeQuery("INSERT INTO users (name) VALUES ('John')");
        db->disconnect();
    }
}

/**
 * @brief Демонстрация адаптации форматов данных
 */
void demonstrateDataFormatAdapter() {
    std::cout << "\n=== Адаптация форматов данных ===" << std::endl;
    
    // Создаем legacy процессор
    LegacyDataProcessor legacyProcessor;
    DataFormatAdapter adapter(legacyProcessor);
    
    // Создаем новые данные
    NewDataFormat newData;
    newData.fields = {"John", "Doe", "john@example.com"};
    newData.metadata = {
        {"type", "1"},
        {"valid", "true"},
        {"source", "new_system"}
    };
    
    std::cout << "Исходные данные:" << std::endl;
    newData.print();
    
    std::cout << "\nОбработка через адаптер:" << std::endl;
    adapter.processData(newData);
}

/**
 * @brief Демонстрация шаблонного адаптера
 */
void demonstrateTemplateAdapter() {
    std::cout << "\n=== Шаблонный адаптер ===" << std::endl;
    
    // Создаем адаптер для LegacyClass
    TemplateAdapter<LegacyClass> adapter;
    
    // Используем адаптированные методы
    adapter.adaptedMethod(42);
    adapter.adaptedMethod("Hello, World!");
    
    // Используем адаптацию с преобразованием
    adapter.adaptedMethodWithConversion(100);
}

/**
 * @brief Демонстрация работы с различными адаптерами
 */
void demonstrateMultipleAdapters() {
    std::cout << "\n=== Множественные адаптеры ===" << std::endl;
    
    // Создаем различные адаптеры
    auto mysqlAdapter = std::make_unique<MySQLAdapter>();
    auto postgresAdapter = std::make_unique<PostgreSQLAdapter>();
    
    // Используем полиморфизм
    std::vector<std::unique_ptr<Database>> databases;
    databases.push_back(std::move(mysqlAdapter));
    databases.push_back(std::move(postgresAdapter));
    
    std::cout << "Работа с несколькими базами данных:" << std::endl;
    for (size_t i = 0; i < databases.size(); ++i) {
        std::cout << "\nБаза данных " << (i + 1) << ":" << std::endl;
        databases[i]->connect();
        databases[i]->executeQuery("SELECT version()");
        databases[i]->disconnect();
    }
}

// ============================================================================
// ОСНОВНАЯ ФУНКЦИЯ
// ============================================================================

int main() {
    std::cout << "🔌 Демонстрация паттерна Adapter" << std::endl;
    std::cout << std::string(50, '=') << std::endl;
    
    demonstrateBasicAdapter();
    demonstrateDatabaseAdapters();
    demonstrateDataFormatAdapter();
    demonstrateTemplateAdapter();
    demonstrateMultipleAdapters();
    
    std::cout << "\n✅ Демонстрация Adapter Pattern завершена!" << std::endl;
    std::cout << "\n🎯 Ключевые выводы:" << std::endl;
    std::cout << "• Adapter позволяет интегрировать несовместимые интерфейсы" << std::endl;
    std::cout << "• Object Adapter использует композицию (предпочтительно)" << std::endl;
    std::cout << "• Шаблонные адаптеры обеспечивают переиспользование кода" << std::endl;
    std::cout << "• Адаптеры скрывают сложность преобразования" << std::endl;
    std::cout << "• Паттерн особенно полезен для интеграции legacy систем" << std::endl;
    
    return 0;
}
