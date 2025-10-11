# Урок 6.4: Type Erasure (Стирание типов)

## 🎯 Цель урока
Изучить Type Erasure - продвинутую технику C++ для создания единого интерфейса для разнородных типов без использования наследования. Понять, как комбинировать преимущества статического и динамического полиморфизма.

## 📚 Что изучаем

### 1. Type Erasure
- **Определение**: Техника для работы с разнородными типами через единый интерфейс
- **Назначение**: Гибкость наследования + производительность шаблонов
- **Применение**: std::function, std::any, std::variant, callbacks, plugin systems
- **Принцип**: "Скрываем тип, сохраняя интерфейс"

### 2. Компоненты Type Erasure
- **External Polymorphism**: Полиморфизм без изменения исходных типов
- **Small Buffer Optimization (SBO)**: Оптимизация для малых объектов
- **Value Semantics**: Поведение как у значения (копирование, перемещение)
- **Type-safe Interface**: Безопасная работа с типами

### 3. Примеры из STL
- **std::function**: Type erasure для callable объектов
- **std::any**: Type erasure для любого типа
- **std::variant**: Type-safe union
- **std::pmr**: Polymorphic allocators

## 🔍 Ключевые концепции

### Проблема: Разнородные типы без общего базового класса
```cpp
// ============================================================================
// ПРОБЛЕМА: Типы без общего интерфейса
// ============================================================================

// Разные типы с одинаковым интерфейсом, но без общего базового класса
class Dog {
public:
    void makeSound() const {
        std::cout << "Woof!" << std::endl;
    }
};

class Cat {
public:
    void makeSound() const {
        std::cout << "Meow!" << std::endl;
    }
};

class Bird {
public:
    void makeSound() const {
        std::cout << "Tweet!" << std::endl;
    }
};

// ❌ ПРОБЛЕМА: Как хранить их в одной коллекции?
// std::vector<???> animals;  // Нет общего типа!

// ❌ Не можем использовать шаблоны для гетерогенной коллекции
template<typename T>
void makeAllSounds(const std::vector<T>& animals) {
    // Работает только для одного типа!
}
```

**Теоретическая база**: Проблема duck typing в статически типизированном языке: разные типы имеют одинаковый интерфейс, но не связаны наследованием. Type erasure решает это через комбинацию наследования (для хранения) и шаблонов (для создания), реализуя паттерн External Polymorphism.

### Решение: Type Erasure
```cpp
// ============================================================================
// TYPE ERASURE IMPLEMENTATION
// ============================================================================

// Класс для хранения любого типа с методом makeSound()
class Animal {
private:
    // Абстрактный концепт - внутренний полиморфный интерфейс
    struct AnimalConcept {
        virtual ~AnimalConcept() = default;
        virtual void makeSound() const = 0;
        virtual std::unique_ptr<AnimalConcept> clone() const = 0;
    };
    
    // Модель - шаблонная обертка для конкретного типа
    template<typename T>
    struct AnimalModel : AnimalConcept {
        T animal_;
        
        explicit AnimalModel(const T& animal) : animal_(animal) {}
        explicit AnimalModel(T&& animal) : animal_(std::move(animal)) {}
        
        void makeSound() const override {
            animal_.makeSound();  // Делегируем конкретному типу
        }
        
        std::unique_ptr<AnimalConcept> clone() const override {
            return std::make_unique<AnimalModel>(animal_);
        }
    };
    
    // Указатель на полиморфный концепт
    std::unique_ptr<AnimalConcept> pimpl_;
    
public:
    // Шаблонный конструктор - принимает любой тип
    template<typename T>
    Animal(T animal) : pimpl_(std::make_unique<AnimalModel<T>>(std::move(animal))) {}
    
    // Copy constructor
    Animal(const Animal& other) : pimpl_(other.pimpl_->clone()) {}
    
    // Copy assignment
    Animal& operator=(const Animal& other) {
        if (this != &other) {
            pimpl_ = other.pimpl_->clone();
        }
        return *this;
    }
    
    // Move конструктор и присваивание по умолчанию
    Animal(Animal&&) noexcept = default;
    Animal& operator=(Animal&&) noexcept = default;
    
    // Единый интерфейс для всех типов
    void makeSound() const {
        pimpl_->makeSound();
    }
};

// ============================================================================
// ИСПОЛЬЗОВАНИЕ
// ============================================================================

void demonstrateTypeErasure() {
    std::cout << "=== Type Erasure ===" << std::endl;
    
    // Теперь можем хранить разные типы в одной коллекции!
    std::vector<Animal> animals;
    
    animals.push_back(Dog{});
    animals.push_back(Cat{});
    animals.push_back(Bird{});
    
    // Единый интерфейс для всех
    std::cout << "Все животные:" << std::endl;
    for (const auto& animal : animals) {
        animal.makeSound();
    }
    
    // Value semantics - можем копировать
    Animal dog = Dog{};
    Animal dogCopy = dog;  // Копирование работает!
    
    dogCopy.makeSound();
}
```

**Теоретическая база**: Type Erasure реализует паттерн Bridge + Template Method. Внешний класс (Animal) использует pimpl idiom для делегирования вызовов к полиморфному концепту. Шаблонная модель (AnimalModel<T>) создается для каждого типа, обеспечивая compile-time type safety с runtime flexibility.

### Сравнение подходов
```cpp
// ============================================================================
// СРАВНЕНИЕ: НАСЛЕДОВАНИЕ VS TYPE ERASURE
// ============================================================================

// Подход 1: Традиционное наследование
class AnimalBase {
public:
    virtual ~AnimalBase() = default;
    virtual void makeSound() const = 0;
};

class DogInherited : public AnimalBase {
public:
    void makeSound() const override {
        std::cout << "Woof!" << std::endl;
    }
};

// Подход 2: Type Erasure
// Dog остается независимым, не требует наследования
class DogErased {
public:
    void makeSound() const {
        std::cout << "Woof!" << std::endl;
    }
};

// Сравнение
void compareApproaches() {
    // Наследование:
    // ✅ Простое и понятное
    // ❌ Требует изменения исходных классов
    // ❌ Невозможно для типов из библиотек
    std::vector<std::unique_ptr<AnimalBase>> animals1;
    animals1.push_back(std::make_unique<DogInherited>());
    
    // Type Erasure:
    // ✅ Не требует изменения исходных классов
    // ✅ Работает с любыми типами (даже из библиотек)
    // ✅ Value semantics
    // ❌ Более сложная реализация
    // ❌ Некоторый runtime overhead
    std::vector<Animal> animals2;
    animals2.push_back(DogErased{});
}
```

## 🤔 Вопросы для размышления

### 1. Когда использовать Type Erasure?
**✅ Хорошие случаи:**
- Нужна гетерогенная коллекция без общего базового класса
- Работа с типами из сторонних библиотек
- Callback системы
- Plugin архитектуры
- Нужна value semantics вместо reference semantics

**❌ Плохие случаи:**
- Можно использовать обычное наследование
- Нужна максимальная производительность
- Интерфейс очень большой
- Типы известны на этапе компиляции

### 2. Преимущества Type Erasure
```cpp
// ✅ Non-intrusive
// Не требует изменения исходных типов
// Можно использовать типы из библиотек

// ✅ Value semantics
Animal a = Dog{};
Animal b = a;  // Копирование работает естественно

// ✅ Type safety
// Ошибки обнаруживаются на этапе компиляции
Animal a = Dog{};  // OK
// Animal b = 42;  // Ошибка компиляции если нет makeSound()

// ✅ Гибкость
// Можем добавлять новые типы без изменения кода
struct Robot {
    void makeSound() const { std::cout << "Beep!" << std::endl; }
};
animals.push_back(Robot{});  // Работает!
```

### 3. Недостатки Type Erasure
```cpp
// ❌ Сложность реализации
// Требует понимания advanced C++ техник

// ❌ Runtime overhead
// Виртуальные вызовы + динамическая память

// ❌ Потеря статической информации о типе
Animal a = Dog{};
// Нельзя вызвать методы специфичные для Dog

// ❌ Размер
// Каждый объект хранит указатель + vtable
```

## 🛠️ Практические примеры

### Пример 1: std::function - Type Erasure для Callables
```cpp
// ============================================================================
// STD::FUNCTION - ГОТОВЫЙ TYPE ERASURE
// ============================================================================

void demonstrateStdFunction() {
    std::cout << "\n=== std::function ===" << std::endl;
    
    // std::function может хранить любой callable
    std::vector<std::function<void()>> callbacks;
    
    // Lambda
    callbacks.push_back([]() {
        std::cout << "Lambda вызвана" << std::endl;
    });
    
    // Function pointer
    callbacks.push_back(&someFunction);
    
    // Functor
    struct Functor {
        void operator()() const {
            std::cout << "Functor вызван" << std::endl;
        }
    };
    callbacks.push_back(Functor{});
    
    // Member function (через bind)
    class MyClass {
    public:
        void method() {
            std::cout << "Member function вызвана" << std::endl;
        }
    };
    MyClass obj;
    callbacks.push_back(std::bind(&MyClass::method, &obj));
    
    // Вызываем все callbacks
    for (const auto& callback : callbacks) {
        callback();
    }
}

static void someFunction() {
    std::cout << "Function pointer вызвана" << std::endl;
}
```

### Пример 2: Custom Type Erasure с несколькими методами
```cpp
// ============================================================================
// TYPE ERASURE ДЛЯ DRAWABLE ОБЪЕКТОВ
// ============================================================================

class Drawable {
private:
    struct DrawableConcept {
        virtual ~DrawableConcept() = default;
        virtual void draw() const = 0;
        virtual void move(int dx, int dy) = 0;
        virtual std::pair<int, int> getPosition() const = 0;
        virtual std::unique_ptr<DrawableConcept> clone() const = 0;
    };
    
    template<typename T>
    struct DrawableModel : DrawableConcept {
        T object_;
        
        explicit DrawableModel(T object) : object_(std::move(object)) {}
        
        void draw() const override {
            object_.draw();
        }
        
        void move(int dx, int dy) override {
            object_.move(dx, dy);
        }
        
        std::pair<int, int> getPosition() const override {
            return object_.getPosition();
        }
        
        std::unique_ptr<DrawableConcept> clone() const override {
            return std::make_unique<DrawableModel>(object_);
        }
    };
    
    std::unique_ptr<DrawableConcept> pimpl_;
    
public:
    template<typename T>
    Drawable(T object) : pimpl_(std::make_unique<DrawableModel<T>>(std::move(object))) {}
    
    Drawable(const Drawable& other) : pimpl_(other.pimpl_->clone()) {}
    Drawable& operator=(const Drawable& other) {
        if (this != &other) {
            pimpl_ = other.pimpl_->clone();
        }
        return *this;
    }
    
    Drawable(Drawable&&) noexcept = default;
    Drawable& operator=(Drawable&&) noexcept = default;
    
    // Публичный интерфейс
    void draw() const {
        pimpl_->draw();
    }
    
    void move(int dx, int dy) {
        pimpl_->move(dx, dy);
    }
    
    std::pair<int, int> getPosition() const {
        return pimpl_->getPosition();
    }
};

// Конкретные типы (не связаны наследованием!)
class Circle {
private:
    int x_, y_;
    int radius_;
    
public:
    Circle(int x, int y, int r) : x_(x), y_(y), radius_(r) {}
    
    void draw() const {
        std::cout << "Рисуем круг в (" << x_ << ", " << y_ << ") радиус " << radius_ << std::endl;
    }
    
    void move(int dx, int dy) {
        x_ += dx;
        y_ += dy;
    }
    
    std::pair<int, int> getPosition() const {
        return {x_, y_};
    }
};

class Rectangle {
private:
    int x_, y_;
    int width_, height_;
    
public:
    Rectangle(int x, int y, int w, int h) : x_(x), y_(y), width_(w), height_(h) {}
    
    void draw() const {
        std::cout << "Рисуем прямоугольник в (" << x_ << ", " << y_ 
                  << ") размер " << width_ << "x" << height_ << std::endl;
    }
    
    void move(int dx, int dy) {
        x_ += dx;
        y_ += dy;
    }
    
    std::pair<int, int> getPosition() const {
        return {x_, y_};
    }
};

// Использование
void demonstrateDrawable() {
    std::cout << "\n=== Drawable Type Erasure ===" << std::endl;
    
    std::vector<Drawable> shapes;
    
    shapes.push_back(Circle{100, 100, 50});
    shapes.push_back(Rectangle{200, 200, 80, 60});
    shapes.push_back(Circle{300, 300, 30});
    
    // Единый интерфейс для всех фигур
    std::cout << "Рисуем все фигуры:" << std::endl;
    for (const auto& shape : shapes) {
        shape.draw();
    }
    
    std::cout << "\nПеремещаем все фигуры на (10, 10):" << std::endl;
    for (auto& shape : shapes) {
        shape.move(10, 10);
        shape.draw();
    }
}
```

### Пример 3: Small Buffer Optimization (SBO)
```cpp
// ============================================================================
// TYPE ERASURE С SBO
// ============================================================================

// Оптимизация для малых объектов - избегаем аллокации
class FastAnimal {
private:
    static constexpr size_t BUFFER_SIZE = 32;  // Размер буфера
    
    struct AnimalConcept {
        virtual ~AnimalConcept() = default;
        virtual void makeSound() const = 0;
        virtual void clone(void* buffer) const = 0;
        virtual void move(void* buffer) = 0;
    };
    
    template<typename T>
    struct AnimalModel : AnimalConcept {
        T animal_;
        
        explicit AnimalModel(T animal) : animal_(std::move(animal)) {}
        
        void makeSound() const override {
            animal_.makeSound();
        }
        
        void clone(void* buffer) const override {
            new (buffer) AnimalModel(animal_);
        }
        
        void move(void* buffer) override {
            new (buffer) AnimalModel(std::move(animal_));
        }
    };
    
    // Буфер для малых объектов или указатель для больших
    alignas(std::max_align_t) char buffer_[BUFFER_SIZE];
    AnimalConcept* ptr_;
    bool usesHeap_;
    
public:
    template<typename T>
    FastAnimal(T animal) {
        // Если объект помещается в буфер - используем его
        if (sizeof(AnimalModel<T>) <= BUFFER_SIZE) {
            usesHeap_ = false;
            ptr_ = new (buffer_) AnimalModel<T>(std::move(animal));
            std::cout << "SBO: используем stack буфер" << std::endl;
        } else {
            usesHeap_ = true;
            ptr_ = new AnimalModel<T>(std::move(animal));
            std::cout << "SBO: используем heap" << std::endl;
        }
    }
    
    ~FastAnimal() {
        if (usesHeap_) {
            delete ptr_;
        } else {
            ptr_->~AnimalConcept();
        }
    }
    
    // Copy constructor с SBO
    FastAnimal(const FastAnimal& other) : usesHeap_(other.usesHeap_) {
        if (usesHeap_) {
            // Создаем копию в heap
            // (упрощенная версия)
        } else {
            other.ptr_->clone(buffer_);
            ptr_ = reinterpret_cast<AnimalConcept*>(buffer_);
        }
    }
    
    void makeSound() const {
        ptr_->makeSound();
    }
};

// Использование
void demonstrateSBO() {
    std::cout << "\n=== Small Buffer Optimization ===" << std::endl;
    
    // Маленький объект - использует stack
    FastAnimal smallDog = Dog{};
    
    // Большой объект - использует heap
    struct BigBird {
        char data[100];  // Большой объект
        void makeSound() const {
            std::cout << "Big tweet!" << std::endl;
        }
    };
    FastAnimal bigBird = BigBird{};
}
```

### Пример 4: Type Erasure для Iterators
```cpp
// ============================================================================
// TYPE ERASURE ДЛЯ ИТЕРАТОРОВ
// ============================================================================

// Универсальный итератор для любого контейнера
template<typename T>
class AnyIterator {
private:
    struct IteratorConcept {
        virtual ~IteratorConcept() = default;
        virtual const T& deref() const = 0;
        virtual void increment() = 0;
        virtual bool equals(const IteratorConcept* other) const = 0;
        virtual std::unique_ptr<IteratorConcept> clone() const = 0;
    };
    
    template<typename Iterator>
    struct IteratorModel : IteratorConcept {
        Iterator it_;
        
        explicit IteratorModel(Iterator it) : it_(it) {}
        
        const T& deref() const override {
            return *it_;
        }
        
        void increment() override {
            ++it_;
        }
        
        bool equals(const IteratorConcept* other) const override {
            auto otherModel = dynamic_cast<const IteratorModel*>(other);
            return otherModel && it_ == otherModel->it_;
        }
        
        std::unique_ptr<IteratorConcept> clone() const override {
            return std::make_unique<IteratorModel>(it_);
        }
    };
    
    std::unique_ptr<IteratorConcept> pimpl_;
    
public:
    template<typename Iterator>
    AnyIterator(Iterator it) : pimpl_(std::make_unique<IteratorModel<Iterator>>(it)) {}
    
    AnyIterator(const AnyIterator& other) : pimpl_(other.pimpl_->clone()) {}
    
    const T& operator*() const {
        return pimpl_->deref();
    }
    
    AnyIterator& operator++() {
        pimpl_->increment();
        return *this;
    }
    
    bool operator==(const AnyIterator& other) const {
        return pimpl_->equals(other.pimpl_.get());
    }
    
    bool operator!=(const AnyIterator& other) const {
        return !(*this == other);
    }
};

// Использование
void demonstrateAnyIterator() {
    std::cout << "\n=== Any Iterator ===" << std::endl;
    
    std::vector<int> vec = {1, 2, 3, 4, 5};
    std::list<int> lst = {10, 20, 30};
    
    // Единый тип для итераторов разных контейнеров
    AnyIterator<int> vecIt = vec.begin();
    AnyIterator<int> lstIt = lst.begin();
    
    std::cout << "Vector элемент: " << *vecIt << std::endl;
    std::cout << "List элемент: " << *lstIt << std::endl;
}
```

## 🎨 Современные подходы

### std::any - Type Erasure в STL
```cpp
#include <any>

void demonstrateStdAny() {
    std::cout << "\n=== std::any ===" << std::endl;
    
    // std::any может хранить любой тип
    std::vector<std::any> data;
    
    data.push_back(42);
    data.push_back(std::string("Hello"));
    data.push_back(3.14);
    data.push_back(Dog{});
    
    // Извлечение значений с проверкой типа
    for (const auto& item : data) {
        if (item.type() == typeid(int)) {
            std::cout << "int: " << std::any_cast<int>(item) << std::endl;
        } else if (item.type() == typeid(std::string)) {
            std::cout << "string: " << std::any_cast<std::string>(item) << std::endl;
        } else if (item.type() == typeid(double)) {
            std::cout << "double: " << std::any_cast<double>(item) << std::endl;
        }
    }
}
```

### C++20 Concepts для Type Erasure
```cpp
#include <concepts>

// Concept для типов с методом draw()
template<typename T>
concept Drawable = requires(T t) {
    { t.draw() } -> std::same_as<void>;
    { t.move(0, 0) } -> std::same_as<void>;
};

// Type erasure с проверкой через concept
template<Drawable T>
class TypeErasedDrawable {
    // Гарантируем что T удовлетворяет требованиям
};
```

## 🚀 Применение в реальных проектах

### Примеры из индустрии
```cpp
// Boost.TypeErasure
// Adobe Poly
// std::function, std::any
// Qt QVariant
// Folly Function
```

## 🎯 Практические упражнения

### Упражнение 1: Custom std::function
Реализуйте упрощенную версию std::function.

### Упражнение 2: Serialization
Создайте type erasure для сериализуемых объектов.

### Упражнение 3: Command Pattern
Реализуйте Command паттерн через type erasure.

### Упражнение 4: Undo/Redo
Создайте систему undo/redo используя type erasure.

## 📈 Связь с другими паттернами

### Type Erasure + Strategy
```cpp
// Стратегии без общего базового класса
class Context {
    Any strategy_;  // Type-erased strategy
};
```

### Type Erasure + Command
```cpp
// Команды разных типов в одной очереди
std::queue<Command> commands;  // Type-erased commands
```

## 📈 Следующие шаги
После изучения Type Erasure вы будете готовы к:
- Модулю 7: Паттерны многопоточности
- Продвинутому метапрограммированию
- Проектированию plugin архитектур
- Оптимизации производительности

## ⚠️ Важные предупреждения

1. **Performance**: Type erasure добавляет overhead (виртуальные вызовы + heap)
2. **Сложность**: Требует глубокого понимания C++
3. **Debugging**: Может быть сложно отлаживать
4. **SBO**: Используйте Small Buffer Optimization для малых объектов

---

*"Type erasure is a technique to hide the concrete type of an object behind a generic interface."* - Sean Parent

