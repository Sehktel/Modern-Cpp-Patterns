# Урок 1.3: Smart Pointers (Умные указатели)

## 🎯 Цель урока
Изучить умные указатели (smart pointers) - фундаментальный механизм современного C++ для автоматического управления памятью. Понять разницу между различными типами умных указателей, их семантику владения и правила использования.

## 📚 Что изучаем

### 1. Типы умных указателей
- **std::unique_ptr**: Единоличное владение объектом
- **std::shared_ptr**: Разделяемое владение с подсчетом ссылок
- **std::weak_ptr**: Слабая ссылка без владения (для решения циклических зависимостей)
- **Устаревший std::auto_ptr**: Исторический контекст и почему он deprecated

### 2. Семантика владения
- **Exclusive ownership**: Только один владелец (unique_ptr)
- **Shared ownership**: Множественные владельцы (shared_ptr)
- **Non-owning reference**: Наблюдатель без владения (weak_ptr)
- **Transfer of ownership**: Передача владения через std::move

### 3. Exception Safety и Memory Safety
- **RAII (Resource Acquisition Is Initialization)**: Автоматическое освобождение при исключениях
- **No manual delete**: Исключение ошибок ручного управления памятью
- **Leak prevention**: Предотвращение утечек памяти
- **Dangling pointer prevention**: Защита от висячих указателей

## 🔍 Ключевые концепции

### std::unique_ptr - Единоличное владение
```cpp
// Создание unique_ptr
std::unique_ptr<int> ptr1 = std::make_unique<int>(42);
std::unique_ptr<int[]> arr = std::make_unique<int[]>(100);

// Доступ к значению
std::cout << *ptr1 << std::endl;

// Передача владения (move semantics)
std::unique_ptr<int> ptr2 = std::move(ptr1);
// ptr1 теперь nullptr, ptr2 владеет объектом

// unique_ptr НЕЛЬЗЯ копировать
// std::unique_ptr<int> ptr3 = ptr2; // ОШИБКА компиляции!

// Освобождение происходит автоматически при выходе из scope
```

**Теоретическая база**: unique_ptr реализует концепцию "уникального владения" из теории типов. Математически это можно выразить как линейный тип (linear type), где значение имеет ровно одного владельца. Это обеспечивает безопасность памяти на уровне системы типов компилятора.

### std::shared_ptr - Разделяемое владение
```cpp
// Создание shared_ptr
std::shared_ptr<int> ptr1 = std::make_shared<int>(42);

// Копирование увеличивает счетчик ссылок
std::shared_ptr<int> ptr2 = ptr1;  // ref_count = 2

// Проверка количества владельцев
std::cout << "Владельцев: " << ptr1.use_count() << std::endl;

// Объект удаляется когда последний shared_ptr уничтожен
{
    std::shared_ptr<int> ptr3 = ptr1;  // ref_count = 3
} // ptr3 уничтожен, ref_count = 2

// ref_count = 0 → объект автоматически удаляется
```

**Теоретическая база**: shared_ptr реализует алгоритм подсчета ссылок (reference counting) из теории сборки мусора. Это детерминированный алгоритм с временной сложностью O(1) для инкремента/декремента счетчика. Overhead: 2 указателя (на объект и на control block) + атомарные операции для thread-safety.

### std::weak_ptr - Слабая ссылка
```cpp
std::shared_ptr<int> shared = std::make_shared<int>(42);

// Создание weak_ptr не увеличивает счетчик ссылок
std::weak_ptr<int> weak = shared;

// Проверка существования объекта
if (auto locked = weak.lock()) {
    // Объект существует, временный shared_ptr создан
    std::cout << *locked << std::endl;
} else {
    // Объект уже удален
    std::cout << "Объект удален" << std::endl;
}

// weak_ptr используется для разрыва циклических зависимостей
```

**Теоретическая база**: weak_ptr решает проблему циклических ссылок, которая является классической проблемой алгоритмов reference counting. Циклические структуры данных (графы, двусвязные списки) требуют разделения strong и weak ссылок для предотвращения утечек памяти.

## 🤔 Вопросы для размышления

### 1. Когда использовать каждый тип?
**unique_ptr** использовать когда:
- ✅ Объект имеет единственного владельца
- ✅ Нужна максимальная производительность (нет overhead на счетчик ссылок)
- ✅ Нужна clear ownership semantics
- ✅ Хотите передавать владение через std::move

**shared_ptr** использовать когда:
- ✅ Множество объектов должны владеть ресурсом
- ✅ Время жизни объекта неопределенно заранее
- ✅ Объект используется в нескольких потоках
- ⚠️ НО учитывайте overhead: atomic операции + дополнительная память

**weak_ptr** использовать когда:
- ✅ Нужна не-владеющая ссылка на shared_ptr
- ✅ Нужно разорвать циклические зависимости
- ✅ Нужен cache/observer паттерн

### 2. Почему НЕ использовать сырые указатели?
**Проблемы сырых указателей:**
```cpp
// ❌ Опасный код с сырыми указателями
int* rawPtr = new int(42);

// Проблема 1: Забыли удалить - утечка памяти
if (someCondition) {
    return;  // Утечка!
}

// Проблема 2: Исключение - утечка памяти
processData();  // Может выбросить исключение → утечка

// Проблема 3: Double delete
delete rawPtr;
delete rawPtr;  // Undefined behavior!

// Проблема 4: Use-after-free
delete rawPtr;
*rawPtr = 10;  // Undefined behavior!
```

**Решение с умными указателями:**
```cpp
// ✅ Безопасный код с умными указателями
std::unique_ptr<int> smartPtr = std::make_unique<int>(42);

// Автоматическое освобождение при:
// - Выходе из scope
// - Исключениях
// - Раннем return
// Нет проблем с:
// - Утечками памяти
// - Double delete
// - Use-after-free
```

### 3. Performance considerations
**unique_ptr**:
- Размер: sizeof(T*) - как сырой указатель
- Overhead: НОЛЬ! Компилятор оптимизирует как сырой указатель
- Move: O(1), тривиальная операция

**shared_ptr**:
- Размер: 2 * sizeof(void*) (указатель + control block)
- Overhead: Атомарные операции для thread-safety
- Copy: O(1), но с атомарным инкрементом
- Thread-safe: Счетчик ссылок атомарный

**weak_ptr**:
- Размер: 2 * sizeof(void*) (как shared_ptr)
- lock(): O(1), но с атомарной проверкой

## 🛠️ Практические примеры

### Пример 1: Управление ресурсами с unique_ptr
```cpp
// Класс для управления файловым дескриптором
class FileHandle {
private:
    int fd_;
    
public:
    explicit FileHandle(const std::string& filename) {
        fd_ = open(filename.c_str(), O_RDONLY);
        if (fd_ == -1) {
            throw std::runtime_error("Не удалось открыть файл");
        }
    }
    
    ~FileHandle() {
        if (fd_ != -1) {
            close(fd_);
        }
    }
    
    // Запрещаем копирование
    FileHandle(const FileHandle&) = delete;
    FileHandle& operator=(const FileHandle&) = delete;
    
    // Разрешаем перемещение
    FileHandle(FileHandle&& other) noexcept : fd_(other.fd_) {
        other.fd_ = -1;
    }
    
    int get() const { return fd_; }
};

// Использование с unique_ptr
std::unique_ptr<FileHandle> openFile(const std::string& filename) {
    return std::make_unique<FileHandle>(filename);
}

void processFile() {
    auto file = openFile("data.txt");
    // Файл автоматически закроется при выходе из функции
    // Даже если произойдет исключение!
}
```

### Пример 2: Граф с циклическими зависимостями
```cpp
// Узел графа с циклическими ссылками
class Node {
public:
    int value;
    std::vector<std::shared_ptr<Node>> children;  // Strong references
    std::weak_ptr<Node> parent;                   // Weak reference
    
    explicit Node(int val) : value(val) {
        std::cout << "Node " << value << " создан" << std::endl;
    }
    
    ~Node() {
        std::cout << "Node " << value << " удален" << std::endl;
    }
    
    void addChild(const std::shared_ptr<Node>& child) {
        children.push_back(child);
        child->parent = shared_from_this();
    }
};

// Создание графа без утечек памяти
void createGraph() {
    auto root = std::make_shared<Node>(1);
    auto child1 = std::make_shared<Node>(2);
    auto child2 = std::make_shared<Node>(3);
    
    root->addChild(child1);
    root->addChild(child2);
    
    // Проверка родителя из дочернего узла
    if (auto parent = child1->parent.lock()) {
        std::cout << "Родитель: " << parent->value << std::endl;
    }
    
    // Все узлы автоматически удалятся при выходе
    // weak_ptr предотвращает циклическую зависимость
}
```

### Пример 3: Factory паттерн с умными указателями
```cpp
// Базовый класс продукта
class Product {
public:
    virtual ~Product() = default;
    virtual void use() = 0;
};

// Конкретные продукты
class ConcreteProductA : public Product {
public:
    void use() override {
        std::cout << "Использую продукт A" << std::endl;
    }
};

class ConcreteProductB : public Product {
public:
    void use() override {
        std::cout << "Использую продукт B" << std::endl;
    }
};

// Фабрика возвращает unique_ptr
class Factory {
public:
    enum class ProductType { A, B };
    
    static std::unique_ptr<Product> createProduct(ProductType type) {
        switch (type) {
            case ProductType::A:
                return std::make_unique<ConcreteProductA>();
            case ProductType::B:
                return std::make_unique<ConcreteProductB>();
            default:
                return nullptr;
        }
    }
};

// Использование
void useFactory() {
    auto productA = Factory::createProduct(Factory::ProductType::A);
    productA->use();
    
    auto productB = Factory::createProduct(Factory::ProductType::B);
    productB->use();
    
    // Продукты автоматически удаляются при выходе из scope
}
```

## 🎨 Альтернативные подходы

### Custom deleters
```cpp
// Кастомный deleter для unique_ptr
auto fileDeleter = [](FILE* fp) {
    if (fp) {
        fclose(fp);
        std::cout << "Файл закрыт" << std::endl;
    }
};

std::unique_ptr<FILE, decltype(fileDeleter)> file(
    fopen("data.txt", "r"),
    fileDeleter
);

// Кастомный deleter для shared_ptr
std::shared_ptr<FILE> sharedFile(
    fopen("data.txt", "r"),
    [](FILE* fp) {
        if (fp) fclose(fp);
    }
);
```

### std::enable_shared_from_this
```cpp
// Класс, который может возвращать shared_ptr на себя
class Widget : public std::enable_shared_from_this<Widget> {
public:
    std::shared_ptr<Widget> getShared() {
        // Безопасно возвращает shared_ptr на this
        return shared_from_this();
    }
    
    void registerCallback() {
        // Можно безопасно передать shared_ptr в callback
        someService.registerCallback(shared_from_this());
    }
};

// Использование
auto widget = std::make_shared<Widget>();
auto sameWidget = widget->getShared();
// widget и sameWidget указывают на один объект
```

## 🚀 Современные подходы в C++

### C++14: std::make_unique
```cpp
// C++14 добавил make_unique (было только make_shared)
auto ptr = std::make_unique<MyClass>(arg1, arg2);

// Преимущества make_unique/make_shared:
// 1. Exception safety
// 2. Меньше дублирования типов
// 3. Эффективнее (make_shared делает одну аллокацию)
```

### C++17: std::shared_ptr<T[]>
```cpp
// C++17 добавил поддержку массивов в shared_ptr
std::shared_ptr<int[]> arr = std::shared_ptr<int[]>(new int[10]);

// Автоматический вызов delete[] вместо delete
```

### C++20: std::make_shared для массивов
```cpp
// C++20 добавил make_shared для массивов
auto arr = std::make_shared<int[]>(10);  // Массив из 10 элементов
```

### C++23: std::out_ptr и std::inout_ptr
```cpp
// C++23 добавил утилиты для работы с C API
void cApiFunction(int** outPtr);

std::unique_ptr<int> ptr;
cApiFunction(std::out_ptr(ptr));
// Автоматически оборачивает сырой указатель в unique_ptr
```

## 🧪 Тестирование и отладка

### Отладка утечек памяти
```cpp
// Использование AddressSanitizer
// g++ -fsanitize=address -g program.cpp

// Использование Valgrind
// valgrind --leak-check=full ./program

// Пример кода для проверки
void testMemoryLeaks() {
    // Утечки НЕТ - автоматическое удаление
    auto ptr1 = std::make_unique<int>(42);
    
    // Утечки НЕТ - даже при исключении
    auto ptr2 = std::make_shared<std::vector<int>>(1000000);
    
    // Можем явно проверить счетчик ссылок
    std::cout << "Владельцев: " << ptr2.use_count() << std::endl;
}
```

### Mock и тестирование
```cpp
// Интерфейс для тестирования
class IDatabase {
public:
    virtual ~IDatabase() = default;
    virtual void query(const std::string& sql) = 0;
};

// Реальная реализация
class RealDatabase : public IDatabase {
public:
    void query(const std::string& sql) override {
        // Реальный запрос к БД
    }
};

// Mock для тестов
class MockDatabase : public IDatabase {
public:
    void query(const std::string& sql) override {
        // Мок-реализация
        queryCalls.push_back(sql);
    }
    
    std::vector<std::string> queryCalls;
};

// Использование с умными указателями
class Service {
private:
    std::unique_ptr<IDatabase> db_;
    
public:
    explicit Service(std::unique_ptr<IDatabase> db) 
        : db_(std::move(db)) {}
    
    void doWork() {
        db_->query("SELECT * FROM users");
    }
};

// Тест
void testService() {
    auto mockDb = std::make_unique<MockDatabase>();
    auto* dbPtr = mockDb.get();  // Сохраняем сырой указатель для проверок
    
    Service service(std::move(mockDb));
    service.doWork();
    
    // Проверяем что query был вызван
    assert(dbPtr->queryCalls.size() == 1);
}
```

## 🎯 Практические упражнения

### Упражнение 1: Рефакторинг с сырых указателей на умные
Перепишите код, использующий new/delete, на умные указатели.

### Упражнение 2: Разрешение циклических зависимостей
Реализуйте двусвязный список используя shared_ptr и weak_ptr.

### Упражнение 3: Custom deleter
Создайте умный указатель с кастомным deleter для управления системным ресурсом (файл, сокет, mutex).

### Упражнение 4: Performance анализ
Сравните производительность unique_ptr, shared_ptr и сырых указателей в различных сценариях.

## 📈 Следующие шаги
После изучения умных указателей вы будете готовы к:
- Уроку 1.4: Move Semantics (семантика перемещения)
- Глубокому пониманию RAII паттерна
- Изучению advanced техник управления памятью
- Написанию exception-safe кода

## ⚠️ Важные предупреждения

### 1. Избегайте смешивания умных и сырых указателей
```cpp
// ❌ ОПАСНО - смешивание умных и сырых указателей
auto smartPtr = std::make_unique<int>(42);
int* rawPtr = smartPtr.get();
delete rawPtr;  // ОШИБКА! Double delete!
```

### 2. Избегайте циклов в shared_ptr
```cpp
// ❌ ОПАСНО - циклическая зависимость
struct Node {
    std::shared_ptr<Node> next;  // Будет утечка!
};

// ✅ ПРАВИЛЬНО - используйте weak_ptr
struct Node {
    std::shared_ptr<Node> next;
    std::weak_ptr<Node> prev;  // Разрывает цикл
};
```

### 3. make_unique/make_shared vs new
```cpp
// ✅ ПРЕДПОЧТИТЕЛЬНО - exception safe
auto ptr = std::make_unique<Widget>(arg1, arg2);

// ❌ ИЗБЕГАЙТЕ - может быть утечка при исключении
processData(std::unique_ptr<Widget>(new Widget(arg1)), computeValue());
// Если computeValue() выбросит исключение, может быть утечка!
```

## 📚 Дополнительные ресурсы

### Теоретические основы
- **Scott Meyers "Effective Modern C++"** - Items 18-22 о умных указателях
- **Herb Sutter "Exceptional C++"** - Exception safety и RAII
- **Bjarne Stroustrup "The C++ Programming Language"** - Глава о управлении ресурсами

### Онлайн ресурсы
- [cppreference.com - Smart pointers](https://en.cppreference.com/w/cpp/memory)
- [C++ Core Guidelines - R (Resource management)](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#S-resource)
- [CppCon talks on memory management](https://www.youtube.com/user/CppCon)

---

*"Make interfaces easy to use correctly and hard to use incorrectly."* - Scott Meyers

