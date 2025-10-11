# Урок 1.4: Move Semantics (Семантика перемещения)

## 🎯 Цель урока
Изучить семантику перемещения (move semantics) - революционную возможность C++11, которая позволяет эффективно передавать владение ресурсами без копирования. Понять разницу между lvalue и rvalue, реализовать move-конструкторы и move-присваивание.

## 📚 Что изучаем

### 1. Value categories
- **lvalue**: "left value" - имеет адрес, персистентный объект
- **rvalue**: "right value" - временный объект, может быть перемещен
- **xvalue**: "expiring value" - объект, который будет уничтожен
- **glvalue**: "generalized lvalue" - lvalue или xvalue
- **prvalue**: "pure rvalue" - временное значение

### 2. Move операции
- **Move constructor**: Конструктор перемещения `T(T&& other)`
- **Move assignment**: Оператор присваивания перемещением `T& operator=(T&& other)`
- **std::move**: Приведение к rvalue reference
- **Perfect forwarding**: std::forward для сохранения категории значения

### 3. Оптимизация производительности
- **Copy elision**: Оптимизация копирования компилятором
- **RVO (Return Value Optimization)**: Оптимизация возврата значений
- **NRVO (Named RVO)**: Оптимизация именованных возвращаемых значений
- **Zero-copy semantics**: Перемещение вместо копирования

## 🔍 Ключевые концепции

### lvalue vs rvalue
```cpp
// lvalue - имеет имя, можно взять адрес
int x = 10;           // x - lvalue
int* ptr = &x;        // OK - lvalue имеет адрес
int& ref = x;         // OK - lvalue reference

// rvalue - временное значение, нет адреса
int y = 20;           // 20 - rvalue (литерал)
int z = x + y;        // (x + y) - rvalue (временный результат)
// int* ptr2 = &(x + y);  // ОШИБКА - нельзя взять адрес rvalue

// rvalue reference - позволяет "поймать" временный объект
int&& rref = 42;      // OK - rvalue reference к временному значению
int&& rref2 = x + y;  // OK - привязывает временный результат
```

**Теоретическая база**: Концепция value categories в C++ формализует различие между объектами с identity (lvalue) и объектами без identity (rvalue). Это позволяет компилятору оптимизировать операции с временными объектами, избегая ненужного копирования. Формально: lvalue ⊆ glvalue, rvalue ⊆ glvalue, xvalue = glvalue ∩ rvalue.

### Move Constructor
```cpp
class MyVector {
private:
    int* data_;
    size_t size_;
    
public:
    // Обычный конструктор
    MyVector(size_t size) : data_(new int[size]), size_(size) {
        std::cout << "Конструктор: аллокация " << size << " элементов" << std::endl;
    }
    
    // Copy constructor - дорогое копирование
    MyVector(const MyVector& other) : data_(new int[other.size_]), size_(other.size_) {
        std::copy(other.data_, other.data_ + size_, data_);
        std::cout << "Copy constructor: копирование " << size_ << " элементов" << std::endl;
    }
    
    // Move constructor - дешевое перемещение
    MyVector(MyVector&& other) noexcept 
        : data_(other.data_), size_(other.size_) {
        // "Крадем" ресурсы у other
        other.data_ = nullptr;
        other.size_ = 0;
        std::cout << "Move constructor: перемещение (без копирования)" << std::endl;
    }
    
    ~MyVector() {
        delete[] data_;
        std::cout << "Деструктор: освобождение памяти" << std::endl;
    }
};

// Использование
MyVector createVector() {
    MyVector v(1000);  // Создаем большой вектор
    return v;          // Move (не copy!) благодаря move semantics
}

void useVector() {
    MyVector v1(500);
    MyVector v2 = std::move(v1);  // Явное перемещение через std::move
    // v1 теперь в "moved-from" состоянии (данные украдены)
    // v2 владеет данными
}
```

**Теоретическая база**: Move constructor обеспечивает константную сложность O(1) вместо линейной O(n) для копирования. Это достигается через "кражу" указателя на данные вместо дублирования. Важно: moved-from объект должен остаться в валидном состоянии (valid but unspecified).

### Move Assignment
```cpp
class String {
private:
    char* data_;
    size_t length_;
    
public:
    // Move assignment operator
    String& operator=(String&& other) noexcept {
        std::cout << "Move assignment" << std::endl;
        
        // 1. Проверка на self-assignment
        if (this == &other) {
            return *this;
        }
        
        // 2. Освобождаем наши текущие ресурсы
        delete[] data_;
        
        // 3. "Крадем" ресурсы у other
        data_ = other.data_;
        length_ = other.length_;
        
        // 4. Обнуляем other (важно!)
        other.data_ = nullptr;
        other.length_ = 0;
        
        return *this;
    }
    
    // noexcept важен для оптимизаций STL!
    String(String&& other) noexcept 
        : data_(other.data_), length_(other.length_) {
        other.data_ = nullptr;
        other.length_ = 0;
    }
};
```

**Важно**: Move operations должны быть помечены как `noexcept`! STL контейнеры используют move только если операция noexcept, иначе fallback на копирование для exception safety.

### std::move - приведение к rvalue
```cpp
// std::move не перемещает! Он только приводит к rvalue reference
template<typename T>
constexpr std::remove_reference_t<T>&& move(T&& t) noexcept {
    return static_cast<std::remove_reference_t<T>&&>(t);
}

// Использование
std::vector<int> v1 = {1, 2, 3, 4, 5};
std::vector<int> v2 = std::move(v1);  // v1 приводится к rvalue
// Теперь move constructor вектора будет вызван
// v1 в состоянии moved-from (обычно пустой)
// v2 владеет данными

// Можно использовать v1 снова, но только присвоить ему новое значение
v1 = {10, 20, 30};  // OK - v1 снова валиден

// НО НЕЛЬЗЯ использовать v1 до повторной инициализации
// std::cout << v1.size();  // Технически OK, но v1 может быть пустой
```

### Perfect Forwarding с std::forward
```cpp
// Универсальная ссылка (forwarding reference)
template<typename T>
void wrapper(T&& arg) {
    // forward сохраняет категорию значения (lvalue или rvalue)
    process(std::forward<T>(arg));
}

// Пример использования
void process(const std::string& s) {
    std::cout << "lvalue: " << s << std::endl;
}

void process(std::string&& s) {
    std::cout << "rvalue: " << s << std::endl;
}

// Тест
std::string str = "hello";
wrapper(str);              // Вызовет process(const std::string&)
wrapper(std::string("hi")); // Вызовет process(std::string&&)
```

**Теоретическая база**: Perfect forwarding сохраняет value category через reference collapsing rules:
- `T& &&` → `T&`
- `T&& &` → `T&`
- `T&& &&` → `T&&`

## 🤔 Вопросы для размышления

### 1. Когда происходит move автоматически?
```cpp
// 1. Return by value (RVO/NRVO может полностью убрать move!)
std::vector<int> createVector() {
    std::vector<int> v = {1, 2, 3};
    return v;  // Move (или RVO - совсем без перемещения)
}

// 2. Передача временного объекта
void takeVector(std::vector<int> v);
takeVector(std::vector<int>{1, 2, 3});  // Move constructor

// 3. std::move явно
std::vector<int> v1 = {1, 2, 3};
std::vector<int> v2 = std::move(v1);  // Move constructor

// 4. Return temporary
std::string getString() {
    return std::string("hello");  // Создается как rvalue
}
```

### 2. Moved-from состояние
```cpp
// После std::move объект в "valid but unspecified" состоянии
std::vector<int> v1 = {1, 2, 3};
std::vector<int> v2 = std::move(v1);

// ЧТО можно делать с v1:
// ✅ Присваивать новое значение
v1 = {10, 20};

// ✅ Уничтожать (вызов деструктора)
// Деструктор автоматически вызовется

// ✅ Проверять на пустоту (если метод есть)
if (v1.empty()) { /* ... */ }

// ⚠️ НЕ РЕКОМЕНДУЕТСЯ использовать значение
// std::cout << v1[0];  // Может быть UB или неожиданное поведение

// ❌ НЕЛЬЗЯ предполагать конкретное состояние
// if (v1.size() == 0) { }  // Может быть false!
```

### 3. Performance: Copy vs Move
```cpp
// Замеры производительности
class BigData {
    std::vector<int> data_;
public:
    BigData() : data_(1'000'000) {}  // 1 миллион элементов
    
    // Copy: O(n) - дорого
    BigData(const BigData& other) : data_(other.data_) {
        // Копирование 1 миллиона элементов
    }
    
    // Move: O(1) - дешево
    BigData(BigData&& other) noexcept : data_(std::move(other.data_)) {
        // Перемещение только указателей/размеров
    }
};

// Тест производительности
auto start = std::chrono::high_resolution_clock::now();

// Copy: ~10ms на 1 млн элементов
BigData d1;
BigData d2 = d1;  // Copy

// Move: ~0.001ms (в тысячи раз быстрее!)
BigData d3;
BigData d4 = std::move(d3);  // Move

auto end = std::chrono::high_resolution_clock::now();
```

## 🛠️ Практические примеры

### Пример 1: Ресурс-менеджер с move semantics
```cpp
class ResourceManager {
private:
    int* resource_;
    size_t size_;
    
public:
    // Конструктор
    explicit ResourceManager(size_t size) 
        : resource_(new int[size]), size_(size) {
        std::cout << "Аллокация ресурса: " << size << " элементов" << std::endl;
    }
    
    // Copy constructor
    ResourceManager(const ResourceManager& other) 
        : resource_(new int[other.size_]), size_(other.size_) {
        std::copy(other.resource_, other.resource_ + size_, resource_);
        std::cout << "Copy constructor: дублирование ресурса" << std::endl;
    }
    
    // Copy assignment
    ResourceManager& operator=(const ResourceManager& other) {
        if (this != &other) {
            delete[] resource_;
            size_ = other.size_;
            resource_ = new int[size_];
            std::copy(other.resource_, other.resource_ + size_, resource_);
            std::cout << "Copy assignment: дублирование ресурса" << std::endl;
        }
        return *this;
    }
    
    // Move constructor
    ResourceManager(ResourceManager&& other) noexcept
        : resource_(other.resource_), size_(other.size_) {
        other.resource_ = nullptr;
        other.size_ = 0;
        std::cout << "Move constructor: перемещение ресурса (без копирования)" << std::endl;
    }
    
    // Move assignment
    ResourceManager& operator=(ResourceManager&& other) noexcept {
        if (this != &other) {
            delete[] resource_;  // Освобождаем старый ресурс
            resource_ = other.resource_;  // Крадем новый
            size_ = other.size_;
            other.resource_ = nullptr;  // Обнуляем источник
            other.size_ = 0;
            std::cout << "Move assignment: перемещение ресурса" << std::endl;
        }
        return *this;
    }
    
    // Деструктор
    ~ResourceManager() {
        delete[] resource_;
        if (size_ > 0) {
            std::cout << "Деструктор: освобождение ресурса" << std::endl;
        }
    }
};

// Использование
void demonstrateMoveSemantics() {
    ResourceManager rm1(1000);
    
    // Copy - дорого
    ResourceManager rm2 = rm1;
    
    // Move - дешево
    ResourceManager rm3 = std::move(rm1);
    // rm1 теперь пустой, rm3 владеет ресурсом
}
```

### Пример 2: Factory с возвратом по значению
```cpp
// Фабрика возвращает объект по значению - эффективно благодаря move
class Widget {
private:
    std::vector<int> data_;
    std::string name_;
    
public:
    Widget(const std::string& name, size_t size) 
        : data_(size), name_(name) {
        std::cout << "Widget '" << name_ << "' создан" << std::endl;
    }
    
    // Move constructor для эффективного возврата из функций
    Widget(Widget&& other) noexcept
        : data_(std::move(other.data_)), name_(std::move(other.name_)) {
        std::cout << "Widget перемещен" << std::endl;
    }
};

// Фабрика эффективно возвращает большой объект
Widget createWidget(const std::string& name) {
    Widget w(name, 1'000'000);  // Большой объект
    // Тяжелая инициализация...
    return w;  // RVO или move (но не copy!)
}

// Использование
void useFactory() {
    // Объект возвращается без копирования
    Widget widget = createWidget("MyWidget");
    // Только одна аллокация благодаря RVO/move!
}
```

### Пример 3: Move-only типы
```cpp
// Класс, который можно только перемещать (не копировать)
class UniqueResource {
private:
    std::unique_ptr<int[]> data_;
    size_t size_;
    
public:
    explicit UniqueResource(size_t size) 
        : data_(std::make_unique<int[]>(size)), size_(size) {}
    
    // Удаляем copy operations
    UniqueResource(const UniqueResource&) = delete;
    UniqueResource& operator=(const UniqueResource&) = delete;
    
    // Разрешаем move operations
    UniqueResource(UniqueResource&&) noexcept = default;
    UniqueResource& operator=(UniqueResource&&) noexcept = default;
    
    size_t size() const { return size_; }
};

// Функция принимает move-only тип
void processResource(UniqueResource resource) {
    std::cout << "Обрабатываем ресурс размером " << resource.size() << std::endl;
}

// Использование
void useMoveOnlyType() {
    UniqueResource res(1000);
    
    // Copy не скомпилируется
    // processResource(res);  // ОШИБКА!
    
    // Нужно явно переместить
    processResource(std::move(res));  // OK
    // res теперь moved-from
}
```

### Пример 4: Контейнер с move semantics
```cpp
// Простой вектор с поддержкой move semantics
template<typename T>
class SimpleVector {
private:
    T* data_;
    size_t size_;
    size_t capacity_;
    
public:
    SimpleVector() : data_(nullptr), size_(0), capacity_(0) {}
    
    explicit SimpleVector(size_t size) 
        : data_(new T[size]), size_(size), capacity_(size) {
        std::cout << "Создан вектор размером " << size << std::endl;
    }
    
    // Copy constructor
    SimpleVector(const SimpleVector& other) 
        : data_(new T[other.capacity_]), size_(other.size_), capacity_(other.capacity_) {
        std::copy(other.data_, other.data_ + size_, data_);
        std::cout << "Copy: дублирование " << size_ << " элементов" << std::endl;
    }
    
    // Move constructor
    SimpleVector(SimpleVector&& other) noexcept
        : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
        std::cout << "Move: перемещение вектора" << std::endl;
    }
    
    // Copy assignment
    SimpleVector& operator=(const SimpleVector& other) {
        if (this != &other) {
            delete[] data_;
            capacity_ = other.capacity_;
            size_ = other.size_;
            data_ = new T[capacity_];
            std::copy(other.data_, other.data_ + size_, data_);
        }
        return *this;
    }
    
    // Move assignment
    SimpleVector& operator=(SimpleVector&& other) noexcept {
        if (this != &other) {
            delete[] data_;
            data_ = other.data_;
            size_ = other.size_;
            capacity_ = other.capacity_;
            other.data_ = nullptr;
            other.size_ = 0;
            other.capacity_ = 0;
        }
        return *this;
    }
    
    ~SimpleVector() {
        delete[] data_;
    }
    
    void push_back(const T& value) {
        if (size_ == capacity_) {
            reserve(capacity_ == 0 ? 1 : capacity_ * 2);
        }
        data_[size_++] = value;
    }
    
    void push_back(T&& value) {
        if (size_ == capacity_) {
            reserve(capacity_ == 0 ? 1 : capacity_ * 2);
        }
        data_[size_++] = std::move(value);  // Move вместо copy
    }
    
private:
    void reserve(size_t newCapacity) {
        T* newData = new T[newCapacity];
        
        // Перемещаем элементы вместо копирования
        for (size_t i = 0; i < size_; ++i) {
            newData[i] = std::move(data_[i]);
        }
        
        delete[] data_;
        data_ = newData;
        capacity_ = newCapacity;
    }
};
```

## 🎨 Альтернативные подходы и оптимизации

### Copy Elision и RVO
```cpp
// Компилятор может полностью убрать копирование/перемещение
class Heavy {
public:
    Heavy() { std::cout << "Constructor" << std::endl; }
    Heavy(const Heavy&) { std::cout << "Copy" << std::endl; }
    Heavy(Heavy&&) noexcept { std::cout << "Move" << std::endl; }
};

Heavy createHeavy() {
    Heavy h;
    return h;  // RVO: нет ни copy, ни move!
}

void test() {
    Heavy h = createHeavy();  // Только "Constructor" - RVO работает!
    // Объект создается сразу в месте назначения
}

// C++17 гарантирует copy elision в некоторых случаях
Heavy h = Heavy();  // Гарантированно нет copy/move
```

### Conditional noexcept
```cpp
// noexcept зависит от noexcept вложенных операций
template<typename T>
class Container {
    T value_;
    
public:
    // Automatically noexcept если T::T(T&&) is noexcept
    Container(Container&& other) noexcept(noexcept(T(std::move(other.value_))))
        : value_(std::move(other.value_)) {}
};

// Почему это важно: STL использует move только если noexcept
std::vector<Container<NonNoexceptType>> vec;
vec.push_back(...);  // Будет использовать copy, не move!

std::vector<Container<NoexceptType>> vec2;
vec2.push_back(...);  // Будет использовать move - эффективнее!
```

### Rule of Five
```cpp
// Современный C++: если определяете один, определите все
class Resource {
public:
    // 1. Destructor
    ~Resource();
    
    // 2. Copy constructor
    Resource(const Resource&);
    
    // 3. Copy assignment
    Resource& operator=(const Resource&);
    
    // 4. Move constructor
    Resource(Resource&&) noexcept;
    
    // 5. Move assignment
    Resource& operator=(Resource&&) noexcept;
};

// Или используйте Rule of Zero - не определяйте вообще
class SmartResource {
    std::unique_ptr<int[]> data_;  // Управляет ресурсом автоматически
    // Все 5 функций генерируются компилятором правильно!
};
```

## 🚀 Современные C++ возможности

### C++14: auto&& и decltype(auto)
```cpp
// auto&& - универсальная ссылка в лямбдах
auto lambda = [](auto&& arg) {
    process(std::forward<decltype(arg)>(arg));
};

// decltype(auto) сохраняет точный тип возврата
decltype(auto) getValue() {
    int x = 42;
    return x;  // Возвращает int, не int&
}
```

### C++17: Гарантированная copy elision
```cpp
// C++17 гарантирует отсутствие move/copy в prvalue
struct NonMovable {
    NonMovable() = default;
    NonMovable(NonMovable&&) = delete;  // Запрещен move
};

NonMovable create() {
    return NonMovable();  // OK в C++17! Нет move/copy
}

NonMovable nm = create();  // OK - guaranteed copy elision
```

### C++20: Concepts для move semantics
```cpp
#include <concepts>

// Concept для проверки movable типа
template<typename T>
concept Movable = std::movable<T>;

template<Movable T>
class Container {
    T value_;
    
public:
    Container(T&& val) : value_(std::move(val)) {}
};

// Использование
Container<std::string> c1{std::string("hello")};  // OK
// Container<NonMovable> c2{NonMovable()};  // Ошибка компиляции
```

## 🎯 Практические упражнения

### Упражнение 1: Реализация move-aware класса
Создайте класс, управляющий динамическим массивом, с правильными move operations.

### Упражнение 2: Анализ производительности
Сравните производительность copy vs move на большом контейнере.

### Упражнение 3: Perfect forwarding
Реализуйте wrapper функцию с perfect forwarding для сохранения value category.

### Упражнение 4: Move-only тип
Создайте move-only класс и используйте его в STL контейнерах.

## 📈 Следующие шаги
После изучения move semantics вы будете готовы к:
- Глубокому пониманию STL контейнеров и их оптимизаций
- Написанию высокопроизводительного кода
- Изучению advanced C++ техник
- Модулю 2: Принципы проектирования

## ⚠️ Важные предупреждения

### 1. Не используйте moved-from объекты
```cpp
std::vector<int> v1 = {1, 2, 3};
std::vector<int> v2 = std::move(v1);
// ❌ ОПАСНО - v1 в неопределенном состоянии
// std::cout << v1[0];  // Может упасть!
```

### 2. std::move не перемещает
```cpp
// std::move только cast к rvalue reference
// Реальное перемещение делает move constructor
std::string s1 = "hello";
std::string&& ref = std::move(s1);  // s1 ЕЩЕ не перемещен!
std::string s2 = std::move(s1);     // ВОТ ЗДЕСЬ перемещение
```

### 3. Return std::move вредит RVO
```cpp
// ❌ ПЛОХО - блокирует RVO
std::vector<int> createVector() {
    std::vector<int> v = {1, 2, 3};
    return std::move(v);  // НЕ ДЕЛАЙТЕ ТАК!
}

// ✅ ХОРОШО - позволяет RVO
std::vector<int> createVector() {
    std::vector<int> v = {1, 2, 3};
    return v;  // Компилятор сам применит move если нужно
}
```

## 📚 Дополнительные ресурсы

- **Scott Meyers "Effective Modern C++"** - Items 23-30 о move semantics
- **Nicolai Josuttis "C++17 - The Complete Guide"** - Copy elision
- **cppreference.com** - Value categories и move semantics
- **CppCon talks** - "Back to Basics: Move Semantics"

---

*"Move semantics is not about moving things around in memory; it's about transferring ownership of resources."* - Howard Hinnant

