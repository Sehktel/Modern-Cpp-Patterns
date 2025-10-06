/**
 * @file principles_comparison.cpp
 * @brief Сравнение принципов DRY, KISS, YAGNI и их взаимодействие
 * @author Senior C++ Developer
 * @date 2024
 * 
 * Этот файл демонстрирует взаимодействие принципов DRY, KISS, YAGNI
 * и показывает, как они могут конфликтовать или дополнять друг друга.
 */

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <memory>
#include <chrono>
#include <iomanip>

/**
 * @brief Анализ взаимодействия принципов
 * 
 * Принципы DRY, KISS и YAGNI часто взаимодействуют и могут конфликтовать:
 * - DRY может увеличить сложность (противоречие с KISS)
 * - KISS может привести к дублированию (противоречие с DRY)
 * - YAGNI может противоречить как DRY, так и KISS
 */

// ============================================================================
// ПРИМЕР 1: КОНФЛИКТ МЕЖДУ DRY И KISS
// ============================================================================

/**
 * @brief Нарушение KISS ради соблюдения DRY
 * 
 * Этот пример показывает, как чрезмерное применение DRY
 * может нарушить принцип KISS.
 */
class OverDRYExample {
private:
    // Сложная система для устранения дублирования
    template<typename T>
    class GenericProcessor {
    public:
        virtual ~GenericProcessor() = default;
        virtual T process(const T& input) = 0;
        virtual std::string getProcessorType() const = 0;
    };
    
    template<typename T>
    class ValidationProcessor : public GenericProcessor<T> {
    public:
        T process(const T& input) override {
            // Сложная логика валидации
            std::cout << "🔍 Валидация " << this->getProcessorType() << ": " << input << "\n";
            return input;
        }
    };
    
    template<typename T>
    class TransformationProcessor : public GenericProcessor<T> {
    public:
        T process(const T& input) override {
            // Сложная логика трансформации
            std::cout << "🔄 Трансформация " << this->getProcessorType() << ": " << input << "\n";
            return input;
        }
    };
    
    template<typename T>
    class PersistenceProcessor : public GenericProcessor<T> {
    public:
        T process(const T& input) override {
            // Сложная логика сохранения
            std::cout << "💾 Сохранение " << this->getProcessorType() << ": " << input << "\n";
            return input;
        }
    };
    
    // Специализации для разных типов
    class StringValidationProcessor : public ValidationProcessor<std::string> {
    public:
        std::string getProcessorType() const override { return "String"; }
    };
    
    class IntValidationProcessor : public ValidationProcessor<int> {
    public:
        std::string getProcessorType() const override { return "Integer"; }
    };
    
    class StringTransformationProcessor : public TransformationProcessor<std::string> {
    public:
        std::string getProcessorType() const override { return "String"; }
    };
    
    class IntTransformationProcessor : public TransformationProcessor<int> {
    public:
        std::string getProcessorType() const override { return "Integer"; }
    };
    
    // Сложная система управления процессорами
    std::map<std::string, std::unique_ptr<GenericProcessor<std::string>>> stringProcessors;
    std::map<std::string, std::unique_ptr<GenericProcessor<int>>> intProcessors;
    
public:
    OverDRYExample() {
        // Инициализация всех процессоров
        stringProcessors["validation"] = std::make_unique<StringValidationProcessor>();
        stringProcessors["transformation"] = std::make_unique<StringTransformationProcessor>();
        
        intProcessors["validation"] = std::make_unique<IntValidationProcessor>();
        intProcessors["transformation"] = std::make_unique<IntTransformationProcessor>();
        
        std::cout << "🏗️ Создана сложная система процессоров (нарушение KISS ради DRY)\n";
    }
    
    std::string processString(const std::string& input, const std::string& operation) {
        auto it = stringProcessors.find(operation);
        if (it != stringProcessors.end()) {
            return it->second->process(input);
        }
        return input;
    }
    
    int processInt(int input, const std::string& operation) {
        auto it = intProcessors.find(operation);
        if (it != intProcessors.end()) {
            return it->second->process(input);
        }
        return input;
    }
};

/**
 * @brief Соблюдение KISS с минимальным дублированием
 * 
 * Этот пример показывает простой подход с приемлемым дублированием.
 */
class SimpleExample {
public:
    // Простые функции с минимальным дублированием
    static std::string validateString(const std::string& input) {
        std::cout << "🔍 Валидация строки: " << input << "\n";
        return input;
    }
    
    static int validateInt(int input) {
        std::cout << "🔍 Валидация числа: " << input << "\n";
        return input;
    }
    
    static std::string transformString(const std::string& input) {
        std::cout << "🔄 Трансформация строки: " << input << "\n";
        return input;
    }
    
    static int transformInt(int input) {
        std::cout << "🔄 Трансформация числа: " << input << "\n";
        return input;
    }
    
    static void processString(const std::string& input) {
        std::string validated = validateString(input);
        std::string transformed = transformString(validated);
        std::cout << "✅ Строка обработана: " << transformed << "\n";
    }
    
    static void processInt(int input) {
        int validated = validateInt(input);
        int transformed = transformInt(validated);
        std::cout << "✅ Число обработано: " << transformed << "\n";
    }
};

// ============================================================================
// ПРИМЕР 2: КОНФЛИКТ МЕЖДУ YAGNI И DRY
// ============================================================================

/**
 * @brief Нарушение YAGNI ради соблюдения DRY
 * 
 * Этот пример показывает, как преждевременное устранение дублирования
 * может нарушить принцип YAGNI.
 */
class OverDRYForYAGNI {
private:
    // Сложная абстракция для будущих потребностей
    class AbstractDataProcessor {
    public:
        virtual ~AbstractDataProcessor() = default;
        virtual void process() = 0;
        virtual void validate() = 0;
        virtual void transform() = 0;
        virtual void persist() = 0;
        virtual void cleanup() = 0;
    };
    
    class UserDataProcessor : public AbstractDataProcessor {
    private:
        std::string data;
    public:
        UserDataProcessor(const std::string& userData) : data(userData) {}
        
        void process() override {
            std::cout << "👤 Обработка пользовательских данных: " << data << "\n";
        }
        
        void validate() override {
            std::cout << "✅ Валидация пользовательских данных\n";
        }
        
        void transform() override {
            std::cout << "🔄 Трансформация пользовательских данных\n";
        }
        
        void persist() override {
            std::cout << "💾 Сохранение пользовательских данных\n";
        }
        
        void cleanup() override {
            std::cout << "🧹 Очистка пользовательских данных\n";
        }
    };
    
    class ProductDataProcessor : public AbstractDataProcessor {
    private:
        std::string data;
    public:
        ProductDataProcessor(const std::string& productData) : data(productData) {}
        
        void process() override {
            std::cout << "📦 Обработка данных продукта: " << data << "\n";
        }
        
        void validate() override {
            std::cout << "✅ Валидация данных продукта\n";
        }
        
        void transform() override {
            std::cout << "🔄 Трансформация данных продукта\n";
        }
        
        void persist() override {
            std::cout << "💾 Сохранение данных продукта\n";
        }
        
        void cleanup() override {
            std::cout << "🧹 Очистка данных продукта\n";
        }
    };
    
    std::vector<std::unique_ptr<AbstractDataProcessor>> processors;
    
public:
    OverDRYForYAGNI() {
        std::cout << "🏗️ Создана сложная система процессоров (нарушение YAGNI ради DRY)\n";
    }
    
    void addUserProcessor(const std::string& data) {
        processors.push_back(std::make_unique<UserDataProcessor>(data));
    }
    
    void addProductProcessor(const std::string& data) {
        processors.push_back(std::make_unique<ProductDataProcessor>(data));
    }
    
    void processAll() {
        for (const auto& processor : processors) {
            processor->validate();
            processor->transform();
            processor->persist();
            processor->cleanup();
        }
    }
};

/**
 * @brief Соблюдение YAGNI с приемлемым дублированием
 * 
 * Этот пример показывает простой подход с минимальной абстракцией.
 */
class SimpleYAGNIExample {
public:
    static void processUserData(const std::string& data) {
        std::cout << "👤 Обработка пользовательских данных: " << data << "\n";
        std::cout << "✅ Валидация пользовательских данных\n";
        std::cout << "🔄 Трансформация пользовательских данных\n";
        std::cout << "💾 Сохранение пользовательских данных\n";
        std::cout << "🧹 Очистка пользовательских данных\n";
    }
    
    static void processProductData(const std::string& data) {
        std::cout << "📦 Обработка данных продукта: " << data << "\n";
        std::cout << "✅ Валидация данных продукта\n";
        std::cout << "🔄 Трансформация данных продукта\n";
        std::cout << "💾 Сохранение данных продукта\n";
        std::cout << "🧹 Очистка данных продукта\n";
    }
};

// ============================================================================
// ПРИМЕР 3: БАЛАНСИРОВКА ВСЕХ ПРИНЦИПОВ
// ============================================================================

/**
 * @brief Сбалансированный подход к принципам
 * 
 * Этот пример показывает, как можно соблюдать все три принципа
 * в разумных пределах.
 */
class BalancedApproach {
public:
    // Простая система валидации (соблюдение KISS)
    class Validator {
    public:
        static bool isValidEmail(const std::string& email) {
            return !email.empty() && email.find('@') != std::string::npos;
        }
        
        static bool isValidPhone(const std::string& phone) {
            return !phone.empty() && phone.length() >= 7;
        }
        
        static bool isValidPassword(const std::string& password) {
            return password.length() >= 8;
        }
    };
    
    // Простая система уведомлений (соблюдение YAGNI)
    class NotificationService {
    public:
        static void sendEmail(const std::string& email, const std::string& message) {
            std::cout << "📧 Отправка email на " << email << ": " << message << "\n";
        }
        
        static void sendSMS(const std::string& phone, const std::string& message) {
            std::cout << "📱 Отправка SMS на " << phone << ": " << message << "\n";
        }
    };
    
    // Простая система логирования (соблюдение DRY)
    class Logger {
    private:
        static void log(const std::string& level, const std::string& message) {
            auto now = std::chrono::system_clock::now();
            auto time_t = std::chrono::system_clock::to_time_t(now);
            std::cout << "[" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") 
                      << "] [" << level << "] " << message << "\n";
        }
        
    public:
        static void info(const std::string& message) {
            log("INFO", message);
        }
        
        static void error(const std::string& message) {
            log("ERROR", message);
        }
        
        static void warn(const std::string& message) {
            log("WARN", message);
        }
    };
    
    // Основной класс, использующий все компоненты
    class UserService {
    public:
        static void registerUser(const std::string& email, const std::string& phone, 
                                const std::string& password) {
            
            Logger::info("Начало регистрации пользователя");
            
            // Валидация (простая, но эффективная)
            if (!Validator::isValidEmail(email)) {
                Logger::error("Невалидный email: " + email);
                return;
            }
            
            if (!Validator::isValidPhone(phone)) {
                Logger::error("Невалидный телефон: " + phone);
                return;
            }
            
            if (!Validator::isValidPassword(password)) {
                Logger::error("Слишком короткий пароль");
                return;
            }
            
            // Регистрация (простая логика)
            Logger::info("Пользователь успешно зарегистрирован: " + email);
            
            // Уведомления (только необходимые)
            NotificationService::sendEmail(email, "Добро пожаловать!");
            NotificationService::sendSMS(phone, "Регистрация завершена");
        }
    };
};

// ============================================================================
// АНАЛИЗ ВЗАИМОДЕЙСТВИЯ ПРИНЦИПОВ
// ============================================================================

void analyzePrincipleInteractions() {
    std::cout << "\n🔬 АНАЛИЗ ВЗАИМОДЕЙСТВИЯ ПРИНЦИПОВ:\n";
    std::cout << std::string(60, '-') << "\n";
    
    std::cout << "📊 КОНФЛИКТЫ:\n";
    std::cout << "• DRY vs KISS: Устранение дублирования может усложнить код\n";
    std::cout << "• DRY vs YAGNI: Преждевременная абстракция для устранения дублирования\n";
    std::cout << "• KISS vs YAGNI: Простота может противоречить планированию\n\n";
    
    std::cout << "🤝 СИНЕРГИЯ:\n";
    std::cout << "• DRY + KISS: Элегантные абстракции без излишней сложности\n";
    std::cout << "• KISS + YAGNI: Простые решения для текущих потребностей\n";
    std::cout << "• DRY + YAGNI: Переиспользуемые компоненты только при необходимости\n\n";
    
    std::cout << "⚖️ БАЛАНСИРОВКА:\n";
    std::cout << "• Приоритизируйте принципы в зависимости от контекста\n";
    std::cout << "• Используйте принципы как руководство, а не догму\n";
    std::cout << "• Применяйте здравый смысл при принятии решений\n";
    std::cout << "• Регулярно пересматривайте архитектурные решения\n";
}

void demonstrateConflicts() {
    std::cout << "\n❌ ДЕМОНСТРАЦИЯ КОНФЛИКТОВ:\n";
    std::cout << std::string(50, '-') << "\n";
    
    std::cout << "Конфликт DRY vs KISS:\n";
    OverDRYExample overDRY;
    overDRY.processString("test", "validation");
    overDRY.processInt(42, "transformation");
    
    std::cout << "\nКонфликт YAGNI vs DRY:\n";
    OverDRYForYAGNI overDRYForYAGNI;
    overDRYForYAGNI.addUserProcessor("user data");
    overDRYForYAGNI.addProductProcessor("product data");
    overDRYForYAGNI.processAll();
}

void demonstrateBalancedApproach() {
    std::cout << "\n✅ СБАЛАНСИРОВАННЫЙ ПОДХОД:\n";
    std::cout << std::string(50, '-') << "\n";
    
    BalancedApproach::UserService::registerUser(
        "user@example.com", 
        "+1234567890", 
        "securepassword"
    );
    
    std::cout << "\nПростые функции (KISS):\n";
    BalancedApproach::SimpleExample::processString("hello");
    BalancedApproach::SimpleExample::processInt(123);
    
    std::cout << "\nПростой YAGNI подход:\n";
    BalancedApproach::SimpleYAGNIExample::processUserData("user info");
    BalancedApproach::SimpleYAGNIExample::processProductData("product info");
}

void provideRecommendations() {
    std::cout << "\n🎯 РЕКОМЕНДАЦИИ ПО БАЛАНСИРОВКЕ:\n";
    std::cout << std::string(50, '-') << "\n";
    
    std::cout << "📋 ПРИОРИТЕТЫ В РАЗНЫХ СЦЕНАРИЯХ:\n";
    std::cout << "• Прототипирование: KISS > YAGNI > DRY\n";
    std::cout << "• Производственный код: DRY > KISS > YAGNI\n";
    std::cout << "• Критичные системы: KISS > DRY > YAGNI\n";
    std::cout << "• Долгосрочные проекты: DRY > YAGNI > KISS\n\n";
    
    std::cout << "🔧 ПРАКТИЧЕСКИЕ СОВЕТЫ:\n";
    std::cout << "• Начинайте с простого решения (KISS)\n";
    std::cout << "• Устраняйте дублирование при третьем повторении (DRY)\n";
    std::cout << "• Добавляйте сложность только при необходимости (YAGNI)\n";
    std::cout << "• Регулярно рефакторите код\n";
    std::cout << "• Используйте метрики для принятия решений\n\n";
    
    std::cout << "📊 МЕТРИКИ ДЛЯ ОЦЕНКИ:\n";
    std::cout << "• Cyclomatic Complexity (KISS)\n";
    std::cout << "• Code Duplication Percentage (DRY)\n";
    std::cout << "• Feature Usage Statistics (YAGNI)\n";
    std::cout << "• Time to Understand Code (KISS)\n";
    std::cout << "• Maintenance Cost (все принципы)\n";
}

int main() {
    std::cout << "🎯 СРАВНЕНИЕ ПРИНЦИПОВ DRY, KISS, YAGNI\n";
    std::cout << "Автор: Senior C++ Developer\n";
    std::cout << "Цель: Анализ взаимодействия принципов разработки\n\n";
    
    demonstrateConflicts();
    demonstrateBalancedApproach();
    analyzePrincipleInteractions();
    provideRecommendations();
    
    std::cout << "\n📚 МАТЕМАТИЧЕСКОЕ ОБОСНОВАНИЕ:\n";
    std::cout << "DRY: ∀K ∈ Knowledge: |Representations(K)| = 1\n";
    std::cout << "KISS: Complexity(Code) = Σ(Complexity(Component)) → min\n";
    std::cout << "YAGNI: ∀F ∈ Features: F ∈ CurrentRequirements ∨ F ∈ ImmediateNeeds\n\n";
    
    std::cout << "💡 КЛЮЧЕВЫЕ ВЫВОДЫ:\n";
    std::cout << "1. Принципы могут конфликтовать друг с другом\n";
    std::cout << "2. Важен баланс между всеми принципами\n";
    std::cout << "3. Контекст определяет приоритеты\n";
    std::cout << "4. Здравый смысл важнее слепого следования принципам\n\n";
    
    std::cout << "🔬 Принципы - это инструменты для мышления о проблемах!\n";
    
    return 0;
}
