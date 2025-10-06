/**
 * @file composition_example.cpp
 * @brief Демонстрация композиции и ее преимуществ
 * @author Senior C++ Developer
 * @date 2024
 * 
 * Этот файл демонстрирует преимущества композиции над наследованием:
 * - Loose Coupling
 * - Flexibility
 * - Easier Testing
 * - Multiple Inheritance Simulation
 */

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <map>

/**
 * @brief Математическая формализация композиции
 * 
 * Композицию можно формализовать следующим образом:
 * ∀A, B ∈ Objects: A composed_of B ⟹ B ∈ parts(A)
 * где parts(A) = {x | x является частью объекта A}
 * 
 * Это означает, что объект A содержит объект B как свою часть.
 */

// ============================================================================
// ПРИМЕР 1: COMPOSITION OVER INHERITANCE
// ============================================================================

/**
 * @brief Проблема: Сложная иерархия наследования
 */
class BadVehicle {
protected:
    std::string name;
    int maxSpeed;
    int fuelCapacity;
    
public:
    BadVehicle(const std::string& vehicleName, int speed, int fuel) 
        : name(vehicleName), maxSpeed(speed), fuelCapacity(fuel) {
        std::cout << "🚗 Транспортное средство создано: " << name << "\n";
    }
    
    virtual ~BadVehicle() = default;
    
    virtual void start() {
        std::cout << "🚀 " << name << " заводится\n";
    }
    
    virtual void stop() {
        std::cout << "🛑 " << name << " останавливается\n";
    }
    
    virtual void accelerate() {
        std::cout << "⚡ " << name << " ускоряется\n";
    }
    
    virtual void brake() {
        std::cout << "🛑 " << name << " тормозит\n";
    }
};

class BadCar : public BadVehicle {
public:
    BadCar(const std::string& name, int speed, int fuel) 
        : BadVehicle(name, speed, fuel) {
        std::cout << "🚙 Автомобиль создан\n";
    }
    
    void openTrunk() {
        std::cout << "📦 " << name << " открывает багажник\n";
    }
};

class BadMotorcycle : public BadVehicle {
public:
    BadMotorcycle(const std::string& name, int speed, int fuel) 
        : BadVehicle(name, speed, fuel) {
        std::cout << "🏍️ Мотоцикл создан\n";
    }
    
    void wheelie() {
        std::cout << "🔄 " << name << " делает вилли\n";
    }
};

// Проблема: Что делать с гибридным транспортом?
class BadHybridVehicle : public BadCar {
private:
    bool electricMode;
    
public:
    BadHybridVehicle(const std::string& name, int speed, int fuel) 
        : BadCar(name, speed, fuel), electricMode(false) {
        std::cout << "🔋 Гибридное транспортное средство создано\n";
    }
    
    void switchToElectric() {
        electricMode = true;
        std::cout << "⚡ " << name << " переключился на электрический режим\n";
    }
    
    void switchToGas() {
        electricMode = false;
        std::cout << "⛽ " << name << " переключился на бензиновый режим\n";
    }
};

/**
 * @brief Решение: Композиция вместо наследования
 */
// Базовые компоненты
class Engine {
private:
    std::string type;
    int power;
    bool running;
    
public:
    Engine(const std::string& engineType, int enginePower) 
        : type(engineType), power(enginePower), running(false) {
        std::cout << "🔧 Двигатель создан: " << type << " (" << power << " л.с.)\n";
    }
    
    void start() {
        running = true;
        std::cout << "🚀 Двигатель " << type << " запущен\n";
    }
    
    void stop() {
        running = false;
        std::cout << "🛑 Двигатель " << type << " остановлен\n";
    }
    
    void accelerate() {
        if (running) {
            std::cout << "⚡ Двигатель " << type << " ускоряется\n";
        }
    }
    
    bool isRunning() const { return running; }
    const std::string& getType() const { return type; }
    int getPower() const { return power; }
};

class Wheel {
private:
    std::string material;
    int diameter;
    bool inflated;
    
public:
    Wheel(const std::string& wheelMaterial, int wheelDiameter) 
        : material(wheelMaterial), diameter(wheelDiameter), inflated(true) {
        std::cout << "⭕ Колесо создано: " << material << " (диаметр: " << diameter << " см)\n";
    }
    
    void inflate() {
        inflated = true;
        std::cout << "💨 Колесо " << material << " накачано\n";
    }
    
    void deflate() {
        inflated = false;
        std::cout << "💨 Колесо " << material << " спущено\n";
    }
    
    void brake() {
        std::cout << "🛑 Колесо " << material << " тормозит\n";
    }
    
    bool isInflated() const { return inflated; }
    const std::string& getMaterial() const { return material; }
    int getDiameter() const { return diameter; }
};

class FuelTank {
private:
    int capacity;
    int currentFuel;
    std::string fuelType;
    
public:
    FuelTank(int tankCapacity, const std::string& type = "gasoline") 
        : capacity(tankCapacity), currentFuel(tankCapacity), fuelType(type) {
        std::cout << "⛽ Топливный бак создан: " << capacity << " л (" << fuelType << ")\n";
    }
    
    void refuel(int amount) {
        currentFuel = std::min(capacity, currentFuel + amount);
        std::cout << "⛽ Заправка " << amount << " л " << fuelType << "\n";
    }
    
    void consume(int amount) {
        currentFuel = std::max(0, currentFuel - amount);
        std::cout << "⛽ Потребление " << amount << " л " << fuelType << "\n";
    }
    
    int getCurrentFuel() const { return currentFuel; }
    int getCapacity() const { return capacity; }
    const std::string& getFuelType() const { return fuelType; }
};

// Поведения как компоненты
class DrivingBehavior {
public:
    virtual ~DrivingBehavior() = default;
    virtual void drive() = 0;
    virtual std::string getBehaviorType() const = 0;
};

class NormalDriving : public DrivingBehavior {
public:
    void drive() override {
        std::cout << "🚗 Обычное вождение\n";
    }
    
    std::string getBehaviorType() const override {
        return "Normal";
    }
};

class AggressiveDriving : public DrivingBehavior {
public:
    void drive() override {
        std::cout << "🏎️ Агрессивное вождение\n";
    }
    
    std::string getBehaviorType() const override {
        return "Aggressive";
    }
};

class EcoDriving : public DrivingBehavior {
public:
    void drive() override {
        std::cout << "🌱 Эко-вождение\n";
    }
    
    std::string getBehaviorType() const override {
        return "Eco";
    }
};

// Основной класс, использующий композицию
class GoodVehicle {
private:
    std::string name;
    std::unique_ptr<Engine> engine;
    std::vector<std::unique_ptr<Wheel>> wheels;
    std::unique_ptr<FuelTank> fuelTank;
    std::unique_ptr<DrivingBehavior> drivingBehavior;
    
public:
    GoodVehicle(const std::string& vehicleName, 
                std::unique_ptr<Engine> vehicleEngine,
                std::unique_ptr<FuelTank> tank,
                std::unique_ptr<DrivingBehavior> behavior)
        : name(vehicleName), engine(std::move(vehicleEngine)), 
          fuelTank(std::move(tank)), drivingBehavior(std::move(behavior)) {
        std::cout << "🚗 Транспортное средство создано: " << name << "\n";
    }
    
    void addWheel(std::unique_ptr<Wheel> wheel) {
        wheels.push_back(std::move(wheel));
        std::cout << "⭕ Колесо добавлено к " << name << "\n";
    }
    
    void start() {
        std::cout << "🚀 Запуск " << name << "\n";
        if (engine) {
            engine->start();
        }
    }
    
    void stop() {
        std::cout << "🛑 Остановка " << name << "\n";
        if (engine) {
            engine->stop();
        }
    }
    
    void drive() {
        std::cout << "🚗 " << name << " начинает движение\n";
        if (drivingBehavior) {
            drivingBehavior->drive();
        }
        if (engine) {
            engine->accelerate();
        }
    }
    
    void brake() {
        std::cout << "🛑 " << name << " тормозит\n";
        for (const auto& wheel : wheels) {
            wheel->brake();
        }
    }
    
    void refuel(int amount) {
        std::cout << "⛽ Заправка " << name << "\n";
        if (fuelTank) {
            fuelTank->refuel(amount);
        }
    }
    
    void changeDrivingBehavior(std::unique_ptr<DrivingBehavior> newBehavior) {
        drivingBehavior = std::move(newBehavior);
        std::cout << "🔄 " << name << " изменил стиль вождения на " 
                  << drivingBehavior->getBehaviorType() << "\n";
    }
    
    // Геттеры для информации
    const std::string& getName() const { return name; }
    int getWheelCount() const { return wheels.size(); }
    std::string getEngineType() const { 
        return engine ? engine->getType() : "No engine"; 
    }
    std::string getFuelType() const { 
        return fuelTank ? fuelTank->getFuelType() : "No fuel tank"; 
    }
    std::string getDrivingBehavior() const { 
        return drivingBehavior ? drivingBehavior->getBehaviorType() : "No behavior"; 
    }
};

// ============================================================================
// ПРИМЕР 2: STRATEGY PATTERN WITH COMPOSITION
// ============================================================================

/**
 * @brief Демонстрация паттерна Strategy через композицию
 */
class PaymentStrategy {
public:
    virtual ~PaymentStrategy() = default;
    virtual bool pay(double amount) = 0;
    virtual std::string getPaymentType() const = 0;
};

class CreditCardPayment : public PaymentStrategy {
private:
    std::string cardNumber;
    std::string expiryDate;
    
public:
    CreditCardPayment(const std::string& card, const std::string& expiry) 
        : cardNumber(card), expiryDate(expiry) {}
    
    bool pay(double amount) override {
        std::cout << "💳 Оплата картой " << cardNumber.substr(0, 4) << "****: $" << amount << "\n";
        return true;
    }
    
    std::string getPaymentType() const override {
        return "Credit Card";
    }
};

class PayPalPayment : public PaymentStrategy {
private:
    std::string email;
    
public:
    PayPalPayment(const std::string& emailAddr) : email(emailAddr) {}
    
    bool pay(double amount) override {
        std::cout << "💰 PayPal оплата с " << email << ": $" << amount << "\n";
        return true;
    }
    
    std::string getPaymentType() const override {
        return "PayPal";
    }
};

class BankTransferPayment : public PaymentStrategy {
private:
    std::string accountNumber;
    
public:
    BankTransferPayment(const std::string& account) : accountNumber(account) {}
    
    bool pay(double amount) override {
        std::cout << "🏦 Банковский перевод с " << accountNumber << ": $" << amount << "\n";
        return true;
    }
    
    std::string getPaymentType() const override {
        return "Bank Transfer";
    }
};

class PaymentProcessor {
private:
    std::unique_ptr<PaymentStrategy> paymentStrategy;
    std::string processorName;
    
public:
    PaymentProcessor(const std::string& name, std::unique_ptr<PaymentStrategy> strategy)
        : processorName(name), paymentStrategy(std::move(strategy)) {}
    
    bool processPayment(double amount) {
        std::cout << "🔄 " << processorName << " обрабатывает платеж\n";
        if (paymentStrategy) {
            return paymentStrategy->pay(amount);
        }
        return false;
    }
    
    void changePaymentMethod(std::unique_ptr<PaymentStrategy> newStrategy) {
        paymentStrategy = std::move(newStrategy);
        std::cout << "🔄 " << processorName << " изменил метод оплаты на " 
                  << paymentStrategy->getPaymentType() << "\n";
    }
    
    std::string getCurrentPaymentMethod() const {
        return paymentStrategy ? paymentStrategy->getPaymentType() : "None";
    }
};

// ============================================================================
// ПРИМЕР 3: COMPOSITION FOR MULTIPLE INHERITANCE SIMULATION
// ============================================================================

/**
 * @brief Симуляция множественного наследования через композицию
 */
class Flyable {
public:
    virtual ~Flyable() = default;
    virtual void fly() = 0;
    virtual std::string getFlyableType() const = 0;
};

class Swimmable {
public:
    virtual ~Swimmable() = default;
    virtual void swim() = 0;
    virtual std::string getSwimmableType() const = 0;
};

class Walkable {
public:
    virtual ~Walkable() = default;
    virtual void walk() = 0;
    virtual std::string getWalkableType() const = 0;
};

// Конкретные реализации способностей
class BirdFlying : public Flyable {
public:
    void fly() override {
        std::cout << "🦅 Птица летает, махая крыльями\n";
    }
    
    std::string getFlyableType() const override {
        return "Bird Flying";
    }
};

class InsectFlying : public Flyable {
public:
    void fly() override {
        std::cout << "🦟 Насекомое летает, жужжа крыльями\n";
    }
    
    std::string getFlyableType() const override {
        return "Insect Flying";
    }
};

class FishSwimming : public Swimmable {
public:
    void swim() override {
        std::cout << "🐟 Рыба плывет, двигая плавниками\n";
    }
    
    std::string getSwimmableType() const override {
        return "Fish Swimming";
    }
};

class DuckSwimming : public Swimmable {
public:
    void swim() override {
        std::cout << "🦆 Утка плывет, гребя лапками\n";
    }
    
    std::string getSwimmableType() const override {
        return "Duck Swimming";
    }
};

class HumanWalking : public Walkable {
public:
    void walk() override {
        std::cout << "🚶 Человек идет на двух ногах\n";
    }
    
    std::string getWalkableType() const override {
        return "Human Walking";
    }
};

class DogWalking : public Walkable {
public:
    void walk() override {
        std::cout << "🐕 Собака идет на четырех лапах\n";
    }
    
    std::string getWalkableType() const override {
        return "Dog Walking";
    }
};

// Животное с композицией способностей
class Animal {
private:
    std::string name;
    std::string species;
    std::unique_ptr<Flyable> flyingAbility;
    std::unique_ptr<Swimmable> swimmingAbility;
    std::unique_ptr<Walkable> walkingAbility;
    
public:
    Animal(const std::string& animalName, const std::string& animalSpecies)
        : name(animalName), species(animalSpecies) {
        std::cout << "🐾 Животное создано: " << name << " (" << species << ")\n";
    }
    
    void addFlyingAbility(std::unique_ptr<Flyable> ability) {
        flyingAbility = std::move(ability);
        std::cout << "🦅 " << name << " получил способность летать: " 
                  << flyingAbility->getFlyableType() << "\n";
    }
    
    void addSwimmingAbility(std::unique_ptr<Swimmable> ability) {
        swimmingAbility = std::move(ability);
        std::cout << "🏊 " << name << " получил способность плавать: " 
                  << swimmingAbility->getSwimmableType() << "\n";
    }
    
    void addWalkingAbility(std::unique_ptr<Walkable> ability) {
        walkingAbility = std::move(ability);
        std::cout << "🚶 " << name << " получил способность ходить: " 
                  << walkingAbility->getWalkableType() << "\n";
    }
    
    void fly() {
        if (flyingAbility) {
            flyingAbility->fly();
        } else {
            std::cout << "❌ " << name << " не умеет летать\n";
        }
    }
    
    void swim() {
        if (swimmingAbility) {
            swimmingAbility->swim();
        } else {
            std::cout << "❌ " << name << " не умеет плавать\n";
        }
    }
    
    void walk() {
        if (walkingAbility) {
            walkingAbility->walk();
        } else {
            std::cout << "❌ " << name << " не умеет ходить\n";
        }
    }
    
    void showAbilities() {
        std::cout << "📋 Способности " << name << ":\n";
        std::cout << "  Летание: " << (flyingAbility ? "Да" : "Нет") << "\n";
        std::cout << "  Плавание: " << (swimmingAbility ? "Да" : "Нет") << "\n";
        std::cout << "  Ходьба: " << (walkingAbility ? "Да" : "Нет") << "\n";
    }
};

// ============================================================================
// ДЕМОНСТРАЦИЯ ПРЕИМУЩЕСТВ
// ============================================================================

void demonstrateCompositionBenefits() {
    std::cout << "✅ ПРЕИМУЩЕСТВА КОМПОЗИЦИИ:\n";
    std::cout << std::string(50, '-') << "\n";
    
    std::cout << "Создание транспортного средства с композицией:\n";
    
    // Создание компонентов
    auto engine = std::make_unique<Engine>("V8", 300);
    auto fuelTank = std::make_unique<FuelTank>(60, "gasoline");
    auto drivingBehavior = std::make_unique<NormalDriving>();
    
    // Создание транспортного средства
    GoodVehicle car("Мой автомобиль", std::move(engine), std::move(fuelTank), std::move(drivingBehavior));
    
    // Добавление колес
    for (int i = 0; i < 4; ++i) {
        car.addWheel(std::make_unique<Wheel>("Rubber", 65));
    }
    
    // Использование транспортного средства
    car.start();
    car.drive();
    car.brake();
    car.refuel(20);
    
    // Изменение поведения во время выполнения
    car.changeDrivingBehavior(std::make_unique<EcoDriving>());
    car.drive();
    
    std::cout << "\nИнформация о транспортном средстве:\n";
    std::cout << "Имя: " << car.getName() << "\n";
    std::cout << "Колес: " << car.getWheelCount() << "\n";
    std::cout << "Двигатель: " << car.getEngineType() << "\n";
    std::cout << "Топливо: " << car.getFuelType() << "\n";
    std::cout << "Стиль вождения: " << car.getDrivingBehavior() << "\n";
}

void demonstrateStrategyPattern() {
    std::cout << "\n✅ STRATEGY PATTERN С КОМПОЗИЦИЕЙ:\n";
    std::cout << std::string(50, '-') << "\n";
    
    // Создание процессора платежей с разными стратегиями
    PaymentProcessor processor1("Онлайн магазин", 
                               std::make_unique<CreditCardPayment>("1234567890123456", "12/25"));
    
    PaymentProcessor processor2("Сервис подписки", 
                               std::make_unique<PayPalPayment>("user@example.com"));
    
    PaymentProcessor processor3("B2B платформа", 
                               std::make_unique<BankTransferPayment>("9876543210987654"));
    
    // Обработка платежей
    processor1.processPayment(100.0);
    processor2.processPayment(25.0);
    processor3.processPayment(1000.0);
    
    // Изменение стратегии во время выполнения
    processor1.changePaymentMethod(std::make_unique<PayPalPayment>("customer@example.com"));
    processor1.processPayment(150.0);
}

void demonstrateMultipleInheritanceSimulation() {
    std::cout << "\n✅ СИМУЛЯЦИЯ МНОЖЕСТВЕННОГО НАСЛЕДОВАНИЯ:\n";
    std::cout << std::string(50, '-') << "\n";
    
    // Создание животных с разными способностями
    Animal duck("Дональд", "Утка");
    duck.addFlyingAbility(std::make_unique<BirdFlying>());
    duck.addSwimmingAbility(std::make_unique<DuckSwimming>());
    duck.addWalkingAbility(std::make_unique<DogWalking>());
    
    Animal fish("Немо", "Рыба-клоун");
    fish.addSwimmingAbility(std::make_unique<FishSwimming>());
    
    Animal bird("Твити", "Канарейка");
    bird.addFlyingAbility(std::make_unique<BirdFlying>());
    
    Animal human("Иван", "Человек");
    human.addWalkingAbility(std::make_unique<HumanWalking>());
    
    // Демонстрация способностей
    std::cout << "\nДемонстрация способностей:\n";
    
    duck.showAbilities();
    duck.fly();
    duck.swim();
    duck.walk();
    
    std::cout << "\n";
    fish.showAbilities();
    fish.swim();
    fish.fly();
    
    std::cout << "\n";
    bird.showAbilities();
    bird.fly();
    bird.swim();
    
    std::cout << "\n";
    human.showAbilities();
    human.walk();
    human.fly();
}

void analyzeCompositionAdvantages() {
    std::cout << "\n🔬 АНАЛИЗ ПРЕИМУЩЕСТВ КОМПОЗИЦИИ:\n";
    std::cout << std::string(50, '-') << "\n";
    
    std::cout << "📊 ОСНОВНЫЕ ПРЕИМУЩЕСТВА:\n";
    std::cout << "• Loose Coupling - слабая связанность между компонентами\n";
    std::cout << "• Flexibility - гибкость в изменении поведения\n";
    std::cout << "• Easier Testing - простота тестирования\n";
    std::cout << "• Multiple Inheritance Simulation - симуляция множественного наследования\n";
    std::cout << "• Runtime Behavior Changes - изменение поведения во время выполнения\n";
    std::cout << "• Better Encapsulation - лучшая инкапсуляция\n\n";
    
    std::cout << "✅ ПОСЛЕДСТВИЯ:\n";
    std::cout << "• Упрощение тестирования\n";
    std::cout << "• Улучшение maintainability\n";
    std::cout << "• Повышение гибкости\n";
    std::cout << "• Соблюдение принципов SOLID\n\n";
    
    std::cout << "🎯 РЕКОМЕНДАЦИИ:\n";
    std::cout << "• Предпочитайте композицию наследованию\n";
    std::cout << "• Используйте Strategy Pattern для изменения поведения\n";
    std::cout << "• Применяйте Dependency Injection\n";
    std::cout << "• Создавайте слабо связанные компоненты\n";
}

int main() {
    std::cout << "🎯 ДЕМОНСТРАЦИЯ ПРЕИМУЩЕСТВ КОМПОЗИЦИИ\n";
    std::cout << "Автор: Senior C++ Developer\n";
    std::cout << "Цель: Понимание преимуществ композиции над наследованием\n\n";
    
    demonstrateCompositionBenefits();
    demonstrateStrategyPattern();
    demonstrateMultipleInheritanceSimulation();
    analyzeCompositionAdvantages();
    
    std::cout << "\n📚 МАТЕМАТИЧЕСКОЕ ОБОСНОВАНИЕ:\n";
    std::cout << "Композиция: ∀A, B ∈ Objects: A composed_of B ⟹ B ∈ parts(A)\n";
    std::cout << "где parts(A) = {x | x является частью объекта A}\n\n";
    
    std::cout << "💡 КЛЮЧЕВЫЕ ВЫВОДЫ:\n";
    std::cout << "1. Композиция обеспечивает слабую связанность\n";
    std::cout << "2. Поведение можно изменять во время выполнения\n";
    std::cout << "3. Компоненты легко тестировать независимо\n";
    std::cout << "4. Композиция следует принципу 'Favor Composition over Inheritance'\n\n";
    
    std::cout << "🔬 Композиция - мощный инструмент для создания гибких систем!\n";
    
    return 0;
}
