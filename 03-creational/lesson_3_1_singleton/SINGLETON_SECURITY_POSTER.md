# 🛡️ Security Poster: Singleton Pattern

## 🎯 Обзор угроз Singleton Pattern

```
┌────────────────────────────────────────────────────────────────────┐
│              КРИТИЧЕСКИЕ УЯЗВИМОСТИ SINGLETON PATTERN              │
└────────────────────────────────────────────────────────────────────┘

🔴 КРИТИЧНЫЕ УГРОЗЫ:
┌─────────────────────┬──────────────────────────────────────────────┐
│ Race Condition      │ CWE-362: Double-checked locking bug          │
│ (Initialization)    │ Impact: Multiple instances, memory leaks, UB │
├─────────────────────┼──────────────────────────────────────────────┤
│ Use-After-Free      │ CWE-416: Access after destruction            │
│ (Static Deinit)     │ Impact: UAF, heap corruption, RCE            │
├─────────────────────┼──────────────────────────────────────────────┤
│ Double-Free         │ CWE-415: Manual destroy() called twice       │
│                     │ Impact: Heap corruption, crash, RCE          │
└─────────────────────┴──────────────────────────────────────────────┘

🟡 ВЫСОКИЕ УГРОЗЫ:
┌─────────────────────┬──────────────────────────────────────────────┐
│ Memory Leaks        │ CWE-401: Exception during initialization    │
│ TOCTOU Issues       │ Check-then-use race на getInstance()         │
│ Static Init Order   │ Fiasco при зависимых singletons              │
└─────────────────────┴──────────────────────────────────────────────┘
```

## 💣 Критическая уязвимость #1: Race Condition в Initialization

### Анатомия атаки

```cpp
// ❌ УЯЗВИМЫЙ КОД - Classic non-thread-safe Singleton
class UnsafeSingleton {
private:
    static UnsafeSingleton* instance_;  // ⚠️ Shared mutable state
    
    UnsafeSingleton() { /* ... */ }
    
public:
    static UnsafeSingleton* getInstance() {
        // ⚠️ RACE CONDITION: Non-atomic check-then-act
        if (instance_ == nullptr) {  // [1] Check
            // ⚠️ RACE WINDOW: Thread 2 может войти здесь
            instance_ = new UnsafeSingleton();  // [2] Act
        }
        return instance_;
    }
};

UnsafeSingleton* UnsafeSingleton::instance_ = nullptr;
```

### 🎯 Exploitation: Multiple instances через race

```cpp
// ЭКСПЛОЙТ: Create multiple instances
void exploit_race_condition() {
    std::atomic<int> instance_count{0};
    std::set<void*> unique_instances;
    std::mutex set_mutex;
    
    std::vector<std::thread> threads;
    for (int i = 0; i < 100; ++i) {
        threads.emplace_back([&]() {
            auto* instance = UnsafeSingleton::getInstance();
            
            std::lock_guard lock(set_mutex);
            unique_instances.insert(instance);
        });
    }
    
    for (auto& t : threads) t.join();
    
    std::cout << "Created " << unique_instances.size() << " instances!\n";
    // Expected: 1
    // Actual: 2-5 (multiple instances created!)
    
    // Последствия:
    // 1. Нарушение Singleton invariant
    // 2. Memory leak (потерянные instances)
    // 3. State inconsistency (разные instances = разное state)
}
```

### Race condition timeline

```
┌──────────────────────────────────────────────────────────────────┐
│                SINGLETON RACE CONDITION TIMELINE                 │
└──────────────────────────────────────────────────────────────────┘

Time    Thread 1              Thread 2              Instance State
────────────────────────────────────────────────────────────────────
T0      getInstance() →       -                     nullptr
T1      if (instance_==nullptr) ✓                   nullptr
T2      -                     getInstance() →       nullptr
T3      -                     if (instance_==nullptr) ✓  nullptr
T4      new Singleton() →     -                     -
T5      -                     new Singleton() →     -
T6      instance_ = 0x100     -                     0x100
T7      -                     instance_ = 0x200     0x200 (overwritten!)
T8      return 0x100          return 0x200          💥 Two instances!

Memory state после race:
0x100: [Singleton instance #1]  ← Thread 1 видит
0x200: [Singleton instance #2]  ← Thread 2 видит, instance_ указывает сюда
       💥 0x100 leaked (никто не владеет)
       💥 0x200 используется, но может быть затерт следующим race
```

### Double-Checked Locking (DCL) Bug

```cpp
// ❌ НЕПРАВИЛЬНАЯ ЗАЩИТА - Broken DCL
class BrokenDCLSingleton {
private:
    static BrokenDCLSingleton* instance_;
    static std::mutex mutex_;
    
public:
    static BrokenDCLSingleton* getInstance() {
        if (instance_ == nullptr) {  // [1] First check (UNPROTECTED)
            std::lock_guard lock(mutex_);
            if (instance_ == nullptr) {  // [2] Second check (PROTECTED)
                instance_ = new BrokenDCLSingleton();  // [3] Create
                // ⚠️ ПРОБЛЕМА: Memory reordering!
                // Возможно:
                //   1. Allocate memory for instance_
                //   2. instance_ = allocated_ptr  ← Видимо другим threads
                //   3. Call constructor            ← Еще не выполнено!
                // → Thread 2 видит instance_ != nullptr
                // → Thread 2 использует partially constructed object!
            }
        }
        return instance_;  // ⚠️ Может вернуть partially constructed!
    }
};
```

### ✅ Защита: Meyers' Singleton (C++11)

```cpp
// ✅ БЕЗОПАСНЫЙ КОД - Magic static (C++11 thread-safe)
class ThreadSafeSingleton {
private:
    ThreadSafeSingleton() { /* ... */ }
    
public:
    // Запрет копирования и перемещения
    ThreadSafeSingleton(const ThreadSafeSingleton&) = delete;
    ThreadSafeSingleton& operator=(const ThreadSafeSingleton&) = delete;
    ThreadSafeSingleton(ThreadSafeSingleton&&) = delete;
    ThreadSafeSingleton& operator=(ThreadSafeSingleton&&) = delete;
    
    static ThreadSafeSingleton& getInstance() {
        // ✅ C++11 гарантирует thread-safe initialization
        static ThreadSafeSingleton instance;
        return instance;
    }
};

// Как работает magic static:
// 1. Компилятор генерирует guard variable
// 2. First thread атомарно устанавливает guard
// 3. Other threads блокируются до завершения init
// 4. После init, guard проверка очень быстрая (no lock)
```

## 💣 Критическая уязвимость #2: Use-After-Free при Destruction

### Анатомия атаки

```cpp
// ❌ УЯЗВИМЫЙ КОД - Manual destroy() method
class ManualDestroySingleton {
private:
    static ManualDestroySingleton* instance_;
    
public:
    static ManualDestroySingleton* getInstance() {
        if (instance_ == nullptr) {
            instance_ = new ManualDestroySingleton();
        }
        return instance_;
    }
    
    // ⚠️ ОПАСНО: Manual destruction
    static void destroy() {
        delete instance_;
        instance_ = nullptr;
    }
    
    void doWork() { /* ... */ }
};
```

### 🎯 Exploitation: Use-After-Free

```cpp
// ЭКСПЛОЙТ: UAF attack
void exploit_use_after_free() {
    // 1. Получаем instance
    auto* singleton = ManualDestroySingleton::getInstance();
    
    // 2. Кто-то вызывает destroy() (другой поток, cleanup code, etc.)
    ManualDestroySingleton::destroy();
    // Memory at singleton freed
    
    // 3. Heap spray: заполняем freed memory
    for (int i = 0; i < 1000; ++i) {
        void* spray = malloc(sizeof(ManualDestroySingleton));
        memset(spray, 0x41, sizeof(ManualDestroySingleton));  // Fill with 'A'
    }
    
    // 4. Use-after-free
    singleton->doWork();
    // 💥 Reads from freed (and possibly reused) memory
    // vtable pointer может указывать на attacker-controlled data
    // → RCE potential
}

// Альтернативный сценарий: Double-free
void exploit_double_free() {
    ManualDestroySingleton::destroy();
    ManualDestroySingleton::destroy();  // 💥 Double-free!
    
    // Последствия:
    // - Heap metadata corruption
    // - Crash
    // - Potential для heap exploitation (fastbin dup, tcache dup)
}
```

### Static Initialization Order Fiasco

```cpp
// ❌ УЯЗВИМЫЙ КОД - Зависимые singletons
class Logger {
public:
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }
    
    void log(const std::string& msg) { /* ... */ }
};

class Database {
public:
    static Database& getInstance() {
        static Database instance;
        return instance;
    }
    
    Database() {
        // ⚠️ ПРОБЛЕМА: Использует Logger в constructor
        Logger::getInstance().log("Database initializing");
        // Если Database инициализируется ПЕРЕД Logger:
        // → Logger еще не инициализирован!
        // → Undefined behavior
    }
};

// Global variables могут инициализироваться в любом порядке
Database& db = Database::getInstance();  // Может быть первым
Logger& logger = Logger::getInstance();  // Может быть вторым
// ⚠️ Если db инициализируется первым → UB!
```

### ✅ Защита: RAII + Nifty Counter Idiom

```cpp
// ✅ БЕЗОПАСНЫЙ КОД - Гарантированный порядок инициализации
class SafeSingleton {
private:
    SafeSingleton() { /* ... */ }
    ~SafeSingleton() { /* ... */ }
    
public:
    // Запрет копирования и перемещения
    SafeSingleton(const SafeSingleton&) = delete;
    SafeSingleton& operator=(const SafeSingleton&) = delete;
    
    static SafeSingleton& getInstance() {
        static SafeSingleton instance;  // Magic static
        return instance;
        // ✅ Автоматическая очистка при exit
        // ✅ Нет UAF (нет manual destroy)
        // ✅ Нет double-free
    }
    
    void doWork() { /* ... */ }
};

// Для зависимых singletons: Dependency Injection
class DependentSingleton {
private:
    Logger& logger_;  // Reference к dependency
    
    DependentSingleton(Logger& logger) : logger_(logger) {}
    
public:
    static DependentSingleton& getInstance() {
        // ✅ Явно запрашиваем dependency ВНУТРИ функции
        // Гарантирует что Logger инициализирован
        static DependentSingleton instance(Logger::getInstance());
        return instance;
    }
};
```

## 💣 Критическая уязвимость #3: Memory Leak при Exceptions

### Анатомия атаки

```cpp
// ❌ УЯЗВИМЫЙ КОД - Exception в constructor
class LeakySingleton {
private:
    static LeakySingleton* instance_;
    std::vector<int>* large_data_;
    
    LeakySingleton() {
        large_data_ = new std::vector<int>(1000000);  // Allocate 4MB
        
        // ⚠️ Exception может быть выброшено здесь
        if (rand() % 2) {
            throw std::runtime_error("Random failure");
        }
        // Если exception → large_data_ leaked!
    }
    
public:
    static LeakySingleton* getInstance() {
        if (instance_ == nullptr) {
            instance_ = new LeakySingleton();  // ⚠️ Может выбросить
            // Если throws → instance_ остается nullptr, но memory leaked
        }
        return instance_;
    }
};
```

### 🎯 Exploitation: Memory exhaustion DoS

```cpp
// ЭКСПЛОЙТ: Trigger memory leak repeatedly
void exploit_memory_leak() {
    size_t leaked_mb = 0;
    
    for (int i = 0; i < 10000; ++i) {
        try {
            LeakySingleton::getInstance();
        } catch (const std::exception&) {
            // Each failure leaks ~4MB
            leaked_mb += 4;
            
            if (leaked_mb % 100 == 0) {
                std::cout << "Leaked " << leaked_mb << " MB so far\n";
            }
        }
    }
    
    // После 2048 exceptions:
    // → ~8GB leaked
    // → OOM killer terminates process
    // → Denial of Service
}
```

### ✅ Защита: Exception-safe initialization

```cpp
// ✅ БЕЗОПАСНЫЙ КОД - RAII для exception safety
class ExceptionSafeSingleton {
private:
    std::unique_ptr<std::vector<int>> large_data_;  // ✅ Smart pointer
    
    ExceptionSafeSingleton() 
        : large_data_(std::make_unique<std::vector<int>>(1000000)) {
        // ✅ Если exception выброшено здесь:
        // → unique_ptr автоматически освободит память
        // → Нет leak
        
        if (rand() % 2) {
            throw std::runtime_error("Random failure");
        }
    }
    
public:
    static ExceptionSafeSingleton& getInstance() {
        static ExceptionSafeSingleton instance;  // ✅ Magic static
        // ✅ Если constructor throws:
        // → Exception propagates
        // → Next call попытается инициализировать снова
        // → Partial objects автоматически cleaned up
        return instance;
    }
};
```

## 🛠️ Инструменты детектирования

### 1. ThreadSanitizer - Race Detection

```bash
# Компиляция с TSan
g++ -fsanitize=thread -O1 -g singleton_vulnerabilities.cpp -o singleton_tsan

./singleton_tsan
```

**Типичный вывод TSan**:
```
WARNING: ThreadSanitizer: data race (pid=12345)
  Write of size 8 at 0x7b04000000 by thread T2:
    #0 UnsafeSingleton::getInstance() singleton.cpp:15
    
  Previous read of size 8 at 0x7b04000000 by thread T1:
    #0 UnsafeSingleton::getInstance() singleton.cpp:13
    
  Location: static UnsafeSingleton::instance_
  
SUMMARY: ThreadSanitizer: data race singleton.cpp:15
```

### 2. AddressSanitizer - UAF Detection

```bash
g++ -fsanitize=address -g singleton_vulnerabilities.cpp -o singleton_asan

./singleton_asan
```

**Типичный вывод ASan**:
```
=================================================================
==12345==ERROR: AddressSanitizer: heap-use-after-free on address 0x602000000010
READ of size 8 at 0x602000000010 thread T0
    #0 0x400b12 in ManualDestroySingleton::doWork() singleton.cpp:67
    #1 0x400c34 in exploit_use_after_free() singleton.cpp:89
    
0x602000000010 is located 0 bytes inside of 32-byte region
freed by thread T0 here:
    #0 0x7f8c operator delete
    #1 0x400a89 in ManualDestroySingleton::destroy() singleton.cpp:59
```

### 3. Valgrind Memcheck - Memory Leaks

```bash
valgrind --leak-check=full \
         --show-leak-kinds=all \
         ./singleton_vulnerabilities

# Вывод:
# LEAK SUMMARY:
#   definitely lost: 4,194,304 bytes in 1 block
#   
# 4,194,304 bytes in 1 block are definitely lost:
#    at 0x4C2FB0F: operator new
#    by 0x400B23: LeakySingleton::LeakySingleton()
```

### 4. Helgrind - Race Conditions

```bash
valgrind --tool=helgrind ./singleton_vulnerabilities

# Вывод:
# Possible data race during write of size 8 at 0x4A2B000 by thread #2
#    at 0x400B45: UnsafeSingleton::getInstance
#
# This conflicts with a previous read of size 8 by thread #1
#    at 0x400A98: UnsafeSingleton::getInstance
```

## 📋 Security Checklist для Singleton Pattern

### Design Phase
```
[ ] Решено использовать Singleton (not anti-pattern для данного случая)
[ ] Определена thread-safety requirement
[ ] Рассмотрены alternatives (Dependency Injection)
[ ] Lifetime management продуман
```

### Implementation Phase
```
[ ] Используется Meyers' Singleton (magic static)
[ ] Нет manual destroy() method
[ ] Конструктор exception-safe (RAII для ресурсов)
[ ] Копирование и перемещение запрещены (=delete)
[ ] Нет static initialization order dependencies
[ ] Нет сырых указателей (use smart pointers)
```

### Testing Phase
```
[ ] Unit tests для getInstance()
[ ] Race condition tests с ThreadSanitizer
[ ] UAF tests с AddressSanitizer
[ ] Memory leak tests с Valgrind
[ ] Stress testing (1000+ threads concurrent access)
[ ] Exception safety tests
```

### Deployment Phase
```
[ ] Monitoring для singleton initialization time
[ ] Logging для debug (opt-in для prod)
[ ] No hot-path в getInstance() (already initialized)
```

## 🎓 Best Practices

### 1. Modern Meyers' Singleton (Recommended)

```cpp
// ✅ ЛУЧШАЯ ПРАКТИКА - C++11 Magic Static
class Singleton {
public:
    // Delete copy/move
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    Singleton(Singleton&&) = delete;
    Singleton& operator=(Singleton&&) = delete;
    
    static Singleton& getInstance() {
        static Singleton instance;  // Thread-safe C++11
        return instance;
    }
    
private:
    Singleton() = default;
    ~Singleton() = default;
};
```

### 2. Dependency Injection (Better Alternative)

```cpp
// ✅ ЛУЧШЕ: Избегайте Singleton через DI
class Service {
    Database& db_;  // Injected dependency
    
public:
    Service(Database& db) : db_(db) {}
    
    void doWork() {
        db_.query("SELECT ...");
    }
};

// В main():
Database db;  // Single instance, но НЕ global
Service service(db);  // Explicit dependency
```

### 3. Double-Checked Locking (Если необходимо)

```cpp
// ✅ Корректный DCL с std::atomic (C++11)
class DCLSingleton {
private:
    static std::atomic<DCLSingleton*> instance_;
    static std::mutex mutex_;
    
public:
    static DCLSingleton* getInstance() {
        DCLSingleton* tmp = instance_.load(std::memory_order_acquire);
        
        if (tmp == nullptr) {
            std::lock_guard<std::mutex> lock(mutex_);
            tmp = instance_.load(std::memory_order_relaxed);
            
            if (tmp == nullptr) {
                tmp = new DCLSingleton();
                instance_.store(tmp, std::memory_order_release);
            }
        }
        
        return tmp;
    }
};

// NOTE: Meyers' Singleton проще и быстрее. Используйте его вместо DCL.
```

## 🚨 Real-World Impact

```
┌────────────────────────────────────────────────────────────────┐
│              ИЗВЕСТНЫЕ ПРОБЛЕМЫ С SINGLETON PATTERN            │
└────────────────────────────────────────────────────────────────┘

Android SQLite Database Singleton Bug (2015)
├─ Тип: Race condition в singleton initialization
├─ Impact: Multiple database connections → data corruption
└─ Причина: Non-thread-safe getInstance()

Java Log4j Singleton Issue (Pre-2.x)
├─ Тип: Static initialization order fiasco
├─ Impact: Logger not initialized → NullPointerException
└─ Причина: Dependent singletons initialized in wrong order

C++ Static Deinit Order Bug (Common)
├─ Тип: Use-after-free при static destruction
├─ Impact: Crashes при shutdown, UAF
└─ Причина: Singleton A использует Singleton B после его destruction
```

## 📚 Дополнительные ресурсы

### Стандарты
- **CWE-362**: Concurrent Execution using Shared Resource
- **CWE-416**: Use After Free
- **CWE-415**: Double Free
- **CWE-401**: Missing Release of Memory
- **CERT C++**: CON50-CPP, OOP52-CPP

### Книги и статьи
- "Effective C++" - Scott Meyers (Item 4: Singleton)
- "Modern C++ Design" - Andrei Alexandrescu (Singleton chapter)
- "C++ Concurrency in Action" - Anthony Williams (Static initialization)

### Инструменты
- ThreadSanitizer: https://github.com/google/sanitizers
- AddressSanitizer: https://github.com/google/sanitizers
- Valgrind: https://valgrind.org/

## 💡 Золотое правило Singleton Security

```
┌────────────────────────────────────────────────────────────────┐
│                                                                │
│  "Singleton - это global mutable state.                       │
│   Используйте Meyers' Singleton для thread-safety.            │
│   Еще лучше - избегайте Singleton через Dependency Injection."│
│                                                                │
│  Никогда не используйте manual destroy().                     │
│  Никогда не реализуйте DCL вручную (используйте magic static).│
│  Всегда =delete copy/move constructors.                       │
│                                                                │
└────────────────────────────────────────────────────────────────┘
```

---

**Версия**: 1.0  
**Дата**: 2025-10-11  
**Курс**: Patterns (Security Integration Track)  
**Лицензия**: MIT

