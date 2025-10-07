# Анализ безопасности паттерна Singleton

## Уязвимости в классическом Singleton

### 1. Race Condition при инициализации
```cpp
// УЯЗВИМАЯ РЕАЛИЗАЦИЯ: Классический небезопасный Singleton
class UnsafeSingleton {
private:
    static UnsafeSingleton* instance;
    
public:
    static UnsafeSingleton* getInstance() {
        // RACE CONDITION: Два потока могут одновременно 
        // проверить instance == nullptr
        if (instance == nullptr) {
            // Thread 1 и Thread 2 оба проходят сюда!
            instance = new UnsafeSingleton();
            // Результат: два объекта созданы, один потерян (memory leak)
        }
        return instance;
    }
};

// АТАКА: Запуск множества потоков одновременно
std::vector<std::thread> threads;
for (int i = 0; i < 100; ++i) {
    threads.emplace_back([]() {
        auto* singleton = UnsafeSingleton::getInstance();
        // Разные потоки могут получить РАЗНЫЕ экземпляры!
    });
}
```

**Последствия**:
- Множественные экземпляры Singleton
- Memory leaks (потерянные экземпляры)
- Нарушение инвариантов паттерна
- Undefined behavior при доступе к разным экземплярам

### 2. Memory Leak при исключениях
```cpp
// УЯЗВИМАЯ РЕАЛИЗАЦИЯ: Singleton с исключением в конструкторе
class LeakySingleton {
private:
    static LeakySingleton* instance;
    std::vector<int>* data;
    
    LeakySingleton() {
        data = new std::vector<int>(1000000);
        // Если здесь выбросится исключение:
        throw std::runtime_error("Initialization failed");
        // memory leak: data не будет удален!
        // instance остается nullptr, но память утекла
    }
    
public:
    static LeakySingleton* getInstance() {
        if (instance == nullptr) {
            instance = new LeakySingleton(); // Может выбросить исключение
        }
        return instance;
    }
};

// АТАКА: Вызвать getInstance() множество раз
for (int i = 0; i < 1000; ++i) {
    try {
        LeakySingleton::getInstance();
    } catch (...) {
        // Каждый раз утекает память!
    }
}
// Результат: 1000 * 1000000 * sizeof(int) ≈ 4 GB утечка!
```

**Последствия**:
- Утечка памяти при каждой неудачной попытке
- DoS через memory exhaustion
- Невозможность создать экземпляр

### 3. Use-After-Free при уничтожении
```cpp
// УЯЗВИМАЯ РЕАЛИЗАЦИЯ: Singleton с manual cleanup
class UseAfterFreeSingleton {
private:
    static UseAfterFreeSingleton* instance;
    
public:
    static UseAfterFreeSingleton* getInstance() {
        if (instance == nullptr) {
            instance = new UseAfterFreeSingleton();
        }
        return instance;
    }
    
    static void destroy() {
        delete instance;
        instance = nullptr;
    }
    
    void doWork() {
        // ...
    }
};

// АТАКА: Use-after-free
auto* singleton = UseAfterFreeSingleton::getInstance();
UseAfterFreeSingleton::destroy();  // Уничтожен
singleton->doWork();  // USE-AFTER-FREE! Undefined behavior

// ИЛИ: Double-free
UseAfterFreeSingleton::destroy();
UseAfterFreeSingleton::destroy();  // DOUBLE-FREE! Crash
```

**Последствия**:
- Use-after-free → arbitrary code execution
- Double-free → heap corruption
- Crash приложения

## Инструменты анализа

### Статический анализ
- **Clang Static Analyzer**: `clang --analyze singleton_pattern.cpp`
- **PVS-Studio**: Анализ на предмет race conditions
- **Cppcheck**: `cppcheck --enable=all singleton_pattern.cpp`

### Динамический анализ
- **ThreadSanitizer**: Обнаружение race conditions
- **AddressSanitizer**: Обнаружение use-after-free
- **Valgrind**: Анализ памяти и многопоточности

## Практические задания

1. **Обнаружение race condition**:
   ```bash
   # Сборка с ThreadSanitizer
   g++ -fsanitize=thread -g -std=c++17 singleton_vulnerabilities.cpp -o singleton_tsan
   ./singleton_tsan
   
   # Вывод ThreadSanitizer покажет:
   # WARNING: ThreadSanitizer: data race on UnsafeSingleton::instance
   ```

2. **Анализ с Valgrind**:
   ```bash
   g++ -g -std=c++17 singleton_vulnerabilities.cpp -o singleton_debug
   valgrind --tool=helgrind ./singleton_debug
   
   # Helgrind обнаружит race condition при доступе к instance
   ```

3. **Обнаружение memory leaks**:
   ```bash
   g++ -fsanitize=address -g -std=c++17 singleton_vulnerabilities.cpp -o singleton_asan
   ./singleton_asan
   
   # AddressSanitizer покажет утечки памяти
   ```

4. **Запуск эксплоитов**:
   ```bash
   cd exploits
   g++ -std=c++17 singleton_exploits.cpp -o singleton_exploits
   ./singleton_exploits
   ```

## Безопасные альтернативы

### 1. Thread-safe Singleton (C++11)
```cpp
// БЕЗОПАСНАЯ РЕАЛИЗАЦИЯ: Meyer's Singleton
class SafeSingleton {
private:
    SafeSingleton() = default;
    
public:
    static SafeSingleton& getInstance() {
        // Thread-safe в C++11: статическая локальная переменная
        // гарантированно инициализируется один раз
        static SafeSingleton instance;
        return instance;
    }
    
    // Запрещаем копирование и перемещение
    SafeSingleton(const SafeSingleton&) = delete;
    SafeSingleton& operator=(const SafeSingleton&) = delete;
    SafeSingleton(SafeSingleton&&) = delete;
    SafeSingleton& operator=(SafeSingleton&&) = delete;
};

// БЕЗОПАСНО: Компилятор гарантирует thread-safety
```

### 2. Singleton с std::call_once
```cpp
// БЕЗОПАСНАЯ РЕАЛИЗАЦИЯ: std::call_once
class CallOnceSingleton {
private:
    static std::unique_ptr<CallOnceSingleton> instance;
    static std::once_flag init_flag;
    
    CallOnceSingleton() = default;
    
public:
    static CallOnceSingleton& getInstance() {
        std::call_once(init_flag, []() {
            instance.reset(new CallOnceSingleton());
        });
        return *instance;
    }
    
    ~CallOnceSingleton() = default;
    
    CallOnceSingleton(const CallOnceSingleton&) = delete;
    CallOnceSingleton& operator=(const CallOnceSingleton&) = delete;
};

std::unique_ptr<CallOnceSingleton> CallOnceSingleton::instance;
std::once_flag CallOnceSingleton::init_flag;

// БЕЗОПАСНО: std::call_once гарантирует однократную инициализацию
```

### 3. Dependency Injection вместо Singleton
```cpp
// АЛЬТЕРНАТИВА: Избегаем Singleton через Dependency Injection
class Logger {
public:
    void log(const std::string& message) {
        std::cout << message << std::endl;
    }
};

class Application {
private:
    Logger& logger_;  // Инжектированная зависимость
    
public:
    explicit Application(Logger& logger) : logger_(logger) {}
    
    void run() {
        logger_.log("Application running");
    }
};

// Использование:
Logger logger;  // Один экземпляр, но НЕ глобальный
Application app(logger);
app.run();

// ПРЕИМУЩЕСТВА:
// - Нет глобального состояния
// - Легко тестировать (mock logger)
// - Явные зависимости
// - Thread-safe (нет shared state)
```

## Эксплоиты и атаки

### 1. Race Condition Exploit
```cpp
// ЭКСПЛОИТ: Использование race condition для получения доступа
// к критическим ресурсам

class PrivilegedSingleton {
private:
    static PrivilegedSingleton* instance;
    bool is_admin_ = false;
    
    PrivilegedSingleton() {
        // Симуляция проверки привилегий
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        is_admin_ = checkAdminRights();
    }
    
public:
    static PrivilegedSingleton* getInstance() {
        if (instance == nullptr) {
            instance = new PrivilegedSingleton();
        }
        return instance;
    }
    
    bool isAdmin() const { return is_admin_; }
};

// АТАКА: Эксплуатация race condition
void attackThread() {
    auto* singleton = PrivilegedSingleton::getInstance();
    
    // Атакующий может получить ДРУГОЙ экземпляр
    // с is_admin_ = true если выиграет race
    if (singleton->isAdmin()) {
        performPrivilegedOperation();  // ESCALATION!
    }
}

// Запуск 1000 потоков увеличивает вероятность success
for (int i = 0; i < 1000; ++i) {
    std::thread(attackThread).detach();
}
```

**Результат атаки**:
- Privilege escalation (получение admin прав)
- Bypass security checks
- Unauthorized access к критичным ресурсам

### 2. Memory Exhaustion (DoS Attack)
```cpp
// АТАКА: DoS через утечки памяти

class ResourceSingleton {
private:
    static ResourceSingleton* instance;
    
    ResourceSingleton() {
        // Аллокация большого объема памяти
        auto* data = new char[100 * 1024 * 1024];  // 100 MB
        
        // Если выбрасывается исключение - память утекает
        if (rand() % 2 == 0) {
            throw std::runtime_error("Random failure");
        }
        
        delete[] data;
    }
    
public:
    static ResourceSingleton* getInstance() {
        if (instance == nullptr) {
            instance = new ResourceSingleton();
        }
        return instance;
    }
};

// ЭКСПЛОИТ: Вызов getInstance() до exhaustion памяти
while (true) {
    try {
        ResourceSingleton::getInstance();
    } catch (...) {
        // Каждый failed вызов = 100 MB утечка
        // 10 вызовов = 1 GB
        // 100 вызовов = 10 GB → СИСТЕМА ПАДАЕТ
    }
}
```

**Результат атаки**:
- Denial of Service
- System crash из-за OOM
- Недоступность приложения

### 3. Timing Attack (Side Channel)
```cpp
// АТАКА: Timing attack для определения состояния Singleton

auto start = std::chrono::high_resolution_clock::now();

auto* singleton = Singleton::getInstance();

auto end = std::chrono::high_resolution_clock::now();
auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
    end - start);

if (duration.count() > 100) {
    // Singleton был создан СЕЙЧАС (медленно)
    // Можно инициировать race condition
    std::cout << "First call detected - attempting race!" << std::endl;
} else {
    // Singleton уже существовал (быстро)
    std::cout << "Already initialized" << std::endl;
}
```

**Результат атаки**:
- Information disclosure (состояние системы)
- Подготовка к race condition attack
- Fingerprinting приложения

## Защитные меры

### 1. Использование Meyer's Singleton (C++11+)
```cpp
static Singleton& getInstance() {
    static Singleton instance;  // Thread-safe в C++11
    return instance;
}
```

### 2. std::call_once для явного контроля
```cpp
static std::once_flag flag;
std::call_once(flag, []() {
    instance.reset(new Singleton());
});
```

### 3. Правильная обработка исключений
```cpp
SafeSingleton() try {
    // Инициализация
} catch (...) {
    // Cleanup перед re-throw
    throw;
}
```

### 4. Избегание глобального состояния
- Используйте Dependency Injection
- Service Locator Pattern
- Explicit passing объектов

### 5. Статический анализ в CI/CD
```yaml
# .github/workflows/security.yml
- name: ThreadSanitizer
  run: |
    g++ -fsanitize=thread -g src/*.cpp
    ./a.out

- name: AddressSanitizer
  run: |
    g++ -fsanitize=address -g src/*.cpp
    ./a.out
```

### 6. Code review checklist
- [ ] Singleton is thread-safe
- [ ] No manual memory management
- [ ] Exception-safe конструктор
- [ ] Deleted copy/move constructors
- [ ] No global mutable state

## Связь с другими паттернами

### Factory Pattern
- **Проблема**: Factory создающая Singleton может унаследовать race conditions
- **Решение**: Thread-safe Factory + Thread-safe Singleton

### Observer Pattern
- **Проблема**: Singleton как Subject может вызывать race conditions у Observers
- **Решение**: Thread-safe уведомления, immutable events

### Proxy Pattern
- **Проблема**: Proxy для Singleton может скрывать уязвимости
- **Решение**: Proxy должен валидировать доступ, не скрывать проблемы

### Thread Pool Pattern
- **Проблема**: Singleton Thread Pool - частая уязвимость
- **Решение**: Meyer's Singleton или DI для Thread Pool

## Чек-лист безопасности Singleton

- [ ] ✅ Thread-safe инициализация (static local или std::call_once)
- [ ] ✅ Нет raw pointers (используйте std::unique_ptr)
- [ ] ✅ Deleted copy/move constructors
- [ ] ✅ Exception-safe конструктор
- [ ] ✅ Нет manual delete (RAII)
- [ ] ✅ Тестирование с ThreadSanitizer
- [ ] ✅ Тестирование с AddressSanitizer
- [ ] ✅ Code review на race conditions
- [ ] ✅ Рассмотреть альтернативу (DI)

## Дополнительные ресурсы

### Статьи и документация
- [CWE-362: Concurrent Execution using Shared Resource with Improper Synchronization](https://cwe.mitre.org/data/definitions/362.html)
- [CWE-415: Double Free](https://cwe.mitre.org/data/definitions/415.html)
- [CWE-416: Use After Free](https://cwe.mitre.org/data/definitions/416.html)

### Инструменты
- [ThreadSanitizer v2](https://github.com/google/sanitizers)
- [AddressSanitizer](https://github.com/google/sanitizers)
- [Valgrind Helgrind](https://valgrind.org/docs/manual/hg-manual.html)

### Best Practices
- Scott Meyers "Effective Modern C++" - Item 4 (Meyer's Singleton)
- Herb Sutter "GotW #66" (Singleton initialization)
- C++ Core Guidelines: I.3 (Avoid singletons)

