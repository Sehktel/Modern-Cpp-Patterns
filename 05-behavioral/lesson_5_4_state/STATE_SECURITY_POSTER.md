# 🛡️ Security Poster: State Pattern

## 🎯 Обзор угроз State Pattern

```
┌────────────────────────────────────────────────────────────────────┐
│              КРИТИЧЕСКИЕ УЯЗВИМОСТИ STATE PATTERN                 │
└────────────────────────────────────────────────────────────────────┘

🔴 КРИТИЧНЫЕ УГРОЗЫ:
┌─────────────────────┬──────────────────────────────────────────────┐
│ Race Conditions     │ CWE-362: Data races в state transitions      │
│ (Гонки данных)      │ Impact: UAF, corruption, undefined behavior  │
├─────────────────────┼──────────────────────────────────────────────┤
│ TOCTOU Атаки        │ CWE-367: Check/Use разделены во времени      │
│ (Time-of-check)     │ Impact: Auth bypass, privilege escalation    │
├─────────────────────┼──────────────────────────────────────────────┤
│ Invalid Transitions │ CWE-662: Недопустимые переходы состояний     │
│ (Неверные переходы) │ Impact: Business logic bypass, fraud         │
└─────────────────────┴──────────────────────────────────────────────┘

🟡 ВЫСОКИЕ УГРОЗЫ:
┌─────────────────────┬──────────────────────────────────────────────┐
│ Memory Leaks        │ CWE-401: Утечки при смене состояний          │
│ State Pollution     │ Загрязнение состояния между контекстами     │
│ State Injection     │ Манипуляция внешним состоянием (cookies)     │
└─────────────────────┴──────────────────────────────────────────────┘
```

## 💣 Критическая уязвимость #1: Race Condition

### Анатомия атаки

```cpp
// ❌ УЯЗВИМЫЙ КОД - Race condition в state transition
class VulnerableContext {
private:
    State* current_state_;  // НЕТ ЗАЩИТЫ!
    
public:
    void setState(State* new_state) {
        delete current_state_;      // [1] Удаление
        current_state_ = new_state;  // [2] Присвоение
        // ⚠️ Между [1] и [2] - RACE WINDOW!
    }
    
    void request() {
        current_state_->handle();  // ⚠️ Может читать dangling pointer!
    }
};
```

### Timeline эксплойта

```
┌──────────────────────────────────────────────────────────────────┐
│                    RACE CONDITION TIMELINE                       │
└──────────────────────────────────────────────────────────────────┘

Time    Thread 1              Thread 2              Memory State
────────────────────────────────────────────────────────────────────
T0       request() →          -                     State A (0x100)
T1       read state=0x100     -                     ✅ Valid
T2       -                    setState(B) →         -
T3       -                    delete 0x100          ❌ Freed!
T4       -                    state = 0x200         State B (new)
T5       handle() на 0x100    -                     💥 UAF!

Результат: Use-After-Free → Crash / RCE / Data corruption
```

### 🎯 Exploitation техника

```cpp
// ЭКСПЛОЙТ: Timing-based race
std::atomic<int> successes{0};

void exploit_race_condition() {
    for (int i = 0; i < 10000; ++i) {  // Race spray
        std::thread victim([&]() {
            context.request();  // Начинает использовать state
        });
        
        std::thread attacker([&]() {
            usleep(50);  // Ждем попадания в race window
            context.setState(new MaliciousState());
        });
        
        victim.join();
        attacker.join();
    }
    
    // Обычно 1-10% success rate
    std::cout << "Эксплойтов: " << successes << "/10000\n";
}
```

### ✅ Защита: Thread-safe transitions

```cpp
// ✅ БЕЗОПАСНЫЙ КОД - Mutex protection
class SecureContext {
private:
    std::unique_ptr<State> state_;
    mutable std::mutex mutex_;
    
public:
    void setState(std::unique_ptr<State> new_state) {
        std::lock_guard<std::mutex> lock(mutex_);
        state_ = std::move(new_state);  // Атомарная смена
    }
    
    void request() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (state_) {
            state_->handle();  // Защищено mutex
        }
    }
};
```

## 💣 Критическая уязвимость #2: TOCTOU Attack

### Анатомия атаки

```cpp
// ❌ УЯЗВИМЫЙ КОД - TOCTOU vulnerability
void processRequest() {
    // Time of Check
    if (session->getState() == AUTHENTICATED) {
        
        // ┌───── RACE WINDOW (10-100ms) ─────┐
        // │ Thread 2 может изменить state    │
        // └───────────────────────────────────┘
        
        // Time of Use
        accessConfidentialData();  // ⚠️ Может выполниться с UNAUTHENTICATED!
    }
}
```

### 🎯 Exploitation: Authentication Bypass

```cpp
// ЭКСПЛОЙТ: TOCTOU для обхода аутентификации
std::thread victim([&]() {
    // Проверяет AUTHENTICATED
    auth.accessSecureResource();
});

std::thread attacker([&]() {
    usleep(50);  // Попадаем между check и use
    auth.setState(UNAUTHENTICATED);  // 💥 Меняем состояние!
});

// Результат: accessSecureResource() выполняется без авторизации!
```

### Измерение race window

```cpp
// Замер размера race window с помощью RDTSC
uint64_t rdtsc() {
    unsigned int lo, hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}

auto start = rdtsc();
if (state == AUTHENTICATED) {  // Check
    auto check_end = rdtsc();
    accessData();              // Use
    auto use_end = rdtsc();
    
    uint64_t window = use_end - check_end;
    // Типичное значение: 100-10,000 cycles = 0.03-3 microseconds
}
```

### ✅ Защита: Атомарная check-and-use

```cpp
// ✅ БЕЗОПАСНЫЙ КОД - Atomic check and use
class SecureAuth {
private:
    State state_;
    std::mutex mutex_;
    
public:
    void accessResource() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // Check и Use под ОДНОЙ блокировкой!
        if (state_ != AUTHENTICATED) {
            throw UnauthorizedException();
        }
        
        doAccessResource();  // Выполняется БЕЗ разблокировки
    }
};

// Альтернатива: Token-based access (без TOCTOU)
class TokenBasedAuth {
    std::atomic<uint64_t> auth_token_{0};
    
    uint64_t authenticate() {
        uint64_t token = generateSecureToken();
        auth_token_.store(token, std::memory_order_release);
        return token;  // Immutable token - нет TOCTOU!
    }
    
    void accessResource(uint64_t provided_token) {
        if (provided_token != auth_token_.load(std::memory_order_acquire)) {
            throw UnauthorizedException();
        }
        doAccessResource();
    }
};
```

## 💣 Критическая уязвимость #3: Invalid State Transitions

### Finite State Machine без валидации

```cpp
// ❌ УЯЗВИМЫЙ КОД - Отсутствие FSM validation
class Order {
    enum State { CREATED, PAID, SHIPPED, DELIVERED, CANCELLED } state_;
    double amount_;
    
public:
    void pay() { state_ = PAID; }
    void ship() { state_ = SHIPPED; }
    void deliver() { state_ = DELIVERED; }
    void cancel() { 
        state_ = CANCELLED;
        refund(amount_);  // ⚠️ Возврат средств
    }
};
```

### 🎯 Exploitation примеры

```cpp
// АТАКА 1: Double Refund
Order order;
order.pay(1000.0);      // CREATED → PAID
order.ship();           // PAID → SHIPPED
order.deliver();        // SHIPPED → DELIVERED

// 💥 Ручное изменение state
order.state_ = PAID;    // DELIVERED → PAID (недопустимо!)
order.cancel();         // Возврат $1000

order.state_ = PAID;    
order.cancel();         // Еще возврат $1000
// Итого: $2000 вместо $1000

// АТАКА 2: Ship без Payment
Order order;
order.ship();           // ⚠️ Отправка без оплаты!
order.deliver();        // Доставка бесплатно

// АТАКА 3: Post-delivery Cancel
order.pay(500);
order.ship();
order.deliver();
order.state_ = SHIPPED; // ⚠️ Откат из DELIVERED
order.cancel();         // Возврат средств + товар получен!
```

### Правильная FSM с transition table

```
┌─────────────────────────────────────────────────────────────────┐
│                    SECURE FSM DIAGRAM                           │
└─────────────────────────────────────────────────────────────────┘

     ┌─────────┐
     │ CREATED │
     └────┬────┘
          │ pay()
          ▼
     ┌─────────┐     cancel()     ┌───────────┐
     │  PAID   │◄─────────────────┤ CANCELLED │
     └────┬────┘                  └───────────┘
          │ ship()                      ▲
          ▼                             │
     ┌─────────┐                        │
     │ SHIPPED │────────────────────────┘
     └────┬────┘     cancel()
          │ deliver()
          ▼
     ┌───────────┐
     │ DELIVERED │ (final - нет переходов)
     └───────────┘

ЗАПРЕЩЕННЫЕ переходы:
❌ DELIVERED → PAID      (нельзя откатить доставку)
❌ CANCELLED → любое     (финальное состояние)
❌ CREATED → SHIPPED     (пропуск оплаты)
```

### ✅ Защита: Enforced Transition Validation

```cpp
// ✅ БЕЗОПАСНЫЙ КОД - Transition table enforcement
const std::map<State, std::set<State>> ALLOWED_TRANSITIONS = {
    {CREATED,   {PAID, CANCELLED}},
    {PAID,      {SHIPPED, CANCELLED}},
    {SHIPPED,   {DELIVERED, CANCELLED}},
    {DELIVERED, {}},  // Финальное - нет переходов
    {CANCELLED, {}}   // Финальное
};

class SecureOrder {
private:
    OrderState state_;
    std::mutex mutex_;
    bool refunded_ = false;
    
    bool isTransitionAllowed(OrderState from, OrderState to) const {
        const auto& allowed = ALLOWED_TRANSITIONS.at(from);
        return allowed.find(to) != allowed.end();
    }
    
    void transition(OrderState new_state) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (!isTransitionAllowed(state_, new_state)) {
            throw std::runtime_error(
                "❌ Invalid transition: " + 
                stateToString(state_) + " → " + stateToString(new_state)
            );
        }
        
        // Логируем для audit trail
        logTransition(state_, new_state);
        state_ = new_state;
    }
    
public:
    void pay(double amount) {
        transition(OrderState::PAID);  // Валидация!
        amount_ = amount;
    }
    
    void cancel() {
        transition(OrderState::CANCELLED);  // Валидация!
        
        // Защита от double refund
        if (amount_ > 0 && !refunded_) {
            refund(amount_);
            refunded_ = true;
        }
    }
};
```

## 🛠️ Инструменты детектирования

### 1. ThreadSanitizer (TSan) - Race Detection

```bash
# Компиляция с ThreadSanitizer
g++ -fsanitize=thread -O1 -g state_vulnerabilities.cpp -o state_tsan

# Настройка опций
export TSAN_OPTIONS="history_size=7:second_deadlock_stack=1"

# Запуск
./state_tsan
```

**Типичный вывод TSan**:
```
WARNING: ThreadSanitizer: data race (pid=12345)
  Write of size 8 at 0x7b04000000 by thread T2:
    #0 Context::setState(State*) state.cpp:45
    
  Previous read of size 8 at 0x7b04000000 by thread T1:
    #0 Context::request() state.cpp:52
    
  Location: heap block of size 16 at 0x7b04000000
  
SUMMARY: ThreadSanitizer: data race state.cpp:45 in Context::setState
```

### 2. Valgrind Memcheck - Memory Leak Detection

```bash
# Запуск Valgrind для поиска утечек памяти
valgrind --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         ./state_vulnerabilities

# Вывод при утечке:
# LEAK SUMMARY:
#   definitely lost: 10,485,760 bytes in 10 blocks
#   
# 10,485,760 bytes in 10 blocks are definitely lost:
#    at 0x4C2FB0F: operator new
#    by 0x400B23: LeakyContext::transitionTo(State*)
```

### 3. AddressSanitizer (ASan) - UAF Detection

```bash
# Компиляция с AddressSanitizer
g++ -fsanitize=address -g state_vulnerabilities.cpp -o state_asan

./state_asan

# Вывод при Use-After-Free:
# ERROR: AddressSanitizer: heap-use-after-free
#   READ of size 8 at 0x602000000010
#   #0 State::handle() state.cpp:25
#   freed by thread T2:
#   #0 operator delete
#   #1 Context::setState state.cpp:45
```

### 4. SPIN Model Checker - FSM Verification

```promela
// state_machine.pml - формальная верификация FSM
mtype = { CREATED, PAID, SHIPPED, DELIVERED, CANCELLED };
mtype state = CREATED;
int refunded = 0;

active proctype Order() {
    do
    :: state == CREATED -> state = PAID;
    :: state == PAID -> state = SHIPPED;
    :: state == SHIPPED -> state = DELIVERED;
    :: (state == CREATED || state == PAID) -> 
        state = CANCELLED; refunded++;
    od
}

// LTL свойства для проверки
ltl no_double_refund { []!(refunded > 1) }
ltl valid_delivery { [](state == DELIVERED -> X(state == DELIVERED)) }
```

```bash
# Компиляция и проверка
spin -a state_machine.pml
gcc -o pan pan.c
./pan -a

# Если найдено нарушение:
# pan: ltl no_double_refund violated
# Trail: state transitions that lead to violation
```

### 5. Race Fuzzing с libFuzzer + TSan

```cpp
// race_fuzzer.cpp - Fuzzing для race conditions
#include <thread>
#include <vector>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if (size < 10) return 0;
    
    Context context;
    std::vector<std::thread> threads;
    
    // Интерпретируем fuzzing data как операции
    for (size_t i = 0; i < size && i < 100; ++i) {
        uint8_t op = data[i];
        
        threads.emplace_back([&context, op]() {
            switch (op % 5) {
                case 0: context.setState(new StateA()); break;
                case 1: context.setState(new StateB()); break;
                case 2: context.request(); break;
                case 3: context.getStateName(); break;
                case 4: context.reset(); break;
            }
        });
    }
    
    for (auto& t : threads) t.join();
    return 0;
}
```

```bash
# Компиляция и запуск fuzzer с TSan
clang++ -fsanitize=fuzzer,thread \
        -g race_fuzzer.cpp state.cpp -o race_fuzzer

./race_fuzzer -workers=8 -max_total_time=300
```

## 📋 Security Checklist для State Pattern

### Design Phase
```
[ ] FSM формально специфицирована (UML State Diagram)
[ ] Все допустимые переходы документированы
[ ] Transition table определена и будет enforced
[ ] Threat model создана (STRIDE/PASTA)
[ ] Sensitive states identified (auth, payment, etc.)
[ ] Security invariants определены
```

### Implementation Phase
```
[ ] std::unique_ptr для владения states (RAII)
[ ] Все state transitions защищены мьютексами или atomic
[ ] Transition validation выполняется ПЕРЕД фактическим переходом
[ ] Нет прямого доступа к state_ (encapsulation)
[ ] Enter/exit методы вызываются корректно
[ ] Исключения обрабатываются без corruption state
[ ] Memory leaks предотвращены (smart pointers)
[ ] Нет TOCTOU: check-and-use атомарны
```

### Testing Phase
```
[ ] Unit tests для всех transitions
[ ] Negative tests для invalid transitions
[ ] Race condition tests с ThreadSanitizer
[ ] Memory leak tests с Valgrind
[ ] UAF tests с AddressSanitizer
[ ] Stress testing (100+ concurrent threads)
[ ] Fuzzing с libFuzzer + TSan
[ ] Model checking с SPIN или TLA+
[ ] Security penetration testing
```

### Deployment Phase
```
[ ] State transitions логируются для audit trail
[ ] Monitoring для обнаружения invalid transitions
[ ] Rate limiting для критических transitions
[ ] Incident response plan для state compromise
[ ] Alerting на аномальные паттерны переходов
```

## 🎓 Best Practices

### 1. Используйте Type-Safe State Machine (C++17)

```cpp
// ✅ Type-safe с std::variant
struct Created {};
struct Paid { double amount; };
struct Shipped {};
struct Delivered {};
struct Cancelled {};

using OrderState = std::variant<Created, Paid, Shipped, Delivered, Cancelled>;

class TypeSafeOrder {
    OrderState state_ = Created{};
    
public:
    void pay(double amount) {
        // Compile-time проверка допустимого перехода
        if (std::holds_alternative<Created>(state_)) {
            state_ = Paid{amount};
        } else {
            throw std::runtime_error("Cannot pay in current state");
        }
    }
    
    void ship() {
        if (auto* paid = std::get_if<Paid>(&state_)) {
            state_ = Shipped{};
        } else {
            throw std::runtime_error("Cannot ship: not paid");
        }
    }
};
```

### 2. State History для Audit Trail

```cpp
// ✅ Audit trail с history
class AuditableStateMachine {
private:
    std::unique_ptr<State> current_;
    std::vector<StateTransition> history_;
    
public:
    void transitionTo(std::unique_ptr<State> new_state) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // Запись в audit log
        history_.push_back({
            .from = current_->getName(),
            .to = new_state->getName(),
            .timestamp = std::chrono::system_clock::now(),
            .thread_id = std::this_thread::get_id()
        });
        
        current_ = std::move(new_state);
    }
    
    // Для forensics и compliance
    std::vector<StateTransition> getHistory() const {
        return history_;
    }
};
```

### 3. Immutable State для Thread Safety

```cpp
// ✅ Immutable state - нет race conditions
class ImmutableSession {
private:
    const std::string user_;
    const State state_;
    
    ImmutableSession(std::string u, State s) 
        : user_(std::move(u)), state_(s) {}
    
public:
    static ImmutableSession create(std::string user) {
        return ImmutableSession(std::move(user), State::UNAUTHENTICATED);
    }
    
    ImmutableSession withAuthenticated() const {
        return ImmutableSession(user_, State::AUTHENTICATED);
    }
    
    // Каждый "переход" создает новый объект
    // Нет shared mutable state → нет race conditions!
};
```

## 🚨 CVE Examples - Real-World Impact

```
┌────────────────────────────────────────────────────────────────┐
│                  ИЗВЕСТНЫЕ CVE С STATE RACES                   │
└────────────────────────────────────────────────────────────────┘

CVE-2016-5195 (Dirty COW)
├─ Тип: Race condition в kernel memory management
├─ CVSS: 7.8 (HIGH)
├─ Impact: Local privilege escalation
└─ State race в copy-on-write mechanism

CVE-2019-14287 (Sudo Bypass)
├─ Тип: TOCTOU в user ID checking
├─ CVSS: 8.8 (HIGH)  
├─ Impact: Выполнение команд от имени root
└─ Race между check UID и exec

CVE-2020-8835 (Linux Kernel eBPF)
├─ Тип: Race condition в eBPF verification
├─ CVSS: 7.8 (HIGH)
├─ Impact: Kernel privilege escalation
└─ State confusion в verifier state machine

CVE-2021-3156 (Sudo Heap Overflow)
├─ Тип: State confusion в command parsing
├─ CVSS: 7.8 (HIGH)
├─ Impact: Root privilege escalation
└─ Invalid state transition в parser FSM
```

## 📚 Дополнительные ресурсы

### Стандарты и Compliance
- **OWASP Top 10 2021**: A01 (Broken Access Control), A04 (Insecure Design)
- **CWE Top 25**: CWE-362 (#11), CWE-662 (#18), CWE-367
- **ISO/IEC 27001**: A.14.2.5 (Secure system engineering)
- **MISRA C++**: Rule 6-4-4 (Switch completeness)

### Инструменты
- ThreadSanitizer: https://github.com/google/sanitizers
- SPIN Model Checker: http://spinroot.com/
- TLA+: https://lamport.azurewebsites.net/tla/tla.html
- Valgrind: https://valgrind.org/

### Книги и статьи
- "Concurrent Programming in C++" - Anthony Williams
- "UML State Machine" - Samek Miro
- "The Art of Software Security Assessment" - Dowd, McDonald, Schuh

## 💡 Золотое правило State Security

```
┌────────────────────────────────────────────────────────────────┐
│                                                                │
│  "Состояние - это shared mutable data.                        │
│   Относитесь к каждому переходу как к критической секции,     │
│   требующей синхронизации и валидации."                        │
│                                                                │
│  В многопоточных системах, любое shared mutable state -        │
│  это уязвимость по умолчанию, пока не доказано обратное       │
│  через правильную синхронизацию!                               │
│                                                                │
└────────────────────────────────────────────────────────────────┘
```

---

**Версия**: 1.0  
**Дата**: 2025-10-11  
**Курс**: Patterns (Security Integration Track)  
**Лицензия**: MIT

