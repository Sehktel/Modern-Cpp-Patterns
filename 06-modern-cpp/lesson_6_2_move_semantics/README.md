# Урок 6.2: Move Semantics (Семантика перемещения)

## 🎯 Цель урока
Изучить семантику перемещения - одну из самых важных особенностей современного C++11/14/17. Понять, как избежать ненужного копирования объектов и как писать эффективный код с минимальными накладными расходами.

## 📚 Что изучаем

### 1. Семантика перемещения
- **Lvalue vs Rvalue**: Понимание категорий значений
- **Move constructor**: Конструктор перемещения
- **Move assignment**: Оператор присваивания перемещения
- **std::move**: Приведение к rvalue

### 2. Rvalue References
- **&&**: Синтаксис rvalue ссылок
- **Perfect Forwarding**: Идеальная пересылка
- **Universal References**: Универсальные ссылки
- **Reference Collapsing**: Схлопывание ссылок

### 3. Практические применения
- **Containers**: Эффективные контейнеры
- **Smart Pointers**: Перемещение умных указателей
- **Functions**: Возврат объектов из функций
- **Algorithms**: Оптимизация алгоритмов

## 🔍 Ключевые концепции

### Lvalue vs Rvalue
```cpp
// Lvalue - имеет имя и адрес
int x = 42;        // x - lvalue
int& ref = x;      // ref - lvalue reference

// Rvalue - временные объекты
int y = x + 1;     // x + 1 - rvalue
int&& rref = 42;   // rref - rvalue reference

// std::move превращает lvalue в rvalue
int z = std::move(x); // x теперь rvalue
```

### Move Constructor
```cpp
class MyString {
private:
    char* data_;
    size_t size_;
    
public:
    // Конструктор перемещения
    MyString(MyString&& other) noexcept 
        : data_(other.data_), size_(other.size_) {
        // "Крадем" ресурсы у other
        other.data_ = nullptr;
        other.size_ = 0;
        std::cout << "Move constructor called" << std::endl;
    }
    
    // Оператор присваивания перемещения
    MyString& operator=(MyString&& other) noexcept {
        if (this != &other) {
            delete[] data_;  // Освобождаем старые ресурсы
            
            data_ = other.data_;
            size_ = other.size_;
            
            other.data_ = nullptr;
            other.size_ = 0;
        }
        std::cout << "Move assignment called" << std::endl;
        return *this;
    }
    
    ~MyString() {
        delete[] data_;
    }
};
```

### Perfect Forwarding
```cpp
template<typename T>
void forwardFunction(T&& arg) {
    // T&& - универсальная ссылка
    // Сохраняет категорию значения (lvalue/rvalue)
    processFunction(std::forward<T>(arg));
}

template<typename T>
void processFunction(T&& arg) {
    if (std::is_lvalue_reference<T>::value) {
        std::cout << "Processing lvalue" << std::endl;
    } else {
        std::cout << "Processing rvalue" << std::endl;
    }
}

// Использование
int x = 42;
forwardFunction(x);        // Передается как lvalue
forwardFunction(42);       // Передается как rvalue
forwardFunction(std::move(x)); // Передается как rvalue
```

## 🤔 Вопросы для размышления

### 1. Когда использовать move semantics?
**✅ Хорошие случаи:**
- Большие объекты (строки, контейнеры)
- Умные указатели
- Возврат объектов из функций
- Временные объекты
- Операции с контейнерами

**❌ Плохие случаи:**
- Простые типы (int, double)
- Маленькие объекты
- Когда копирование дешевле перемещения
- Когда объект нужен после перемещения

### 2. Преимущества move semantics
- **Производительность**: Избегание ненужного копирования
- **Эффективность**: Перемещение ресурсов вместо копирования
- **Безопасность**: Гарантия, что объект не будет использован после перемещения
- **Современность**: Соответствие стандартам C++11+

### 3. Недостатки move semantics
- **Сложность**: Усложняет код и понимание
- **Ошибки**: Легко забыть про std::move
- **Совместимость**: Проблемы с legacy кодом
- **Отладка**: Сложнее отлаживать перемещения

## 🛠️ Практические примеры

### Эффективные контейнеры
```cpp
class EfficientContainer {
private:
    std::vector<std::string> data_;
    
public:
    // Добавление с перемещением
    void addString(std::string&& str) {
        data_.push_back(std::move(str));
    }
    
    // Добавление с perfect forwarding
    template<typename T>
    void emplaceString(T&& str) {
        data_.emplace_back(std::forward<T>(str));
    }
    
    // Возврат с перемещением
    std::vector<std::string> getData() && {
        return std::move(data_);
    }
    
    // Возврат с копированием (для lvalue)
    std::vector<std::string> getData() const & {
        return data_;
    }
};
```

### Factory с перемещением
```cpp
class ObjectFactory {
public:
    static std::unique_ptr<MyClass> createObject() {
        auto obj = std::make_unique<MyClass>("Factory Object");
        // ... инициализация
        return obj; // Автоматическое перемещение
    }
    
    static std::vector<MyClass> createObjects(size_t count) {
        std::vector<MyClass> objects;
        objects.reserve(count);
        
        for (size_t i = 0; i < count; ++i) {
            objects.emplace_back("Object " + std::to_string(i));
        }
        
        return objects; // Автоматическое перемещение
    }
};
```

### Move-only типы
```cpp
class MoveOnlyClass {
private:
    std::unique_ptr<int> data_;
    
public:
    explicit MoveOnlyClass(int value) 
        : data_(std::make_unique<int>(value)) {}
    
    // Запрещаем копирование
    MoveOnlyClass(const MoveOnlyClass&) = delete;
    MoveOnlyClass& operator=(const MoveOnlyClass&) = delete;
    
    // Разрешаем перемещение
    MoveOnlyClass(MoveOnlyClass&&) = default;
    MoveOnlyClass& operator=(MoveOnlyClass&&) = default;
    
    int getValue() const {
        return data_ ? *data_ : 0;
    }
};
```

## 🎨 Современные подходы в C++

### SFINAE и move semantics
```cpp
template<typename T>
typename std::enable_if<std::is_move_constructible<T>::value, void>::type
processMovable(T&& obj) {
    std::cout << "Processing movable object" << std::endl;
    // Используем std::move
    auto moved = std::move(obj);
}

template<typename T>
typename std::enable_if<!std::is_move_constructible<T>::value, void>::type
processMovable(T&& obj) {
    std::cout << "Processing non-movable object" << std::endl;
    // Используем копирование
    auto copied = obj;
}
```

### Variadic templates с perfect forwarding
```cpp
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

// Использование
auto obj = make_unique<MyClass>("name", 42, true);
```

### Move semantics в алгоритмах
```cpp
template<typename Container>
void sortAndMove(Container&& container) {
    std::sort(container.begin(), container.end());
    // container может быть перемещен дальше
}

// Использование
std::vector<int> vec = {3, 1, 4, 1, 5};
sortAndMove(std::move(vec)); // Перемещение
sortAndMove(vec);            // Копирование
```

## 🧪 Тестирование Move Semantics

### Проверка перемещения
```cpp
TEST(MoveSemanticsTest, MoveConstructor) {
    MyString str1("Hello");
    MyString str2 = std::move(str1);
    
    EXPECT_EQ(str1.getData(), nullptr);
    EXPECT_NE(str2.getData(), nullptr);
    EXPECT_STREQ(str2.getData(), "Hello");
}

TEST(MoveSemanticsTest, MoveAssignment) {
    MyString str1("Hello");
    MyString str2("World");
    
    str2 = std::move(str1);
    
    EXPECT_EQ(str1.getData(), nullptr);
    EXPECT_STREQ(str2.getData(), "Hello");
}
```

## 🎯 Практические упражнения

### Упражнение 1: Move-only класс
Создайте класс, который можно только перемещать, но не копировать.

### Упражнение 2: Эффективный контейнер
Реализуйте контейнер с эффективными операциями перемещения.

### Упражнение 3: Perfect forwarding
Создайте функцию с perfect forwarding для различных типов аргументов.

### Упражнение 4: Move semantics в алгоритмах
Оптимизируйте алгоритм сортировки с использованием move semantics.

## 📈 Связь с другими паттернами

### Move Semantics vs Copy Semantics
- **Move**: Перемещение ресурсов
- **Copy**: Копирование ресурсов

### Move Semantics vs RAII
- **Move**: Эффективное управление ресурсами
- **RAII**: Автоматическое управление ресурсами

### Move Semantics vs Smart Pointers
- **Move**: Перемещение владения
- **Smart Pointers**: Автоматическое управление памятью

## 📈 Следующие шаги
После изучения Move Semantics вы будете готовы к:
- Уроку 6.3: CRTP (Curiously Recurring Template Pattern)
- Пониманию современного C++
- Написанию эффективного кода
- Оптимизации производительности

## 💡 Важные принципы

1. **Предпочитайте перемещение копированию**: Для больших объектов
2. **Используйте std::move**: Когда объект больше не нужен
3. **Пишите noexcept move**: Для оптимизации
4. **Используйте perfect forwarding**: В шаблонах
5. **Проверяйте после перемещения**: Объект должен быть в валидном состоянии
