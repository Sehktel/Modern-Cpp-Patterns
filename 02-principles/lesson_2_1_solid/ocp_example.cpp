/**
 * @file ocp_example.cpp
 * @brief Детальная демонстрация Open/Closed Principle (OCP)
 * @author Senior C++ Developer
 * @date 2024
 * 
 * Этот файл содержит углубленный анализ OCP с математическим обоснованием
 * и практическими примерами из реальной разработки.
 */

#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <map>
#include <algorithm>

/**
 * @brief Математическая формализация OCP
 * 
 * OCP можно формализовать следующим образом:
 * ∀E ∈ Entities: E ∈ Extensible ∧ E ∉ Modifiable
 * где Extensible = {e | можно добавить функциональность без изменения e}
 * 
 * Другими словами: программные сущности должны быть открыты для расширения,
 * но закрыты для модификации.
 */

// ============================================================================
// ПРИМЕР 1: СИСТЕМА ОБРАБОТКИ ПЛАТЕЖЕЙ
// ============================================================================

/**
 * @brief Нарушение OCP: Модификация для добавления функциональности
 * 
 * Этот класс нарушает OCP, так как при добавлении нового типа платежа
 * необходимо модифицировать существующий код.
 */
class BadPaymentProcessor {
public:
    enum class PaymentType {
        CREDIT_CARD,
        PAYPAL,
        BANK_TRANSFER,
        CRYPTOCURRENCY  // Новый тип - требует модификации!
    };
    
    struct PaymentRequest {
        PaymentType type;
        double amount;
        std::string currency;
        std::map<std::string, std::string> metadata;
    };
    
    bool processPayment(const PaymentRequest& request) {
        std::cout << "💳 Обработка платежа на сумму $" << request.amount << "\n";
        
        switch (request.type) {
            case PaymentType::CREDIT_CARD:
                return processCreditCard(request);
            case PaymentType::PAYPAL:
                return processPayPal(request);
            case PaymentType::BANK_TRANSFER:
                return processBankTransfer(request);
            case PaymentType::CRYPTOCURRENCY:
                return processCryptocurrency(request);  // Новый case!
            default:
                std::cout << "❌ Неподдерживаемый тип платежа\n";
                return false;
        }
    }
    
private:
    bool processCreditCard(const PaymentRequest& request) {
        std::cout << "💳 Обработка кредитной карты...\n";
        // Логика обработки кредитной карты
        return true;
    }
    
    bool processPayPal(const PaymentRequest& request) {
        std::cout << "💰 Обработка PayPal...\n";
        // Логика обработки PayPal
        return true;
    }
    
    bool processBankTransfer(const PaymentRequest& request) {
        std::cout << "🏦 Обработка банковского перевода...\n";
        // Логика обработки банковского перевода
        return true;
    }
    
    // Новый метод - нарушение OCP!
    bool processCryptocurrency(const PaymentRequest& request) {
        std::cout << "₿ Обработка криптовалютного платежа...\n";
        // Логика обработки криптовалюты
        return true;
    }
};

/**
 * @brief Соблюдение OCP: Расширение без модификации
 * 
 * Новые типы платежей добавляются через наследование от абстрактного класса,
 * без изменения существующего кода.
 */

// Абстрактный класс для стратегии платежей
class PaymentStrategy {
public:
    virtual ~PaymentStrategy() = default;
    virtual bool processPayment(double amount, const std::string& currency, 
                               const std::map<std::string, std::string>& metadata) = 0;
    virtual std::string getPaymentType() const = 0;
    virtual double getProcessingFee() const = 0;
    virtual bool supportsCurrency(const std::string& currency) const = 0;
    virtual std::vector<std::string> getSupportedCurrencies() const = 0;
};

// Конкретные реализации для разных типов платежей
class CreditCardPayment : public PaymentStrategy {
private:
    std::string cardNumber;
    std::string expiryDate;
    std::string cvv;
    
public:
    CreditCardPayment(const std::string& card, const std::string& expiry, const std::string& cvvCode)
        : cardNumber(card), expiryDate(expiry), cvv(cvvCode) {}
    
    bool processPayment(double amount, const std::string& currency, 
                       const std::map<std::string, std::string>& metadata) override {
        std::cout << "💳 Обработка кредитной карты: $" << amount << " " << currency << "\n";
        
        // Валидация карты
        if (!validateCard()) {
            std::cout << "❌ Невалидная кредитная карта\n";
            return false;
        }
        
        // Проверка лимитов
        if (!checkLimits(amount)) {
            std::cout << "❌ Превышен лимит по карте\n";
            return false;
        }
        
        // Обработка платежа
        std::cout << "✅ Платеж по кредитной карте успешно обработан\n";
        return true;
    }
    
    std::string getPaymentType() const override {
        return "Credit Card";
    }
    
    double getProcessingFee() const override {
        return 0.029; // 2.9% комиссия
    }
    
    bool supportsCurrency(const std::string& currency) const override {
        std::vector<std::string> supported = {"USD", "EUR", "GBP", "JPY"};
        return std::find(supported.begin(), supported.end(), currency) != supported.end();
    }
    
    std::vector<std::string> getSupportedCurrencies() const override {
        return {"USD", "EUR", "GBP", "JPY"};
    }
    
private:
    bool validateCard() {
        // Упрощенная валидация
        return cardNumber.length() >= 13 && cardNumber.length() <= 19;
    }
    
    bool checkLimits(double amount) {
        // Упрощенная проверка лимитов
        return amount <= 10000.0;
    }
};

class PayPalPayment : public PaymentStrategy {
private:
    std::string email;
    std::string password;
    
public:
    PayPalPayment(const std::string& emailAddr, const std::string& pass)
        : email(emailAddr), password(pass) {}
    
    bool processPayment(double amount, const std::string& currency, 
                       const std::map<std::string, std::string>& metadata) override {
        std::cout << "💰 Обработка PayPal: $" << amount << " " << currency << "\n";
        
        // Аутентификация
        if (!authenticate()) {
            std::cout << "❌ Ошибка аутентификации PayPal\n";
            return false;
        }
        
        // Проверка баланса
        if (!checkBalance(amount)) {
            std::cout << "❌ Недостаточно средств на PayPal\n";
            return false;
        }
        
        // Обработка платежа
        std::cout << "✅ Платеж через PayPal успешно обработан\n";
        return true;
    }
    
    std::string getPaymentType() const override {
        return "PayPal";
    }
    
    double getProcessingFee() const override {
        return 0.034; // 3.4% комиссия
    }
    
    bool supportsCurrency(const std::string& currency) const override {
        std::vector<std::string> supported = {"USD", "EUR", "GBP", "CAD", "AUD", "JPY"};
        return std::find(supported.begin(), supported.end(), currency) != supported.end();
    }
    
    std::vector<std::string> getSupportedCurrencies() const override {
        return {"USD", "EUR", "GBP", "CAD", "AUD", "JPY"};
    }
    
private:
    bool authenticate() {
        // Упрощенная аутентификация
        return !email.empty() && !password.empty();
    }
    
    bool checkBalance(double amount) {
        // Упрощенная проверка баланса
        return amount <= 5000.0;
    }
};

class BankTransferPayment : public PaymentStrategy {
private:
    std::string accountNumber;
    std::string routingNumber;
    std::string bankName;
    
public:
    BankTransferPayment(const std::string& account, const std::string& routing, const std::string& bank)
        : accountNumber(account), routingNumber(routing), bankName(bank) {}
    
    bool processPayment(double amount, const std::string& currency, 
                       const std::map<std::string, std::string>& metadata) override {
        std::cout << "🏦 Обработка банковского перевода: $" << amount << " " << currency << "\n";
        
        // Валидация банковских данных
        if (!validateBankAccount()) {
            std::cout << "❌ Невалидные банковские данные\n";
            return false;
        }
        
        // Проверка лимитов банка
        if (!checkBankLimits(amount)) {
            std::cout << "❌ Превышен лимит банка\n";
            return false;
        }
        
        // Обработка перевода
        std::cout << "✅ Банковский перевод успешно обработан\n";
        return true;
    }
    
    std::string getPaymentType() const override {
        return "Bank Transfer";
    }
    
    double getProcessingFee() const override {
        return 0.015; // 1.5% комиссия
    }
    
    bool supportsCurrency(const std::string& currency) const override {
        // Банковские переводы поддерживают много валют
        return currency.length() == 3; // Упрощенная проверка
    }
    
    std::vector<std::string> getSupportedCurrencies() const override {
        return {"USD", "EUR", "GBP", "CAD", "AUD", "JPY", "CHF", "SEK", "NOK", "DKK"};
    }
    
private:
    bool validateBankAccount() {
        // Упрощенная валидация
        return accountNumber.length() >= 8 && routingNumber.length() == 9;
    }
    
    bool checkBankLimits(double amount) {
        // Упрощенная проверка лимитов
        return amount <= 50000.0;
    }
};

// Новый тип платежа - добавляется БЕЗ изменения существующего кода!
class CryptocurrencyPayment : public PaymentStrategy {
private:
    std::string walletAddress;
    std::string cryptoType;
    
public:
    CryptocurrencyPayment(const std::string& wallet, const std::string& crypto)
        : walletAddress(wallet), cryptoType(crypto) {}
    
    bool processPayment(double amount, const std::string& currency, 
                       const std::map<std::string, std::string>& metadata) override {
        std::cout << "₿ Обработка криптовалютного платежа: " << amount << " " << cryptoType << "\n";
        
        // Валидация кошелька
        if (!validateWallet()) {
            std::cout << "❌ Невалидный адрес кошелька\n";
            return false;
        }
        
        // Проверка баланса криптовалюты
        if (!checkCryptoBalance(amount)) {
            std::cout << "❌ Недостаточно средств в кошельке\n";
            return false;
        }
        
        // Обработка криптовалютного платежа
        std::cout << "✅ Криптовалютный платеж успешно обработан\n";
        return true;
    }
    
    std::string getPaymentType() const override {
        return "Cryptocurrency (" + cryptoType + ")";
    }
    
    double getProcessingFee() const override {
        return 0.005; // 0.5% комиссия (низкая для криптовалют)
    }
    
    bool supportsCurrency(const std::string& currency) const override {
        // Криптовалюты поддерживают свои валюты
        return cryptoType == currency;
    }
    
    std::vector<std::string> getSupportedCurrencies() const override {
        return {cryptoType};
    }
    
private:
    bool validateWallet() {
        // Упрощенная валидация адреса кошелька
        return walletAddress.length() >= 26 && walletAddress.length() <= 35;
    }
    
    bool checkCryptoBalance(double amount) {
        // Упрощенная проверка баланса
        return amount <= 100.0; // Лимит в криптовалюте
    }
};

// Еще один новый тип - Apple Pay!
class ApplePayPayment : public PaymentStrategy {
private:
    std::string deviceId;
    std::string passcode;
    
public:
    ApplePayPayment(const std::string& device, const std::string& pass)
        : deviceId(device), passcode(pass) {}
    
    bool processPayment(double amount, const std::string& currency, 
                       const std::map<std::string, std::string>& metadata) override {
        std::cout << "🍎 Обработка Apple Pay: $" << amount << " " << currency << "\n";
        
        // Проверка устройства
        if (!validateDevice()) {
            std::cout << "❌ Невалидное устройство Apple\n";
            return false;
        }
        
        // Биометрическая аутентификация
        if (!biometricAuth()) {
            std::cout << "❌ Ошибка биометрической аутентификации\n";
            return false;
        }
        
        // Обработка платежа
        std::cout << "✅ Платеж через Apple Pay успешно обработан\n";
        return true;
    }
    
    std::string getPaymentType() const override {
        return "Apple Pay";
    }
    
    double getProcessingFee() const override {
        return 0.025; // 2.5% комиссия
    }
    
    bool supportsCurrency(const std::string& currency) const override {
        std::vector<std::string> supported = {"USD", "EUR", "GBP", "CAD", "AUD", "JPY", "CHF"};
        return std::find(supported.begin(), supported.end(), currency) != supported.end();
    }
    
    std::vector<std::string> getSupportedCurrencies() const override {
        return {"USD", "EUR", "GBP", "CAD", "AUD", "JPY", "CHF"};
    }
    
private:
    bool validateDevice() {
        // Упрощенная валидация устройства
        return deviceId.length() >= 10;
    }
    
    bool biometricAuth() {
        // Упрощенная биометрическая аутентификация
        return passcode.length() >= 6;
    }
};

// Класс для обработки платежей (открыт для расширения, закрыт для модификации)
class PaymentProcessor {
private:
    std::vector<std::unique_ptr<PaymentStrategy>> strategies;
    
public:
    void addPaymentStrategy(std::unique_ptr<PaymentStrategy> strategy) {
        strategies.push_back(std::move(strategy));
    }
    
    bool processPayment(const std::string& paymentType, double amount, 
                       const std::string& currency = "USD",
                       const std::map<std::string, std::string>& metadata = {}) {
        
        auto strategy = findStrategy(paymentType);
        if (!strategy) {
            std::cout << "❌ Стратегия платежа '" << paymentType << "' не найдена\n";
            return false;
        }
        
        // Проверка поддержки валюты
        if (!strategy->supportsCurrency(currency)) {
            std::cout << "❌ Валюта '" << currency << "' не поддерживается для " 
                      << strategy->getPaymentType() << "\n";
            return false;
        }
        
        // Обработка платежа
        bool result = strategy->processPayment(amount, currency, metadata);
        
        if (result) {
            double fee = amount * strategy->getProcessingFee();
            std::cout << "💸 Комиссия (" << strategy->getPaymentType() << "): $" 
                      << fee << " (" << (strategy->getProcessingFee() * 100) << "%)\n";
        }
        
        return result;
    }
    
    void listSupportedPayments() {
        std::cout << "💳 Поддерживаемые типы платежей:\n";
        for (const auto& strategy : strategies) {
            std::cout << "  - " << strategy->getPaymentType() 
                      << " (комиссия: " << (strategy->getProcessingFee() * 100) << "%)\n";
            std::cout << "    Поддерживаемые валюты: ";
            auto currencies = strategy->getSupportedCurrencies();
            for (size_t i = 0; i < currencies.size(); ++i) {
                std::cout << currencies[i];
                if (i < currencies.size() - 1) std::cout << ", ";
            }
            std::cout << "\n";
        }
    }
    
private:
    PaymentStrategy* findStrategy(const std::string& paymentType) {
        for (const auto& strategy : strategies) {
            if (strategy->getPaymentType() == paymentType) {
                return strategy.get();
            }
        }
        return nullptr;
    }
};

// ============================================================================
// ПРИМЕР 2: СИСТЕМА ФИЛЬТРАЦИИ ДАННЫХ
// ============================================================================

/**
 * @brief Нарушение OCP: Фильтр с жестко заданными критериями
 */
class BadDataFilter {
public:
    enum class FilterType {
        BY_NAME,
        BY_AGE,
        BY_SALARY,
        BY_DEPARTMENT  // Новый критерий - требует модификации!
    };
    
    struct Employee {
        std::string name;
        int age;
        double salary;
        std::string department;
    };
    
    std::vector<Employee> filter(const std::vector<Employee>& employees, FilterType type, const std::string& value) {
        std::vector<Employee> result;
        
        for (const auto& emp : employees) {
            bool matches = false;
            
            switch (type) {
                case FilterType::BY_NAME:
                    matches = emp.name.find(value) != std::string::npos;
                    break;
                case FilterType::BY_AGE:
                    matches = std::to_string(emp.age) == value;
                    break;
                case FilterType::BY_SALARY:
                    matches = std::to_string(static_cast<int>(emp.salary)) == value;
                    break;
                case FilterType::BY_DEPARTMENT:  // Новый case!
                    matches = emp.department == value;
                    break;
            }
            
            if (matches) {
                result.push_back(emp);
            }
        }
        
        return result;
    }
};

/**
 * @brief Соблюдение OCP: Расширяемый фильтр
 */
class FilterCriteria {
public:
    virtual ~FilterCriteria() = default;
    virtual bool matches(const BadDataFilter::Employee& employee) const = 0;
    virtual std::string getDescription() const = 0;
};

class NameFilter : public FilterCriteria {
private:
    std::string namePattern;
    
public:
    NameFilter(const std::string& pattern) : namePattern(pattern) {}
    
    bool matches(const BadDataFilter::Employee& employee) const override {
        return employee.name.find(namePattern) != std::string::npos;
    }
    
    std::string getDescription() const override {
        return "Имя содержит: " + namePattern;
    }
};

class AgeFilter : public FilterCriteria {
private:
    int minAge;
    int maxAge;
    
public:
    AgeFilter(int min, int max) : minAge(min), maxAge(max) {}
    
    bool matches(const BadDataFilter::Employee& employee) const override {
        return employee.age >= minAge && employee.age <= maxAge;
    }
    
    std::string getDescription() const override {
        return "Возраст от " + std::to_string(minAge) + " до " + std::to_string(maxAge);
    }
};

class SalaryFilter : public FilterCriteria {
private:
    double minSalary;
    double maxSalary;
    
public:
    SalaryFilter(double min, double max) : minSalary(min), maxSalary(max) {}
    
    bool matches(const BadDataFilter::Employee& employee) const override {
        return employee.salary >= minSalary && employee.salary <= maxSalary;
    }
    
    std::string getDescription() const override {
        return "Зарплата от $" + std::to_string(static_cast<int>(minSalary)) + 
               " до $" + std::to_string(static_cast<int>(maxSalary));
    }
};

// Новый фильтр - добавляется БЕЗ изменения существующего кода!
class DepartmentFilter : public FilterCriteria {
private:
    std::string department;
    
public:
    DepartmentFilter(const std::string& dept) : department(dept) {}
    
    bool matches(const BadDataFilter::Employee& employee) const override {
        return employee.department == department;
    }
    
    std::string getDescription() const override {
        return "Отдел: " + department;
    }
};

// Еще один новый фильтр!
class SeniorityFilter : public FilterCriteria {
private:
    int minYears;
    
public:
    SeniorityFilter(int years) : minYears(years) {}
    
    bool matches(const BadDataFilter::Employee& employee) const override {
        // Упрощенная логика: считаем стаж по возрасту минус 22 (возраст выпуска)
        int estimatedExperience = employee.age - 22;
        return estimatedExperience >= minYears;
    }
    
    std::string getDescription() const override {
        return "Стаж не менее " + std::to_string(minYears) + " лет";
    }
};

class GoodDataFilter {
public:
    std::vector<BadDataFilter::Employee> filter(const std::vector<BadDataFilter::Employee>& employees,
                                                const std::vector<std::unique_ptr<FilterCriteria>>& criteria) {
        std::vector<BadDataFilter::Employee> result;
        
        for (const auto& emp : employees) {
            bool matchesAll = true;
            
            for (const auto& criterion : criteria) {
                if (!criterion->matches(emp)) {
                    matchesAll = false;
                    break;
                }
            }
            
            if (matchesAll) {
                result.push_back(emp);
            }
        }
        
        return result;
    }
};

// ============================================================================
// ДЕМОНСТРАЦИЯ ПРИНЦИПА
// ============================================================================

void demonstrateBadOCP() {
    std::cout << "❌ НАРУШЕНИЕ OCP - Модификация для добавления функциональности:\n";
    std::cout << std::string(60, '-') << "\n";
    
    BadPaymentProcessor badProcessor;
    
    BadPaymentProcessor::PaymentRequest request1{};
    request1.type = BadPaymentProcessor::PaymentType::CREDIT_CARD;
    request1.amount = 100.0;
    request1.currency = "USD";
    badProcessor.processPayment(request1);
    
    BadPaymentProcessor::PaymentRequest request2{};
    request2.type = BadPaymentProcessor::PaymentType::CRYPTOCURRENCY;  // Новый тип!
    request2.amount = 50.0;
    request2.currency = "BTC";
    badProcessor.processPayment(request2);
}

void demonstrateGoodOCP() {
    std::cout << "\n✅ СОБЛЮДЕНИЕ OCP - Расширение без модификации:\n";
    std::cout << std::string(60, '-') << "\n";
    
    PaymentProcessor processor;
    
    // Добавление стратегий платежей
    processor.addPaymentStrategy(std::make_unique<CreditCardPayment>("1234567890123456", "12/25", "123"));
    processor.addPaymentStrategy(std::make_unique<PayPalPayment>("user@example.com", "password"));
    processor.addPaymentStrategy(std::make_unique<BankTransferPayment>("12345678", "123456789", "Bank of America"));
    
    // Новые типы платежей - добавляются БЕЗ изменения существующего кода!
    processor.addPaymentStrategy(std::make_unique<CryptocurrencyPayment>("1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa", "BTC"));
    processor.addPaymentStrategy(std::make_unique<ApplePayPayment>("iPhone123456", "123456"));
    
    // Список поддерживаемых платежей
    processor.listSupportedPayments();
    
    std::cout << "\n💳 Тестирование платежей:\n";
    
    // Тестирование разных типов платежей
    processor.processPayment("Credit Card", 100.0, "USD");
    processor.processPayment("PayPal", 250.0, "EUR");
    processor.processPayment("Bank Transfer", 1000.0, "GBP");
    processor.processPayment("Cryptocurrency (BTC)", 0.001, "BTC");  // Новый тип!
    processor.processPayment("Apple Pay", 75.0, "USD");  // Еще один новый тип!
}

void demonstrateFilterOCP() {
    std::cout << "\n🔍 ДЕМОНСТРАЦИЯ OCP В СИСТЕМЕ ФИЛЬТРАЦИИ:\n";
    std::cout << std::string(50, '-') << "\n";
    
    // Создание тестовых данных
    std::vector<BadDataFilter::Employee> employees = {
        {"Иван Иванов", 30, 50000, "IT"},
        {"Петр Петров", 25, 45000, "Marketing"},
        {"Мария Сидорова", 35, 60000, "IT"},
        {"Анна Козлова", 28, 48000, "HR"},
        {"Сергей Смирнов", 40, 70000, "IT"}
    };
    
    std::cout << "👥 Все сотрудники:\n";
    for (const auto& emp : employees) {
        std::cout << "  - " << emp.name << " (возраст: " << emp.age 
                  << ", зарплата: $" << emp.salary << ", отдел: " << emp.department << ")\n";
    }
    
    // Демонстрация расширяемого фильтра
    GoodDataFilter filter;
    
    // Фильтрация по IT отделу и возрасту
    std::vector<std::unique_ptr<FilterCriteria>> criteria1;
    criteria1.push_back(std::make_unique<DepartmentFilter>("IT"));
    criteria1.push_back(std::make_unique<AgeFilter>(25, 35));
    
    auto filtered1 = filter.filter(employees, criteria1);
    std::cout << "\n🔍 IT сотрудники возрастом 25-35 лет:\n";
    for (const auto& emp : filtered1) {
        std::cout << "  - " << emp.name << " (возраст: " << emp.age 
                  << ", зарплата: $" << emp.salary << ")\n";
    }
    
    // Новый фильтр - стаж - добавляется БЕЗ изменения существующего кода!
    std::vector<std::unique_ptr<FilterCriteria>> criteria2;
    criteria2.push_back(std::make_unique<SalaryFilter>(45000, 65000));
    criteria2.push_back(std::make_unique<SeniorityFilter>(5));  // Новый критерий!
    
    auto filtered2 = filter.filter(employees, criteria2);
    std::cout << "\n🔍 Сотрудники с зарплатой $45k-$65k и стажем 5+ лет:\n";
    for (const auto& emp : filtered2) {
        std::cout << "  - " << emp.name << " (возраст: " << emp.age 
                  << ", зарплата: $" << emp.salary << ", отдел: " << emp.department << ")\n";
    }
}

void analyzeTradeOffs() {
    std::cout << "\n🔬 АНАЛИЗ КОМПРОМИССОВ OCP:\n";
    std::cout << std::string(50, '-') << "\n";
    
    std::cout << "📊 ПРЕИМУЩЕСТВА:\n";
    std::cout << "• Легкость добавления новой функциональности\n";
    std::cout << "• Снижение риска внесения багов в существующий код\n";
    std::cout << "• Улучшенная модульность и переиспользуемость\n";
    std::cout << "• Возможность независимой разработки расширений\n";
    std::cout << "• Соответствие принципу единственной ответственности\n\n";
    
    std::cout << "⚠️ НЕДОСТАТКИ:\n";
    std::cout << "• Потенциальный overhead от виртуальных вызовов\n";
    std::cout << "• Усложнение архитектуры (больше абстракций)\n";
    std::cout << "• Необходимость предварительного проектирования\n";
    std::cout << "• Возможное over-engineering для простых случаев\n\n";
    
    std::cout << "🎯 РЕКОМЕНДАЦИИ:\n";
    std::cout << "• Применяйте OCP для часто изменяющихся модулей\n";
    std::cout << "• Используйте Strategy Pattern для реализации OCP\n";
    std::cout << "• Рассмотрите Template Method Pattern для стабильных алгоритмов\n";
    std::cout << "• Избегайте преждевременной абстракции (YAGNI)\n";
}

int main() {
    std::cout << "🎯 ДЕТАЛЬНАЯ ДЕМОНСТРАЦИЯ OPEN/CLOSED PRINCIPLE (OCP)\n";
    std::cout << "Автор: Senior C++ Developer\n";
    std::cout << "Цель: Углубленное изучение принципа открытости/закрытости\n\n";
    
    demonstrateBadOCP();
    demonstrateGoodOCP();
    demonstrateFilterOCP();
    analyzeTradeOffs();
    
    std::cout << "\n📚 МАТЕМАТИЧЕСКОЕ ОБОСНОВАНИЕ:\n";
    std::cout << "OCP: ∀E ∈ Entities: E ∈ Extensible ∧ E ∉ Modifiable\n";
    std::cout << "где Extensible = {e | можно добавить функциональность без изменения e}\n\n";
    
    std::cout << "💡 КЛЮЧЕВЫЕ ВЫВОДЫ:\n";
    std::cout << "1. Программные сущности должны быть открыты для расширения\n";
    std::cout << "2. Программные сущности должны быть закрыты для модификации\n";
    std::cout << "3. OCP достигается через абстракции и полиморфизм\n";
    std::cout << "4. Применение требует баланса между гибкостью и сложностью\n\n";
    
    std::cout << "🔬 Принципы - это инструменты для мышления о проблемах!\n";
    
    return 0;
}
