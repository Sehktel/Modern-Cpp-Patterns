#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <cstring>
#include <limits>

/**
 * @file builder_vulnerabilities.cpp
 * @brief Уязвимые реализации паттерна Builder
 * 
 * ⚠️  ТОЛЬКО ДЛЯ ОБРАЗОВАТЕЛЬНЫХ ЦЕЛЕЙ!
 */

// ============================================================================
// УЯЗВИМОСТЬ 1: BUFFER OVERFLOW ПРИ ПОСТРОЕНИИ ОБЪЕКТА
// Проблема: Нет проверки границ при копировании данных
// ============================================================================

class Message {
public:
    char header[64];
    char body[256];
    char footer[32];
    
    void display() const {
        std::cout << "Header: " << header << "\n";
        std::cout << "Body: " << body << "\n";
        std::cout << "Footer: " << footer << "\n";
    }
};

// УЯЗВИМЫЙ BUILDER
class VulnerableMessageBuilder {
private:
    Message msg_;
    
public:
    VulnerableMessageBuilder& setHeader(const char* header) {
        // ОПАСНО: Нет проверки длины!
        strcpy(msg_.header, header);  // BUFFER OVERFLOW!
        return *this;
    }
    
    VulnerableMessageBuilder& setBody(const char* body) {
        strcpy(msg_.body, body);  // BUFFER OVERFLOW!
        return *this;
    }
    
    VulnerableMessageBuilder& setFooter(const char* footer) {
        strcpy(msg_.footer, footer);  // BUFFER OVERFLOW!
        return *this;
    }
    
    Message build() {
        return msg_;
    }
};

void demonstrateBufferOverflow() {
    std::cout << "\n=== УЯЗВИМОСТЬ 1: Buffer Overflow ===\n";
    
    VulnerableMessageBuilder builder;
    
    // Легитимное использование
    builder.setHeader("Hello");
    
    // АТАКА: Переполнение буфера header
    char overflow[128];
    memset(overflow, 'A', 127);
    overflow[127] = '\0';
    
    std::cout << "Попытка переполнения header (127 символов)...\n";
    builder.setHeader(overflow);  // BUFFER OVERFLOW!
    
    auto msg = builder.build();
    msg.display();
    
    std::cout << "⚠️  Buffer overflow может перезаписать соседние поля!\n";
}

// ============================================================================
// УЯЗВИМОСТЬ 2: INTEGER OVERFLOW В РАЗМЕРАХ
// Проблема: Переполнение при вычислении размера аллокации
// ============================================================================

class DataBlock {
public:
    char* data;
    size_t size;
    
    DataBlock() : data(nullptr), size(0) {}
    ~DataBlock() { delete[] data; }
};

class VulnerableDataBlockBuilder {
private:
    size_t chunk_size_;
    size_t chunk_count_;
    
public:
    VulnerableDataBlockBuilder() : chunk_size_(0), chunk_count_(0) {}
    
    VulnerableDataBlockBuilder& setChunkSize(size_t size) {
        chunk_size_ = size;
        return *this;
    }
    
    VulnerableDataBlockBuilder& setChunkCount(size_t count) {
        chunk_count_ = count;
        return *this;
    }
    
    DataBlock build() {
        DataBlock block;
        
        // УЯЗВИМОСТЬ: Integer overflow!
        block.size = chunk_size_ * chunk_count_;  // Может переполниться!
        
        if (block.size == 0) {
            std::cout << "⚠️  Integer overflow: size = 0\n";
            return block;
        }
        
        std::cout << "Выделение " << block.size << " байт\n";
        block.data = new char[block.size];  // Может выделить МЕНЬШЕ чем ожидалось!
        
        return block;
    }
};

void demonstrateIntegerOverflow() {
    std::cout << "\n=== УЯЗВИМОСТЬ 2: Integer Overflow ===\n";
    
    VulnerableDataBlockBuilder builder;
    
    // АТАКА: Переполнение size_t
    size_t huge_size = std::numeric_limits<size_t>::max() / 2 + 1;
    
    std::cout << "Chunk size: " << huge_size << "\n";
    std::cout << "Chunk count: 2\n";
    std::cout << "Expected total: " << (huge_size * 2) << " (overflow!)\n";
    
    auto block = builder
        .setChunkSize(huge_size)
        .setChunkCount(2)
        .build();
    
    std::cout << "⚠️  Выделено меньше памяти чем планировалось!\n";
}

// ============================================================================
// УЯЗВИМОСТЬ 3: VALIDATION BYPASS ЧЕРЕЗ METHOD CHAINING
// Проблема: Валидация только в build(), можно обойти
// ============================================================================

class UserAccount {
public:
    std::string username;
    std::string email;
    int age;
    bool is_admin;
    
    void display() const {
        std::cout << "User: " << username << "\n";
        std::cout << "Email: " << email << "\n";
        std::cout << "Age: " << age << "\n";
        std::cout << "Admin: " << (is_admin ? "Yes" : "No") << "\n";
    }
};

class VulnerableUserBuilder {
private:
    UserAccount account_;
    
public:
    VulnerableUserBuilder() {
        account_.age = 0;
        account_.is_admin = false;
    }
    
    VulnerableUserBuilder& setUsername(const std::string& name) {
        account_.username = name;
        return *this;
    }
    
    VulnerableUserBuilder& setEmail(const std::string& email) {
        account_.email = email;
        return *this;
    }
    
    VulnerableUserBuilder& setAge(int age) {
        // УЯЗВИМОСТЬ: Нет валидации в setter!
        account_.age = age;
        return *this;
    }
    
    VulnerableUserBuilder& setAdmin(bool admin) {
        // УЯЗВИМОСТЬ: Любой может установить admin=true!
        account_.is_admin = admin;
        return *this;
    }
    
    UserAccount build() {
        // Валидация только здесь - слишком поздно!
        if (account_.age < 0 || account_.age > 150) {
            std::cout << "⚠️  Некорректный возраст (но уже установлен)\n";
        }
        
        return account_;
    }
};

void demonstrateValidationBypass() {
    std::cout << "\n=== УЯЗВИМОСТЬ 3: Validation Bypass ===\n";
    
    VulnerableUserBuilder builder;
    
    // АТАКА: Установка некорректных значений
    auto account = builder
        .setUsername("attacker")
        .setEmail("invalid-email")  // Нет проверки формата!
        .setAge(-1)  // Отрицательный возраст!
        .setAdmin(true)  // Обычный пользователь получает admin!
        .build();
    
    account.display();
    
    std::cout << "\n⚠️  Валидация обойдена через method chaining!\n";
}

// ============================================================================
// УЯЗВИМОСТЬ 4: USE OF INCOMPLETE OBJECTS
// Проблема: Объект используется до завершения построения
// ============================================================================

class Database {
public:
    std::string host;
    int port;
    std::string username;
    std::string password;
    bool is_connected;
    
    Database() : port(0), is_connected(false) {}
    
    void connect() {
        if (host.empty() || username.empty()) {
            std::cout << "❌ Не удалось подключиться: неполная конфигурация\n";
            return;
        }
        is_connected = true;
        std::cout << "✅ Подключено к " << host << ":" << port << "\n";
    }
};

class VulnerableDatabaseBuilder {
private:
    Database* db_;  // ОПАСНО: Сырой указатель на неполный объект!
    
public:
    VulnerableDatabaseBuilder() {
        db_ = new Database();
    }
    
    VulnerableDatabaseBuilder& setHost(const std::string& host) {
        db_->host = host;
        return *this;
    }
    
    VulnerableDatabaseBuilder& setPort(int port) {
        db_->port = port;
        return *this;
    }
    
    VulnerableDatabaseBuilder& setCredentials(const std::string& user, const std::string& pass) {
        db_->username = user;
        db_->password = pass;
        return *this;
    }
    
    // УЯЗВИМОСТЬ: Можно получить неполный объект!
    Database* getPartial() {
        return db_;  // Объект может быть не готов!
    }
    
    Database build() {
        Database result = *db_;
        delete db_;
        db_ = new Database();
        return result;
    }
    
    ~VulnerableDatabaseBuilder() {
        delete db_;
    }
};

void demonstrateIncompleteObject() {
    std::cout << "\n=== УЯЗВИМОСТЬ 4: Use of Incomplete Objects ===\n";
    
    VulnerableDatabaseBuilder builder;
    
    builder.setHost("localhost");
    // Не установили port и credentials!
    
    // АТАКА: Получаем неполный объект
    Database* partial = builder.getPartial();
    std::cout << "Попытка подключения с неполными данными:\n";
    partial->connect();  // Работает с неполным объектом!
    
    std::cout << "\n⚠️  Объект использован до завершения построения!\n";
}

// ============================================================================
// УЯЗВИМОСТЬ 5: STATE POLLUTION МЕЖДУ ВЫЗОВАМИ BUILD()
// Проблема: Builder не очищает state между вызовами
// ============================================================================

class Request {
public:
    std::string method;
    std::string url;
    std::vector<std::string> headers;
    std::string body;
    
    void display() const {
        std::cout << method << " " << url << "\n";
        for (const auto& h : headers) {
            std::cout << "  " << h << "\n";
        }
        std::cout << "Body: " << body << "\n";
    }
};

class VulnerableRequestBuilder {
private:
    Request request_;
    
public:
    VulnerableRequestBuilder& setMethod(const std::string& method) {
        request_.method = method;
        return *this;
    }
    
    VulnerableRequestBuilder& setUrl(const std::string& url) {
        request_.url = url;
        return *this;
    }
    
    VulnerableRequestBuilder& addHeader(const std::string& header) {
        request_.headers.push_back(header);  // НАКАПЛИВАЮТСЯ!
        return *this;
    }
    
    VulnerableRequestBuilder& setBody(const std::string& body) {
        request_.body = body;
        return *this;
    }
    
    Request build() {
        // УЯЗВИМОСТЬ: Не очищает state!
        return request_;  // Копирует, но state остается
    }
};

void demonstrateStatePollution() {
    std::cout << "\n=== УЯЗВИМОСТЬ 5: State Pollution ===\n";
    
    VulnerableRequestBuilder builder;
    
    // Первый запрос
    auto req1 = builder
        .setMethod("GET")
        .setUrl("/api/public")
        .addHeader("Authorization: Bearer token123")
        .build();
    
    std::cout << "Запрос 1:\n";
    req1.display();
    
    // Второй запрос - builder НЕ очищен!
    auto req2 = builder
        .setMethod("POST")
        .setUrl("/api/admin")
        .setBody("{\"action\":\"delete\"}")
        .build();
    
    std::cout << "\nЗапрос 2:\n";
    req2.display();
    
    std::cout << "\n⚠️  Authorization header утек во второй запрос!\n";
}

// ============================================================================
// УЯЗВИМОСТЬ 6: INJECTION ЧЕРЕЗ BUILDER PARAMETERS
// Проблема: Параметры builder'а используются в SQL/Shell без экранирования
// ============================================================================

class Query {
public:
    std::string table;
    std::string where_clause;
    std::vector<std::string> columns;
    
    std::string toSQL() const {
        std::string sql = "SELECT ";
        
        if (columns.empty()) {
            sql += "*";
        } else {
            for (size_t i = 0; i < columns.size(); ++i) {
                sql += columns[i];
                if (i < columns.size() - 1) sql += ", ";
            }
        }
        
        sql += " FROM " + table;
        
        if (!where_clause.empty()) {
            sql += " WHERE " + where_clause;  // INJECTION!
        }
        
        return sql;
    }
};

class VulnerableQueryBuilder {
private:
    Query query_;
    
public:
    VulnerableQueryBuilder& from(const std::string& table) {
        query_.table = table;  // Нет валидации!
        return *this;
    }
    
    VulnerableQueryBuilder& select(const std::string& column) {
        query_.columns.push_back(column);  // Нет валидации!
        return *this;
    }
    
    VulnerableQueryBuilder& where(const std::string& condition) {
        query_.where_clause = condition;  // SQL INJECTION!
        return *this;
    }
    
    Query build() {
        return query_;
    }
};

void demonstrateSQLInjection() {
    std::cout << "\n=== УЯЗВИМОСТЬ 6: SQL Injection через Builder ===\n";
    
    VulnerableQueryBuilder builder;
    
    // Легитимный запрос
    auto query1 = builder
        .from("users")
        .select("username")
        .where("id = 1")
        .build();
    
    std::cout << "Легитимный SQL:\n" << query1.toSQL() << "\n\n";
    
    // АТАКА: SQL Injection
    VulnerableQueryBuilder evil_builder;
    auto query2 = evil_builder
        .from("users")
        .select("username")
        .where("1=1 OR id=1; DROP TABLE users; --")  // INJECTION!
        .build();
    
    std::cout << "Вредоносный SQL:\n" << query2.toSQL() << "\n";
    
    std::cout << "\n⚠️  SQL injection через параметры builder!\n";
}

// ============================================================================
// MAIN
// ============================================================================

int main() {
    std::cout << "=== УЯЗВИМОСТИ BUILDER PATTERN ===\n";
    std::cout << "⚠️  ВНИМАНИЕ: Этот код содержит уязвимости для обучения!\n";
    
    try {
    demonstrateBufferOverflow();
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << "\n";
    }
    
    try {
    demonstrateIntegerOverflow();
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << "\n";
    }
    
    try {
        demonstrateValidationBypass();
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << "\n";
    }
    
    try {
        demonstrateIncompleteObject();
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << "\n";
    }
    
    try {
        demonstrateStatePollution();
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << "\n";
    }
    
    try {
        demonstrateSQLInjection();
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << "\n";
    }
    
    std::cout << "\n=== ИНСТРУМЕНТЫ АНАЛИЗА ===\n";
    std::cout << "• AddressSanitizer: g++ -fsanitize=address builder_vulnerabilities.cpp\n";
    std::cout << "• UBSanitizer: g++ -fsanitize=undefined builder_vulnerabilities.cpp\n";
    std::cout << "• Valgrind: valgrind --tool=memcheck ./builder_vulnerabilities\n";
    std::cout << "• Static Analysis: clang --analyze builder_vulnerabilities.cpp\n";
    
    return 0;
}
