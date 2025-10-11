#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <stdexcept>
#include <regex>

/**
 * @file secure_builder_alternatives.cpp
 * @brief Безопасные реализации паттерна Builder
 */

// ============================================================================
// БЕЗОПАСНАЯ РЕАЛИЗАЦИЯ 1: BUILDER С BOUNDS CHECKING
// Решает: Buffer Overflow
// ============================================================================

class SafeMessage {
private:
    std::string header_;
    std::string body_;
    std::string footer_;
    
public:
    SafeMessage(std::string h, std::string b, std::string f)
        : header_(std::move(h)), body_(std::move(b)), footer_(std::move(f)) {}
    
    void display() const {
        std::cout << "Header: " << header_ << "\n";
        std::cout << "Body: " << body_ << "\n";
        std::cout << "Footer: " << footer_ << "\n";
    }
};

class SafeMessageBuilder {
private:
    std::string header_;
    std::string body_;
    std::string footer_;
    
    static constexpr size_t MAX_HEADER = 64;
    static constexpr size_t MAX_BODY = 256;
    static constexpr size_t MAX_FOOTER = 32;
    
public:
    SafeMessageBuilder& setHeader(const std::string& header) {
        if (header.length() > MAX_HEADER) {
            throw std::length_error("Header exceeds maximum length");
        }
        header_ = header;
        return *this;
    }
    
    SafeMessageBuilder& setBody(const std::string& body) {
        if (body.length() > MAX_BODY) {
            throw std::length_error("Body exceeds maximum length");
        }
        body_ = body;
        return *this;
    }
    
    SafeMessageBuilder& setFooter(const std::string& footer) {
        if (footer.length() > MAX_FOOTER) {
            throw std::length_error("Footer exceeds maximum length");
        }
        footer_ = footer;
        return *this;
    }
    
    SafeMessage build() {
        return SafeMessage(header_, body_, footer_);
    }
};

void demonstrateSafeBoundsChecking() {
    std::cout << "\n=== БЕЗОПАСНАЯ РЕАЛИЗАЦИЯ 1: Bounds Checking ===\n";
    
    SafeMessageBuilder builder;
    
    try {
        std::string huge(100, 'A');
        builder.setHeader(huge);  // Вызовет exception
    } catch (const std::length_error& e) {
        std::cout << "✅ Блокировано: " << e.what() << "\n";
    }
    
    auto msg = builder
        .setHeader("Valid")
        .setBody("Content")
        .setFooter("End")
        .build();
    
    msg.display();
    std::cout << "✅ Безопасно: все проверки пройдены\n";
}

// ============================================================================
// БЕЗОПАСНАЯ РЕАЛИЗАЦИЯ 2: BUILDER С OVERFLOW PROTECTION
// Решает: Integer Overflow
// ============================================================================

class SafeDataBlock {
private:
    std::vector<char> data_;
    
public:
    explicit SafeDataBlock(std::vector<char> data) : data_(std::move(data)) {}
    size_t size() const { return data_.size(); }
};

class SafeDataBlockBuilder {
private:
    size_t chunk_size_ = 0;
    size_t chunk_count_ = 0;
    
    static constexpr size_t MAX_SIZE = 1024 * 1024 * 100;  // 100 MB limit
    
public:
    SafeDataBlockBuilder& setChunkSize(size_t size) {
        chunk_size_ = size;
        return *this;
    }
    
    SafeDataBlockBuilder& setChunkCount(size_t count) {
        chunk_count_ = count;
        return *this;
    }
    
    SafeDataBlock build() {
        // Проверка переполнения
        if (chunk_count_ > 0 && chunk_size_ > MAX_SIZE / chunk_count_) {
            throw std::overflow_error("Size calculation would overflow");
        }
        
        size_t total_size = chunk_size_ * chunk_count_;
        
        if (total_size > MAX_SIZE) {
            throw std::length_error("Total size exceeds maximum");
        }
        
        std::vector<char> data(total_size, 0);
        return SafeDataBlock(std::move(data));
    }
};

void demonstrateSafeOverflowProtection() {
    std::cout << "\n=== БЕЗОПАСНАЯ РЕАЛИЗАЦИЯ 2: Overflow Protection ===\n";
    
    SafeDataBlockBuilder builder;
    
    try {
        size_t huge = std::numeric_limits<size_t>::max() / 2 + 1;
        auto block = builder.setChunkSize(huge).setChunkCount(2).build();
    } catch (const std::overflow_error& e) {
        std::cout << "✅ Блокировано: " << e.what() << "\n";
    }
    
    auto block = builder.setChunkSize(1024).setChunkCount(100).build();
    std::cout << "✅ Создан блок размером " << block.size() << " байт\n";
}

// ============================================================================
// БЕЗОПАСНАЯ РЕАЛИЗАЦИЯ 3: BUILDER С VALIDATION В SETTERS
// Решает: Validation Bypass
// ============================================================================

class ValidatedUser {
private:
    std::string username_;
    std::string email_;
    int age_;
    bool is_admin_;
    
public:
    ValidatedUser(std::string u, std::string e, int a, bool admin)
        : username_(std::move(u)), email_(std::move(e)), age_(a), is_admin_(admin) {}
    
    void display() const {
        std::cout << "User: " << username_ << ", Email: " << email_ 
                  << ", Age: " << age_ << ", Admin: " << is_admin_ << "\n";
    }
};

class ValidatedUserBuilder {
private:
    std::string username_;
    std::string email_;
    int age_ = 0;
    bool is_admin_ = false;
    bool admin_approved_ = false;  // Требуется подтверждение
    
    static bool isValidEmail(const std::string& email) {
        std::regex pattern(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
        return std::regex_match(email, pattern);
    }
    
public:
    ValidatedUserBuilder& setUsername(const std::string& name) {
        if (name.empty() || name.length() > 50) {
            throw std::invalid_argument("Invalid username");
        }
        username_ = name;
        return *this;
    }
    
    ValidatedUserBuilder& setEmail(const std::string& email) {
        if (!isValidEmail(email)) {
            throw std::invalid_argument("Invalid email format");
        }
        email_ = email;
        return *this;
    }
    
    ValidatedUserBuilder& setAge(int age) {
        if (age < 0 || age > 150) {
            throw std::invalid_argument("Invalid age");
        }
        age_ = age;
        return *this;
    }
    
    // Требует специальное подтверждение
    ValidatedUserBuilder& setAdminWithApproval(bool admin, const std::string& approval_token) {
        if (admin && approval_token != "SECRET_ADMIN_TOKEN") {
            throw std::runtime_error("Unauthorized admin access");
        }
        is_admin_ = admin;
        admin_approved_ = true;
        return *this;
    }
    
    ValidatedUser build() {
        if (username_.empty() || email_.empty()) {
            throw std::runtime_error("Required fields missing");
        }
        return ValidatedUser(username_, email_, age_, is_admin_);
    }
};

void demonstrateValidatedBuilder() {
    std::cout << "\n=== БЕЗОПАСНАЯ РЕАЛИЗАЦИЯ 3: Validation в setters ===\n";
    
    ValidatedUserBuilder builder;
    
    try {
        builder.setEmail("invalid-email");
    } catch (const std::invalid_argument& e) {
        std::cout << "✅ Блокировано: " << e.what() << "\n";
    }
    
    try {
        builder.setAdminWithApproval(true, "wrong_token");
    } catch (const std::runtime_error& e) {
        std::cout << "✅ Блокировано: " << e.what() << "\n";
    }
    
    auto user = builder
        .setUsername("alice")
        .setEmail("alice@example.com")
        .setAge(25)
        .build();
    
    user.display();
    std::cout << "✅ Валидация на каждом этапе\n";
}

// ============================================================================
// БЕЗОПАСНАЯ РЕАЛИЗАЦИЯ 4: BUILDER С НЕВОЗМОЖНОСТЬЮ ПОЛУЧИТЬ PARTIAL OBJECT
// Решает: Use of Incomplete Objects
// ============================================================================

class SecureDatabase {
private:
    std::string host_;
    int port_;
    std::string username_;
    
public:
    SecureDatabase(std::string h, int p, std::string u)
        : host_(std::move(h)), port_(p), username_(std::move(u)) {}
    
    void connect() {
        std::cout << "✅ Подключено к " << host_ << ":" << port_ 
                  << " (user: " << username_ << ")\n";
    }
};

class SecureDatabaseBuilder {
private:
    std::optional<std::string> host_;
    std::optional<int> port_;
    std::optional<std::string> username_;
    
public:
    SecureDatabaseBuilder& setHost(const std::string& host) {
        host_ = host;
        return *this;
    }
    
    SecureDatabaseBuilder& setPort(int port) {
        if (port <= 0 || port > 65535) {
            throw std::invalid_argument("Invalid port");
        }
        port_ = port;
        return *this;
    }
    
    SecureDatabaseBuilder& setUsername(const std::string& user) {
        username_ = user;
        return *this;
    }
    
    // Можно получить только полный объект!
    SecureDatabase build() {
        if (!host_ || !port_ || !username_) {
            throw std::runtime_error("Incomplete configuration");
        }
        return SecureDatabase(*host_, *port_, *username_);
    }
    
    // НЕТ метода getPartial()!
};

void demonstrateCompleteObjectOnly() {
    std::cout << "\n=== БЕЗОПАСНАЯ РЕАЛИЗАЦИЯ 4: Только полные объекты ===\n";
    
    SecureDatabaseBuilder builder;
    
    builder.setHost("localhost");
    // Не установили порт и username
    
    try {
        auto db = builder.build();  // Вызовет exception
    } catch (const std::runtime_error& e) {
        std::cout << "✅ Блокировано: " << e.what() << "\n";
    }
    
    auto db = builder
        .setPort(5432)
        .setUsername("admin")
        .build();
    
    db.connect();
    std::cout << "✅ Объект создан только когда полностью готов\n";
}

// ============================================================================
// БЕЗОПАСНАЯ РЕАЛИЗАЦИЯ 5: BUILDER С AUTO-RESET
// Решает: State Pollution
// ============================================================================

class CleanRequest {
private:
    std::string method_;
    std::string url_;
    std::vector<std::string> headers_;
    
public:
    CleanRequest(std::string m, std::string u, std::vector<std::string> h)
        : method_(std::move(m)), url_(std::move(u)), headers_(std::move(h)) {}
    
    void display() const {
        std::cout << method_ << " " << url_ << "\n";
        for (const auto& h : headers_) std::cout << "  " << h << "\n";
    }
};

class CleanRequestBuilder {
private:
    std::string method_;
    std::string url_;
    std::vector<std::string> headers_;
    
    void reset() {
        method_.clear();
        url_.clear();
        headers_.clear();
    }
    
public:
    CleanRequestBuilder& setMethod(const std::string& method) {
        method_ = method;
        return *this;
    }
    
    CleanRequestBuilder& setUrl(const std::string& url) {
        url_ = url;
        return *this;
    }
    
    CleanRequestBuilder& addHeader(const std::string& header) {
        headers_.push_back(header);
        return *this;
    }
    
    CleanRequest build() {
        CleanRequest req(method_, url_, headers_);
        reset();  // Автоматически очищаем state!
        return req;
    }
};

void demonstrateCleanBuilder() {
    std::cout << "\n=== БЕЗОПАСНАЯ РЕАЛИЗАЦИЯ 5: Auto-Reset Builder ===\n";
    
    CleanRequestBuilder builder;
    
    auto req1 = builder
        .setMethod("GET")
        .setUrl("/api/public")
        .addHeader("Authorization: Bearer token123")
        .build();
    
    std::cout << "Запрос 1:\n";
    req1.display();
    
    auto req2 = builder
        .setMethod("POST")
        .setUrl("/api/admin")
        .build();
    
    std::cout << "\nЗапрос 2:\n";
    req2.display();
    
    std::cout << "\n✅ State автоматически очищен\n";
}

// ============================================================================
// MAIN
// ============================================================================

int main() {
    std::cout << "=== БЕЗОПАСНЫЕ РЕАЛИЗАЦИИ BUILDER PATTERN ===\n";
    
    demonstrateSafeBoundsChecking();
    demonstrateSafeOverflowProtection();
    demonstrateValidatedBuilder();
    demonstrateCompleteObjectOnly();
    demonstrateCleanBuilder();
    
    std::cout << "\n=== РЕКОМЕНДАЦИИ ===\n";
    std::cout << "✅ Проверяйте границы в каждом setter\n";
    std::cout << "✅ Защищайтесь от integer overflow\n";
    std::cout << "✅ Валидируйте на каждом этапе, не только в build()\n";
    std::cout << "✅ Не позволяйте получить неполный объект\n";
    std::cout << "✅ Очищайте state после build()\n";
    std::cout << "✅ Используйте std::string вместо char[]\n";
    std::cout << "✅ Применяйте std::optional для обязательных полей\n";
    
    return 0;
}
