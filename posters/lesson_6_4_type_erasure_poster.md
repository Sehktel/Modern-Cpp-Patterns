# 🎭 Type Erasure - Плакат

## 📋 Обзор паттерна

**Type Erasure** - это техника C++, которая позволяет скрыть конкретные типы за единым интерфейсом, сохраняя при этом их специфичное поведение. Это достигается через комбинацию виртуальных функций, шаблонов и полиморфизма.

## 🎯 Назначение

- **Скрытие типов**: Сокрытие конкретных типов за единым интерфейсом
- **Гетерогенные коллекции**: Хранение объектов разных типов в одной коллекции
- **Полиморфизм без наследования**: Полиморфизм для типов без общего базового класса
- **Гибкость**: Возможность работы с неизвестными на этапе компиляции типами

## 🏗️ Структура паттерна

```
┌─────────────────┐
│   Interface     │
│   (интерфейс)   │
├─────────────────┤
│ + operation()   │
└─────────────────┘
         ▲
         │
┌────────┴────────┐
│   Type Eraser   │
│   (стиратель)   │
├─────────────────┤
│ - concrete      │
│ + operation()   │
└─────────────────┘
         ▲
         │
    ┌────┴────┐
    │         │
┌───▼───┐ ┌──▼───┐
│Type A │ │Type B│
├───────┤ ├───────┤
│+method│ │+method│
└───────┘ └───────┘
```

## 💡 Ключевые компоненты

### 1. Базовый интерфейс
```cpp
// Базовый интерфейс для type erasure
class Drawable {
public:
    virtual ~Drawable() = default;
    virtual void draw() = 0;
    virtual std::unique_ptr<Drawable> clone() const = 0;
};
```

### 2. Конкретная реализация
```cpp
// Конкретная реализация для конкретного типа
template<typename T>
class DrawableImpl : public Drawable {
private:
    T object;
    
public:
    DrawableImpl(T obj) : object(std::move(obj)) {}
    
    void draw() override {
        object.draw();
    }
    
    std::unique_ptr<Drawable> clone() const override {
        return std::make_unique<DrawableImpl<T>>(object);
    }
};
```

### 3. Type Eraser класс
```cpp
// Основной класс для type erasure
class AnyDrawable {
private:
    std::unique_ptr<Drawable> impl;
    
public:
    // Конструктор для любого типа с методом draw()
    template<typename T>
    AnyDrawable(T obj) : impl(std::make_unique<DrawableImpl<T>>(std::move(obj))) {}
    
    void draw() {
        impl->draw();
    }
    
    AnyDrawable clone() const {
        return AnyDrawable(impl->clone());
    }
};
```

## 🔧 Практический пример: Система обработки задач

```cpp
#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <type_traits>

// Базовый интерфейс для задач
class Task {
public:
    virtual ~Task() = default;
    virtual void execute() = 0;
    virtual std::string getDescription() const = 0;
    virtual int getPriority() const = 0;
    virtual std::unique_ptr<Task> clone() const = 0;
};

// Реализация для конкретного типа задачи
template<typename TaskType>
class TaskImpl : public Task {
private:
    TaskType task;
    
public:
    TaskImpl(TaskType t) : task(std::move(t)) {}
    
    void execute() override {
        task.execute();
    }
    
    std::string getDescription() const override {
        return task.getDescription();
    }
    
    int getPriority() const override {
        return task.getPriority();
    }
    
    std::unique_ptr<Task> clone() const override {
        return std::make_unique<TaskImpl<TaskType>>(task);
    }
};

// Type Eraser для задач
class AnyTask {
private:
    std::unique_ptr<Task> impl;
    
public:
    // Конструктор для любого типа с методами Task
    template<typename T>
    AnyTask(T task) : impl(std::make_unique<TaskImpl<T>>(std::move(task))) {
        static_assert(
            std::is_invocable_v<decltype(&T::execute), T> &&
            std::is_invocable_v<decltype(&T::getDescription), const T> &&
            std::is_invocable_v<decltype(&T::getPriority), const T>,
            "Task type must have execute(), getDescription() and getPriority() methods"
        );
    }
    
    // Копирующий конструктор
    AnyTask(const AnyTask& other) : impl(other.impl->clone()) {}
    
    // Оператор присваивания
    AnyTask& operator=(const AnyTask& other) {
        if (this != &other) {
            impl = other.impl->clone();
        }
        return *this;
    }
    
    // Методы делегирования
    void execute() {
        impl->execute();
    }
    
    std::string getDescription() const {
        return impl->getDescription();
    }
    
    int getPriority() const {
        return impl->getPriority();
    }
    
    AnyTask clone() const {
        return AnyTask(impl->clone());
    }
};

// Конкретные типы задач
class EmailTask {
private:
    std::string recipient;
    std::string subject;
    std::string body;
    int priority;
    
public:
    EmailTask(const std::string& rec, const std::string& subj, const std::string& b, int prio = 5)
        : recipient(rec), subject(subj), body(b), priority(prio) {}
    
    void execute() {
        std::cout << "📧 Отправка email:" << std::endl;
        std::cout << "   Кому: " << recipient << std::endl;
        std::cout << "   Тема: " << subject << std::endl;
        std::cout << "   Содержание: " << body << std::endl;
    }
    
    std::string getDescription() const {
        return "Email to " + recipient + ": " + subject;
    }
    
    int getPriority() const {
        return priority;
    }
};

class DatabaseTask {
private:
    std::string query;
    std::string database;
    int priority;
    
public:
    DatabaseTask(const std::string& q, const std::string& db, int prio = 3)
        : query(q), database(db), priority(prio) {}
    
    void execute() {
        std::cout << "🗄️ Выполнение SQL запроса:" << std::endl;
        std::cout << "   База данных: " << database << std::endl;
        std::cout << "   Запрос: " << query << std::endl;
    }
    
    std::string getDescription() const {
        return "SQL query on " + database + ": " + query;
    }
    
    int getPriority() const {
        return priority;
    }
};

class FileTask {
private:
    std::string filename;
    std::string operation;
    int priority;
    
public:
    FileTask(const std::string& file, const std::string& op, int prio = 2)
        : filename(file), operation(op), priority(prio) {}
    
    void execute() {
        std::cout << "📁 Файловая операция:" << std::endl;
        std::cout << "   Файл: " << filename << std::endl;
        std::cout << "   Операция: " << operation << std::endl;
    }
    
    std::string getDescription() const {
        return "File " + operation + ": " + filename;
    }
    
    int getPriority() const {
        return priority;
    }
};

class NetworkTask {
private:
    std::string url;
    std::string method;
    int priority;
    
public:
    NetworkTask(const std::string& u, const std::string& m, int prio = 4)
        : url(u), method(m), priority(prio) {}
    
    void execute() {
        std::cout << "🌐 Сетевой запрос:" << std::endl;
        std::cout << "   URL: " << url << std::endl;
        std::cout << "   Метод: " << method << std::endl;
    }
    
    std::string getDescription() const {
        return "HTTP " + method + " " + url;
    }
    
    int getPriority() const {
        return priority;
    }
};

// Менеджер задач с поддержкой type erasure
class TaskManager {
private:
    std::vector<AnyTask> tasks;
    
public:
    // Добавление задачи любого типа
    template<typename TaskType>
    void addTask(TaskType task) {
        tasks.emplace_back(std::move(task));
    }
    
    // Выполнение всех задач
    void executeAll() {
        std::cout << "\n🚀 Выполнение всех задач:" << std::endl;
        for (size_t i = 0; i < tasks.size(); ++i) {
            std::cout << "\n--- Задача " << (i + 1) << " ---" << std::endl;
            tasks[i].execute();
        }
    }
    
    // Выполнение задач по приоритету
    void executeByPriority() {
        std::cout << "\n📊 Выполнение задач по приоритету:" << std::endl;
        
        // Сортировка по приоритету (высший приоритет = меньшее число)
        std::vector<std::pair<int, size_t>> priorityIndex;
        for (size_t i = 0; i < tasks.size(); ++i) {
            priorityIndex.emplace_back(tasks[i].getPriority(), i);
        }
        
        std::sort(priorityIndex.begin(), priorityIndex.end());
        
        for (const auto& [priority, index] : priorityIndex) {
            std::cout << "\n--- Приоритет " << priority << " ---" << std::endl;
            std::cout << "Описание: " << tasks[index].getDescription() << std::endl;
            tasks[index].execute();
        }
    }
    
    // Получение информации о задачах
    void printTaskInfo() {
        std::cout << "\n📋 Информация о задачах:" << std::endl;
        for (size_t i = 0; i < tasks.size(); ++i) {
            std::cout << "Задача " << (i + 1) << ":" << std::endl;
            std::cout << "   Описание: " << tasks[i].getDescription() << std::endl;
            std::cout << "   Приоритет: " << tasks[i].getPriority() << std::endl;
        }
    }
    
    // Клонирование всех задач
    TaskManager clone() const {
        TaskManager cloned;
        for (const auto& task : tasks) {
            cloned.tasks.push_back(task.clone());
        }
        return cloned;
    }
    
    size_t getTaskCount() const {
        return tasks.size();
    }
    
    // Очистка задач
    void clear() {
        tasks.clear();
    }
};

// Демонстрация использования
int main() {
    std::cout << "=== Демонстрация Type Erasure ===" << std::endl;
    
    TaskManager manager;
    
    // Добавление задач разных типов в одну коллекцию
    manager.addTask(EmailTask("admin@company.com", "Отчет", "Еженедельный отчет готов", 1));
    manager.addTask(DatabaseTask("SELECT * FROM users", "production", 3));
    manager.addTask(FileTask("backup.sql", "compress", 2));
    manager.addTask(NetworkTask("https://api.example.com/data", "GET", 4));
    manager.addTask(EmailTask("support@company.com", "Поддержка", "Новый тикет", 5));
    manager.addTask(DatabaseTask("UPDATE users SET last_login = NOW()", "production", 3));
    
    std::cout << "📊 Статистика:" << std::endl;
    std::cout << "Количество задач: " << manager.getTaskCount() << std::endl;
    
    // Вывод информации о задачах
    manager.printTaskInfo();
    
    // Выполнение задач по приоритету
    manager.executeByPriority();
    
    // Демонстрация клонирования
    std::cout << "\n🔄 Клонирование менеджера задач:" << std::endl;
    auto clonedManager = manager.clone();
    std::cout << "Клонированный менеджер содержит " << clonedManager.getTaskCount() << " задач" << std::endl;
    
    // Выполнение клонированных задач
    std::cout << "\n🚀 Выполнение клонированных задач:" << std::endl;
    clonedManager.executeAll();
    
    // Демонстрация работы с отдельными задачами
    std::cout << "\n🎯 Работа с отдельными задачами:" << std::endl;
    
    // Создание задачи и обертывание в type eraser
    EmailTask emailTask("test@example.com", "Тест", "Тестовое сообщение", 1);
    AnyTask anyTask(emailTask);
    
    std::cout << "Описание задачи: " << anyTask.getDescription() << std::endl;
    std::cout << "Приоритет: " << anyTask.getPriority() << std::endl;
    anyTask.execute();
    
    // Клонирование отдельной задачи
    auto clonedTask = anyTask.clone();
    std::cout << "\nКлонированная задача:" << std::endl;
    clonedTask.execute();
    
    return 0;
}
```

## 🎯 Применение в реальных проектах

### 1. **std::function**
```cpp
#include <functional>

// std::function - классический пример type erasure
std::function<void()> task;

// Можно присвоить любую функцию с подходящей сигнатурой
task = []() { std::cout << "Lambda function" << std::endl; };
task = std::bind(&SomeClass::method, &obj);

// Вызов без знания конкретного типа
task();
```

### 2. **Системы плагинов**
```cpp
// Type erasure для плагинов
class Plugin {
public:
    virtual ~Plugin() = default;
    virtual void initialize() = 0;
    virtual void execute() = 0;
    virtual void cleanup() = 0;
};

template<typename PluginType>
class PluginWrapper : public Plugin {
private:
    PluginType plugin;
    
public:
    PluginWrapper(PluginType p) : plugin(std::move(p)) {}
    
    void initialize() override { plugin.initialize(); }
    void execute() override { plugin.execute(); }
    void cleanup() override { plugin.cleanup(); }
};

class PluginManager {
private:
    std::vector<std::unique_ptr<Plugin>> plugins;
    
public:
    template<typename PluginType>
    void loadPlugin(PluginType plugin) {
        plugins.push_back(std::make_unique<PluginWrapper<PluginType>>(std::move(plugin)));
    }
};
```

### 3. **Системы сериализации**
```cpp
// Type erasure для сериализации
class Serializable {
public:
    virtual ~Serializable() = default;
    virtual std::string serialize() = 0;
    virtual void deserialize(const std::string& data) = 0;
};

template<typename T>
class SerializableWrapper : public Serializable {
private:
    T object;
    
public:
    SerializableWrapper(T obj) : object(std::move(obj)) {}
    
    std::string serialize() override {
        return object.serialize();
    }
    
    void deserialize(const std::string& data) override {
        object.deserialize(data);
    }
};
```

## ⚡ Преимущества паттерна

### ✅ **Плюсы:**
- **Гетерогенные коллекции**: Хранение объектов разных типов в одной коллекции
- **Полиморфизм без наследования**: Работа с типами без общего базового класса
- **Гибкость**: Возможность работы с неизвестными типами
- **Инкапсуляция**: Сокрытие конкретных типов за единым интерфейсом
- **Переиспользование**: Единый интерфейс для разных типов

### ❌ **Минусы:**
- **Накладные расходы**: Виртуальные вызовы и динамическое выделение памяти
- **Сложность**: Сложнее понять и отладить
- **Ограничения**: Не все операции доступны через интерфейс
- **Производительность**: Медленнее прямых вызовов

## 🔄 Альтернативы и связанные паттерны

### **Альтернативы:**
- **std::variant**: Для ограниченного набора типов
- **std::any**: Для полного стирания типов
- **CRTP**: Для статического полиморфизма

### **Связанные паттерны:**
- **Bridge**: Type erasure может реализовывать Bridge
- **Adapter**: Адаптация интерфейсов
- **Facade**: Упрощение сложных интерфейсов

## 🎨 Современные улучшения в C++

### 1. **Использование std::any (C++17)**
```cpp
#include <any>

// Полное стирание типов
class AnyContainer {
private:
    std::any data;
    
public:
    template<typename T>
    void store(T value) {
        data = std::move(value);
    }
    
    template<typename T>
    T retrieve() {
        return std::any_cast<T>(data);
    }
    
    template<typename T>
    bool contains() const {
        return data.type() == typeid(T);
    }
};
```

### 2. **Использование std::variant (C++17)**
```cpp
#include <variant>

// Type erasure для ограниченного набора типов
using TaskVariant = std::variant<EmailTask, DatabaseTask, FileTask>;

class VariantTaskManager {
private:
    std::vector<TaskVariant> tasks;
    
public:
    template<typename T>
    void addTask(T task) {
        tasks.emplace_back(std::move(task));
    }
    
    void executeAll() {
        for (auto& task : tasks) {
            std::visit([](auto& t) { t.execute(); }, task);
        }
    }
};
```

### 3. **Концепты и шаблоны (C++20)**
```cpp
#include <concepts>

// Концепт для задач
template<typename T>
concept TaskConcept = requires(T t) {
    t.execute();
    t.getDescription();
    t.getPriority();
};

// Безопасный type eraser с концептами
template<TaskConcept T>
class SafeTaskWrapper {
private:
    T task;
    
public:
    SafeTaskWrapper(T t) : task(std::move(t)) {}
    
    void execute() { task.execute(); }
    std::string getDescription() const { return task.getDescription(); }
    int getPriority() const { return task.getPriority(); }
};
```

## 🚀 Лучшие практики

### 1. **Правильное использование**
```cpp
// ✅ Хорошо: Четкий интерфейс
class Drawable {
public:
    virtual ~Drawable() = default;
    virtual void draw() = 0;
    virtual std::unique_ptr<Drawable> clone() const = 0;
};

// ✅ Хорошо: Правильная реализация
template<typename T>
class DrawableImpl : public Drawable {
private:
    T object;
    
public:
    DrawableImpl(T obj) : object(std::move(obj)) {}
    
    void draw() override { object.draw(); }
    std::unique_ptr<Drawable> clone() const override {
        return std::make_unique<DrawableImpl<T>>(object);
    }
};
```

### 2. **Управление памятью**
```cpp
// ✅ Хорошо: RAII для управления памятью
class SafeTypeEraser {
private:
    std::unique_ptr<Interface> impl;
    
public:
    template<typename T>
    SafeTypeEraser(T obj) : impl(std::make_unique<Impl<T>>(std::move(obj))) {}
    
    // Автоматическая очистка при уничтожении
    ~SafeTypeEraser() = default;
    
    // Правильное копирование
    SafeTypeEraser(const SafeTypeEraser& other) : impl(other.impl->clone()) {}
};
```

### 3. **Обработка ошибок**
```cpp
// ✅ Хорошо: Обработка ошибок в type eraser
class RobustTypeEraser {
private:
    std::unique_ptr<Interface> impl;
    
public:
    template<typename T>
    RobustTypeEraser(T obj) try : impl(std::make_unique<Impl<T>>(std::move(obj))) {
    } catch (const std::exception& e) {
        // Обработка ошибок создания
        throw std::runtime_error("Failed to create type eraser: " + std::string(e.what()));
    }
    
    void operation() {
        try {
            impl->operation();
        } catch (const std::exception& e) {
            // Обработка ошибок выполнения
            std::cerr << "Operation failed: " << e.what() << std::endl;
        }
    }
};
```

## 📊 Сравнение с другими подходами

| Аспект | Type Erasure | std::variant | std::any | Виртуальные функции |
|--------|--------------|--------------|----------|---------------------|
| **Гибкость** | Высокая | Средняя | Очень высокая | Высокая |
| **Производительность** | Средняя | Высокая | Низкая | Средняя |
| **Типобезопасность** | Высокая | Высокая | Низкая | Высокая |
| **Сложность** | Высокая | Средняя | Низкая | Низкая |
| **Применение** | Неизвестные типы | Ограниченный набор | Любые типы | Иерархия классов |

## 🎯 Заключение

**Type Erasure** - это мощная техника C++ для создания гибких и полиморфных систем без жестких ограничений наследования. Она особенно полезна для библиотек и систем, где нужно работать с неизвестными типами.

### **Когда использовать:**
- ✅ Нужны гетерогенные коллекции
- ✅ Типы неизвестны на этапе компиляции
- ✅ Нет общего базового класса
- ✅ Нужна гибкость в работе с типами

### **Когда НЕ использовать:**
- ❌ Производительность критична
- ❌ Типы известны и ограничены
- ❌ Есть общий базовый класс
- ❌ Сложность кода критична

**Type Erasure** - это продвинутая техника C++, которая позволяет создавать гибкие и расширяемые системы! 🎭✨
