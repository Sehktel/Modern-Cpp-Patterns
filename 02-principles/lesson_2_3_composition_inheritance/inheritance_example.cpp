/**
 * @file inheritance_example.cpp
 * @brief Демонстрация наследования и его проблем
 * @author Senior C++ Developer
 * @date 2024
 * 
 * Этот файл демонстрирует различные проблемы наследования:
 * - Fragile Base Class Problem
 * - Diamond Problem
 * - Tight Coupling
 * - Violation of LSP
 */

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <stdexcept>

/**
 * @brief Математическая формализация наследования
 * 
 * Наследование можно формализовать следующим образом:
 * ∀A, B ∈ Classes: B inherits A ⟹ B ⊆ A
 * где ⊆ означает "является подмножеством"
 * 
 * Это означает, что все объекты класса B также являются объектами класса A.
 */

// ============================================================================
// ПРИМЕР 1: FRAGILE BASE CLASS PROBLEM
// ============================================================================

/**
 * @brief Проблема хрупкого базового класса
 * 
 * Изменения в базовом классе могут сломать производные классы,
 * даже если изменения кажутся безвредными.
 */
class BadBaseClass {
protected:
    std::vector<int> data;
    
public:
    BadBaseClass() {
        data = {1, 2, 3, 4, 5};
        std::cout << "🏗️ Базовый класс создан с данными: ";
        printData();
    }
    
    virtual ~BadBaseClass() = default;
    
    // Метод, который может быть изменен в будущем
    virtual void processData() {
        std::cout << "📊 Обработка данных в базовом классе\n";
        for (size_t i = 0; i < data.size(); ++i) {
            data[i] *= 2; // Простая обработка
        }
        printData();
    }
    
    virtual void printData() const {
        std::cout << "Данные: ";
        for (size_t i = 0; i < data.size(); ++i) {
            std::cout << data[i];
            if (i < data.size() - 1) std::cout << ", ";
        }
        std::cout << "\n";
    }
    
    // Метод, который может быть добавлен позже
    virtual void addData(int value) {
        data.push_back(value);
        std::cout << "➕ Добавлено значение: " << value << "\n";
    }
    
    // Геттер для данных
    const std::vector<int>& getData() const {
        return data;
    }
};

class BadDerivedClass : public BadBaseClass {
private:
    int multiplier;
    
public:
    BadDerivedClass(int mult = 3) : multiplier(mult) {
        std::cout << "🏗️ Производный класс создан с множителем: " << multiplier << "\n";
    }
    
    // Переопределение метода
    void processData() override {
        std::cout << "📊 Обработка данных в производном классе\n";
        
        // Производный класс полагается на размер данных из базового класса
        for (size_t i = 0; i < data.size(); ++i) {
            data[i] *= multiplier;
        }
        printData();
    }
    
    // Специальный метод производного класса
    void specialProcess() {
        std::cout << "⭐ Специальная обработка в производном классе\n";
        
        // Этот метод может сломаться, если базовый класс изменится
        if (data.size() > 0) {
            int sum = 0;
            for (int value : data) {
                sum += value;
            }
            std::cout << "Сумма данных: " << sum << "\n";
        }
    }
};

// ============================================================================
// ПРИМЕР 2: DIAMOND PROBLEM
// ============================================================================

/**
 * @brief Проблема ромбовидного наследования
 * 
 * Когда класс наследуется от двух классов, которые сами наследуются
 * от одного базового класса, возникает неоднозначность.
 */
class Animal {
protected:
    std::string name;
    int age;
    
public:
    Animal(const std::string& animalName, int animalAge) 
        : name(animalName), age(animalAge) {
        std::cout << "🐾 Животное создано: " << name << " (возраст: " << age << ")\n";
    }
    
    virtual ~Animal() = default;
    
    virtual void makeSound() {
        std::cout << "🔊 " << name << " издает звук\n";
    }
    
    virtual void move() {
        std::cout << "🚶 " << name << " двигается\n";
    }
    
    const std::string& getName() const { return name; }
    int getAge() const { return age; }
};

class Mammal : public Animal {
protected:
    bool hasFur;
    
public:
    Mammal(const std::string& name, int age, bool fur = true) 
        : Animal(name, age), hasFur(fur) {
        std::cout << "🐕 Млекопитающее создано с мехом: " << (hasFur ? "да" : "нет") << "\n";
    }
    
    void makeSound() override {
        std::cout << "🔊 " << name << " издает звук млекопитающего\n";
    }
    
    void feedMilk() {
        std::cout << "🥛 " << name << " кормит молоком\n";
    }
    
    bool hasFur() const { return hasFur; }
};

class WingedAnimal : public Animal {
protected:
    int wingSpan;
    
public:
    WingedAnimal(const std::string& name, int age, int span = 100) 
        : Animal(name, age), wingSpan(span) {
        std::cout << "🦅 Крылатое животное создано с размахом крыльев: " << wingSpan << " см\n";
    }
    
    void makeSound() override {
        std::cout << "🔊 " << name << " издает звук крылатого животного\n";
    }
    
    void fly() {
        std::cout << "✈️ " << name << " летает с размахом крыльев " << wingSpan << " см\n";
    }
    
    int getWingSpan() const { return wingSpan; }
};

// Проблема: Bat наследуется от Mammal и WingedAnimal
// Оба родителя наследуются от Animal, создавая неоднозначность
class BadBat : public Mammal, public WingedAnimal {
public:
    BadBat(const std::string& name, int age, int span = 50) 
        : Mammal(name, age, true), WingedAnimal(name, age, span) {
        std::cout << "🦇 Летучая мышь создана (проблемное наследование)\n";
    }
    
    void makeSound() override {
        std::cout << "🔊 " << name << " издает ультразвук\n";
    }
    
    // Проблема: какой name использовать? Mammal::name или WingedAnimal::name?
    void demonstrateProblem() {
        std::cout << "❌ Проблема: неоднозначность доступа к полям\n";
        std::cout << "Имя из Mammal: " << Mammal::name << "\n";
        std::cout << "Имя из WingedAnimal: " << WingedAnimal::name << "\n";
        std::cout << "Возраст из Mammal: " << Mammal::age << "\n";
        std::cout << "Возраст из WingedAnimal: " << WingedAnimal::age << "\n";
    }
};

// ============================================================================
// ПРИМЕР 3: VIOLATION OF LISKOV SUBSTITUTION PRINCIPLE
// ============================================================================

/**
 * @brief Нарушение принципа подстановки Лисков
 * 
 * Производные классы должны быть заменяемы базовыми классами
 * без изменения ожидаемого поведения.
 */
class BadRectangle {
protected:
    int width;
    int height;
    
public:
    BadRectangle(int w, int h) : width(w), height(h) {
        std::cout << "📐 Прямоугольник создан: " << width << "x" << height << "\n";
    }
    
    virtual void setWidth(int w) {
        width = w;
        std::cout << "📐 Ширина установлена: " << width << "\n";
    }
    
    virtual void setHeight(int h) {
        height = h;
        std::cout << "📐 Высота установлена: " << height << "\n";
    }
    
    virtual int getWidth() const { return width; }
    virtual int getHeight() const { return height; }
    
    virtual int getArea() const {
        return width * height;
    }
    
    virtual void printInfo() const {
        std::cout << "📐 Прямоугольник: " << width << "x" << height 
                  << ", площадь: " << getArea() << "\n";
    }
};

class BadSquare : public BadRectangle {
public:
    BadSquare(int side) : BadRectangle(side, side) {
        std::cout << "⬜ Квадрат создан со стороной: " << side << "\n";
    }
    
    // Нарушение LSP: изменение поведения базового класса
    void setWidth(int w) override {
        width = w;
        height = w; // Квадрат всегда имеет равные стороны
        std::cout << "⬜ Сторона квадрата установлена: " << w << "\n";
    }
    
    void setHeight(int h) override {
        width = h;  // Нарушение LSP: изменяем width при изменении height
        height = h;
        std::cout << "⬜ Сторона квадрата установлена: " << h << "\n";
    }
    
    void printInfo() const override {
        std::cout << "⬜ Квадрат: сторона " << width 
                  << ", площадь: " << getArea() << "\n";
    }
};

/**
 * @brief Функция, демонстрирующая нарушение LSP
 */
void demonstrateLSPViolation(BadRectangle& rect) {
    std::cout << "\n🔍 Тестирование LSP для " << (dynamic_cast<BadSquare*>(&rect) ? "квадрата" : "прямоугольника") << ":\n";
    
    int originalWidth = rect.getWidth();
    int originalHeight = rect.getHeight();
    int originalArea = rect.getArea();
    
    std::cout << "Исходные размеры: " << originalWidth << "x" << originalHeight 
              << ", площадь: " << originalArea << "\n";
    
    // Изменяем только ширину
    rect.setWidth(10);
    
    int newWidth = rect.getWidth();
    int newHeight = rect.getHeight();
    int newArea = rect.getArea();
    
    std::cout << "После setWidth(10): " << newWidth << "x" << newHeight 
              << ", площадь: " << newArea << "\n";
    
    // Проверяем инварианты LSP
    bool widthChanged = (newWidth != originalWidth);
    bool heightPreserved = (newHeight == originalHeight);
    bool behaviorConsistent = widthChanged && heightPreserved;
    
    std::cout << "✅ Ширина изменилась: " << (widthChanged ? "Да" : "Нет") << "\n";
    std::cout << "✅ Высота сохранилась: " << (heightPreserved ? "Да" : "Нет") << "\n";
    std::cout << "❌ Поведение согласовано: " << (behaviorConsistent ? "Да" : "Нет") << "\n";
    
    if (!behaviorConsistent) {
        std::cout << "🚨 НАРУШЕНИЕ LSP: Изменение ширины повлияло на высоту!\n";
    }
}

// ============================================================================
// ПРИМЕР 4: TIGHT COUPLING PROBLEM
// ============================================================================

/**
 * @brief Проблема тесной связанности
 * 
 * Наследование создает тесную связанность между базовым
 * и производными классами.
 */
class BadDatabaseConnection {
protected:
    std::string connectionString;
    bool connected;
    
public:
    BadDatabaseConnection(const std::string& connStr) 
        : connectionString(connStr), connected(false) {
        std::cout << "🗄️ Соединение с БД создано: " << connectionString << "\n";
    }
    
    virtual ~BadDatabaseConnection() {
        if (connected) {
            disconnect();
        }
    }
    
    virtual void connect() {
        connected = true;
        std::cout << "🔌 Подключение к БД установлено\n";
    }
    
    virtual void disconnect() {
        connected = false;
        std::cout << "🔌 Соединение с БД закрыто\n";
    }
    
    virtual void executeQuery(const std::string& query) {
        if (!connected) {
            throw std::runtime_error("Нет соединения с БД");
        }
        std::cout << "📝 Выполнение запроса: " << query << "\n";
    }
    
    // Метод, который может изменить сигнатуру в будущем
    virtual void beginTransaction() {
        if (!connected) {
            throw std::runtime_error("Нет соединения с БД");
        }
        std::cout << "🔄 Начало транзакции\n";
    }
    
    virtual void commitTransaction() {
        std::cout << "✅ Подтверждение транзакции\n";
    }
    
    virtual void rollbackTransaction() {
        std::cout << "❌ Откат транзакции\n";
    }
};

class BadMySQLConnection : public BadDatabaseConnection {
private:
    std::string mysqlVersion;
    
public:
    BadMySQLConnection(const std::string& connStr, const std::string& version = "8.0") 
        : BadDatabaseConnection(connStr), mysqlVersion(version) {
        std::cout << "🐬 MySQL соединение создано (версия: " << mysqlVersion << ")\n";
    }
    
    void connect() override {
        BadDatabaseConnection::connect();
        std::cout << "🐬 MySQL подключение установлено\n";
    }
    
    void executeQuery(const std::string& query) override {
        BadDatabaseConnection::executeQuery(query);
        std::cout << "🐬 MySQL запрос выполнен\n";
    }
    
    // Специфичные для MySQL методы
    void useDatabase(const std::string& dbName) {
        if (!connected) {
            throw std::runtime_error("Нет соединения с MySQL");
        }
        std::cout << "🐬 Использование базы данных: " << dbName << "\n";
    }
    
    void showTables() {
        if (!connected) {
            throw std::runtime_error("Нет соединения с MySQL");
        }
        std::cout << "🐬 Показать таблицы\n";
    }
};

class BadPostgreSQLConnection : public BadDatabaseConnection {
private:
    std::string pgVersion;
    
public:
    BadPostgreSQLConnection(const std::string& connStr, const std::string& version = "13") 
        : BadDatabaseConnection(connStr), pgVersion(version) {
        std::cout << "🐘 PostgreSQL соединение создано (версия: " << pgVersion << ")\n";
    }
    
    void connect() override {
        BadDatabaseConnection::connect();
        std::cout << "🐘 PostgreSQL подключение установлено\n";
    }
    
    void executeQuery(const std::string& query) override {
        BadDatabaseConnection::executeQuery(query);
        std::cout << "🐘 PostgreSQL запрос выполнен\n";
    }
    
    // Специфичные для PostgreSQL методы
    void createSchema(const std::string& schemaName) {
        if (!connected) {
            throw std::runtime_error("Нет соединения с PostgreSQL");
        }
        std::cout << "🐘 Создание схемы: " << schemaName << "\n";
    }
    
    void createExtension(const std::string& extensionName) {
        if (!connected) {
            throw std::runtime_error("Нет соединения с PostgreSQL");
        }
        std::cout << "🐘 Создание расширения: " << extensionName << "\n";
    }
};

// ============================================================================
// ДЕМОНСТРАЦИЯ ПРОБЛЕМ
// ============================================================================

void demonstrateFragileBaseClass() {
    std::cout << "❌ FRAGILE BASE CLASS PROBLEM:\n";
    std::cout << std::string(50, '-') << "\n";
    
    BadBaseClass base;
    base.processData();
    
    BadDerivedClass derived(5);
    derived.processData();
    derived.specialProcess();
    
    std::cout << "\n⚠️ Проблема: Изменения в BadBaseClass могут сломать BadDerivedClass\n";
    std::cout << "Например, если изменить порядок инициализации или добавить новые методы\n";
}

void demonstrateDiamondProblem() {
    std::cout << "\n❌ DIAMOND PROBLEM:\n";
    std::cout << std::string(50, '-') << "\n";
    
    BadBat bat("Бэтмен", 5, 30);
    bat.makeSound();
    bat.feedMilk();
    bat.fly();
    
    std::cout << "\n";
    bat.demonstrateProblem();
    
    std::cout << "\n⚠️ Проблема: Неоднозначность доступа к полям и методам\n";
    std::cout << "Дублирование данных и сложность разрешения имен\n";
}

void demonstrateLSPViolation() {
    std::cout << "\n❌ VIOLATION OF LSP:\n";
    std::cout << std::string(50, '-') << "\n";
    
    BadRectangle rect(5, 10);
    demonstrateLSPViolation(rect);
    
    BadSquare square(5);
    demonstrateLSPViolation(square);
    
    std::cout << "\n⚠️ Проблема: Square не может быть заменен на Rectangle\n";
    std::cout << "Нарушение ожидаемого поведения при замене объектов\n";
}

void demonstrateTightCoupling() {
    std::cout << "\n❌ TIGHT COUPLING PROBLEM:\n";
    std::cout << std::string(50, '-') << "\n";
    
    BadMySQLConnection mysql("mysql://localhost:3306/mydb");
    mysql.connect();
    mysql.useDatabase("testdb");
    mysql.executeQuery("SELECT * FROM users");
    mysql.showTables();
    
    std::cout << "\n";
    BadPostgreSQLConnection postgres("postgresql://localhost:5432/mydb");
    postgres.connect();
    postgres.createSchema("public");
    postgres.executeQuery("SELECT * FROM users");
    postgres.createExtension("uuid-ossp");
    
    std::cout << "\n⚠️ Проблема: Тесная связанность с базовым классом\n";
    std::cout << "Изменения в BadDatabaseConnection влияют на все производные классы\n";
}

void analyzeInheritanceProblems() {
    std::cout << "\n🔬 АНАЛИЗ ПРОБЛЕМ НАСЛЕДОВАНИЯ:\n";
    std::cout << std::string(50, '-') << "\n";
    
    std::cout << "📊 ОСНОВНЫЕ ПРОБЛЕМЫ:\n";
    std::cout << "• Fragile Base Class - изменения в базовом классе ломают производные\n";
    std::cout << "• Diamond Problem - неоднозначность при множественном наследовании\n";
    std::cout << "• Tight Coupling - тесная связанность между классами\n";
    std::cout << "• Violation of LSP - нарушение принципа подстановки Лисков\n";
    std::cout << "• Inheritance Hell - глубокие иерархии наследования\n\n";
    
    std::cout << "⚠️ ПОСЛЕДСТВИЯ:\n";
    std::cout << "• Сложность тестирования\n";
    std::cout << "• Сложность сопровождения\n";
    std::cout << "• Снижение гибкости\n";
    std::cout << "• Нарушение принципов SOLID\n\n";
    
    std::cout << "🎯 РЕКОМЕНДАЦИИ:\n";
    std::cout << "• Используйте наследование только для истинных 'is-a' отношений\n";
    std::cout << "• Предпочитайте композицию наследованию\n";
    std::cout << "• Избегайте глубоких иерархий наследования\n";
    std::cout << "• Применяйте принципы SOLID\n";
}

int main() {
    std::cout << "🎯 ДЕМОНСТРАЦИЯ ПРОБЛЕМ НАСЛЕДОВАНИЯ\n";
    std::cout << "Автор: Senior C++ Developer\n";
    std::cout << "Цель: Понимание проблем наследования и необходимости альтернатив\n\n";
    
    demonstrateFragileBaseClass();
    demonstrateDiamondProblem();
    demonstrateLSPViolation();
    demonstrateTightCoupling();
    analyzeInheritanceProblems();
    
    std::cout << "\n📚 МАТЕМАТИЧЕСКОЕ ОБОСНОВАНИЕ:\n";
    std::cout << "Наследование: ∀A, B ∈ Classes: B inherits A ⟹ B ⊆ A\n";
    std::cout << "где ⊆ означает 'является подмножеством'\n\n";
    
    std::cout << "💡 КЛЮЧЕВЫЕ ВЫВОДЫ:\n";
    std::cout << "1. Наследование создает тесную связанность\n";
    std::cout << "2. Изменения в базовом классе могут сломать производные классы\n";
    std::cout << "3. Множественное наследование создает неоднозначности\n";
    std::cout << "4. Наследование может нарушать принципы SOLID\n\n";
    
    std::cout << "🔬 Композиция часто является лучшим выбором!\n";
    
    return 0;
}
