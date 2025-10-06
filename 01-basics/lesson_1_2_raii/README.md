# Урок 1.2: RAII и управление ресурсами

## 🎯 Цель урока
Изучить принцип RAII (Resource Acquisition Is Initialization) - фундаментальный принцип C++, который лежит в основе всех паттернов управления ресурсами и является ключевым для понимания паттернов проектирования.

## 📚 Что изучаем

### 1. Принцип RAII
- **Определение**: Ресурсы захватываются при инициализации, освобождаются при уничтожении
- **Применимость**: Память, файлы, сетевые соединения, мьютексы, любые ресурсы
- **Преимущества**: Автоматическое управление, exception safety, предсказуемость

### 2. Типы ресурсов
- **Память**: Smart pointers, контейнеры STL
- **Файлы**: std::fstream, custom file wrappers
- **Сетевые соединения**: RAII wrappers для sockets
- **Синхронизация**: std::lock_guard, std::unique_lock

### 3. Exception Safety
- **Базовые гарантии**: Не утечки ресурсов
- **Строгие гарантии**: Rollback при исключениях
- **Гарантии отсутствия исключений**: noexcept функции

## 🔍 Ключевые концепции

### Классический RAII паттерн
```cpp
class Resource {
private:
    ResourceType* resource_;
    
public:
    Resource() : resource_(acquireResource()) {
        // Ресурс захватывается в конструкторе
    }
    
    ~Resource() {
        releaseResource(resource_);
        // Ресурс автоматически освобождается в деструкторе
    }
    
    // Запрещаем копирование для единоличного владения
    Resource(const Resource&) = delete;
    Resource& operator=(const Resource&) = delete;
};
```

### RAII с копированием
```cpp
class CopyableResource {
private:
    std::unique_ptr<ResourceType> resource_;
    
public:
    CopyableResource() : resource_(std::make_unique<ResourceType>()) {}
    
    // Move constructor - эффективная передача владения
    CopyableResource(CopyableResource&& other) noexcept 
        : resource_(std::move(other.resource_)) {}
    
    // Move assignment
    CopyableResource& operator=(CopyableResource&& other) noexcept {
        if (this != &other) {
            resource_ = std::move(other.resource_);
        }
        return *this;
    }
};
```

### Exception Safety уровни
```cpp
// 1. Базовые гарантии - не утечки ресурсов
class BasicExceptionSafety {
    std::unique_ptr<int> data_;
public:
    void operation() {
        // Если исключение, data_ автоматически освободится
        // Но состояние объекта может быть частично изменено
    }
};

// 2. Строгие гарантии - rollback при исключениях
class StrongExceptionSafety {
    std::vector<int> data_;
public:
    void operation() {
        auto backup = data_;  // Создаем резервную копию
        try {
            // Изменяем data_
        } catch (...) {
            data_ = std::move(backup);  // Откатываем изменения
            throw;
        }
    }
};

// 3. Noexcept гарантии - функция не выбрасывает исключений
class NoexceptSafety {
public:
    void operation() noexcept {
        // Эта функция гарантированно не выбросит исключение
    }
};
```

## 🤔 Вопросы для размышления

### 1. Почему RAII так важен в C++?
- **Автоматическое управление**: Нет необходимости помнить об освобождении ресурсов
- **Exception safety**: Ресурсы освобождаются даже при исключениях
- **Предсказуемость**: Время жизни ресурсов привязано к времени жизни объектов
- **Производительность**: Нет накладных расходов на garbage collection

### 2. Когда НЕ стоит использовать RAII?
- **Системное программирование**: Когда нужен точный контроль над ресурсами
- **Interop с C**: Когда работаете с C API, не поддерживающим RAII
- **Критические секции**: Когда нужно явное управление временем жизни

### 3. Как RAII связан с паттернами проектирования?
- **Singleton**: RAII для глобальных ресурсов
- **Factory**: RAII для создания объектов с ресурсами
- **Observer**: RAII для управления подписками
- **Command**: RAII для undo/redo операций

## 🛠️ Практические примеры

### Файловый менеджер с RAII
```cpp
class FileManager {
private:
    std::unique_ptr<std::fstream> file_;
    std::string filename_;
    
public:
    explicit FileManager(const std::string& filename)
        : filename_(filename) {
        file_ = std::make_unique<std::fstream>(filename);
        if (!file_->is_open()) {
            throw std::runtime_error("Не удалось открыть файл: " + filename);
        }
    }
    
    ~FileManager() {
        // Файл автоматически закрывается при уничтожении объекта
    }
    
    void write(const std::string& data) {
        *file_ << data;
    }
    
    std::string read() {
        std::string content;
        std::string line;
        while (std::getline(*file_, line)) {
            content += line + "\n";
        }
        return content;
    }
};
```

### Сетевые соединения с RAII
```cpp
class NetworkConnection {
private:
    int socket_fd_;
    bool connected_;
    
public:
    NetworkConnection(const std::string& host, int port) 
        : socket_fd_(-1), connected_(false) {
        socket_fd_ = createSocket(host, port);
        if (socket_fd_ == -1) {
            throw std::runtime_error("Не удалось создать сокет");
        }
        connected_ = true;
    }
    
    ~NetworkConnection() {
        if (connected_) {
            close(socket_fd_);
        }
    }
    
    // Запрещаем копирование
    NetworkConnection(const NetworkConnection&) = delete;
    NetworkConnection& operator=(const NetworkConnection&) = delete;
    
    // Разрешаем перемещение
    NetworkConnection(NetworkConnection&& other) noexcept 
        : socket_fd_(other.socket_fd_), connected_(other.connected_) {
        other.connected_ = false;
        other.socket_fd_ = -1;
    }
};
```

## 🎨 Альтернативные подходы

### Вместо manual resource management
```cpp
// Плохо - ручное управление
void badExample() {
    int* data = new int[1000];
    FILE* file = fopen("data.txt", "r");
    
    try {
        // Работа с ресурсами
        processData(data);
        readFile(file);
    } catch (...) {
        delete[] data;  // Легко забыть!
        fclose(file);   // Легко забыть!
        throw;
    }
    
    delete[] data;  // Дублирование кода
    fclose(file);
}

// Хорошо - RAII
void goodExample() {
    std::unique_ptr<int[]> data = std::make_unique<int[]>(1000);
    std::ifstream file("data.txt");
    
    // Работа с ресурсами
    processData(data.get());
    readFile(file);
    
    // Автоматическое освобождение при выходе из области видимости
}
```

### Вместо try-catch блоков для каждого ресурса
```cpp
// Плохо - множественные try-catch
void badResourceManagement() {
    Resource1 r1;
    try {
        Resource2 r2;
        try {
            Resource3 r3;
            // Работа с ресурсами
        } catch (...) {
            // Обработка ошибок r3
            throw;
        }
    } catch (...) {
        // Обработка ошибок r2
        throw;
    }
    // Обработка ошибок r1
}

// Хорошо - RAII автоматически освобождает все ресурсы
void goodResourceManagement() {
    Resource1 r1;
    Resource2 r2;
    Resource3 r3;
    
    // Работа с ресурсами
    // При исключении все ресурсы автоматически освободятся
}
```

## 📈 Связь с паттернами проектирования

### 1. Singleton + RAII
```cpp
class DatabaseConnection {
private:
    static std::unique_ptr<DatabaseConnection> instance_;
    std::string connectionString_;
    
    DatabaseConnection(const std::string& connStr) 
        : connectionString_(connStr) {
        // Инициализация соединения
    }
    
public:
    static DatabaseConnection& getInstance() {
        if (!instance_) {
            instance_ = std::make_unique<DatabaseConnection>("localhost:5432");
        }
        return *instance_;
    }
    
    ~DatabaseConnection() {
        // Автоматическое закрытие соединения
    }
};
```

### 2. Factory + RAII
```cpp
class ResourceFactory {
public:
    static std::unique_ptr<FileManager> createFileManager(const std::string& filename) {
        return std::make_unique<FileManager>(filename);
    }
    
    static std::unique_ptr<NetworkConnection> createConnection(const std::string& host, int port) {
        return std::make_unique<NetworkConnection>(host, port);
    }
};
```

## 🎯 Практические упражнения

### Упражнение 1: Создание RAII wrapper для C API
Создайте RAII wrapper для библиотеки, которая предоставляет C API с функциями `open_resource()` и `close_resource()`.

### Упражнение 2: Exception Safety анализ
Проанализируйте код на предмет exception safety и определите, какие гарантии предоставляет каждый метод.

### Упражнение 3: Move semantics в RAII
Реализуйте класс с RAII, который поддерживает move semantics для эффективной передачи владения ресурсами.

## 📈 Следующие шаги
После изучения RAII вы будете готовы к:
- Уроку 1.3: Smart Pointers (углубление в управление памятью)
- Пониманию паттернов, основанных на RAII
- Созданию exception-safe кода
- Изучению паттернов управления ресурсами
