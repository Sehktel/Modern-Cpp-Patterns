/**
 * @file dry_example.cpp
 * @brief Детальная демонстрация принципа DRY (Don't Repeat Yourself)
 * @author Senior C++ Developer
 * @date 2024
 * 
 * Этот файл демонстрирует различные аспекты принципа DRY:
 * - Выявление дублирования кода
 * - Методы устранения дублирования
 * - Компромиссы при применении DRY
 */

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cmath>

/**
 * @brief Математическая формализация DRY
 * 
 * DRY можно формализовать следующим образом:
 * ∀K ∈ Knowledge: |Representations(K)| = 1
 * где Representations(K) = {r | r представляет знание K в системе}
 * 
 * Другими словами: каждый фрагмент знания должен иметь единственное,
 * однозначное, авторитетное представление в системе.
 */

// ============================================================================
// ПРИМЕР 1: ДУБЛИРОВАНИЕ ЛОГИКИ ВАЛИДАЦИИ
// ============================================================================

/**
 * @brief Нарушение DRY: Дублирование логики валидации
 * 
 * Этот код нарушает DRY, так как логика валидации email, телефона и пароля
 * дублируется в разных местах.
 */
class BadUserValidator {
public:
    // Нарушение DRY: дублирование логики валидации email
    static bool isValidEmail(const std::string& email) {
        if (email.empty()) {
            std::cout << "❌ Email не может быть пустым\n";
            return false;
        }
        
        if (email.find('@') == std::string::npos) {
            std::cout << "❌ Email должен содержать символ @\n";
            return false;
        }
        
        if (email.find('.') == std::string::npos) {
            std::cout << "❌ Email должен содержать точку\n";
            return false;
        }
        
        if (email.length() < 5) {
            std::cout << "❌ Email слишком короткий\n";
            return false;
        }
        
        std::cout << "✅ Email валиден: " << email << "\n";
        return true;
    }
    
    // Нарушение DRY: дублирование логики валидации телефона
    static bool isValidPhone(const std::string& phone) {
        if (phone.empty()) {
            std::cout << "❌ Телефон не может быть пустым\n";
            return false;
        }
        
        // Проверка на цифры
        bool hasDigits = false;
        for (char c : phone) {
            if (std::isdigit(c)) {
                hasDigits = true;
                break;
            }
        }
        
        if (!hasDigits) {
            std::cout << "❌ Телефон должен содержать цифры\n";
            return false;
        }
        
        if (phone.length() < 7) {
            std::cout << "❌ Телефон слишком короткий\n";
            return false;
        }
        
        std::cout << "✅ Телефон валиден: " << phone << "\n";
        return true;
    }
    
    // Нарушение DRY: дублирование логики валидации пароля
    static bool isValidPassword(const std::string& password) {
        if (password.empty()) {
            std::cout << "❌ Пароль не может быть пустым\n";
            return false;
        }
        
        if (password.length() < 8) {
            std::cout << "❌ Пароль слишком короткий (минимум 8 символов)\n";
            return false;
        }
        
        bool hasUpper = false, hasLower = false, hasDigit = false;
        for (char c : password) {
            if (std::isupper(c)) hasUpper = true;
            if (std::islower(c)) hasLower = true;
            if (std::isdigit(c)) hasDigit = true;
        }
        
        if (!hasUpper) {
            std::cout << "❌ Пароль должен содержать заглавную букву\n";
            return false;
        }
        
        if (!hasLower) {
            std::cout << "❌ Пароль должен содержать строчную букву\n";
            return false;
        }
        
        if (!hasDigit) {
            std::cout << "❌ Пароль должен содержать цифру\n";
            return false;
        }
        
        std::cout << "✅ Пароль валиден\n";
        return true;
    }
};

/**
 * @brief Соблюдение DRY: Единая система валидации
 * 
 * Этот код соблюдает DRY, устраняя дублирование через:
 * - Абстракцию общих проверок
 * - Переиспользуемые компоненты валидации
 * - Единую систему сообщений об ошибках
 */

// Базовый класс для валидаторов
class BaseValidator {
protected:
    std::string fieldName;
    bool isValid = true;
    std::vector<std::string> errors;
    
public:
    BaseValidator(const std::string& name) : fieldName(name) {}
    
    virtual ~BaseValidator() = default;
    virtual bool validate(const std::string& value) = 0;
    
    bool getIsValid() const { return isValid; }
    const std::vector<std::string>& getErrors() const { return errors; }
    const std::string& getFieldName() const { return fieldName; }
    
protected:
    void addError(const std::string& error) {
        errors.push_back(fieldName + ": " + error);
        isValid = false;
    }
    
    bool checkNotEmpty(const std::string& value) {
        if (value.empty()) {
            addError("не может быть пустым");
            return false;
        }
        return true;
    }
    
    bool checkMinLength(const std::string& value, size_t minLength) {
        if (value.length() < minLength) {
            addError("слишком короткий (минимум " + std::to_string(minLength) + " символов)");
            return false;
        }
        return true;
    }
    
    bool checkMaxLength(const std::string& value, size_t maxLength) {
        if (value.length() > maxLength) {
            addError("слишком длинный (максимум " + std::to_string(maxLength) + " символов)");
            return false;
        }
        return true;
    }
};

// Валидатор для email
class EmailValidator : public BaseValidator {
public:
    EmailValidator() : BaseValidator("Email") {}
    
    bool validate(const std::string& email) override {
        isValid = true;
        errors.clear();
        
        if (!checkNotEmpty(email)) return false;
        if (!checkMinLength(email, 5)) return false;
        
        if (email.find('@') == std::string::npos) {
            addError("должен содержать символ @");
            return false;
        }
        
        if (email.find('.') == std::string::npos) {
            addError("должен содержать точку");
            return false;
        }
        
        // Проверка на валидные символы
        std::string validChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789@.-_";
        for (char c : email) {
            if (validChars.find(c) == std::string::npos) {
                addError("содержит недопустимые символы");
                return false;
            }
        }
        
        return isValid;
    }
};

// Валидатор для телефона
class PhoneValidator : public BaseValidator {
public:
    PhoneValidator() : BaseValidator("Телефон") {}
    
    bool validate(const std::string& phone) override {
        isValid = true;
        errors.clear();
        
        if (!checkNotEmpty(phone)) return false;
        if (!checkMinLength(phone, 7)) return false;
        if (!checkMaxLength(phone, 15)) return false;
        
        // Подсчет цифр
        int digitCount = 0;
        for (char c : phone) {
            if (std::isdigit(c)) {
                digitCount++;
            } else if (c != '+' && c != '-' && c != '(' && c != ')' && c != ' ') {
                addError("содержит недопустимые символы");
                return false;
            }
        }
        
        if (digitCount < 7) {
            addError("должен содержать минимум 7 цифр");
            return false;
        }
        
        return isValid;
    }
};

// Валидатор для пароля
class PasswordValidator : public BaseValidator {
private:
    int minLength;
    bool requireUpper;
    bool requireLower;
    bool requireDigit;
    bool requireSpecial;
    
public:
    PasswordValidator(int minLen = 8, bool upper = true, bool lower = true, 
                     bool digit = true, bool special = false)
        : BaseValidator("Пароль"), minLength(minLen), requireUpper(upper),
          requireLower(lower), requireDigit(digit), requireSpecial(special) {}
    
    bool validate(const std::string& password) override {
        isValid = true;
        errors.clear();
        
        if (!checkNotEmpty(password)) return false;
        if (!checkMinLength(password, minLength)) return false;
        
        bool hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
        
        for (char c : password) {
            if (std::isupper(c)) hasUpper = true;
            else if (std::islower(c)) hasLower = true;
            else if (std::isdigit(c)) hasDigit = true;
            else if (requireSpecial && !std::isalnum(c)) hasSpecial = true;
        }
        
        if (requireUpper && !hasUpper) {
            addError("должен содержать заглавную букву");
        }
        
        if (requireLower && !hasLower) {
            addError("должен содержать строчную букву");
        }
        
        if (requireDigit && !hasDigit) {
            addError("должен содержать цифру");
        }
        
        if (requireSpecial && !hasSpecial) {
            addError("должен содержать специальный символ");
        }
        
        return isValid;
    }
};

// Менеджер валидации
class ValidationManager {
private:
    std::vector<std::unique_ptr<BaseValidator>> validators;
    
public:
    void addValidator(std::unique_ptr<BaseValidator> validator) {
        validators.push_back(std::move(validator));
    }
    
    bool validateAll(const std::map<std::string, std::string>& data) {
        bool allValid = true;
        
        for (const auto& validator : validators) {
            auto it = data.find(validator->getFieldName());
            if (it != data.end()) {
                if (!validator->validate(it->second)) {
                    allValid = false;
                    for (const auto& error : validator->getErrors()) {
                        std::cout << "❌ " << error << "\n";
                    }
                } else {
                    std::cout << "✅ " << validator->getFieldName() << " валиден: " << it->second << "\n";
                }
            }
        }
        
        return allValid;
    }
    
    void printSummary() {
        std::cout << "\n📊 Сводка валидации:\n";
        std::cout << "Всего валидаторов: " << validators.size() << "\n";
        for (const auto& validator : validators) {
            std::cout << "- " << validator->getFieldName() << "\n";
        }
    }
};

// ============================================================================
// ПРИМЕР 2: ДУБЛИРОВАНИЕ ФОРМАТИРОВАНИЯ
// ============================================================================

/**
 * @brief Нарушение DRY: Дублирование логики форматирования
 */
class BadFormatter {
public:
    // Нарушение DRY: дублирование логики форматирования валют
    static std::string formatCurrency(double amount, const std::string& currency = "USD") {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2);
        
        if (currency == "USD") {
            oss << "$" << amount;
        } else if (currency == "EUR") {
            oss << "€" << amount;
        } else if (currency == "GBP") {
            oss << "£" << amount;
        } else if (currency == "JPY") {
            oss << "¥" << amount;
        } else {
            oss << currency << " " << amount;
        }
        
        return oss.str();
    }
    
    // Нарушение DRY: дублирование логики форматирования процентов
    static std::string formatPercentage(double value) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2);
        oss << value << "%";
        return oss.str();
    }
    
    // Нарушение DRY: дублирование логики форматирования чисел
    static std::string formatNumber(double value, int decimals = 2) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(decimals);
        oss << value;
        return oss.str();
    }
    
    // Нарушение DRY: дублирование логики форматирования дат
    static std::string formatDate(int year, int month, int day) {
        std::ostringstream oss;
        oss << std::setfill('0') << std::setw(4) << year << "-"
            << std::setw(2) << month << "-" << std::setw(2) << day;
        return oss.str();
    }
};

/**
 * @brief Соблюдение DRY: Единая система форматирования
 */
class GoodFormatter {
public:
    // Единый метод форматирования с параметрами
    static std::string format(const std::string& format, const std::string& value, 
                             int precision = 2, const std::string& symbol = "") {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(precision);
        
        if (format == "currency") {
            if (symbol.empty()) {
                oss << "$" << value;
            } else {
                oss << symbol << value;
            }
        } else if (format == "percentage") {
            oss << value << "%";
        } else if (format == "number") {
            oss << value;
        } else if (format == "date") {
            // Обработка даты
            oss << value;
        } else {
            oss << value;
        }
        
        return oss.str();
    }
    
    // Специализированные методы, использующие общую логику
    static std::string formatCurrency(double amount, const std::string& currency = "USD") {
        std::string symbol;
        if (currency == "USD") symbol = "$";
        else if (currency == "EUR") symbol = "€";
        else if (currency == "GBP") symbol = "£";
        else if (currency == "JPY") symbol = "¥";
        else symbol = currency + " ";
        
        return format("currency", std::to_string(amount), 2, symbol);
    }
    
    static std::string formatPercentage(double value) {
        return format("percentage", std::to_string(value), 2);
    }
    
    static std::string formatNumber(double value, int decimals = 2) {
        return format("number", std::to_string(value), decimals);
    }
    
    static std::string formatDate(int year, int month, int day) {
        std::ostringstream oss;
        oss << std::setfill('0') << std::setw(4) << year << "-"
            << std::setw(2) << month << "-" << std::setw(2) << day;
        return oss.str();
    }
};

// ============================================================================
// ПРИМЕР 3: ДУБЛИРОВАНИЕ КОНФИГУРАЦИИ
// ============================================================================

/**
 * @brief Нарушение DRY: Дублирование конфигурационных данных
 */
class BadConfig {
public:
    // Нарушение DRY: дублирование настроек базы данных
    static std::string getDatabaseHost() {
        return "localhost";
    }
    
    static int getDatabasePort() {
        return 5432;
    }
    
    static std::string getDatabaseName() {
        return "myapp";
    }
    
    static std::string getDatabaseUser() {
        return "admin";
    }
    
    static std::string getDatabasePassword() {
        return "secret123";
    }
    
    // Нарушение DRY: дублирование настроек Redis
    static std::string getRedisHost() {
        return "localhost";
    }
    
    static int getRedisPort() {
        return 6379;
    }
    
    static std::string getRedisPassword() {
        return "redis123";
    }
    
    // Нарушение DRY: дублирование настроек API
    static std::string getApiHost() {
        return "api.example.com";
    }
    
    static int getApiPort() {
        return 443;
    }
    
    static std::string getApiKey() {
        return "api_key_123";
    }
};

/**
 * @brief Соблюдение DRY: Единая система конфигурации
 */
class GoodConfig {
private:
    static std::map<std::string, std::string> config;
    static bool initialized;
    
public:
    static void initialize() {
        if (initialized) return;
        
        // Централизованная конфигурация
        config["database.host"] = "localhost";
        config["database.port"] = "5432";
        config["database.name"] = "myapp";
        config["database.user"] = "admin";
        config["database.password"] = "secret123";
        
        config["redis.host"] = "localhost";
        config["redis.port"] = "6379";
        config["redis.password"] = "redis123";
        
        config["api.host"] = "api.example.com";
        config["api.port"] = "443";
        config["api.key"] = "api_key_123";
        
        initialized = true;
    }
    
    static std::string get(const std::string& key) {
        if (!initialized) initialize();
        
        auto it = config.find(key);
        if (it != config.end()) {
            return it->second;
        }
        return "";
    }
    
    static int getInt(const std::string& key) {
        std::string value = get(key);
        return value.empty() ? 0 : std::stoi(value);
    }
    
    static bool getBool(const std::string& key) {
        std::string value = get(key);
        return value == "true" || value == "1";
    }
    
    // Специализированные методы, использующие общую логику
    static std::string getDatabaseHost() { return get("database.host"); }
    static int getDatabasePort() { return getInt("database.port"); }
    static std::string getDatabaseName() { return get("database.name"); }
    static std::string getDatabaseUser() { return get("database.user"); }
    static std::string getDatabasePassword() { return get("database.password"); }
    
    static std::string getRedisHost() { return get("redis.host"); }
    static int getRedisPort() { return getInt("redis.port"); }
    static std::string getRedisPassword() { return get("redis.password"); }
    
    static std::string getApiHost() { return get("api.host"); }
    static int getApiPort() { return getInt("api.port"); }
    static std::string getApiKey() { return get("api.key"); }
    
    static void set(const std::string& key, const std::string& value) {
        if (!initialized) initialize();
        config[key] = value;
    }
    
    static void printAll() {
        if (!initialized) initialize();
        
        std::cout << "\n📋 Конфигурация:\n";
        for (const auto& pair : config) {
            std::cout << "  " << pair.first << " = " << pair.second << "\n";
        }
    }
};

// Инициализация статических членов
std::map<std::string, std::string> GoodConfig::config;
bool GoodConfig::initialized = false;

// ============================================================================
// ДЕМОНСТРАЦИЯ ПРИНЦИПА
// ============================================================================

void demonstrateBadDRY() {
    std::cout << "❌ НАРУШЕНИЕ DRY - Дублирование кода:\n";
    std::cout << std::string(50, '-') << "\n";
    
    std::cout << "Тестирование валидации:\n";
    BadUserValidator::isValidEmail("test@example.com");
    BadUserValidator::isValidPhone("+1234567890");
    BadUserValidator::isValidPassword("Password123");
    
    std::cout << "\nТестирование форматирования:\n";
    std::cout << "Валюта: " << BadFormatter::formatCurrency(123.45, "USD") << "\n";
    std::cout << "Процент: " << BadFormatter::formatPercentage(15.75) << "\n";
    std::cout << "Число: " << BadFormatter::formatNumber(123.456, 3) << "\n";
    
    std::cout << "\nТестирование конфигурации:\n";
    std::cout << "DB Host: " << BadConfig::getDatabaseHost() << "\n";
    std::cout << "Redis Host: " << BadConfig::getRedisHost() << "\n";
    std::cout << "API Host: " << BadConfig::getApiHost() << "\n";
}

void demonstrateGoodDRY() {
    std::cout << "\n✅ СОБЛЮДЕНИЕ DRY - Устранение дублирования:\n";
    std::cout << std::string(50, '-') << "\n";
    
    std::cout << "Тестирование системы валидации:\n";
    ValidationManager validator;
    validator.addValidator(std::make_unique<EmailValidator>());
    validator.addValidator(std::make_unique<PhoneValidator>());
    validator.addValidator(std::make_unique<PasswordValidator>());
    
    std::map<std::string, std::string> userData = {
        {"Email", "user@example.com"},
        {"Телефон", "+1234567890"},
        {"Пароль", "SecurePass123"}
    };
    
    validator.validateAll(userData);
    validator.printSummary();
    
    std::cout << "\nТестирование системы форматирования:\n";
    std::cout << "Валюта: " << GoodFormatter::formatCurrency(123.45, "USD") << "\n";
    std::cout << "Процент: " << GoodFormatter::formatPercentage(15.75) << "\n";
    std::cout << "Число: " << GoodFormatter::formatNumber(123.456, 3) << "\n";
    
    std::cout << "\nТестирование системы конфигурации:\n";
    GoodConfig::initialize();
    std::cout << "DB Host: " << GoodConfig::getDatabaseHost() << "\n";
    std::cout << "Redis Host: " << GoodConfig::getRedisHost() << "\n";
    std::cout << "API Host: " << GoodConfig::getApiHost() << "\n";
    GoodConfig::printAll();
}

void analyzeTradeOffs() {
    std::cout << "\n🔬 АНАЛИЗ КОМПРОМИССОВ DRY:\n";
    std::cout << std::string(50, '-') << "\n";
    
    std::cout << "📊 ПРЕИМУЩЕСТВА:\n";
    std::cout << "• Снижение дублирования кода\n";
    std::cout << "• Единая точка изменения для общего функционала\n";
    std::cout << "• Улучшенная maintainability\n";
    std::cout << "• Снижение вероятности ошибок\n";
    std::cout << "• Упрощение тестирования\n\n";
    
    std::cout << "⚠️ НЕДОСТАТКИ:\n";
    std::cout << "• Потенциальное усложнение архитектуры\n";
    std::cout << "• Необходимость понимания абстракций\n";
    std::cout << "• Возможный overhead от дополнительных слоев\n";
    std::cout << "• Риск over-abstraction\n\n";
    
    std::cout << "🎯 РЕКОМЕНДАЦИИ:\n";
    std::cout << "• Применяйте DRY при обнаружении дублирования\n";
    std::cout << "• Используйте абстракции разумно\n";
    std::cout << "• Избегайте преждевременной абстракции\n";
    std::cout << "• Рассмотрите компромисс между DRY и простотой\n";
}

int main() {
    std::cout << "🎯 ДЕТАЛЬНАЯ ДЕМОНСТРАЦИЯ ПРИНЦИПА DRY (Don't Repeat Yourself)\n";
    std::cout << "Автор: Senior C++ Developer\n";
    std::cout << "Цель: Углубленное изучение принципа устранения дублирования\n\n";
    
    demonstrateBadDRY();
    demonstrateGoodDRY();
    analyzeTradeOffs();
    
    std::cout << "\n📚 МАТЕМАТИЧЕСКОЕ ОБОСНОВАНИЕ:\n";
    std::cout << "DRY: ∀K ∈ Knowledge: |Representations(K)| = 1\n";
    std::cout << "где Representations(K) = {r | r представляет знание K в системе}\n\n";
    
    std::cout << "💡 КЛЮЧЕВЫЕ ВЫВОДЫ:\n";
    std::cout << "1. Каждый фрагмент знания должен иметь единственное представление\n";
    std::cout << "2. DRY улучшает maintainability и снижает ошибки\n";
    std::cout << "3. Применение требует баланса с другими принципами\n";
    std::cout << "4. Современные инструменты помогают выявлять дублирование\n\n";
    
    std::cout << "🔬 Принципы - это инструменты для мышления о проблемах!\n";
    
    return 0;
}
