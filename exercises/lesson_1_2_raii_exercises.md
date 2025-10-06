# 🏋️ Упражнения: RAII (Resource Acquisition Is Initialization)

## 📋 Задание 1: Создание RAII-обертки для файлов

### Описание
Создайте RAII-обертку для работы с файлами, которая автоматически закрывает файл при выходе из области видимости.

### Требования
1. **FileWrapper класс**: Автоматически открывает файл в конструкторе и закрывает в деструкторе
2. **Исключения**: Корректно обрабатывает ошибки открытия файла
3. **Move семантика**: Поддерживает перемещение ресурса
4. **Копирование**: Запрещено (файл может быть открыт только одним объектом)

### Интерфейс
```cpp
class FileWrapper {
public:
    explicit FileWrapper(const std::string& filename, const std::string& mode = "r");
    ~FileWrapper();
    
    // Запрещаем копирование
    FileWrapper(const FileWrapper&) = delete;
    FileWrapper& operator=(const FileWrapper&) = delete;
    
    // Разрешаем перемещение
    FileWrapper(FileWrapper&& other) noexcept;
    FileWrapper& operator=(FileWrapper&& other) noexcept;
    
    // Методы для работы с файлом
    bool isOpen() const;
    std::string readLine();
    void writeLine(const std::string& line);
    void close();
    
private:
    FILE* file_;
    std::string filename_;
};
```

### Тестовый случай
```cpp
int main() {
    try {
        // Создаем файл для записи
        {
            FileWrapper writer("test.txt", "w");
            writer.writeLine("Первая строка");
            writer.writeLine("Вторая строка");
            // Файл автоматически закроется здесь
        }
        
        // Читаем файл
        {
            FileWrapper reader("test.txt", "r");
            while (reader.isOpen()) {
                std::string line = reader.readLine();
                if (line.empty()) break;
                std::cout << "Прочитано: " << line << std::endl;
            }
            // Файл автоматически закроется здесь
        }
        
        // Демонстрация move семантики
        FileWrapper file1("test.txt", "r");
        FileWrapper file2 = std::move(file1); // file1 теперь пустой
        
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
    }
    
    return 0;
}
```

---

## 📋 Задание 2: Менеджер сетевых соединений

### Описание
Создайте RAII-менеджер для управления сетевыми соединениями (симуляция).

### Требования
1. **Connection класс**: Представляет сетевое соединение
2. **ConnectionManager класс**: Управляет пулом соединений
3. **Автоматическое управление**: Соединения автоматически закрываются
4. **Исключения**: Корректная обработка ошибок соединения

### Интерфейс
```cpp
class Connection {
public:
    explicit Connection(const std::string& host, int port);
    ~Connection();
    
    bool isConnected() const;
    void send(const std::string& data);
    std::string receive();
    void disconnect();
    
    // Запрещаем копирование
    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;
    
    // Разрешаем перемещение
    Connection(Connection&& other) noexcept;
    Connection& operator=(Connection&& other) noexcept;
    
private:
    std::string host_;
    int port_;
    bool connected_;
    int socket_fd_; // Симуляция
};

class ConnectionManager {
public:
    ConnectionManager(size_t maxConnections = 10);
    ~ConnectionManager();
    
    std::unique_ptr<Connection> acquireConnection(const std::string& host, int port);
    void releaseConnection(std::unique_ptr<Connection> conn);
    
    size_t getActiveConnections() const;
    size_t getMaxConnections() const;
    
private:
    std::vector<std::unique_ptr<Connection>> connections_;
    size_t maxConnections_;
};
```

### Тестовый случай
```cpp
int main() {
    ConnectionManager manager(5);
    
    try {
        // Получаем соединения
        auto conn1 = manager.acquireConnection("localhost", 8080);
        auto conn2 = manager.acquireConnection("example.com", 443);
        
        std::cout << "Активных соединений: " << manager.getActiveConnections() << std::endl;
        
        // Используем соединения
        if (conn1 && conn1->isConnected()) {
            conn1->send("Hello Server!");
            std::string response = conn1->receive();
            std::cout << "Ответ: " << response << std::endl;
        }
        
        // Соединения автоматически освободятся при выходе из области видимости
        
    } catch (const std::exception& e) {
        std::cerr << "Ошибка соединения: " << e.what() << std::endl;
    }
    
    std::cout << "Активных соединений после освобождения: " << manager.getActiveConnections() << std::endl;
    
    return 0;
}
```

---

## 📋 Задание 3: RAII для управления памятью

### Описание
Создайте RAII-обертку для управления блоками памяти с автоматическим освобождением.

### Требования
1. **MemoryBlock класс**: Управляет блоком памяти фиксированного размера
2. **MemoryPool класс**: Управляет пулом блоков памяти
3. **Автоматическое освобождение**: Память освобождается автоматически
4. **Защита от утечек**: Невозможно забыть освободить память

### Интерфейс
```cpp
template<size_t BlockSize>
class MemoryBlock {
public:
    MemoryBlock();
    ~MemoryBlock();
    
    // Запрещаем копирование
    MemoryBlock(const MemoryBlock&) = delete;
    MemoryBlock& operator=(const MemoryBlock&) = delete;
    
    // Разрешаем перемещение
    MemoryBlock(MemoryBlock&& other) noexcept;
    MemoryBlock& operator=(MemoryBlock&& other) noexcept;
    
    void* get() const;
    size_t size() const;
    bool isValid() const;
    
private:
    void* data_;
};

class MemoryPool {
public:
    explicit MemoryPool(size_t blockSize, size_t initialBlocks = 10);
    ~MemoryPool();
    
    template<size_t BlockSize>
    std::unique_ptr<MemoryBlock<BlockSize>> allocate();
    
    void deallocate(void* ptr);
    
    size_t getTotalBlocks() const;
    size_t getAllocatedBlocks() const;
    size_t getFreeBlocks() const;
    
private:
    size_t blockSize_;
    std::vector<void*> freeBlocks_;
    std::vector<void*> allocatedBlocks_;
    std::mutex mutex_;
};
```

### Тестовый случай
```cpp
int main() {
    MemoryPool pool(1024, 5); // Блоки по 1KB, начально 5 блоков
    
    std::cout << "Всего блоков: " << pool.getTotalBlocks() << std::endl;
    std::cout << "Свободных блоков: " << pool.getFreeBlocks() << std::endl;
    
    {
        // Выделяем блоки памяти
        auto block1 = pool.allocate<1024>();
        auto block2 = pool.allocate<1024>();
        
        std::cout << "Выделено блоков: " << pool.getAllocatedBlocks() << std::endl;
        std::cout << "Свободных блоков: " << pool.getFreeBlocks() << std::endl;
        
        // Используем память
        if (block1 && block1->isValid()) {
            int* numbers = static_cast<int*>(block1->get());
            for (int i = 0; i < 10; ++i) {
                numbers[i] = i * i;
            }
            
            std::cout << "Квадраты чисел: ";
            for (int i = 0; i < 10; ++i) {
                std::cout << numbers[i] << " ";
            }
            std::cout << std::endl;
        }
        
        // Блоки автоматически освободятся здесь
    }
    
    std::cout << "После освобождения:" << std::endl;
    std::cout << "Выделено блоков: " << pool.getAllocatedBlocks() << std::endl;
    std::cout << "Свободных блоков: " << pool.getFreeBlocks() << std::endl;
    
    return 0;
}
```

---

## 📋 Задание 4: RAII для работы с базой данных

### Описание
Создайте RAII-обертку для управления транзакциями базы данных.

### Требования
1. **Transaction класс**: Автоматически начинает и завершает транзакцию
2. **DatabaseConnection класс**: Управляет соединением с БД
3. **Автоматический rollback**: При исключении транзакция откатывается
4. **Commit/rollback**: Возможность явного управления транзакцией

### Интерфейс
```cpp
class DatabaseConnection {
public:
    explicit DatabaseConnection(const std::string& connectionString);
    ~DatabaseConnection();
    
    bool isConnected() const;
    void execute(const std::string& query);
    std::vector<std::map<std::string, std::string>> query(const std::string& sql);
    
private:
    std::string connectionString_;
    bool connected_;
    // Здесь был бы реальный указатель на соединение с БД
};

class Transaction {
public:
    explicit Transaction(DatabaseConnection& db);
    ~Transaction();
    
    void commit();
    void rollback();
    bool isActive() const;
    
    // Запрещаем копирование
    Transaction(const Transaction&) = delete;
    Transaction& operator=(const Transaction&) = delete;
    
    // Разрешаем перемещение
    Transaction(Transaction&& other) noexcept;
    Transaction& operator=(Transaction&& other) noexcept;
    
private:
    DatabaseConnection* db_;
    bool active_;
    bool committed_;
};
```

### Тестовый случай
```cpp
int main() {
    DatabaseConnection db("postgresql://localhost/mydb");
    
    try {
        // Успешная транзакция
        {
            Transaction txn(db);
            
            db.execute("INSERT INTO users (name, email) VALUES ('Иван', 'ivan@example.com')");
            db.execute("INSERT INTO users (name, email) VALUES ('Петр', 'petr@example.com')");
            
            txn.commit(); // Явный commit
            std::cout << "Транзакция успешно завершена" << std::endl;
        }
        
        // Транзакция с автоматическим rollback при исключении
        {
            Transaction txn(db);
            
            db.execute("INSERT INTO users (name, email) VALUES ('Мария', 'maria@example.com')");
            
            // Симулируем ошибку
            throw std::runtime_error("Ошибка в бизнес-логике");
            
            // txn.commit() не будет вызван, произойдет автоматический rollback
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
    }
    
    // Проверяем результат
    auto users = db.query("SELECT * FROM users");
    std::cout << "Количество пользователей: " << users.size() << std::endl;
    
    return 0;
}
```

---

## 📋 Задание 5: Комплексное RAII приложение

### Описание
Создайте приложение для обработки файлов, демонстрирующее все принципы RAII.

### Требования
1. **FileProcessor класс**: Обрабатывает файлы с автоматическим управлением ресурсами
2. **LogManager класс**: Управляет лог-файлами
3. **ConfigManager класс**: Управляет конфигурационными файлами
4. **Исключения**: Корректная обработка всех ошибок

### Архитектура
```cpp
class LogManager {
public:
    explicit LogManager(const std::string& logFile);
    ~LogManager();
    
    void log(const std::string& message);
    void logError(const std::string& error);
    
private:
    std::unique_ptr<FileWrapper> logFile_;
};

class ConfigManager {
public:
    explicit ConfigManager(const std::string& configFile);
    ~ConfigManager();
    
    std::string getValue(const std::string& key) const;
    void setValue(const std::string& key, const std::string& value);
    
private:
    std::unique_ptr<FileWrapper> configFile_;
    std::map<std::string, std::string> config_;
};

class FileProcessor {
public:
    FileProcessor(const std::string& inputFile, const std::string& outputFile);
    ~FileProcessor();
    
    void process();
    
private:
    std::unique_ptr<FileWrapper> inputFile_;
    std::unique_ptr<FileWrapper> outputFile_;
    std::unique_ptr<LogManager> logger_;
    std::unique_ptr<ConfigManager> config_;
};
```

### Тестовый случай
```cpp
int main() {
    try {
        // Создаем тестовые файлы
        {
            FileWrapper testInput("input.txt", "w");
            testInput.writeLine("Строка 1");
            testInput.writeLine("Строка 2");
            testInput.writeLine("Строка 3");
        }
        
        {
            FileWrapper testConfig("config.txt", "w");
            testConfig.writeLine("max_lines=100");
            testConfig.writeLine("encoding=utf8");
        }
        
        // Обрабатываем файл
        FileProcessor processor("input.txt", "output.txt");
        processor.process();
        
        std::cout << "Обработка завершена успешно" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Критическая ошибка: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

---

## 🎯 Критерии оценки

### Отлично (5 баллов)
- ✅ Все RAII принципы соблюдены
- ✅ Автоматическое управление ресурсами работает корректно
- ✅ Исключения обрабатываются правильно
- ✅ Move семантика реализована
- ✅ Копирование запрещено где необходимо
- ✅ Нет утечек ресурсов

### Хорошо (4 балла)
- ✅ Основные RAII принципы соблюдены
- ✅ Ресурсы освобождаются автоматически
- ✅ Есть небольшие недочеты в обработке исключений
- ✅ Код работает стабильно

### Удовлетворительно (3 балла)
- ✅ Базовые RAII принципы соблюдены
- ✅ Код компилируется и работает
- ❌ Есть проблемы с управлением ресурсами
- ❌ Не все исключения обрабатываются

### Неудовлетворительно (2 балла)
- ❌ RAII принципы не соблюдены
- ❌ Есть утечки ресурсов
- ❌ Код не компилируется или работает неправильно

---

## 💡 Подсказки

1. **Помните правило**: Ресурс должен быть получен в конструкторе и освобожден в деструкторе
2. **Исключения**: Используйте RAII для автоматической очистки при исключениях
3. **Move семантика**: Реализуйте для эффективного перемещения ресурсов
4. **Запрет копирования**: Используйте `= delete` для предотвращения копирования
5. **Тестирование**: Проверяйте, что ресурсы действительно освобождаются

---

*RAII - это основа безопасного управления ресурсами в C++. Освойте эти принципы, и ваши программы станут намного надежнее!*
