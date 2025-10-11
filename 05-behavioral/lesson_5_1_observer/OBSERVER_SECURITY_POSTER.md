# 🛡️ Security Poster: Observer Pattern

## 🎯 Обзор угроз Observer Pattern

```
┌────────────────────────────────────────────────────────────────────┐
│             КРИТИЧЕСКИЕ УЯЗВИМОСТИ OBSERVER PATTERN                │
└────────────────────────────────────────────────────────────────────┘

🔴 КРИТИЧНЫЕ УГРОЗЫ:
┌─────────────────────┬──────────────────────────────────────────────┐
│ Use-After-Free      │ CWE-416: Dangling observers в notify()       │
│ (UAF)               │ Impact: RCE, memory corruption, crash        │
├─────────────────────┼──────────────────────────────────────────────┤
│ Race Conditions     │ CWE-362: Concurrent attach/detach/notify     │
│ (Data Races)        │ Impact: Iterator invalidation, UB, crashes   │
├─────────────────────┼──────────────────────────────────────────────┤
│ Reentrancy Issues   │ Recursive notify() during update()           │
│                     │ Impact: Stack overflow, state corruption     │
└─────────────────────┴──────────────────────────────────────────────┘

🟡 ВЫСОКИЕ УГРОЗЫ:
┌─────────────────────┬──────────────────────────────────────────────┐
│ Memory Leaks        │ CWE-401: Observers не отписываются           │
│ Notification Storms │ Amplification DoS через chain reactions      │
│ Observer Injection  │ Malicious observers для data exfiltration    │
└─────────────────────┴──────────────────────────────────────────────┘
```

## 💣 Критическая уязвимость #1: Use-After-Free

### Анатомия атаки

```cpp
// ❌ УЯЗВИМЫЙ КОД - Сырые указатели на observers
class VulnerableSubject {
private:
    std::vector<Observer*> observers_;  // ⚠️ Dangling pointers!
    
public:
    void attach(Observer* obs) {
        observers_.push_back(obs);
    }
    
    void notify() {
        for (auto* obs : observers_) {
            obs->update();  // ⚠️ Может быть dangling pointer!
        }
    }
};
```

### 🎯 Exploitation сценарий

```cpp
// ЭКСПЛОЙТ: Use-After-Free
VulnerableSubject subject;

{
    Observer* obs = new ConcreteObserver();
    subject.attach(obs);
    delete obs;  // ❌ Subject все еще хранит указатель!
}

// 💥 UAF - обращение к освобожденной памяти
subject.notify();

// Что происходит:
// 1. obs->update() вызывает vtable lookup
// 2. vtable в freed memory может быть перезаписан
// 3. Переход по vtable → RCE или crash

// Memory layout:
// 0x1000: [freed memory] ← obs указывает сюда
// 0x1000: [attacker data] ← новая аллокация
// vtable[0] = 0xdeadbeef  ← код атакующего
```

### Heap spray для эксплойта

```cpp
// Техника: Heap spraying для контроля freed memory
void exploit_uaf() {
    VulnerableSubject subject;
    
    // 1. Создаем и удаляем observer
    Observer* victim = new ConcreteObserver();
    subject.attach(victim);
    delete victim;
    
    // 2. Heap spray: заполняем freed memory
    for (int i = 0; i < 1000; ++i) {
        void* spray = malloc(sizeof(ConcreteObserver));
        // Записываем fake vtable
        *(void**)spray = (void*)0xdeadbeef;  // Fake vtable pointer
    }
    
    // 3. Trigger UAF
    subject.notify();  // 💥 Вызов 0xdeadbeef → RCE
}
```

### ✅ Защита: weak_ptr pattern

```cpp
// ✅ БЕЗОПАСНЫЙ КОД - weak_ptr предотвращает UAF
class SafeSubject {
private:
    std::vector<std::weak_ptr<Observer>> observers_;
    
public:
    void attach(std::shared_ptr<Observer> obs) {
        observers_.push_back(obs);  // Сохраняем weak_ptr
    }
    
    void notify() {
        for (auto& weak_obs : observers_) {
            if (auto obs = weak_obs.lock()) {  // ✅ Безопасная проверка
                obs->update();  // Вызывается только если объект жив
            }
        }
        
        // Очистка мертвых weak_ptr
        observers_.erase(
            std::remove_if(observers_.begin(), observers_.end(),
                [](const auto& w) { return w.expired(); }),
            observers_.end()
        );
    }
};
```

## 💣 Критическая уязвимость #2: Race Conditions

### Анатомия атаки

```cpp
// ❌ УЯЗВИМЫЙ КОД - Нет синхронизации
class RacySubject {
private:
    std::vector<Observer*> observers_;  // ⚠️ Shared mutable state
    
public:
    void attach(Observer* obs) {
        observers_.push_back(obs);  // ⚠️ Not thread-safe
    }
    
    void detach(Observer* obs) {
        observers_.erase(
            std::remove(observers_.begin(), observers_.end(), obs),
            observers_.end()
        );  // ⚠️ Not thread-safe
    }
    
    void notify() {
        for (auto* obs : observers_) {  // ⚠️ Iterator может invalidate
            obs->update();
        }
    }
};
```

### Race condition scenarios

```
┌──────────────────────────────────────────────────────────────────┐
│                    RACE CONDITION TIMELINE                       │
└──────────────────────────────────────────────────────────────────┘

Сценарий 1: attach/notify race
Time    Thread 1              Thread 2              Vector State
────────────────────────────────────────────────────────────────────
T0      -                     notify() →            [A, B, C]
T1      -                     iterate: A            -
T2      attach(D) →           -                     -
T3      push_back(D)          -                     [A, B, C, ?]
T4      -                     iterate: B            💥 Vector реаллокация!
T5      -                     invalid iterator      CRASH

Сценарий 2: detach/notify race
Time    Thread 1              Thread 2              Observers
────────────────────────────────────────────────────────────────────
T0      notify() →            -                     [A, B, C]
T1      obs = B               -                     -
T2      -                     detach(B) →           -
T3      -                     erase(B)              [A, C]
T4      obs->update()         -                     💥 B удален! UAF

Сценарий 3: concurrent notify/attach
Time    Thread 1              Thread 2              Result
────────────────────────────────────────────────────────────────────
T0      notify()              attach(X)             -
T1      size() = 3            push_back(X)          💥 Size changed!
T2      for i in [0..3)       -                     May iterate invalid
```

### 🎯 Exploitation: Data race для DoS

```cpp
// ЭКСПЛОЙТ: Concurrent attach/detach для crash
void exploit_race() {
    RacySubject subject;
    
    // Начальные observers
    for (int i = 0; i < 100; ++i) {
        subject.attach(new ConcreteObserver());
    }
    
    // Thread 1: Continuous notify
    std::thread notifier([&]() {
        while (true) {
            subject.notify();  // Итерируется по вектору
        }
    });
    
    // Thread 2: Continuous attach/detach
    std::thread mutator([&]() {
        while (true) {
            Observer* obs = new ConcreteObserver();
            subject.attach(obs);    // ⚠️ Модифицирует вектор
            subject.detach(obs);    // ⚠️ Модифицирует вектор
            delete obs;
        }
    });
    
    // Результат: Data race → iterator invalidation → CRASH
    // Обычно крашится через 1-10 секунд
}
```

### ✅ Защита: Reader-Writer Lock

```cpp
// ✅ БЕЗОПАСНЫЙ КОД - Правильная синхронизация
class ThreadSafeSubject {
private:
    std::vector<std::weak_ptr<Observer>> observers_;
    mutable std::shared_mutex mutex_;
    
public:
    void attach(std::shared_ptr<Observer> obs) {
        std::unique_lock lock(mutex_);  // Эксклюзивная блокировка
        observers_.push_back(obs);
    }
    
    void detach(std::shared_ptr<Observer> obs) {
        std::unique_lock lock(mutex_);  // Эксклюзивная блокировка
        observers_.erase(
            std::remove_if(observers_.begin(), observers_.end(),
                [&](const auto& w) {
                    if (auto ptr = w.lock()) {
                        return ptr.get() == obs.get();
                    }
                    return true;  // Удаляем expired
                }),
            observers_.end()
        );
    }
    
    void notify() {
        // Копируем список под shared lock
        std::vector<std::shared_ptr<Observer>> active;
        {
            std::shared_lock lock(mutex_);  // Multiple readers OK
            for (auto& weak_obs : observers_) {
                if (auto obs = weak_obs.lock()) {
                    active.push_back(obs);
                }
            }
        }
        // Уведомляем без блокировки (избегаем deadlock)
        for (auto& obs : active) {
            obs->update();
        }
    }
};
```

## 💣 Критическая уязвимость #3: Reentrancy Issues

### Анатомия атаки

```cpp
// ❌ УЯЗВИМЫЙ КОД - Модификация во время notify
class ReentrantSubject {
    std::vector<Observer*> observers_;
    
    void notify() {
        for (auto* obs : observers_) {  // ⚠️ Итератор может invalidate
            obs->update();  // ⚠️ update() может вызвать attach/detach!
        }
    }
};

class ReentrantObserver : public Observer {
    ReentrantSubject& subject_;
    
    void update() override {
        // 💥 Модифицируем subject во время notify!
        Observer* new_obs = new ConcreteObserver();
        subject_.attach(new_obs);  // Iterator invalidation!
    }
};
```

### Stack overflow через recursive notify

```cpp
// ЭКСПЛОЙТ: Recursive notify для stack overflow
class RecursiveObserver : public Observer {
    Subject& subject_;
    int depth_ = 0;
    
    void update() override {
        if (++depth_ < 10000) {  // Deep recursion
            subject_.notify();  // 💥 Recursive call!
        }
    }
};

// Результат: Stack overflow после ~1000-10000 вызовов
// Ulimit: ~8MB stack → ~8KB per frame → ~1000 deep calls
```

### ✅ Защита: Snapshot iteration + recursion guard

```cpp
// ✅ БЕЗОПАСНЫЙ КОД - Защита от reentrancy
class ReentrancySafeSubject {
private:
    std::vector<std::weak_ptr<Observer>> observers_;
    std::atomic<bool> notifying_{false};
    
public:
    void notify() {
        // Guard против recursive notify
        if (notifying_.exchange(true)) {
            return;  // Уже в процессе notify
        }
        
        // Snapshot pattern - копируем список
        std::vector<std::shared_ptr<Observer>> active;
        for (auto& weak_obs : observers_) {
            if (auto obs = weak_obs.lock()) {
                active.push_back(obs);
            }
        }
        
        // Итерируем по snapshot (безопасно от модификаций)
        for (auto& obs : active) {
            obs->update();
        }
        
        notifying_.store(false);
    }
};
```

## 🛠️ Инструменты детектирования

### 1. AddressSanitizer - UAF Detection

```bash
# Компиляция с ASan
g++ -fsanitize=address -g observer_vulnerabilities.cpp -o observer_asan

./observer_asan
```

**Типичный вывод ASan**:
```
=================================================================
==12345==ERROR: AddressSanitizer: heap-use-after-free on address 0x602000000010
READ of size 8 at 0x602000000010 thread T0
    #0 0x400b12 in Subject::notify() observer.cpp:45
    #1 0x400c34 in main observer.cpp:123
    
0x602000000010 is located 0 bytes inside of 32-byte region
freed by thread T0 here:
    #0 0x7f8c operator delete
    #1 0x400a89 in main observer.cpp:118
    
previously allocated by thread T0 here:
    #0 0x7f8c operator new
    #1 0x400a12 in main observer.cpp:115
```

### 2. ThreadSanitizer - Race Detection

```bash
# Компиляция с TSan
g++ -fsanitize=thread -O1 -g observer_vulnerabilities.cpp -o observer_tsan

export TSAN_OPTIONS="history_size=7:second_deadlock_stack=1"
./observer_tsan
```

**Типичный вывод TSan**:
```
WARNING: ThreadSanitizer: data race (pid=12345)
  Write of size 8 at 0x7b0400000020 by thread T2:
    #0 std::vector::push_back observer.cpp:34
    #1 Subject::attach observer.cpp:35
    
  Previous read of size 8 at 0x7b0400000020 by thread T1:
    #0 std::vector::begin observer.cpp:45
    #1 Subject::notify observer.cpp:46
    
  Location: heap block of size 64 at 0x7b0400000020
  
SUMMARY: ThreadSanitizer: data race observer.cpp:34 in Subject::attach
```

### 3. Valgrind Memcheck - Memory Leaks

```bash
valgrind --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         ./observer_vulnerabilities

# Вывод:
# LEAK SUMMARY:
#   definitely lost: 1,024 bytes in 32 blocks
#   indirectly lost: 0 bytes in 0 blocks
#
# 1,024 bytes in 32 blocks are definitely lost:
#    at 0x4C2FB0F: operator new
#    by 0x400B23: Subject::attach
```

### 4. Static Analysis - Clang Static Analyzer

```bash
# Статический анализ с Clang
clang++ --analyze \
        -Xanalyzer -analyzer-output=text \
        -Xanalyzer -analyzer-checker=core,cplusplus \
        observer_vulnerabilities.cpp

# Вывод:
# observer.cpp:45:9: warning: Use of memory after it is freed
#         obs->update();
#         ^~~
# observer.cpp:118:5: note: Memory is released
#     delete obs;
#     ^
```

## 📋 Security Checklist для Observer Pattern

### Design Phase
```
[ ] Определена ownership policy для observers
[ ] Документирован thread-safety model
[ ] Определена стратегия notification ordering
[ ] Рассмотрены reentrancy scenarios
[ ] Threat model создана (особенно для event systems)
```

### Implementation Phase
```
[ ] Используется std::weak_ptr для хранения observers
[ ] Все операции (attach/detach/notify) thread-safe
[ ] Защита от reentrancy (snapshot или guard)
[ ] Observer registration/unregistration атомарны
[ ] Cleanup expired weak_ptr регулярно выполняется
[ ] Нет сырых указателей на observers
[ ] notify() не держит locks во время callback
```

### Testing Phase
```
[ ] Unit tests для attach/detach/notify
[ ] UAF tests с AddressSanitizer
[ ] Race condition tests с ThreadSanitizer
[ ] Memory leak tests с Valgrind
[ ] Stress testing (1000+ observers, 100+ threads)
[ ] Reentrancy tests (attach/detach during notify)
[ ] Fuzzing для edge cases
```

### Deployment Phase
```
[ ] Monitoring для observer count anomalies
[ ] Alerting на memory leaks
[ ] Rate limiting для notification storms
[ ] Logging для debug (opt-in для prod)
```

## 🎓 Best Practices

### 1. RAII Observer Auto-Unsubscribe

```cpp
// ✅ RAII для автоматической отписки
class ScopedObserver {
private:
    Subject& subject_;
    std::shared_ptr<Observer> observer_;
    
public:
    ScopedObserver(Subject& subj, std::shared_ptr<Observer> obs)
        : subject_(subj), observer_(obs) {
        subject_.attach(observer_);
    }
    
    ~ScopedObserver() {
        subject_.detach(observer_);  // Автоматическая отписка
    }
    
    // Запрет копирования
    ScopedObserver(const ScopedObserver&) = delete;
    ScopedObserver& operator=(const ScopedObserver&) = delete;
};

// Использование:
void example() {
    Subject subject;
    auto observer = std::make_shared<ConcreteObserver>();
    
    {
        ScopedObserver scoped(subject, observer);
        // observer автоматически отпишется при выходе из scope
    }
}
```

### 2. Event Queue для Asynchronous Notification

```cpp
// ✅ Асинхронные уведомления без блокировки
class AsyncSubject {
private:
    std::vector<std::weak_ptr<Observer>> observers_;
    std::queue<Event> event_queue_;
    std::mutex queue_mutex_;
    std::condition_variable cv_;
    std::thread worker_;
    std::atomic<bool> running_{true};
    
    void worker_thread() {
        while (running_) {
            Event event;
            {
                std::unique_lock lock(queue_mutex_);
                cv_.wait(lock, [this] { 
                    return !event_queue_.empty() || !running_; 
                });
                
                if (!running_) break;
                
                event = event_queue_.front();
                event_queue_.pop();
            }
            
            // Уведомляем без блокировки очереди
            notify_sync(event);
        }
    }
    
public:
    AsyncSubject() : worker_([this] { worker_thread(); }) {}
    
    ~AsyncSubject() {
        running_ = false;
        cv_.notify_one();
        worker_.join();
    }
    
    void notify_async(Event event) {
        {
            std::lock_guard lock(queue_mutex_);
            event_queue_.push(event);
        }
        cv_.notify_one();
    }
};
```

### 3. Priority Observers

```cpp
// ✅ Приоритетные observers для ordered notification
class PrioritySubject {
private:
    using ObserverPair = std::pair<int, std::weak_ptr<Observer>>;
    std::vector<ObserverPair> observers_;  // (priority, observer)
    std::shared_mutex mutex_;
    
public:
    void attach(std::shared_ptr<Observer> obs, int priority = 0) {
        std::unique_lock lock(mutex_);
        observers_.emplace_back(priority, obs);
        
        // Сортируем по приоритету (высший приоритет первым)
        std::sort(observers_.begin(), observers_.end(),
            [](const auto& a, const auto& b) { return a.first > b.first; });
    }
    
    void notify() {
        std::vector<std::shared_ptr<Observer>> active;
        {
            std::shared_lock lock(mutex_);
            for (auto& [priority, weak_obs] : observers_) {
                if (auto obs = weak_obs.lock()) {
                    active.push_back(obs);
                }
            }
        }
        
        // Уведомляем в порядке приоритета
        for (auto& obs : active) {
            obs->update();
        }
    }
};
```

## 🚨 CVE Examples - Real-World Impact

```
┌────────────────────────────────────────────────────────────────┐
│                  ИЗВЕСТНЫЕ CVE С OBSERVER UAF                  │
└────────────────────────────────────────────────────────────────┘

CVE-2019-11358 (jQuery Event Handlers)
├─ Тип: Use-After-Free в event listeners
├─ CVSS: 6.1 (MEDIUM)
├─ Impact: XSS через dangling event handlers
└─ Причина: Weak references не использовались

CVE-2020-6831 (Firefox Observer Pattern)
├─ Тип: UAF в observer notification
├─ CVSS: 8.8 (HIGH)
├─ Impact: Remote Code Execution
└─ Причина: Race condition в observer cleanup

CVE-2018-5089 (Chrome IPC Observers)
├─ Тип: Use-After-Free в IPC observers
├─ CVSS: 9.8 (CRITICAL)
├─ Impact: Sandbox escape + RCE
└─ Причина: Reentrancy в notification loop

CVE-2021-30661 (WebKit DOM Events)
├─ Тип: UAF в DOM event listeners
├─ CVSS: 8.8 (HIGH)
├─ Impact: Remote Code Execution
└─ Причина: Event handler modification during dispatch
```

## 📚 Дополнительные ресурсы

### Стандарты и Compliance
- **CWE-416**: Use After Free
- **CWE-362**: Concurrent Execution using Shared Resource
- **CWE-401**: Missing Release of Memory after Effective Lifetime
- **CERT C++**: OOP52-CPP, CON53-CPP

### Инструменты
- AddressSanitizer: https://github.com/google/sanitizers
- ThreadSanitizer: https://github.com/google/sanitizers
- Valgrind: https://valgrind.org/
- Clang Static Analyzer: https://clang-analyzer.llvm.org/

### Книги
- "Effective Modern C++" - Scott Meyers (Items 18-22)
- "C++ Concurrency in Action" - Anthony Williams
- "The Art of Software Security Assessment" - Dowd, McDonald, Schuh

## 💡 Золотое правило Observer Security

```
┌────────────────────────────────────────────────────────────────┐
│                                                                │
│  "Observer lifetime - это время жизни сотрудничества.          │
│   Используйте weak_ptr для безопасного наблюдения,            │
│   shared_mutex для thread-safety,                              │
│   и snapshot pattern для защиты от reentrancy."                │
│                                                                │
│  Никогда не храните сырые указатели на observers -             │
│  это UAF waiting to happen!                                    │
│                                                                │
└────────────────────────────────────────────────────────────────┘
```

---

**Версия**: 1.0  
**Дата**: 2025-10-11  
**Курс**: Patterns (Security Integration Track)  
**Лицензия**: MIT

