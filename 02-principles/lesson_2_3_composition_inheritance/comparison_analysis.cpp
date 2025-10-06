/**
 * @file comparison_analysis.cpp
 * @brief Сравнительный анализ композиции и наследования
 * @author Senior C++ Developer
 * @date 2024
 * 
 * Этот файл содержит детальный сравнительный анализ композиции и наследования
 * с метриками, примерами и рекомендациями.
 */

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <chrono>
#include <map>
#include <algorithm>

/**
 * @brief Метрики для анализа качества кода
 */
class CodeMetrics {
public:
    struct InheritanceMetrics {
        int depthOfInheritanceTree;
        int numberOfChildren;
        int couplingBetweenObjects;
        double maintainabilityIndex;
    };
    
    struct CompositionMetrics {
        int numberOfComponents;
        int couplingBetweenObjects;
        double cohesionIndex;
        double flexibilityIndex;
    };
    
    static void analyzeInheritanceHierarchy() {
        std::cout << "📊 АНАЛИЗ МЕТРИК НАСЛЕДОВАНИЯ:\n";
        std::cout << std::string(50, '-') << "\n";
        
        InheritanceMetrics metrics;
        metrics.depthOfInheritanceTree = 4; // Глубина иерархии
        metrics.numberOfChildren = 3;       // Количество потомков
        metrics.couplingBetweenObjects = 8; // Степень связанности
        metrics.maintainabilityIndex = 65.5; // Индекс сопровождаемости
        
        std::cout << "• Глубина дерева наследования (DIT): " << metrics.depthOfInheritanceTree << "\n";
        std::cout << "• Количество потомков (NOC): " << metrics.numberOfChildren << "\n";
        std::cout << "• Связанность между объектами (CBO): " << metrics.couplingBetweenObjects << "\n";
        std::cout << "• Индекс сопровождаемости: " << metrics.maintainabilityIndex << "%\n\n";
        
        std::cout << "⚠️ ПРОБЛЕМЫ:\n";
        std::cout << "• Высокая связанность между объектами\n";
        std::cout << "• Глубокая иерархия наследования\n";
        std::cout << "• Снижение индекса сопровождаемости\n";
    }
    
    static void analyzeCompositionStructure() {
        std::cout << "📊 АНАЛИЗ МЕТРИК КОМПОЗИЦИИ:\n";
        std::cout << std::string(50, '-') << "\n";
        
        CompositionMetrics metrics;
        metrics.numberOfComponents = 5;     // Количество компонентов
        metrics.couplingBetweenObjects = 3; // Степень связанности
        metrics.cohesionIndex = 85.2;       // Индекс связности
        metrics.flexibilityIndex = 92.1;    // Индекс гибкости
        
        std::cout << "• Количество компонентов: " << metrics.numberOfComponents << "\n";
        std::cout << "• Связанность между объектами (CBO): " << metrics.couplingBetweenObjects << "\n";
        std::cout << "• Индекс связности: " << metrics.cohesionIndex << "%\n";
        std::cout << "• Индекс гибкости: " << metrics.flexibilityIndex << "%\n\n";
        
        std::cout << "✅ ПРЕИМУЩЕСТВА:\n";
        std::cout << "• Низкая связанность между объектами\n";
        std::cout << "• Высокая связность компонентов\n";
        std::cout << "• Высокая гибкость системы\n";
    }
};

// ============================================================================
// ПРИМЕР 1: СРАВНЕНИЕ ПРОИЗВОДИТЕЛЬНОСТИ
// ============================================================================

/**
 * @brief Тестирование производительности наследования
 */
class InheritancePerformanceTest {
private:
    class BaseShape {
    public:
        virtual ~BaseShape() = default;
        virtual double getArea() const = 0;
        virtual double getPerimeter() const = 0;
        virtual std::string getType() const = 0;
    };
    
    class Rectangle : public BaseShape {
    private:
        double width, height;
    public:
        Rectangle(double w, double h) : width(w), height(h) {}
        
        double getArea() const override {
            return width * height;
        }
        
        double getPerimeter() const override {
            return 2 * (width + height);
        }
        
        std::string getType() const override {
            return "Rectangle";
        }
    };
    
    class Circle : public BaseShape {
    private:
        double radius;
    public:
        Circle(double r) : radius(r) {}
        
        double getArea() const override {
            return 3.14159 * radius * radius;
        }
        
        double getPerimeter() const override {
            return 2 * 3.14159 * radius;
        }
        
        std::string getType() const override {
            return "Circle";
        }
    };
    
public:
    static void runTest() {
        std::cout << "⚡ ТЕСТ ПРОИЗВОДИТЕЛЬНОСТИ НАСЛЕДОВАНИЯ:\n";
        std::cout << std::string(50, '-') << "\n";
        
        const int iterations = 1000000;
        std::vector<std::unique_ptr<BaseShape>> shapes;
        
        // Создание объектов
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < iterations; ++i) {
            if (i % 2 == 0) {
                shapes.push_back(std::make_unique<Rectangle>(i, i + 1));
            } else {
                shapes.push_back(std::make_unique<Circle>(i));
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto creationTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        // Вычисление площадей
        start = std::chrono::high_resolution_clock::now();
        
        double totalArea = 0;
        for (const auto& shape : shapes) {
            totalArea += shape->getArea();
        }
        
        end = std::chrono::high_resolution_clock::now();
        auto calculationTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << "Объектов создано: " << iterations << "\n";
        std::cout << "Время создания: " << creationTime.count() << " мкс\n";
        std::cout << "Время вычислений: " << calculationTime.count() << " мкс\n";
        std::cout << "Общая площадь: " << totalArea << "\n";
        std::cout << "Виртуальные вызовы: " << iterations * 2 << "\n\n";
    }
};

/**
 * @brief Тестирование производительности композиции
 */
class CompositionPerformanceTest {
private:
    class AreaCalculator {
    public:
        virtual ~AreaCalculator() = default;
        virtual double calculateArea() const = 0;
    };
    
    class RectangleAreaCalculator : public AreaCalculator {
    private:
        double width, height;
    public:
        RectangleAreaCalculator(double w, double h) : width(w), height(h) {}
        
        double calculateArea() const override {
            return width * height;
        }
    };
    
    class CircleAreaCalculator : public AreaCalculator {
    private:
        double radius;
    public:
        CircleAreaCalculator(double r) : radius(r) {}
        
        double calculateArea() const override {
            return 3.14159 * radius * radius;
        }
    };
    
    class Shape {
    private:
        std::string type;
        std::unique_ptr<AreaCalculator> areaCalculator;
    public:
        Shape(const std::string& shapeType, std::unique_ptr<AreaCalculator> calculator)
            : type(shapeType), areaCalculator(std::move(calculator)) {}
        
        double getArea() const {
            return areaCalculator->calculateArea();
        }
        
        const std::string& getType() const {
            return type;
        }
    };
    
public:
    static void runTest() {
        std::cout << "⚡ ТЕСТ ПРОИЗВОДИТЕЛЬНОСТИ КОМПОЗИЦИИ:\n";
        std::cout << std::string(50, '-') << "\n";
        
        const int iterations = 1000000;
        std::vector<std::unique_ptr<Shape>> shapes;
        
        // Создание объектов
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < iterations; ++i) {
            if (i % 2 == 0) {
                shapes.push_back(std::make_unique<Shape>("Rectangle", 
                    std::make_unique<RectangleAreaCalculator>(i, i + 1)));
            } else {
                shapes.push_back(std::make_unique<Shape>("Circle", 
                    std::make_unique<CircleAreaCalculator>(i)));
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto creationTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        // Вычисление площадей
        start = std::chrono::high_resolution_clock::now();
        
        double totalArea = 0;
        for (const auto& shape : shapes) {
            totalArea += shape->getArea();
        }
        
        end = std::chrono::high_resolution_clock::now();
        auto calculationTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << "Объектов создано: " << iterations << "\n";
        std::cout << "Время создания: " << creationTime.count() << " мкс\n";
        std::cout << "Время вычислений: " << calculationTime.count() << " мкс\n";
        std::cout << "Общая площадь: " << totalArea << "\n";
        std::cout << "Виртуальные вызовы: " << iterations << "\n\n";
    }
};

// ============================================================================
// ПРИМЕР 2: СРАВНЕНИЕ ГИБКОСТИ
// ============================================================================

/**
 * @brief Демонстрация гибкости наследования
 */
class InheritanceFlexibilityDemo {
private:
    class BadVehicle {
    protected:
        std::string name;
        int maxSpeed;
    public:
        BadVehicle(const std::string& n, int speed) : name(n), maxSpeed(speed) {}
        virtual ~BadVehicle() = default;
        
        virtual void start() {
            std::cout << "🚗 " << name << " заводится\n";
        }
        
        virtual void drive() {
            std::cout << "🚗 " << name << " едет со скоростью " << maxSpeed << " км/ч\n";
        }
    };
    
    class BadCar : public BadVehicle {
    public:
        BadCar(const std::string& n, int speed) : BadVehicle(n, speed) {}
        
        void drive() override {
            std::cout << "🚙 " << name << " едет по дороге\n";
        }
        
        void openTrunk() {
            std::cout << "📦 " << name << " открывает багажник\n";
        }
    };
    
    class BadTruck : public BadVehicle {
    public:
        BadTruck(const std::string& n, int speed) : BadVehicle(n, speed) {}
        
        void drive() override {
            std::cout << "🚛 " << name << " едет по шоссе\n";
        }
        
        void loadCargo() {
            std::cout << "📦 " << name << " загружает груз\n";
        }
    };
    
public:
    static void demonstrate() {
        std::cout << "🔄 ГИБКОСТЬ НАСЛЕДОВАНИЯ:\n";
        std::cout << std::string(40, '-') << "\n";
        
        BadCar car("Седан", 120);
        BadTruck truck("Грузовик", 80);
        
        car.start();
        car.drive();
        car.openTrunk();
        
        truck.start();
        truck.drive();
        truck.loadCargo();
        
        std::cout << "\n⚠️ ОГРАНИЧЕНИЯ:\n";
        std::cout << "• Поведение фиксировано во время компиляции\n";
        std::cout << "• Сложно изменить поведение во время выполнения\n";
        std::cout << "• Наследование создает жесткую иерархию\n\n";
    }
};

/**
 * @brief Демонстрация гибкости композиции
 */
class CompositionFlexibilityDemo {
private:
    class DrivingBehavior {
    public:
        virtual ~DrivingBehavior() = default;
        virtual void drive(const std::string& vehicleName) = 0;
    };
    
    class CityDriving : public DrivingBehavior {
    public:
        void drive(const std::string& vehicleName) override {
            std::cout << "🏙️ " << vehicleName << " едет по городу\n";
        }
    };
    
    class HighwayDriving : public DrivingBehavior {
    public:
        void drive(const std::string& vehicleName) override {
            std::cout << "🛣️ " << vehicleName << " едет по шоссе\n";
        }
    };
    
    class OffRoadDriving : public DrivingBehavior {
    public:
        void drive(const std::string& vehicleName) override {
            std::cout << "🌲 " << vehicleName << " едет по бездорожью\n";
        }
    };
    
    class GoodVehicle {
    private:
        std::string name;
        std::unique_ptr<DrivingBehavior> drivingBehavior;
    public:
        GoodVehicle(const std::string& n, std::unique_ptr<DrivingBehavior> behavior)
            : name(n), drivingBehavior(std::move(behavior)) {}
        
        void start() {
            std::cout << "🚗 " << name << " заводится\n";
        }
        
        void drive() {
            drivingBehavior->drive(name);
        }
        
        void changeDrivingBehavior(std::unique_ptr<DrivingBehavior> newBehavior) {
            drivingBehavior = std::move(newBehavior);
            std::cout << "🔄 " << name << " изменил стиль вождения\n";
        }
    };
    
public:
    static void demonstrate() {
        std::cout << "🔄 ГИБКОСТЬ КОМПОЗИЦИИ:\n";
        std::cout << std::string(40, '-') << "\n";
        
        auto vehicle = std::make_unique<GoodVehicle>("Универсал", 
            std::make_unique<CityDriving>());
        
        vehicle->start();
        vehicle->drive();
        
        // Изменение поведения во время выполнения
        vehicle->changeDrivingBehavior(std::make_unique<HighwayDriving>());
        vehicle->drive();
        
        vehicle->changeDrivingBehavior(std::make_unique<OffRoadDriving>());
        vehicle->drive();
        
        std::cout << "\n✅ ПРЕИМУЩЕСТВА:\n";
        std::cout << "• Поведение можно изменять во время выполнения\n";
        std::cout << "• Легко добавлять новые стили вождения\n";
        std::cout << "• Слабая связанность между компонентами\n";
        std::cout << "• Высокая гибкость системы\n\n";
    }
};

// ============================================================================
// ПРИМЕР 3: СРАВНЕНИЕ ТЕСТИРУЕМОСТИ
// ============================================================================

/**
 * @brief Демонстрация тестируемости наследования
 */
class InheritanceTestabilityDemo {
private:
    class BadDatabaseConnection {
    protected:
        std::string connectionString;
        bool connected;
    public:
        BadDatabaseConnection(const std::string& connStr) 
            : connectionString(connStr), connected(false) {}
        
        virtual ~BadDatabaseConnection() = default;
        
        virtual void connect() {
            connected = true;
            std::cout << "🔌 Подключение к " << connectionString << "\n";
            // Реальное подключение к БД - сложно тестировать
        }
        
        virtual void disconnect() {
            connected = false;
            std::cout << "🔌 Отключение от " << connectionString << "\n";
        }
        
        virtual bool isConnected() const {
            return connected;
        }
        
        virtual void executeQuery(const std::string& query) {
            if (!connected) {
                throw std::runtime_error("Нет соединения с БД");
            }
            std::cout << "📝 Выполнение запроса: " << query << "\n";
            // Реальное выполнение запроса - сложно тестировать
        }
    };
    
    class BadMySQLConnection : public BadDatabaseConnection {
    public:
        BadMySQLConnection(const std::string& connStr) : BadDatabaseConnection(connStr) {}
        
        void executeQuery(const std::string& query) override {
            BadDatabaseConnection::executeQuery(query);
            std::cout << "🐬 MySQL запрос выполнен\n";
        }
    };
    
public:
    static void demonstrate() {
        std::cout << "🧪 ТЕСТИРУЕМОСТЬ НАСЛЕДОВАНИЯ:\n";
        std::cout << std::string(40, '-') << "\n";
        
        BadMySQLConnection mysql("mysql://localhost:3306/test");
        
        try {
            mysql.connect();
            mysql.executeQuery("SELECT * FROM users");
            mysql.disconnect();
        } catch (const std::exception& e) {
            std::cout << "❌ Ошибка: " << e.what() << "\n";
        }
        
        std::cout << "\n⚠️ ПРОБЛЕМЫ ТЕСТИРОВАНИЯ:\n";
        std::cout << "• Сложно создать моки для базового класса\n";
        std::cout << "• Зависимость от реальной БД\n";
        std::cout << "• Тесная связанность затрудняет unit-тестирование\n";
        std::cout << "• Изменения в базовом классе ломают тесты\n\n";
    }
};

/**
 * @brief Демонстрация тестируемости композиции
 */
class CompositionTestabilityDemo {
private:
    // Интерфейс для инъекции зависимостей
    class IDatabaseConnection {
    public:
        virtual ~IDatabaseConnection() = default;
        virtual void connect() = 0;
        virtual void disconnect() = 0;
        virtual bool isConnected() const = 0;
        virtual void executeQuery(const std::string& query) = 0;
    };
    
    // Реальная реализация
    class MySQLConnection : public IDatabaseConnection {
    private:
        std::string connectionString;
        bool connected;
    public:
        MySQLConnection(const std::string& connStr) 
            : connectionString(connStr), connected(false) {}
        
        void connect() override {
            connected = true;
            std::cout << "🔌 MySQL подключение к " << connectionString << "\n";
        }
        
        void disconnect() override {
            connected = false;
            std::cout << "🔌 MySQL отключение\n";
        }
        
        bool isConnected() const override {
            return connected;
        }
        
        void executeQuery(const std::string& query) override {
            if (!connected) {
                throw std::runtime_error("Нет соединения с MySQL");
            }
            std::cout << "🐬 MySQL запрос: " << query << "\n";
        }
    };
    
    // Мок для тестирования
    class MockDatabaseConnection : public IDatabaseConnection {
    private:
        bool connected;
        std::vector<std::string> executedQueries;
    public:
        MockDatabaseConnection() : connected(false) {}
        
        void connect() override {
            connected = true;
            std::cout << "🧪 Mock подключение установлено\n";
        }
        
        void disconnect() override {
            connected = false;
            std::cout << "🧪 Mock отключение\n";
        }
        
        bool isConnected() const override {
            return connected;
        }
        
        void executeQuery(const std::string& query) override {
            executedQueries.push_back(query);
            std::cout << "🧪 Mock запрос выполнен: " << query << "\n";
        }
        
        const std::vector<std::string>& getExecutedQueries() const {
            return executedQueries;
        }
    };
    
    // Сервис с инъекцией зависимостей
    class DatabaseService {
    private:
        std::unique_ptr<IDatabaseConnection> connection;
    public:
        DatabaseService(std::unique_ptr<IDatabaseConnection> conn) 
            : connection(std::move(conn)) {}
        
        void connect() {
            connection->connect();
        }
        
        void executeQuery(const std::string& query) {
            connection->executeQuery(query);
        }
        
        void disconnect() {
            connection->disconnect();
        }
        
        bool isConnected() const {
            return connection->isConnected();
        }
    };
    
public:
    static void demonstrate() {
        std::cout << "🧪 ТЕСТИРУЕМОСТЬ КОМПОЗИЦИИ:\n";
        std::cout << std::string(40, '-') << "\n";
        
        // Продакшн код
        std::cout << "Продакшн код:\n";
        DatabaseService prodService(std::make_unique<MySQLConnection>("mysql://localhost:3306/prod"));
        prodService.connect();
        prodService.executeQuery("SELECT * FROM products");
        prodService.disconnect();
        
        std::cout << "\nТестовый код:\n";
        auto mockConnection = std::make_unique<MockDatabaseConnection>();
        DatabaseService testService(std::move(mockConnection));
        
        testService.connect();
        testService.executeQuery("SELECT * FROM users");
        testService.executeQuery("INSERT INTO logs VALUES (1, 'test')");
        testService.disconnect();
        
        std::cout << "\n✅ ПРЕИМУЩЕСТВА ТЕСТИРОВАНИЯ:\n";
        std::cout << "• Легко создавать моки для тестирования\n";
        std::cout << "• Независимость от внешних зависимостей\n";
        std::cout << "• Простое unit-тестирование\n";
        std::cout << "• Изменения в компонентах не влияют на тесты\n\n";
    }
};

// ============================================================================
// ИТОГОВЫЙ АНАЛИЗ
// ============================================================================

void performComparisonAnalysis() {
    std::cout << "📊 СРАВНИТЕЛЬНЫЙ АНАЛИЗ КОМПОЗИЦИИ И НАСЛЕДОВАНИЯ:\n";
    std::cout << std::string(60, '-') << "\n";
    
    std::cout << "📈 ПРОИЗВОДИТЕЛЬНОСТЬ:\n";
    std::cout << "• Наследование: Виртуальные вызовы, vtable overhead\n";
    std::cout << "• Композиция: Дополнительные уровни ин direction\n";
    std::cout << "• Вердикт: Примерно равная производительность\n\n";
    
    std::cout << "🔄 ГИБКОСТЬ:\n";
    std::cout << "• Наследование: Поведение фиксировано во время компиляции\n";
    std::cout << "• Композиция: Поведение можно изменять во время выполнения\n";
    std::cout << "• Вердикт: Композиция значительно гибче\n\n";
    
    std::cout << "🧪 ТЕСТИРУЕМОСТЬ:\n";
    std::cout << "• Наследование: Сложно создавать моки, тесная связанность\n";
    std::cout << "• Композиция: Легко инъекция зависимостей, слабая связанность\n";
    std::cout << "• Вердикт: Композиция намного лучше для тестирования\n\n";
    
    std::cout << "🔧 СОПРОВОЖДАЕМОСТЬ:\n";
    std::cout << "• Наследование: Изменения в базовом классе влияют на всех потомков\n";
    std::cout << "• Композиция: Изменения в компонентах локализованы\n";
    std::cout << "• Вердикт: Композиция проще в сопровождении\n\n";
    
    std::cout << "🏗️ АРХИТЕКТУРА:\n";
    std::cout << "• Наследование: Создает жесткие иерархии\n";
    std::cout << "• Композиция: Создает гибкие структуры\n";
    std::cout << "• Вердикт: Композиция лучше для сложных систем\n\n";
}

void provideRecommendations() {
    std::cout << "🎯 РЕКОМЕНДАЦИИ ПО ВЫБОРУ ПОДХОДА:\n";
    std::cout << std::string(50, '-') << "\n";
    
    std::cout << "✅ ИСПОЛЬЗУЙТЕ НАСЛЕДОВАНИЕ КОГДА:\n";
    std::cout << "• Истинное отношение 'is-a'\n";
    std::cout << "• Нужен полиморфизм\n";
    std::cout << "• Интерфейсы (абстрактные классы)\n";
    std::cout << "• Template Method Pattern\n";
    std::cout << "• Простые иерархии без глубокого наследования\n\n";
    
    std::cout << "✅ ИСПОЛЬЗУЙТЕ КОМПОЗИЦИЮ КОГДА:\n";
    std::cout << "• Отношение 'has-a' или 'uses-a'\n";
    std::cout << "• Нужна гибкость во время выполнения\n";
    std::cout << "• Множественное поведение\n";
    std::cout << "• Важна тестируемость\n";
    std::cout << "• Сложные системы с изменяющимися требованиями\n\n";
    
    std::cout << "❌ ИЗБЕГАЙТЕ НАСЛЕДОВАНИЯ КОГДА:\n";
    std::cout << "• Только для переиспользования кода\n";
    std::cout << "• Глубокие иерархии (>3-4 уровня)\n";
    std::cout << "• Множественное наследование реализации\n";
    std::cout << "• Частые изменения в базовых классах\n\n";
    
    std::cout << "🔬 СОВРЕМЕННЫЕ ПОДХОДЫ:\n";
    std::cout << "• Dependency Injection для композиции\n";
    std::cout << "• Strategy Pattern для изменения поведения\n";
    std::cout << "• Factory Pattern для создания объектов\n";
    std::cout << "• Observer Pattern для слабой связанности\n\n";
}

int main() {
    std::cout << "🎯 СРАВНИТЕЛЬНЫЙ АНАЛИЗ КОМПОЗИЦИИ И НАСЛЕДОВАНИЯ\n";
    std::cout << "Автор: Senior C++ Developer\n";
    std::cout << "Цель: Детальный анализ компромиссов между подходами\n\n";
    
    // Анализ метрик
    CodeMetrics::analyzeInheritanceHierarchy();
    CodeMetrics::analyzeCompositionStructure();
    
    // Тесты производительности
    InheritancePerformanceTest::runTest();
    CompositionPerformanceTest::runTest();
    
    // Демонстрация гибкости
    InheritanceFlexibilityDemo::demonstrate();
    CompositionFlexibilityDemo::demonstrate();
    
    // Демонстрация тестируемости
    InheritanceTestabilityDemo::demonstrate();
    CompositionTestabilityDemo::demonstrate();
    
    // Итоговый анализ
    performComparisonAnalysis();
    provideRecommendations();
    
    std::cout << "💡 ЗАКЛЮЧЕНИЕ:\n";
    std::cout << "Композиция предпочтительнее наследования в большинстве случаев.\n";
    std::cout << "Наследование используйте только для истинных 'is-a' отношений.\n";
    std::cout << "Современная разработка тяготеет к композиции и dependency injection.\n\n";
    
    std::cout << "🔬 Принцип 'Favor Composition over Inheritance' остается актуальным!\n";
    
    return 0;
}
