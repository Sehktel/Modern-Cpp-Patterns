#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <algorithm>
#include <cmath>
#include <random>

/**
 * @file strategy_pattern.cpp
 * @brief Демонстрация паттерна Strategy
 * 
 * Этот файл показывает различные реализации паттерна Strategy,
 * от классической до современных подходов с использованием C++17/20.
 */

// ============================================================================
// КЛАССИЧЕСКИЙ STRATEGY PATTERN
// ============================================================================

/**
 * @brief Интерфейс стратегии сортировки
 */
class SortingStrategy {
public:
    virtual ~SortingStrategy() = default;
    virtual void sort(std::vector<int>& data) = 0;
    virtual std::string getName() const = 0;
};

/**
 * @brief Пузырьковая сортировка
 */
class BubbleSort : public SortingStrategy {
public:
    void sort(std::vector<int>& data) override {
        std::cout << "Выполняется пузырьковая сортировка..." << std::endl;
        
        size_t n = data.size();
        for (size_t i = 0; i < n - 1; ++i) {
            for (size_t j = 0; j < n - i - 1; ++j) {
                if (data[j] > data[j + 1]) {
                    std::swap(data[j], data[j + 1]);
                }
            }
        }
    }
    
    std::string getName() const override {
        return "Bubble Sort";
    }
};

/**
 * @brief Быстрая сортировка
 */
class QuickSort : public SortingStrategy {
private:
    void quickSort(std::vector<int>& data, int low, int high) {
        if (low < high) {
            int pivotIndex = partition(data, low, high);
            quickSort(data, low, pivotIndex - 1);
            quickSort(data, pivotIndex + 1, high);
        }
    }
    
    int partition(std::vector<int>& data, int low, int high) {
        int pivot = data[high];
        int i = low - 1;
        
        for (int j = low; j < high; ++j) {
            if (data[j] <= pivot) {
                ++i;
                std::swap(data[i], data[j]);
            }
        }
        std::swap(data[i + 1], data[high]);
        return i + 1;
    }
    
public:
    void sort(std::vector<int>& data) override {
        std::cout << "Выполняется быстрая сортировка..." << std::endl;
        if (!data.empty()) {
            quickSort(data, 0, static_cast<int>(data.size()) - 1);
        }
    }
    
    std::string getName() const override {
        return "Quick Sort";
    }
};

/**
 * @brief Сортировка вставками
 */
class InsertionSort : public SortingStrategy {
public:
    void sort(std::vector<int>& data) override {
        std::cout << "Выполняется сортировка вставками..." << std::endl;
        
        for (size_t i = 1; i < data.size(); ++i) {
            int key = data[i];
            int j = static_cast<int>(i) - 1;
            
            while (j >= 0 && data[j] > key) {
                data[j + 1] = data[j];
                --j;
            }
            data[j + 1] = key;
        }
    }
    
    std::string getName() const override {
        return "Insertion Sort";
    }
};

/**
 * @brief Контекст для использования стратегий сортировки
 */
class Sorter {
private:
    std::unique_ptr<SortingStrategy> strategy_;
    
public:
    void setStrategy(std::unique_ptr<SortingStrategy> strategy) {
        strategy_ = std::move(strategy);
    }
    
    void sortData(std::vector<int>& data) {
        if (strategy_) {
            std::cout << "Используется стратегия: " << strategy_->getName() << std::endl;
            strategy_->sort(data);
        } else {
            std::cout << "Стратегия не установлена!" << std::endl;
        }
    }
    
    const std::string& getCurrentStrategyName() const {
        static const std::string noStrategy = "No Strategy";
        return strategy_ ? strategy_->getName() : noStrategy;
    }
};

// ============================================================================
// СИСТЕМА ВАЛИДАЦИИ
// ============================================================================

/**
 * @brief Интерфейс стратегии валидации
 */
class ValidationStrategy {
public:
    virtual ~ValidationStrategy() = default;
    virtual bool validate(const std::string& data) = 0;
    virtual std::string getErrorMessage() const = 0;
};

/**
 * @brief Валидация email
 */
class EmailValidation : public ValidationStrategy {
public:
    bool validate(const std::string& data) override {
        return data.find("@") != std::string::npos && 
               data.find(".") != std::string::npos &&
               data.find("@") < data.find(".");
    }
    
    std::string getErrorMessage() const override {
        return "Неверный формат email";
    }
};

/**
 * @brief Валидация номера телефона
 */
class PhoneValidation : public ValidationStrategy {
public:
    bool validate(const std::string& data) override {
        if (data.length() < 10) return false;
        
        for (char c : data) {
            if (!std::isdigit(c) && c != '+' && c != '-' && c != ' ' && c != '(' && c != ')') {
                return false;
            }
        }
        return true;
    }
    
    std::string getErrorMessage() const override {
        return "Неверный формат номера телефона";
    }
};

/**
 * @brief Валидация пароля
 */
class PasswordValidation : public ValidationStrategy {
public:
    bool validate(const std::string& data) override {
        if (data.length() < 8) return false;
        
        bool hasUpper = false, hasLower = false, hasDigit = false;
        for (char c : data) {
            if (std::isupper(c)) hasUpper = true;
            if (std::islower(c)) hasLower = true;
            if (std::isdigit(c)) hasDigit = true;
        }
        
        return hasUpper && hasLower && hasDigit;
    }
    
    std::string getErrorMessage() const override {
        return "Пароль должен содержать минимум 8 символов, включая заглавные и строчные буквы, а также цифры";
    }
};

/**
 * @brief Валидатор
 */
class Validator {
private:
    std::unique_ptr<ValidationStrategy> strategy_;
    
public:
    void setStrategy(std::unique_ptr<ValidationStrategy> strategy) {
        strategy_ = std::move(strategy);
    }
    
    bool validate(const std::string& data) {
        if (strategy_) {
            return strategy_->validate(data);
        }
        return false;
    }
    
    std::string getErrorMessage() const {
        return strategy_ ? strategy_->getErrorMessage() : "Стратегия валидации не установлена";
    }
};

// ============================================================================
// СИСТЕМА РАСЧЕТА СТОИМОСТИ
// ============================================================================

/**
 * @brief Интерфейс стратегии ценообразования
 */
class PricingStrategy {
public:
    virtual ~PricingStrategy() = default;
    virtual double calculatePrice(double basePrice, int quantity) = 0;
    virtual std::string getDescription() const = 0;
};

/**
 * @brief Обычное ценообразование
 */
class RegularPricing : public PricingStrategy {
public:
    double calculatePrice(double basePrice, int quantity) override {
        return basePrice * quantity;
    }
    
    std::string getDescription() const override {
        return "Обычная цена";
    }
};

/**
 * @brief Оптовое ценообразование
 */
class BulkPricing : public PricingStrategy {
private:
    double discountThreshold_;
    double discountRate_;
    
public:
    BulkPricing(int threshold = 10, double rate = 0.1) 
        : discountThreshold_(threshold), discountRate_(rate) {}
    
    double calculatePrice(double basePrice, int quantity) override {
        double totalPrice = basePrice * quantity;
        
        if (quantity >= discountThreshold_) {
            double discount = totalPrice * discountRate_;
            totalPrice -= discount;
        }
        
        return totalPrice;
    }
    
    std::string getDescription() const override {
        return "Оптовая цена (скидка " + std::to_string(discountRate_ * 100) + 
               "% при покупке от " + std::to_string(static_cast<int>(discountThreshold_)) + " штук)";
    }
};

/**
 * @brief Премиум ценообразование
 */
class PremiumPricing : public PricingStrategy {
private:
    double markupRate_;
    
public:
    explicit PremiumPricing(double markup = 0.2) : markupRate_(markup) {}
    
    double calculatePrice(double basePrice, int quantity) override {
        return basePrice * quantity * (1.0 + markupRate_);
    }
    
    std::string getDescription() const override {
        return "Премиум цена (надбавка " + std::to_string(markupRate_ * 100) + "%)";
    }
};

/**
 * @brief Сезонное ценообразование
 */
class SeasonalPricing : public PricingStrategy {
private:
    double seasonalMultiplier_;
    std::string season_;
    
public:
    SeasonalPricing(double multiplier, const std::string& season) 
        : seasonalMultiplier_(multiplier), season_(season) {}
    
    double calculatePrice(double basePrice, int quantity) override {
        return basePrice * quantity * seasonalMultiplier_;
    }
    
    std::string getDescription() const override {
        return "Сезонная цена (" + season_ + ", коэффициент " + std::to_string(seasonalMultiplier_) + ")";
    }
};

/**
 * @brief Калькулятор цены
 */
class PriceCalculator {
private:
    std::unique_ptr<PricingStrategy> strategy_;
    
public:
    void setStrategy(std::unique_ptr<PricingStrategy> strategy) {
        strategy_ = std::move(strategy);
    }
    
    double calculatePrice(double basePrice, int quantity) {
        if (strategy_) {
            return strategy_->calculatePrice(basePrice, quantity);
        }
        return 0.0;
    }
    
    std::string getStrategyDescription() const {
        return strategy_ ? strategy_->getDescription() : "Стратегия ценообразования не установлена";
    }
};

// ============================================================================
// СОВРЕМЕННЫЙ ПОДХОД С std::function
// ============================================================================

/**
 * @brief Современный сортировщик с функциональными стратегиями
 */
class ModernSorter {
private:
    std::function<void(std::vector<int>&)> sortFunction_;
    std::string strategyName_;
    
public:
    void setSortStrategy(std::function<void(std::vector<int>&)> strategy, const std::string& name) {
        sortFunction_ = strategy;
        strategyName_ = name;
    }
    
    void sortData(std::vector<int>& data) {
        if (sortFunction_) {
            std::cout << "Используется функциональная стратегия: " << strategyName_ << std::endl;
            sortFunction_(data);
        } else {
            std::cout << "Функциональная стратегия не установлена!" << std::endl;
        }
    }
    
    const std::string& getStrategyName() const {
        return strategyName_;
    }
};

// ============================================================================
// TEMPLATE STRATEGY
// ============================================================================

/**
 * @brief Шаблонный контекст для стратегий
 */
template<typename Strategy>
class TemplateContext {
private:
    Strategy strategy_;
    
public:
    template<typename... Args>
    explicit TemplateContext(Args&&... args) 
        : strategy_(std::forward<Args>(args)...) {}
    
    template<typename... Args>
    auto execute(Args&&... args) -> decltype(strategy_.execute(args...)) {
        return strategy_.execute(std::forward<Args>(args...));
    }
    
    const Strategy& getStrategy() const {
        return strategy_;
    }
};

/**
 * @brief Стратегии для математических операций
 */
struct AddStrategy {
    int execute(int a, int b) { 
        std::cout << "Выполняется сложение: " << a << " + " << b << std::endl;
        return a + b; 
    }
};

struct MultiplyStrategy {
    int execute(int a, int b) { 
        std::cout << "Выполняется умножение: " << a << " * " << b << std::endl;
        return a * b; 
    }
};

struct PowerStrategy {
    int execute(int a, int b) { 
        std::cout << "Выполняется возведение в степень: " << a << " ^ " << b << std::endl;
        return static_cast<int>(std::pow(a, b)); 
    }
};

// ============================================================================
// ДЕМОНСТРАЦИОННЫЕ ФУНКЦИИ
// ============================================================================

/**
 * @brief Демонстрация классического Strategy
 */
void demonstrateClassicStrategy() {
    std::cout << "\n=== Классический Strategy Pattern ===" << std::endl;
    
    Sorter sorter;
    std::vector<int> data = {64, 34, 25, 12, 22, 11, 90};
    
    std::cout << "Исходные данные: ";
    for (int value : data) {
        std::cout << value << " ";
    }
    std::cout << std::endl;
    
    // Используем разные стратегии сортировки
    std::vector<std::unique_ptr<SortingStrategy>> strategies;
    strategies.push_back(std::make_unique<BubbleSort>());
    strategies.push_back(std::make_unique<QuickSort>());
    strategies.push_back(std::make_unique<InsertionSort>());
    
    for (auto& strategy : strategies) {
        std::vector<int> testData = data; // Копируем данные для каждого теста
        
        sorter.setStrategy(std::move(strategy));
        sorter.sortData(testData);
        
        std::cout << "Результат: ";
        for (int value : testData) {
            std::cout << value << " ";
        }
        std::cout << std::endl << std::endl;
    }
}

/**
 * @brief Демонстрация системы валидации
 */
void demonstrateValidationStrategy() {
    std::cout << "\n=== Стратегии валидации ===" << std::endl;
    
    Validator validator;
    
    // Тестовые данные
    std::vector<std::pair<std::string, std::string>> testData = {
        {"user@example.com", "email"},
        {"+1234567890", "phone"},
        {"Password123", "password"},
        {"invalid-email", "email"},
        {"123", "phone"},
        {"weak", "password"}
    };
    
    // Стратегии валидации
    std::vector<std::unique_ptr<ValidationStrategy>> strategies;
    strategies.push_back(std::make_unique<EmailValidation>());
    strategies.push_back(std::make_unique<PhoneValidation>());
    strategies.push_back(std::make_unique<PasswordValidation>());
    
    for (const auto& [data, type] : testData) {
        std::cout << "Валидация '" << data << "' как " << type << ":" << std::endl;
        
        // Выбираем стратегию в зависимости от типа
        if (type == "email") {
            validator.setStrategy(std::make_unique<EmailValidation>());
        } else if (type == "phone") {
            validator.setStrategy(std::make_unique<PhoneValidation>());
        } else if (type == "password") {
            validator.setStrategy(std::make_unique<PasswordValidation>());
        }
        
        bool isValid = validator.validate(data);
        std::cout << "Результат: " << (isValid ? "✅ Валидно" : "❌ Невалидно") << std::endl;
        
        if (!isValid) {
            std::cout << "Ошибка: " << validator.getErrorMessage() << std::endl;
        }
        std::cout << std::endl;
    }
}

/**
 * @brief Демонстрация системы ценообразования
 */
void demonstratePricingStrategy() {
    std::cout << "\n=== Стратегии ценообразования ===" << std::endl;
    
    PriceCalculator calculator;
    double basePrice = 100.0;
    int quantity = 15;
    
    std::cout << "Базовая цена: $" << basePrice << ", Количество: " << quantity << std::endl << std::endl;
    
    // Различные стратегии ценообразования
    std::vector<std::unique_ptr<PricingStrategy>> strategies;
    strategies.push_back(std::make_unique<RegularPricing>());
    strategies.push_back(std::make_unique<BulkPricing>(10, 0.1));
    strategies.push_back(std::make_unique<PremiumPricing>(0.2));
    strategies.push_back(std::make_unique<SeasonalPricing>(1.5, "Пик сезона"));
    strategies.push_back(std::make_unique<SeasonalPricing>(0.7, "Низкий сезон"));
    
    for (auto& strategy : strategies) {
        calculator.setStrategy(std::move(strategy));
        
        double price = calculator.calculatePrice(basePrice, quantity);
        
        std::cout << "Стратегия: " << calculator.getStrategyDescription() << std::endl;
        std::cout << "Итоговая цена: $" << price << std::endl;
        std::cout << "Цена за единицу: $" << (price / quantity) << std::endl;
        std::cout << std::endl;
    }
}

/**
 * @brief Демонстрация современного подхода
 */
void demonstrateModernStrategy() {
    std::cout << "\n=== Современный подход с std::function ===" << std::endl;
    
    ModernSorter sorter;
    std::vector<int> data = {5, 2, 8, 1, 9, 3, 7, 4, 6};
    
    std::cout << "Исходные данные: ";
    for (int value : data) {
        std::cout << value << " ";
    }
    std::cout << std::endl;
    
    // Используем lambda функции как стратегии
    sorter.setSortStrategy([](std::vector<int>& data) {
        std::cout << "Выполняется сортировка с помощью std::sort..." << std::endl;
        std::sort(data.begin(), data.end());
    }, "STL Sort");
    
    sorter.sortData(data);
    
    std::cout << "Результат: ";
    for (int value : data) {
        std::cout << value << " ";
    }
    std::cout << std::endl << std::endl;
    
    // Другая стратегия - обратная сортировка
    sorter.setSortStrategy([](std::vector<int>& data) {
        std::cout << "Выполняется обратная сортировка..." << std::endl;
        std::sort(data.begin(), data.end(), std::greater<int>());
    }, "Reverse Sort");
    
    sorter.sortData(data);
    
    std::cout << "Результат: ";
    for (int value : data) {
        std::cout << value << " ";
    }
    std::cout << std::endl;
}

/**
 * @brief Демонстрация шаблонных стратегий
 */
void demonstrateTemplateStrategy() {
    std::cout << "\n=== Шаблонные стратегии ===" << std::endl;
    
    int a = 5, b = 3;
    
    // Создаем контексты с разными стратегиями
    TemplateContext<AddStrategy> addContext;
    TemplateContext<MultiplyStrategy> multiplyContext;
    TemplateContext<PowerStrategy> powerContext;
    
    std::cout << "Операции с числами " << a << " и " << b << ":" << std::endl;
    
    int result1 = addContext.execute(a, b);
    std::cout << "Результат сложения: " << result1 << std::endl << std::endl;
    
    int result2 = multiplyContext.execute(a, b);
    std::cout << "Результат умножения: " << result2 << std::endl << std::endl;
    
    int result3 = powerContext.execute(a, b);
    std::cout << "Результат возведения в степень: " << result3 << std::endl << std::endl;
}

// ============================================================================
// ОСНОВНАЯ ФУНКЦИЯ
// ============================================================================

int main() {
    std::cout << "🎯 Демонстрация паттерна Strategy" << std::endl;
    std::cout << std::string(50, '=') << std::endl;
    
    demonstrateClassicStrategy();
    demonstrateValidationStrategy();
    demonstratePricingStrategy();
    demonstrateModernStrategy();
    demonstrateTemplateStrategy();
    
    std::cout << "\n✅ Демонстрация Strategy Pattern завершена!" << std::endl;
    std::cout << "\n🎯 Ключевые выводы:" << std::endl;
    std::cout << "• Strategy инкапсулирует алгоритмы и делает их взаимозаменяемыми" << std::endl;
    std::cout << "• std::function позволяет использовать функциональный подход" << std::endl;
    std::cout << "• Шаблонные стратегии обеспечивают compile-time полиморфизм" << std::endl;
    std::cout << "• Паттерн устраняет необходимость в условных операторах" << std::endl;
    std::cout << "• Strategy особенно полезен для систем с множественными алгоритмами" << std::endl;
    
    return 0;
}
