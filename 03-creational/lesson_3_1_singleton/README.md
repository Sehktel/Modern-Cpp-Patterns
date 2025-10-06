# Урок 3.1: Singleton Pattern (Одиночка)

## 🎯 Цель урока
Изучить паттерн Singleton - один из самых известных и спорных паттернов проектирования. Понять, когда его использовать, а когда избегать, и как правильно реализовать в современном C++.

## 📚 Что изучаем

### 1. Паттерн Singleton
- **Определение**: Обеспечивает существование только одного экземпляра класса
- **Назначение**: Контроль доступа к единственному ресурсу
- **Применение**: Логгеры, конфигурации, соединения с БД, кэши

### 2. Проблемы и решения
- **Thread Safety**: Безопасность в многопоточном коде
- **Lazy vs Eager Initialization**: Когда создавать экземпляр
- **Destruction Order**: Проблемы с порядком уничтожения
- **Testing**: Сложности с тестированием

### 3. Современные подходы
- **Meyers Singleton**: Thread-safe с C++11
- **std::call_once**: Гарантированная однократная инициализация
- **Dependency Injection**: Альтернатива Singleton
- **Global State**: Когда лучше использовать глобальные переменные

## 🔍 Ключевые концепции

### Классический Singleton (проблемный)
```cpp
class ClassicSingleton {
private:
    static ClassicSingleton* instance_;
    ClassicSingleton() = default;
    
public:
    static ClassicSingleton* getInstance() {
        if (!instance_) {
            instance_ = new ClassicSingleton();
        }
        return instance_;
    }
    
    // ПРОБЛЕМЫ:
    // - Не thread-safe
    // - Утечка памяти (нет delete)
    // - Сложно тестировать
};
```

### Thread-Safe Singleton (C++11+)
```cpp
class ThreadSafeSingleton {
private:
    ThreadSafeSingleton() = default;
    
public:
    static ThreadSafeSingleton& getInstance() {
        static ThreadSafeSingleton instance;  // Thread-safe с C++11
        return instance;
    }
    
    // Запрещаем копирование и присваивание
    ThreadSafeSingleton(const ThreadSafeSingleton&) = delete;
    ThreadSafeSingleton& operator=(const ThreadSafeSingleton&) = delete;
};
```

### Lazy vs Eager Initialization
```cpp
// Lazy (ленивая) инициализация
class LazySingleton {
private:
    static std::unique_ptr<LazySingleton> instance_;
    static std::once_flag initialized_;
    
public:
    static LazySingleton& getInstance() {
        std::call_once(initialized_, []() {
            instance_ = std::make_unique<LazySingleton>();
        });
        return *instance_;
    }
};

// Eager (жадная) инициализация
class EagerSingleton {
private:
    static EagerSingleton instance_;  // Создается при загрузке программы
    
public:
    static EagerSingleton& getInstance() {
        return instance_;
    }
};
```

## 🤔 Вопросы для размышления

### 1. Когда использовать Singleton?
**✅ Хорошие случаи:**
- Логгеры (один глобальный логгер)
- Конфигурации приложения
- Соединения с базой данных
- Кэши (один глобальный кэш)

**❌ Плохие случаи:**
- Когда нужны несколько экземпляров
- Для объектов с состоянием
- Когда нужно тестировать изоляцию
- Для простых утилитарных функций

### 2. Почему Singleton считается антипаттерном?
- **Скрытые зависимости**: Код зависит от глобального состояния
- **Сложность тестирования**: Невозможно изолировать тесты
- **Нарушение принципов**: Нарушает SRP и DIP
- **Thread safety**: Сложности в многопоточном коде
- **Порядок инициализации**: Проблемы с static initialization order

### 3. Альтернативы Singleton
- **Dependency Injection**: Передавать зависимости явно
- **Service Locator**: Реестр сервисов
- **Monostate**: Несколько объектов с общим состоянием
- **Global variables**: Простые глобальные переменные

## 🛠️ Практические примеры

### Логгер как Singleton
```cpp
class Logger {
private:
    std::ofstream logFile_;
    Logger() {
        logFile_.open("application.log", std::ios::app);
    }
    
public:
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }
    
    void log(const std::string& message) {
        if (logFile_.is_open()) {
            logFile_ << message << std::endl;
        }
    }
    
    // Запрещаем копирование
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
};
```

### Конфигурация приложения
```cpp
class Config {
private:
    std::map<std::string, std::string> settings_;
    Config() {
        loadFromFile("config.ini");
    }
    
public:
    static Config& getInstance() {
        static Config instance;
        return instance;
    }
    
    std::string get(const std::string& key) const {
        auto it = settings_.find(key);
        return (it != settings_.end()) ? it->second : "";
    }
    
    void set(const std::string& key, const std::string& value) {
        settings_[key] = value;
    }
};
```

## 🎨 Альтернативные подходы

### Dependency Injection вместо Singleton
```cpp
// Плохо - скрытая зависимость
class UserService {
public:
    void saveUser(const User& user) {
        Logger::getInstance().log("Saving user");  // Скрытая зависимость
        // ...
    }
};

// Хорошо - явная зависимость
class UserService {
private:
    Logger& logger_;
    
public:
    explicit UserService(Logger& logger) : logger_(logger) {}
    
    void saveUser(const User& user) {
        logger_.log("Saving user");  // Явная зависимость
        // ...
    }
};
```

### Service Locator
```cpp
class ServiceLocator {
private:
    static std::map<std::string, std::any> services_;
    
public:
    template<typename T>
    static void registerService(const std::string& name, T service) {
        services_[name] = service;
    }
    
    template<typename T>
    static T& getService(const std::string& name) {
        return std::any_cast<T&>(services_[name]);
    }
};

// Использование
ServiceLocator::registerService<Logger>("logger", std::make_unique<Logger>());
auto& logger = ServiceLocator::getService<Logger>("logger");
```

## 🧪 Тестирование Singleton

### Проблемы с тестированием
```cpp
// Сложно тестировать из-за глобального состояния
class DatabaseConnection {
public:
    static DatabaseConnection& getInstance() {
        static DatabaseConnection instance;
        return instance;
    }
    
    void executeQuery(const std::string& query) {
        // Реальное подключение к БД
    }
};

// Тест будет зависеть от реальной БД
TEST(DatabaseTest, ExecuteQuery) {
    auto& db = DatabaseConnection::getInstance();
    // Проблема: тест зависит от глобального состояния
}
```

### Решение через Dependency Injection
```cpp
// Легко тестировать с моками
class DatabaseConnection {
public:
    virtual ~DatabaseConnection() = default;
    virtual void executeQuery(const std::string& query) = 0;
};

class MockDatabase : public DatabaseConnection {
public:
    void executeQuery(const std::string& query) override {
        // Мок-реализация для тестов
    }
};

// Тест с моком
TEST(DatabaseTest, ExecuteQuery) {
    MockDatabase mockDb;
    UserService service(mockDb);  // Легко тестировать
}
```

## 📈 Современные подходы в C++

### Template Singleton
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

// Использование
class MyService : public Singleton<MyService> {
    friend class Singleton<MyService>;
private:
    MyService() = default;
};
```

### RAII Singleton
```cpp
class RAIISingleton {
private:
    static std::unique_ptr<RAIISingleton> instance_;
    static std::once_flag initialized_;
    
    RAIISingleton() = default;
    
public:
    static RAIISingleton& getInstance() {
        std::call_once(initialized_, []() {
            instance_ = std::make_unique<RAIISingleton>();
        });
        return *instance_;
    }
    
    ~RAIISingleton() = default;
    
    // Запрещаем копирование
    RAIISingleton(const RAIISingleton&) = delete;
    RAIISingleton& operator=(const RAIISingleton&) = delete;
};
```

## 🎯 Практические упражнения

### Упражнение 1: Рефакторинг Singleton в Dependency Injection
Перепишите код, использующий Singleton, на Dependency Injection.

### Упражнение 2: Thread-Safe Singleton
Реализуйте thread-safe Singleton с использованием std::call_once.

### Упражнение 3: Template Singleton
Создайте шаблонный Singleton для переиспользования.

### Упражнение 4: Тестирование Singleton
Напишите тесты для Singleton и покажите проблемы с тестированием.

## 📈 Следующие шаги
После изучения Singleton вы будете готовы к:
- Уроку 3.2: Factory Method Pattern
- Пониманию креационных паттернов
- Изучению принципов SOLID
- Созданию тестируемого кода

## ⚠️ Важные предупреждения

1. **Singleton - это глобальная переменная в маскировке**
2. **Рассмотрите альтернативы перед использованием**
3. **Документируйте причины использования Singleton**
4. **Помните о проблемах с тестированием**
5. **Используйте современные thread-safe реализации**
