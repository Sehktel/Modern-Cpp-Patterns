# Урок 6.1: Smart Pointers (Умные указатели)

## 🎯 Цель урока
Изучить современные умные указатели C++11/14/17 - один из самых важных паттернов современного C++. Понять, как автоматически управлять памятью и избежать утечек памяти без потери производительности.

## 📚 Что изучаем

### 1. Умные указатели C++
- **std::unique_ptr**: Единоличное владение ресурсом
- **std::shared_ptr**: Разделяемое владение ресурсом
- **std::weak_ptr**: Слабая ссылка на shared_ptr
- **std::auto_ptr**: Устаревший (удален в C++17)

### 2. Принципы работы
- **RAII**: Resource Acquisition Is Initialization
- **Automatic cleanup**: Автоматическая очистка ресурсов
- **Exception safety**: Безопасность при исключениях
- **Move semantics**: Семантика перемещения

### 3. Современные подходы
- **Custom deleters**: Пользовательские удалители
- **make_unique/make_shared**: Фабричные функции
- **enable_shared_from_this**: Разделение указателей из this
- **Aliasing constructors**: Алиасинг конструкторы

## 🔍 Ключевые концепции

### std::unique_ptr
```cpp
// Единоличное владение
std::unique_ptr<int> ptr1 = std::make_unique<int>(42);
std::unique_ptr<int> ptr2 = std::move(ptr1); // Передача владения

// Автоматическая очистка
{
    std::unique_ptr<MyClass> obj = std::make_unique<MyClass>();
    // obj автоматически удалится при выходе из области видимости
}

// Пользовательский удалитель
auto deleter = [](int* ptr) {
    std::cout << "Удаляем int: " << *ptr << std::endl;
    delete ptr;
};
std::unique_ptr<int, decltype(deleter)> ptr(new int(100), deleter);
```

### std::shared_ptr
```cpp
// Разделяемое владение
std::shared_ptr<int> ptr1 = std::make_shared<int>(42);
std::shared_ptr<int> ptr2 = ptr1; // Разделяемое владение

std::cout << "Счетчик ссылок: " << ptr1.use_count() << std::endl; // 2

// Слабая ссылка
std::weak_ptr<int> weak = ptr1;
if (auto locked = weak.lock()) {
    std::cout << "Значение: " << *locked << std::endl;
}

ptr1.reset(); // Сбрасываем одну ссылку
// ptr2 все еще владеет объектом
```

### Пользовательские удалители
```cpp
// Для файлов
auto fileDeleter = [](FILE* f) {
    if (f) {
        fclose(f);
        std::cout << "Файл закрыт" << std::endl;
    }
};
std::unique_ptr<FILE, decltype(fileDeleter)> file(fopen("test.txt", "r"), fileDeleter);

// Для массивов
std::unique_ptr<int[]> array = std::make_unique<int[]>(100);

// Для объектов с custom deleter
struct CustomDeleter {
    void operator()(MyClass* obj) {
        obj->cleanup();
        delete obj;
    }
};
std::unique_ptr<MyClass, CustomDeleter> customObj(new MyClass(), CustomDeleter{});
```

## 🤔 Вопросы для размышления

### 1. Когда использовать какой умный указатель?
**✅ std::unique_ptr:**
- Единоличное владение ресурсом
- Когда нужна максимальная производительность
- Когда не нужны циклические ссылки
- Для управления файлами, сокетами, ресурсами

**✅ std::shared_ptr:**
- Разделяемое владение ресурсом
- Когда количество владельцев неизвестно заранее
- Для кэширования и пулов объектов
- Когда нужны слабые ссылки

**✅ std::weak_ptr:**
- Для разрыва циклических ссылок
- Для кэширования с автоматической очисткой
- Для временных ссылок на shared_ptr

### 2. Преимущества умных указателей
- **Автоматическая очистка**: Нет утечек памяти
- **Exception safety**: Безопасность при исключениях
- **Явное владение**: Четкое понимание владения ресурсами
- **Производительность**: Минимальные накладные расходы

### 3. Недостатки умных указателей
- **Циклические ссылки**: shared_ptr может создавать циклы
- **Производительность**: shared_ptr имеет накладные расходы
- **Сложность**: Может усложнить понимание кода
- **Совместимость**: Проблемы с C API

## 🛠️ Практические примеры

### Управление файлами
```cpp
class FileManager {
private:
    std::unique_ptr<FILE, decltype(&fclose)> file_;
    
public:
    explicit FileManager(const std::string& filename) 
        : file_(fopen(filename.c_str(), "r"), &fclose) {
        if (!file_) {
            throw std::runtime_error("Не удалось открыть файл: " + filename);
        }
    }
    
    std::string readLine() {
        char buffer[256];
        if (fgets(buffer, sizeof(buffer), file_.get())) {
            return std::string(buffer);
        }
        return "";
    }
    
    // Файл автоматически закроется при уничтожении объекта
};
```

### Управление ресурсами
```cpp
class ResourceManager {
private:
    std::unique_ptr<int[]> buffer_;
    size_t size_;
    
public:
    explicit ResourceManager(size_t size) 
        : buffer_(std::make_unique<int[]>(size)), size_(size) {
        std::cout << "Выделена память для " << size << " элементов" << std::endl;
    }
    
    void fillWithData() {
        for (size_t i = 0; i < size_; ++i) {
            buffer_[i] = static_cast<int>(i * i);
        }
    }
    
    int getValue(size_t index) const {
        if (index < size_) {
            return buffer_[index];
        }
        throw std::out_of_range("Индекс вне диапазона");
    }
    
    // Память автоматически освободится при уничтожении объекта
};
```

### Разделяемое владение
```cpp
class Document {
private:
    std::string content_;
    
public:
    Document(const std::string& content) : content_(content) {}
    
    void addContent(const std::string& text) {
        content_ += text;
    }
    
    const std::string& getContent() const {
        return content_;
    }
};

class DocumentManager {
private:
    std::vector<std::shared_ptr<Document>> documents_;
    
public:
    std::shared_ptr<Document> createDocument(const std::string& content) {
        auto doc = std::make_shared<Document>(content);
        documents_.push_back(doc);
        return doc;
    }
    
    void removeDocument(std::shared_ptr<Document> doc) {
        documents_.erase(
            std::remove(documents_.begin(), documents_.end(), doc),
            documents_.end());
    }
    
    size_t getDocumentCount() const {
        return documents_.size();
    }
};
```

## 🎨 Современные подходы в C++

### enable_shared_from_this
```cpp
class SharedObject : public std::enable_shared_from_this<SharedObject> {
private:
    int value_;
    
public:
    explicit SharedObject(int value) : value_(value) {}
    
    std::shared_ptr<SharedObject> getSharedPtr() {
        return shared_from_this();
    }
    
    void doSomething() {
        auto self = shared_from_this();
        // Передаем self в асинхронную операцию
        std::async(std::launch::async, [self]() {
            // self остается живым до завершения операции
            std::cout << "Значение: " << self->value_ << std::endl;
        });
    }
};
```

### Алиасинг конструкторы
```cpp
class Container {
private:
    std::shared_ptr<int[]> data_;
    size_t size_;
    
public:
    Container(size_t size) : size_(size) {
        data_ = std::shared_ptr<int[]>(new int[size], std::default_delete<int[]>());
    }
    
    std::shared_ptr<int> getElement(size_t index) {
        if (index < size_) {
            // Создаем shared_ptr, который разделяет владение с data_
            return std::shared_ptr<int>(data_, &data_[index]);
        }
        return nullptr;
    }
};
```

### Custom deleter с состоянием
```cpp
class StatefulDeleter {
private:
    std::string name_;
    
public:
    explicit StatefulDeleter(const std::string& name) : name_(name) {}
    
    void operator()(int* ptr) {
        std::cout << "Удаляем " << name_ << ": " << *ptr << std::endl;
        delete ptr;
    }
};

// Использование
std::unique_ptr<int, StatefulDeleter> ptr(new int(42), StatefulDeleter("my_int"));
```

## 🧪 Тестирование Smart Pointers

### Проверка владения
```cpp
TEST(SmartPointerTest, Ownership) {
    std::unique_ptr<int> ptr1 = std::make_unique<int>(42);
    std::unique_ptr<int> ptr2 = std::move(ptr1);
    
    EXPECT_EQ(ptr1.get(), nullptr);
    EXPECT_NE(ptr2.get(), nullptr);
    EXPECT_EQ(*ptr2, 42);
}

TEST(SmartPointerTest, SharedOwnership) {
    std::shared_ptr<int> ptr1 = std::make_shared<int>(42);
    std::shared_ptr<int> ptr2 = ptr1;
    
    EXPECT_EQ(ptr1.use_count(), 2);
    EXPECT_EQ(ptr2.use_count(), 2);
    EXPECT_EQ(*ptr1, 42);
    EXPECT_EQ(*ptr2, 42);
}
```

## 🎯 Практические упражнения

### Упражнение 1: Менеджер ресурсов
Создайте класс для управления различными типами ресурсов с использованием unique_ptr.

### Упражнение 2: Кэш с weak_ptr
Реализуйте кэш объектов с автоматической очисткой неиспользуемых элементов.

### Упражнение 3: Асинхронные операции
Создайте систему асинхронных операций с использованием shared_ptr для управления жизненным циклом объектов.

### Упражнение 4: RAII обертки
Создайте RAII обертки для различных системных ресурсов (файлы, мьютексы, сокеты).

## 📈 Связь с другими паттернами

### Smart Pointers vs RAII
- **Smart Pointers**: Реализация RAII для указателей
- **RAII**: Общий принцип управления ресурсами

### Smart Pointers vs Garbage Collection
- **Smart Pointers**: Детерминированная очистка
- **Garbage Collection**: Неопределенная очистка

### Smart Pointers vs Manual Memory Management
- **Smart Pointers**: Автоматическое управление
- **Manual Management**: Ручное управление

## 📈 Следующие шаги
После изучения Smart Pointers вы будете готовы к:
- Уроку 6.2: Move Semantics
- Пониманию современного C++
- Созданию безопасного кода
- Управлению ресурсами

## 💡 Важные принципы

1. **Предпочитайте умные указатели**: Избегайте сырых указателей
2. **Выбирайте правильный тип**: unique_ptr для единоличного владения, shared_ptr для разделяемого
3. **Используйте make_unique/make_shared**: Более эффективно и безопасно
4. **Избегайте циклических ссылок**: Используйте weak_ptr
5. **Не смешивайте умные и сырые указатели**: Может привести к ошибкам
