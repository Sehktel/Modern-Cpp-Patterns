# 🔗 Композиция vs Наследование - Плакат

## 📋 Обзор концепции

**Композиция vs Наследование** - это фундаментальный принцип объектно-ориентированного проектирования, который определяет, когда использовать композицию объектов вместо наследования классов. Правильный выбор между этими подходами критически важен для создания гибких и поддерживаемых систем.

## 🎯 Назначение

- **Выбор архитектуры**: Определение оптимального способа организации объектов
- **Гибкость дизайна**: Создание более гибких и расширяемых систем
- **Избежание проблем**: Предотвращение типичных проблем наследования
- **Следование принципам**: Применение принципов SOLID и лучших практик

## 🏗️ Сравнение подходов

### Наследование (Inheritance)
```
┌─────────────────┐
│   Base Class    │
│   (базовый)     │
├─────────────────┤
│ + commonMethod()│
│ + virtualMethod()│
└─────────────────┘
         ▲
         │
    ┌────┴────┐
    │         │
┌───▼───┐ ┌──▼───┐
│Derived1│ │Derived2│
├───────┤ ├───────┤
│+method │ │+method │
└───────┘ └───────┘
```

### Композиция (Composition)
```
┌─────────────────┐    ┌─────────────────┐
│   Container     │    │   Component     │
│   (контейнер)   │◄───┤   (компонент)   │
├─────────────────┤    ├─────────────────┤
│ - component     │    │ + operation()   │
│ + delegate()    │    └─────────────────┘
└─────────────────┘
```

## 💡 Ключевые принципы

### 1. "Предпочитайте композицию наследованию"
```cpp
// ❌ Плохо: Наследование для переиспользования кода
class Bird {
public:
    virtual void fly() = 0;
    virtual void eat() = 0;
};

class Penguin : public Bird {
public:
    void fly() override {
        throw std::runtime_error("Пингвины не летают!");
    }
    void eat() override {
        std::cout << "Пингвин ест рыбу" << std::endl;
    }
};

// ✅ Хорошо: Композиция для переиспользования поведения
class Flyable {
public:
    virtual void fly() = 0;
};

class Eatable {
public:
    virtual void eat() = 0;
};

class Penguin {
private:
    std::unique_ptr<Eatable> eatingBehavior;
    
public:
    Penguin(std::unique_ptr<Eatable> behavior) 
        : eatingBehavior(std::move(behavior)) {}
    
    void eat() {
        eatingBehavior->eat();
    }
    
    // Пингвины не летают, поэтому не включаем Flyable
};
```

### 2. "IS-A vs HAS-A отношения"
```cpp
// IS-A: Наследование (когда объект ЯВЛЯЕТСЯ типом)
class Animal {
public:
    virtual void makeSound() = 0;
};

class Dog : public Animal {  // Собака ЯВЛЯЕТСЯ животным
public:
    void makeSound() override {
        std::cout << "Гав!" << std::endl;
    }
};

// HAS-A: Композиция (когда объект ИМЕЕТ компонент)
class Car {
private:
    std::unique_ptr<Engine> engine;  // Машина ИМЕЕТ двигатель
    
public:
    Car(std::unique_ptr<Engine> eng) : engine(std::move(eng)) {}
    
    void start() {
        engine->start();
    }
};
```

## 🔧 Практический пример: Система рендеринга

```cpp
#include <iostream>
#include <memory>
#include <vector>
#include <string>

// Базовый класс для рендеринга (наследование оправдано)
class Renderable {
public:
    virtual ~Renderable() = default;
    virtual void render() = 0;
    virtual std::string getType() const = 0;
};

// Компоненты для различных аспектов поведения
class Transform {
private:
    float x, y, z;
    
public:
    Transform(float x = 0, float y = 0, float z = 0) 
        : x(x), y(y), z(z) {}
    
    void setPosition(float newX, float newY, float newZ) {
        x = newX; y = newY; z = newZ;
    }
    
    void translate(float dx, float dy, float dz) {
        x += dx; y += dy; z += dz;
    }
    
    std::string getPosition() const {
        return "(" + std::to_string(x) + ", " + 
               std::to_string(y) + ", " + std::to_string(z) + ")";
    }
};

class Material {
private:
    std::string name;
    std::string color;
    float shininess;
    
public:
    Material(const std::string& matName, const std::string& matColor, float shine = 0.5f)
        : name(matName), color(matColor), shininess(shine) {}
    
    void setColor(const std::string& newColor) {
        color = newColor;
    }
    
    std::string getInfo() const {
        return name + " (" + color + ", shine: " + std::to_string(shininess) + ")";
    }
};

class CollisionDetector {
public:
    virtual ~CollisionDetector() = default;
    virtual bool checkCollision(const Transform& transform) = 0;
    virtual std::string getCollisionType() const = 0;
};

class BoxCollision : public CollisionDetector {
private:
    float width, height, depth;
    
public:
    BoxCollision(float w, float h, float d) : width(w), height(h), depth(d) {}
    
    bool checkCollision(const Transform& transform) override {
        // Простая проверка коллизий
        std::cout << "Проверка коллизии Box (" << width << "x" << height << "x" << depth << ")" << std::endl;
        return true; // Упрощенная логика
    }
    
    std::string getCollisionType() const override {
        return "Box Collision";
    }
};

class SphereCollision : public CollisionDetector {
private:
    float radius;
    
public:
    SphereCollision(float r) : radius(r) {}
    
    bool checkCollision(const Transform& transform) override {
        std::cout << "Проверка коллизии Sphere (r=" << radius << ")" << std::endl;
        return true; // Упрощенная логика
    }
    
    std::string getCollisionType() const override {
        return "Sphere Collision";
    }
};

// Игровой объект с композицией компонентов
class GameObject : public Renderable {
private:
    std::string name;
    Transform transform;
    std::unique_ptr<Material> material;
    std::unique_ptr<CollisionDetector> collisionDetector;
    
public:
    GameObject(const std::string& objName) : name(objName) {}
    
    // Установка компонентов
    void setMaterial(std::unique_ptr<Material> mat) {
        material = std::move(mat);
    }
    
    void setCollisionDetector(std::unique_ptr<CollisionDetector> detector) {
        collisionDetector = std::move(detector);
    }
    
    // Делегирование к компонентам
    void move(float dx, float dy, float dz) {
        transform.translate(dx, dy, dz);
    }
    
    void setPosition(float x, float y, float z) {
        transform.setPosition(x, y, z);
    }
    
    bool checkCollision() {
        if (collisionDetector) {
            return collisionDetector->checkCollision(transform);
        }
        return false;
    }
    
    // Реализация Renderable
    void render() override {
        std::cout << "🎮 Рендеринг объекта: " << name << std::endl;
        std::cout << "   Позиция: " << transform.getPosition() << std::endl;
        
        if (material) {
            std::cout << "   Материал: " << material->getInfo() << std::endl;
        }
        
        if (collisionDetector) {
            std::cout << "   Коллизия: " << collisionDetector->getCollisionType() << std::endl;
        }
    }
    
    std::string getType() const override {
        return "GameObject";
    }
    
    std::string getName() const {
        return name;
    }
};

// Специализированные игровые объекты (наследование для полиморфизма)
class Player : public GameObject {
private:
    int health;
    int score;
    
public:
    Player(const std::string& playerName) : GameObject(playerName), health(100), score(0) {}
    
    void takeDamage(int damage) {
        health -= damage;
        std::cout << "Игрок получил урон: " << damage << ", здоровье: " << health << std::endl;
    }
    
    void addScore(int points) {
        score += points;
        std::cout << "Игрок получил очки: " << points << ", общий счет: " << score << std::endl;
    }
    
    void render() override {
        GameObject::render();
        std::cout << "   Здоровье: " << health << ", Счет: " << score << std::endl;
    }
    
    std::string getType() const override {
        return "Player";
    }
};

class Enemy : public GameObject {
private:
    int damage;
    bool isAggressive;
    
public:
    Enemy(const std::string& enemyName, int enemyDamage) 
        : GameObject(enemyName), damage(enemyDamage), isAggressive(false) {}
    
    void attack() {
        std::cout << "Враг атакует с уроном: " << damage << std::endl;
    }
    
    void setAggressive(bool aggressive) {
        isAggressive = aggressive;
        std::cout << "Враг " << (aggressive ? "стал" : "перестал быть") << " агрессивным" << std::endl;
    }
    
    void render() override {
        GameObject::render();
        std::cout << "   Урон: " << damage << ", Агрессивный: " << (isAggressive ? "Да" : "Нет") << std::endl;
    }
    
    std::string getType() const override {
        return "Enemy";
    }
};

// Менеджер объектов с композицией
class ObjectManager {
private:
    std::vector<std::unique_ptr<Renderable>> objects;
    
public:
    void addObject(std::unique_ptr<Renderable> obj) {
        objects.push_back(std::move(obj));
    }
    
    void renderAll() {
        std::cout << "\n🎨 Рендеринг всех объектов:" << std::endl;
        for (auto& obj : objects) {
            obj->render();
            std::cout << "---" << std::endl;
        }
    }
    
    void updateAll() {
        std::cout << "\n🔄 Обновление всех объектов:" << std::endl;
        for (auto& obj : objects) {
            // Обновление логики объектов
            std::cout << "Обновление " << obj->getType() << std::endl;
        }
    }
    
    size_t getObjectCount() const {
        return objects.size();
    }
};

// Демонстрация использования
int main() {
    std::cout << "=== Демонстрация Композиция vs Наследование ===" << std::endl;
    
    ObjectManager manager;
    
    // Создание игрока с компонентами
    auto player = std::make_unique<Player>("Герой");
    player->setMaterial(std::make_unique<Material>("Metal", "Silver", 0.8f));
    player->setCollisionDetector(std::make_unique<BoxCollision>(2.0f, 3.0f, 1.0f));
    player->setPosition(0, 0, 0);
    
    // Создание врага с компонентами
    auto enemy = std::make_unique<Enemy>("Орк", 25);
    enemy->setMaterial(std::make_unique<Material>("Leather", "Brown", 0.3f));
    enemy->setCollisionDetector(std::make_unique<SphereCollision>(1.5f));
    enemy->setPosition(5, 0, 0);
    enemy->setAggressive(true);
    
    // Добавление объектов в менеджер
    manager.addObject(std::move(player));
    manager.addObject(std::move(enemy));
    
    // Демонстрация работы
    std::cout << "\n📊 Статистика:" << std::endl;
    std::cout << "Количество объектов: " << manager.getObjectCount() << std::endl;
    
    manager.renderAll();
    manager.updateAll();
    
    // Демонстрация взаимодействия
    std::cout << "\n⚔️ Игровые события:" << std::endl;
    
    // Получаем объекты обратно для демонстрации (в реальном коде это было бы через ID или указатели)
    auto player2 = std::make_unique<Player>("Герой2");
    auto enemy2 = std::make_unique<Enemy>("Гоблин", 15);
    
    player2->takeDamage(20);
    player2->addScore(100);
    enemy2->attack();
    
    return 0;
}
```

## 🎯 Применение в реальных проектах

### 1. **Игровые движки**
```cpp
// Композиция компонентов вместо глубокого наследования
class Entity {
private:
    std::unordered_map<std::string, std::unique_ptr<Component>> components;
    
public:
    template<typename T>
    void addComponent(std::unique_ptr<T> component) {
        components[typeid(T).name()] = std::move(component);
    }
    
    template<typename T>
    T* getComponent() {
        auto it = components.find(typeid(T).name());
        return it != components.end() ? static_cast<T*>(it->second.get()) : nullptr;
    }
};
```

### 2. **GUI фреймворки**
```cpp
// Композиция виджетов вместо наследования
class Widget {
private:
    std::vector<std::unique_ptr<Widget>> children;
    std::unique_ptr<Layout> layout;
    std::unique_ptr<Style> style;
    
public:
    void addChild(std::unique_ptr<Widget> child) {
        children.push_back(std::move(child));
    }
    
    void setLayout(std::unique_ptr<Layout> newLayout) {
        layout = std::move(newLayout);
    }
};
```

### 3. **Системы плагинов**
```cpp
// Композиция функциональности через плагины
class Application {
private:
    std::vector<std::unique_ptr<Plugin>> plugins;
    
public:
    void loadPlugin(std::unique_ptr<Plugin> plugin) {
        plugins.push_back(std::move(plugin));
    }
    
    void executeFeature(const std::string& featureName) {
        for (auto& plugin : plugins) {
            if (plugin->supportsFeature(featureName)) {
                plugin->execute(featureName);
            }
        }
    }
};
```

## ⚡ Преимущества и недостатки

### ✅ **Композиция:**
**Плюсы:**
- **Гибкость**: Легко изменять поведение во время выполнения
- **Переиспользование**: Компоненты можно использовать в разных контекстах
- **Тестируемость**: Легко тестировать отдельные компоненты
- **Слабая связанность**: Компоненты независимы друг от друга

**Минусы:**
- **Сложность**: Больше кода для делегирования
- **Производительность**: Дополнительные вызовы методов
- **Понимание**: Сложнее понять полную картину системы

### ✅ **Наследование:**
**Плюсы:**
- **Простота**: Прямой доступ к методам базового класса
- **Полиморфизм**: Естественная поддержка виртуальных функций
- **Производительность**: Прямые вызовы методов
- **Понятность**: Четкая иерархия классов

**Минусы:**
- **Жесткость**: Сложно изменять поведение во время выполнения
- **Связанность**: Тесная связь между классами
- **Хрупкость**: Изменения в базовом классе влияют на все производные
- **Ограниченность**: Один базовый класс на класс

## 🔄 Когда использовать каждый подход

### **Используйте наследование когда:**
- ✅ Отношение IS-A (объект ЯВЛЯЕТСЯ типом)
- ✅ Нужен полиморфизм
- ✅ Поведение не изменяется во время выполнения
- ✅ Есть общая функциональность для всех производных классов

### **Используйте композицию когда:**
- ✅ Отношение HAS-A (объект ИМЕЕТ компонент)
- ✅ Нужна гибкость во время выполнения
- ✅ Компоненты могут быть переиспользованы
- ✅ Нужна слабая связанность

## 🎨 Современные улучшения в C++

### 1. **Использование std::variant (C++17)**
```cpp
#include <variant>

// Композиция с type-safe вариантами
class FlexibleObject {
private:
    std::variant<Transform2D, Transform3D> transform;
    std::variant<Material, Texture> appearance;
    
public:
    template<typename T>
    void setTransform(T&& t) {
        transform = std::forward<T>(t);
    }
    
    void render() {
        std::visit([](const auto& t) {
            // Обработка различных типов трансформаций
        }, transform);
    }
};
```

### 2. **Концепты и шаблоны (C++20)**
```cpp
#include <concepts>

// Концепт для компонентов
template<typename T>
concept Component = requires(T t) {
    t.update();
    t.render();
};

// Шаблонный менеджер компонентов
template<Component... Components>
class ComponentManager {
private:
    std::tuple<std::vector<Components>...> componentArrays;
    
public:
    template<typename T>
    void addComponent(T&& component) {
        std::get<std::vector<T>>(componentArrays).push_back(std::forward<T>(component));
    }
};
```

## 🚀 Лучшие практики

### 1. **Правильный выбор подхода**
```cpp
// ✅ Хорошо: Наследование для полиморфизма
class Shape {
public:
    virtual double area() const = 0;
    virtual void draw() const = 0;
};

class Circle : public Shape {
    // Circle IS-A Shape
};

// ✅ Хорошо: Композиция для поведения
class Car {
private:
    std::unique_ptr<Engine> engine;  // Car HAS-A Engine
    std::unique_ptr<Wheels> wheels; // Car HAS-A Wheels
    
public:
    void start() { engine->start(); }
    void stop() { engine->stop(); }
};
```

### 2. **Избегание проблем наследования**
```cpp
// ❌ Плохо: Наследование для переиспользования кода
class Database {
public:
    void connect() { /* ... */ }
    void disconnect() { /* ... */ }
};

class FileSystem : public Database {  // FileSystem IS-NOT-A Database!
    // Принудительное наследование для переиспользования кода
};

// ✅ Хорошо: Композиция для переиспользования
class ConnectionManager {
public:
    virtual void connect() = 0;
    virtual void disconnect() = 0;
};

class Database {
private:
    std::unique_ptr<ConnectionManager> connection;
    
public:
    Database(std::unique_ptr<ConnectionManager> conn) 
        : connection(std::move(conn)) {}
    
    void connect() { connection->connect(); }
    void disconnect() { connection->disconnect(); }
};
```

### 3. **Гибкая архитектура**
```cpp
// ✅ Хорошо: Гибкая система с композицией
class FlexibleSystem {
private:
    std::vector<std::unique_ptr<Component>> components;
    
public:
    template<typename T, typename... Args>
    void addComponent(Args&&... args) {
        components.push_back(std::make_unique<T>(std::forward<Args>(args)...));
    }
    
    template<typename T>
    T* getComponent() {
        for (auto& comp : components) {
            if (auto* t = dynamic_cast<T*>(comp.get())) {
                return t;
            }
        }
        return nullptr;
    }
};
```

## 📊 Сравнение подходов

| Аспект | Наследование | Композиция |
|--------|--------------|------------|
| **Гибкость** | Низкая | Высокая |
| **Производительность** | Высокая | Средняя |
| **Тестируемость** | Средняя | Высокая |
| **Переиспользование** | Ограниченное | Высокое |
| **Связанность** | Высокая | Низкая |
| **Сложность** | Низкая | Средняя |
| **Расширяемость** | Ограниченная | Высокая |

## 🎯 Заключение

**Композиция vs Наследование** - это фундаментальный выбор в архитектуре программного обеспечения. Правильный выбор между этими подходами определяет качество, гибкость и поддерживаемость системы.

### **Ключевые принципы:**
- ✅ **"Предпочитайте композицию наследованию"** - основной принцип
- ✅ **IS-A vs HAS-A** - используйте наследование для IS-A, композицию для HAS-A
- ✅ **Гибкость во время выполнения** - композиция обеспечивает большую гибкость
- ✅ **Слабая связанность** - композиция создает более слабо связанные системы

### **Когда использовать:**
- **Наследование**: Для полиморфизма и IS-A отношений
- **Композиция**: Для гибкости, переиспользования и HAS-A отношений

**Правильный выбор между композицией и наследованием - это основа создания качественной, гибкой и поддерживаемой архитектуры!** 🔗✨
