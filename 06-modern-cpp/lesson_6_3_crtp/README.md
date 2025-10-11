# Урок 6.3: CRTP - Curiously Recurring Template Pattern

## 🎯 Цель урока
Изучить CRTP - мощную идиому современного C++ для compile-time полиморфизма. Понять, как использовать статический полиморфизм для создания высокопроизводительного кода без overhead виртуальных вызовов.

## 📚 Что изучаем

### 1. CRTP (Curiously Recurring Template Pattern)
- **Определение**: Производный класс наследуется от шаблона, инстанциированного самим собой
- **Назначение**: Статический полиморфизм без виртуальных функций
- **Применение**: Миксины, интерфейсы без overhead, compile-time оптимизации
- **Принцип**: "Compile-time полиморфизм вместо runtime"

### 2. Преимущества CRTP
- **Zero overhead**: Нет виртуальных таблиц и косвенных вызовов
- **Compile-time resolution**: Все решается на этапе компиляции
- **Inline-friendly**: Компилятор может агрессивно инлайнить
- **Type safety**: Ошибки обнаруживаются на этапе компиляции

### 3. Применение
- **Static interfaces**: Интерфейсы без virtual
- **Mixins**: Добавление функциональности через наследование
- **Compile-time plugins**: Расширение поведения на этапе компиляции
- **Performance-critical code**: Когда каждый цикл процессора важен

## 🔍 Ключевые концепции

### Базовая структура CRTP
```cpp
// ============================================================================
// БАЗОВАЯ ИДИОМА CRTP
// ============================================================================

// Базовый класс-шаблон, параметризованный производным классом
template<typename Derived>
class Base {
public:
    // Метод, который вызывает метод производного класса
    void interface() {
        // Downcast к производному классу
        static_cast<Derived*>(this)->implementation();
    }
    
    // Static polymorphism
    void staticPolymorphicMethod() {
        std::cout << "Base: вызов метода производного класса" << std::endl;
        static_cast<Derived*>(this)->derivedMethod();
    }
    
protected:
    // Защищенный конструктор - только для производных классов
    Base() = default;
    
    // Дружественность для доступа к приватным членам
    friend Derived;
};

// Производный класс наследуется от Base<Derived>
class Derived : public Base<Derived> {
public:
    // Реализация метода, который будет вызван через базовый класс
    void implementation() {
        std::cout << "Derived::implementation() вызван" << std::endl;
    }
    
    void derivedMethod() {
        std::cout << "Derived::derivedMethod() вызван" << std::endl;
    }
};

// Использование
void demonstrateBasicCRTP() {
    Derived d;
    d.interface();              // Вызывает Derived::implementation()
    d.staticPolymorphicMethod(); // Вызывает Derived::derivedMethod()
    
    // Нет виртуальных вызовов! Все resolve на этапе компиляции!
}
```

**Теоретическая база**: CRTP использует механизм Barton-Nackman trick из метапрограммирования. Через static_cast<Derived*>(this) базовый класс получает доступ к методам производного класса во время компиляции. Сложность вызова: O(1) как прямой вызов функции, vs O(1) + overhead на vtable lookup для виртуальных функций.

### CRTP vs Virtual Functions
```cpp
// ============================================================================
// СРАВНЕНИЕ: CRTP VS VIRTUAL
// ============================================================================

// Runtime полиморфизм (традиционный ООП)
class BaseVirtual {
public:
    virtual ~BaseVirtual() = default;
    virtual void operation() = 0;  // Виртуальная функция
};

class DerivedVirtual : public BaseVirtual {
public:
    void operation() override {
        std::cout << "DerivedVirtual::operation()" << std::endl;
    }
};

// Compile-time полиморфизм (CRTP)
template<typename Derived>
class BaseCRTP {
public:
    void operation() {
        static_cast<Derived*>(this)->operation();
    }
};

class DerivedCRTP : public BaseCRTP<DerivedCRTP> {
public:
    void operation() {
        std::cout << "DerivedCRTP::operation()" << std::endl;
    }
};

// Performance benchmark
void performanceBenchmark() {
    const int iterations = 1000000;
    
    // Virtual function
    BaseVirtual* virt = new DerivedVirtual();
    auto start1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        virt->operation();  // Виртуальный вызов через vtable
    }
    auto end1 = std::chrono::high_resolution_clock::now();
    auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1);
    
    // CRTP
    DerivedCRTP crtp;
    auto start2 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        crtp.operation();  // Прямой вызов, может быть инлайнен
    }
    auto end2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2);
    
    std::cout << "Virtual: " << duration1.count() << " μs" << std::endl;
    std::cout << "CRTP: " << duration2.count() << " μs" << std::endl;
    std::cout << "Ускорение: " << (double)duration1.count() / duration2.count() << "x" << std::endl;
    
    delete virt;
}
```

### CRTP для интерфейсов
```cpp
// ============================================================================
// CRTP ДЛЯ СОЗДАНИЯ ИНТЕРФЕЙСОВ БЕЗ VIRTUAL
// ============================================================================

// Generic алгоритм, работающий с CRTP интерфейсом
template<typename Derived>
class Drawable {
public:
    void render() {
        // Вызываем метод производного класса
        static_cast<Derived*>(this)->draw();
    }
    
    void renderWithBorder() {
        std::cout << "=== Рендеринг с границей ===" << std::endl;
        static_cast<Derived*>(this)->draw();
        std::cout << "=== Граница завершена ===" << std::endl;
    }
};

// Конкретные реализации
class Circle : public Drawable<Circle> {
public:
    void draw() {
        std::cout << "Рисуем круг" << std::endl;
    }
};

class Rectangle : public Drawable<Rectangle> {
public:
    void draw() {
        std::cout << "Рисуем прямоугольник" << std::endl;
    }
};

class Triangle : public Drawable<Triangle> {
public:
    void draw() {
        std::cout << "Рисуем треугольник" << std::endl;
    }
};

// Generic функция, работающая с любым Drawable
template<typename T>
void renderShape(Drawable<T>& shape) {
    shape.render();
}

// Использование
void demonstrateDrawableInterface() {
    Circle circle;
    Rectangle rect;
    Triangle triangle;
    
    renderShape(circle);     // Статический полиморфизм!
    renderShape(rect);
    renderShape(triangle);
    
    circle.renderWithBorder();
    
    // Всё resolve на этапе компиляции
    // Нет overhead виртуальных вызовов
}
```

**Теоретическая база**: CRTP реализует parametric polymorphism через templates, в отличие от subtype polymorphism через inheritance. Это позволяет компилятору генерировать specialized код для каждого типа, что открывает возможности для агрессивных оптимизаций: inlining, constant propagation, dead code elimination.

## 🤔 Вопросы для размышления

### 1. Когда использовать CRTP?
**✅ Хорошие случаи:**
- Performance-critical код (игры, HFT, embedded)
- Статические интерфейсы
- Миксины и добавление функциональности
- Compile-time полиморфизм
- Когда runtime полиморфизм не нужен

**❌ Плохие случаи:**
- Нужен runtime полиморфизм
- Коллекции разнородных объектов
- Плагины, загружаемые в runtime
- Когда простота важнее производительности

### 2. Преимущества CRTP
```cpp
// ✅ Нет виртуальных вызовов
// Компилятор может инлайнить всё
template<typename T>
inline void processDrawable(Drawable<T>& obj) {
    obj.render();  // Может быть полностью инлайнен
}

// ✅ Compile-time ошибки
// Ошибки обнаруживаются на этапе компиляции
class BadCircle : public Drawable<BadCircle> {
    // Если забыли draw(), получим ошибку компиляции
};

// ✅ Zero overhead
// Нет vtable, нет косвенных вызовов
sizeof(Circle) == размер данных  // Без vtable pointer

// ✅ Оптимизации компилятора
// Компилятор знает точный тип, может оптимизировать агрессивнее
```

### 3. Недостатки CRTP
```cpp
// ❌ Нет runtime полиморфизма
// Нельзя хранить в одной коллекции
std::vector<Drawable<???>> shapes;  // Невозможно!

// ❌ Более сложный код
// CRTP менее интуитивен чем virtual

// ❌ Раздутие кода (code bloat)
// Компилятор генерирует код для каждого типа
// Может увеличить размер бинарника

// ❌ Длинные имена типов
Drawable<Circle> circle;  // vs просто Circle в случае с virtual
```

## 🛠️ Практические примеры

### Пример 1: Миксины с CRTP
```cpp
// ============================================================================
// МИКСИНЫ С CRTP
// ============================================================================

// Миксин для логирования
template<typename Derived>
class Loggable {
protected:
    void log(const std::string& message) const {
        std::cout << "[" << typeid(Derived).name() << "] " << message << std::endl;
    }
    
public:
    void enableLogging() {
        std::cout << "Логирование включено для " << typeid(Derived).name() << std::endl;
    }
};

// Миксин для сериализации
template<typename Derived>
class Serializable {
public:
    std::string serialize() const {
        std::ostringstream oss;
        oss << "Serialized " << typeid(Derived).name();
        return oss.str();
    }
    
    void deserialize(const std::string& data) {
        std::cout << "Десериализация: " << data << std::endl;
    }
};

// Миксин для клонирования
template<typename Derived>
class Cloneable {
public:
    std::unique_ptr<Derived> clone() const {
        return std::make_unique<Derived>(*static_cast<const Derived*>(this));
    }
};

// Класс, использующий несколько миксинов
class MyClass : public Loggable<MyClass>,
                public Serializable<MyClass>,
                public Cloneable<MyClass> {
private:
    int value_;
    
public:
    explicit MyClass(int value) : value_(value) {}
    
    void doSomething() {
        log("Выполнение операции");
        std::cout << "Значение: " << value_ << std::endl;
    }
    
    int getValue() const { return value_; }
};

// Использование
void demonstrateMixins() {
    std::cout << "\n=== CRTP Миксины ===" << std::endl;
    
    MyClass obj(42);
    
    obj.enableLogging();
    obj.doSomething();
    
    std::string serialized = obj.serialize();
    std::cout << "Serialized: " << serialized << std::endl;
    
    auto cloned = obj.clone();
    std::cout << "Клонированное значение: " << cloned->getValue() << std::endl;
    
    // Все методы добавлены через миксины!
    // Нет виртуальных вызовов!
}
```

### Пример 2: Static Interface
```cpp
// ============================================================================
// СТАТИЧЕСКИЙ ИНТЕРФЕЙС С CRTP
// ============================================================================

// Базовый интерфейс для контейнера
template<typename Derived>
class Container {
public:
    size_t size() const {
        return static_cast<const Derived*>(this)->size_impl();
    }
    
    bool empty() const {
        return size() == 0;
    }
    
    void clear() {
        static_cast<Derived*>(this)->clear_impl();
    }
    
    // Generic алгоритм, работающий для всех контейнеров
    void printInfo() const {
        std::cout << "Контейнер " << typeid(Derived).name() << std::endl;
        std::cout << "Размер: " << size() << std::endl;
        std::cout << "Пустой: " << (empty() ? "да" : "нет") << std::endl;
    }
};

// Конкретный контейнер: вектор
class MyVector : public Container<MyVector> {
private:
    std::vector<int> data_;
    
public:
    size_t size_impl() const {
        return data_.size();
    }
    
    void clear_impl() {
        data_.clear();
    }
    
    void push_back(int value) {
        data_.push_back(value);
    }
};

// Конкретный контейнер: список
class MyList : public Container<MyList> {
private:
    std::list<int> data_;
    
public:
    size_t size_impl() const {
        return data_.size();
    }
    
    void clear_impl() {
        data_.clear();
    }
    
    void push_back(int value) {
        data_.push_back(value);
    }
};

// Generic функция для любого Container
template<typename T>
void processContainer(Container<T>& container) {
    container.printInfo();
    
    if (!container.empty()) {
        std::cout << "Контейнер не пустой, очищаем..." << std::endl;
        container.clear();
        container.printInfo();
    }
}

// Использование
void demonstrateStaticInterface() {
    std::cout << "\n=== Static Interface ===" << std::endl;
    
    MyVector vec;
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);
    
    MyList list;
    list.push_back(10);
    list.push_back(20);
    
    processContainer(vec);
    std::cout << std::endl;
    processContainer(list);
}
```

### Пример 3: Counted Base (подсчет экземпляров)
```cpp
// ============================================================================
// CRTP ДЛЯ ПОДСЧЕТА ЭКЗЕМПЛЯРОВ КЛАССОВ
// ============================================================================

// Базовый класс для подсчета экземпляров
template<typename Derived>
class Counted {
private:
    static inline int count_ = 0;  // C++17 inline static
    
protected:
    Counted() {
        ++count_;
    }
    
    Counted(const Counted&) {
        ++count_;
    }
    
    ~Counted() {
        --count_;
    }
    
public:
    static int getCount() {
        return count_;
    }
    
    static void printCount() {
        std::cout << "Количество экземпляров " << typeid(Derived).name() 
                  << ": " << count_ << std::endl;
    }
};

// Классы, которые хотят подсчет экземпляров
class Widget : public Counted<Widget> {
private:
    int id_;
    
public:
    explicit Widget(int id) : id_(id) {
        std::cout << "Widget " << id_ << " создан" << std::endl;
    }
    
    ~Widget() {
        std::cout << "Widget " << id_ << " уничтожен" << std::endl;
    }
};

class Gadget : public Counted<Gadget> {
private:
    std::string name_;
    
public:
    explicit Gadget(const std::string& name) : name_(name) {
        std::cout << "Gadget '" << name_ << "' создан" << std::endl;
    }
    
    ~Gadget() {
        std::cout << "Gadget '" << name_ << "' уничтожен" << std::endl;
    }
};

// Использование
void demonstrateCounted() {
    std::cout << "\n=== Counted Base ===" << std::endl;
    
    Widget::printCount();  // 0
    Gadget::printCount();  // 0
    
    {
        Widget w1(1);
        Widget w2(2);
        Widget::printCount();  // 2
        
        Gadget g1("Alpha");
        Gadget::printCount();  // 1
        
        {
            Gadget g2("Beta");
            Gadget g3("Gamma");
            Gadget::printCount();  // 3
        }  // g2, g3 уничтожены
        
        Gadget::printCount();  // 1
    }  // Все уничтожены
    
    Widget::printCount();  // 0
    Gadget::printCount();  // 0
}
```

### Пример 4: Equality Comparison
```cpp
// ============================================================================
// CRTP ДЛЯ АВТОМАТИЧЕСКОЙ ГЕНЕРАЦИИ ОПЕРАТОРОВ СРАВНЕНИЯ
// ============================================================================

// Базовый класс, предоставляющий все операторы сравнения
template<typename Derived>
class EqualityComparable {
public:
    // Производный класс должен предоставить operator==
    friend bool operator==(const Derived& lhs, const Derived& rhs) {
        return lhs.equals(rhs);
    }
    
    // Автоматически генерируем operator!=
    friend bool operator!=(const Derived& lhs, const Derived& rhs) {
        return !(lhs == rhs);
    }
};

template<typename Derived>
class Orderable : public EqualityComparable<Derived> {
public:
    // Производный класс должен предоставить operator<
    friend bool operator<(const Derived& lhs, const Derived& rhs) {
        return lhs.lessThan(rhs);
    }
    
    // Автоматически генерируем остальные операторы
    friend bool operator>(const Derived& lhs, const Derived& rhs) {
        return rhs < lhs;
    }
    
    friend bool operator<=(const Derived& lhs, const Derived& rhs) {
        return !(rhs < lhs);
    }
    
    friend bool operator>=(const Derived& lhs, const Derived& rhs) {
        return !(lhs < rhs);
    }
};

// Класс, использующий CRTP для операторов
class Point : public Orderable<Point> {
private:
    int x_, y_;
    
public:
    Point(int x, int y) : x_(x), y_(y) {}
    
    // Нужно реализовать только эти два метода
    bool equals(const Point& other) const {
        return x_ == other.x_ && y_ == other.y_;
    }
    
    bool lessThan(const Point& other) const {
        if (x_ != other.x_) return x_ < other.x_;
        return y_ < other.y_;
    }
    
    void print() const {
        std::cout << "(" << x_ << ", " << y_ << ")" << std::endl;
    }
};

// Использование
void demonstrateComparison() {
    std::cout << "\n=== Equality Comparison ===" << std::endl;
    
    Point p1(1, 2);
    Point p2(1, 2);
    Point p3(2, 3);
    
    std::cout << "p1 == p2: " << (p1 == p2) << std::endl;  // true
    std::cout << "p1 != p3: " << (p1 != p3) << std::endl;  // true
    std::cout << "p1 < p3: " << (p1 < p3) << std::endl;    // true
    std::cout << "p1 <= p2: " << (p1 <= p2) << std::endl;  // true
    std::cout << "p3 > p1: " << (p3 > p1) << std::endl;    // true
    
    // Все операторы сгенерированы автоматически!
}
```

## 🎨 Современные подходы

### CRTP с C++20 Concepts
```cpp
#include <concepts>

// Concept для CRTP
template<typename T, typename Base>
concept CRTPDerived = std::is_base_of_v<Base, T>;

// CRTP с проверкой через concept
template<typename Derived>
class CRTPBase {
    static_assert(std::is_base_of_v<CRTPBase, Derived>, 
                  "Derived must inherit from CRTPBase<Derived>");
public:
    void interface() {
        static_cast<Derived*>(this)->implementation();
    }
};
```

### CRTP с Variadic Templates
```cpp
// Множественное наследование от CRTP базовых классов
template<typename Derived, template<typename> class... Bases>
class MultiCRTP : public Bases<Derived>... {
protected:
    MultiCRTP() = default;
};

// Использование
class MyClass : public MultiCRTP<MyClass, Loggable, Serializable, Cloneable> {
    // Наследуем от всех миксинов одновременно
};
```

## 🚀 Применение в STL

### std::enable_shared_from_this
```cpp
// Пример использования CRTP в STL
class MyClass : public std::enable_shared_from_this<MyClass> {
public:
    std::shared_ptr<MyClass> getptr() {
        return shared_from_this();  // CRTP магия!
    }
};
```

## 🎯 Практические упражнения

### Упражнение 1: Arithmetic Operations
Создайте CRTP базовый класс для автоматической генерации арифметических операторов.

### Упражнение 2: Singleton CRTP
Реализуйте Singleton через CRTP для любого класса.

### Упражнение 3: Visitor Pattern
Реализуйте Visitor паттерн используя CRTP.

### Упражнение 4: Expression Templates
Создайте систему для оптимизации выражений через CRTP.

## 📈 Связь с другими паттернами

### CRTP + Mixin
```cpp
// Комбинирование множественных CRTP базовых классов
class FullFeatured : public Loggable<FullFeatured>,
                     public Serializable<FullFeatured>,
                     public Cloneable<FullFeatured> {
    // Все возможности через миксины
};
```

### CRTP + Singleton
```cpp
template<typename T>
class Singleton {
protected:
    Singleton() = default;
    
public:
    static T& getInstance() {
        static T instance;
        return instance;
    }
    
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
};

class MyService : public Singleton<MyService> {
    friend class Singleton<MyService>;
private:
    MyService() = default;
};
```

## 📈 Следующие шаги
После изучения CRTP вы будете готовы к:
- Уроку 6.4: Type Erasure
- Продвинутому метапрограммированию
- Expression templates
- Оптимизации производительности

## ⚠️ Важные предупреждения

1. **Проверяйте правильность наследования**: Derived должен наследоваться от Base<Derived>
2. **Учитывайте code bloat**: Каждый тип генерирует свой код
3. **Документируйте**: CRTP может быть неочевиден
4. **Не переусложняйте**: Используйте только когда нужна производительность

---

*"CRTP is the C++ idiom of having a class template take a derived class as a template argument."* - C++ Templates: The Complete Guide

