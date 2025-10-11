# Анализ безопасности паттерна State

## Обзор

Паттерн State позволяет объекту изменять свое поведение в зависимости от внутреннего состояния. Это мощный механизм для моделирования state machines, но при этом создает множество уязвимостей, особенно в многопоточных системах. Переходы между состояниями становятся критическими точками, где могут возникать race conditions, TOCTOU атаки, и invalid state transitions.

## Почему State паттерн уязвим

### 1. Временная природа состояний
Состояния изменяются во времени, создавая **race windows** между проверкой и использованием состояния.

### 2. Множественные точки перехода
Каждый переход - потенциальная точка атаки:
- Проверка допустимости перехода
- Actual переход (смена указателя)
- Cleanup старого состояния
- Инициализация нового состояния

### 3. Shared State в многопоточности
Context часто shared между потоками:
- Web servers (session state)
- Game engines (entity state)
- Network protocols (connection state)
- Financial systems (transaction state)

### 4. Business Logic в переходах
Критическая логика (платежи, авторизация) завязана на состояния, делая их prime targets для атак.

## Критические уязвимости

### 1. Race Condition на State Transitions (CWE-362)

#### Описание
Race condition происходит когда несколько потоков одновременно читают и изменяют состояние без должной синхронизации. Это одна из самых опасных уязвимостей в State паттерне.

#### Технические детали

**Проблема неатомарного перехода**:
```cpp
// УЯЗВИМЫЙ КОД
class Context {
private:
    State* current_state_;  // НЕТ ЗАЩИТЫ!
    
public:
    void setState(State* new_state) {
        delete current_state_;      // [1] Удаление
        current_state_ = new_state;  // [2] Присвоение
        // Между [1] и [2] - race window!
    }
    
    void request() {
        current_state_->handle();  // Может читать dangling pointer!
    }
};
```

**Что происходит в памяти**:

```
Время    Thread 1              Thread 2              State
--------------------------------------------------------------
T0       request() →           -                     State A (valid)
T1       read state_ = 0x100   -                     State A (valid)
T2       -                     setState(B) →         -
T3       -                     delete 0x100          State A (freed!)
T4       -                     state_ = 0x200        State B (new)
T5       handle() на 0x100  ←  -                     UAF!
```

**Memory corruption пример**:
```cpp
// Thread 1                    // Thread 2
State* ptr = current_state_;   // -
// ptr = 0x7f8c00001000         
// -                            delete current_state_;
// -                            // 0x7f8c00001000 freed
// -                            current_state_ = new State();
// -                            // new object at 0x7f8c00001000
ptr->handle();                  // UAF or access to wrong object!
```

#### Exploitation: TOCTOU Attack

**Сценарий - Authentication Bypass**:
```cpp
class AuthContext {
    enum State { UNAUTHENTICATED, AUTHENTICATED } state_;
    
    void accessSecureResource() {
        if (state_ == AUTHENTICATED) {  // Time of Check
            // [RACE WINDOW: 10-100ms]
            // Thread 2 может изменить state_ здесь
            
            // Time of Use
            grantAccess();  // Может выполниться с UNAUTHENTICATED!
        }
    }
};

// ЭКСПЛОЙТ
std::thread attacker([&auth]() {
    auth.accessSecureResource();  // Начинает проверку
});

std::thread race_exploit([&auth]() {
    usleep(50);  // Ждем момента между check и use
    auth.setState(UNAUTHENTICATED);  // Меняем state!
});
```

**Увеличение вероятности успеха**:
```cpp
// 1. CPU affinity для контроля scheduling
cpu_set_t cpuset;
CPU_ZERO(&cpuset);
CPU_SET(0, &cpuset);  // Thread 1 на CPU 0
pthread_setaffinity_np(thread1, sizeof(cpu_set_t), &cpuset);

CPU_SET(1, &cpuset);  // Thread 2 на CPU 1
pthread_setaffinity_np(thread2, sizeof(cpu_set_t), &cpuset);

// 2. Множественные попытки
for (int i = 0; i < 10000; ++i) {
    attempt_race_exploit();
}

// 3. Timing analysis для определения race window
auto start = rdtsc();  // Read Time-Stamp Counter
check_state();
auto check_time = rdtsc();
use_state();
auto use_time = rdtsc();

uint64_t window = use_time - check_time;
// Теперь знаем точный размер race window в CPU cycles
```

#### CVE примеры

- **CVE-2016-5195 (Dirty COW)**: Race condition в kernel memory management
- **CVE-2019-14287 (Sudo)**: TOCTOU в user ID checking
- **CVE-2020-8835 (Linux Kernel)**: Race condition в eBPF verification

#### Детектирование

**ThreadSanitizer**:
```bash
g++ -fsanitize=thread -O1 -g state_vulnerabilities.cpp -o state_tsan

export TSAN_OPTIONS="history_size=7:second_deadlock_stack=1"

./state_tsan

# Вывод:
WARNING: ThreadSanitizer: data race (pid=12345)
  Write of size 8 at 0x7b04000000 by thread T2:
    #0 Context::setState(State*) state.cpp:45
    
  Previous read of size 8 at 0x7b04000000 by thread T1:
    #0 Context::request() state.cpp:52
    
  Location is heap block of size 16 at 0x7b04000000 allocated by main thread:
    #0 operator new
    #1 main state.cpp:123
```

**Helgrind (Valgrind)**:
```bash
valgrind --tool=helgrind --history-level=full ./state_vulnerabilities

# Вывод:
Possible data race during write of size 8 at 0x4A2B000 by thread #2
   at 0x400B45: Context::setState (state.cpp:45)
   by 0x400C12: demonstrateRace (state.cpp:78)

This conflicts with a previous read of size 8 by thread #1
   at 0x400A98: Context::request (state.cpp:52)
   by 0x400B99: demonstrateRace (state.cpp:75)
```

#### Защита

**Решение 1: Mutex Protection**:
```cpp
class ThreadSafeContext {
private:
    std::unique_ptr<State> state_;
    mutable std::mutex mutex_;
    
public:
    void setState(std::unique_ptr<State> new_state) {
        std::lock_guard<std::mutex> lock(mutex_);
        state_ = std::move(new_state);
    }
    
    void request() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (state_) {
            state_->handle();
        }
    }
};
```

**Решение 2: Atomic для простых состояний**:
```cpp
class AtomicStateContext {
private:
    std::atomic<ConnectionState> state_{ConnectionState::DISCONNECTED};
    
public:
    bool connect() {
        ConnectionState expected = ConnectionState::DISCONNECTED;
        
        // Атомарный CAS (Compare-And-Swap)
        if (state_.compare_exchange_strong(
            expected, 
            ConnectionState::CONNECTING,
            std::memory_order_acq_rel)) {
            
            // Только один поток войдет сюда
            doConnect();
            state_.store(ConnectionState::CONNECTED, std::memory_order_release);
            return true;
        }
        
        return false;  // Уже connecting/connected
    }
};
```

**Решение 3: Reader-Writer Lock**:
```cpp
class RWLockContext {
private:
    std::unique_ptr<State> state_;
    mutable std::shared_mutex mutex_;
    
public:
    void setState(std::unique_ptr<State> new_state) {
        std::unique_lock lock(mutex_);  // Exclusive lock для записи
        state_ = std::move(new_state);
    }
    
    void request() {
        std::shared_lock lock(mutex_);  // Shared lock для чтения
        if (state_) {
            state_->handle();
        }
    }
    
    std::string getStateName() const {
        std::shared_lock lock(mutex_);  // Multiple readers OK
        return state_ ? state_->getName() : "None";
    }
};
```

### 2. Invalid State Transitions (CWE-662)

#### Описание
Invalid state transitions происходят когда система переходит в состояние, которое недопустимо из текущего состояния, нарушая business logic и создавая security holes.

#### Проблема отсутствия State Transition Table

**Уязвимый код**:
```cpp
class Order {
    enum State { CREATED, PAID, SHIPPED, DELIVERED, CANCELLED } state_;
    double amount_;
    
public:
    void pay() { state_ = PAID; }
    void ship() { state_ = SHIPPED; }
    void deliver() { state_ = DELIVERED; }
    void cancel() { 
        state_ = CANCELLED;
        refund(amount_);  // Возврат средств
    }
};
```

**Что не так**:
- Можно вызвать `ship()` до `pay()` → Отправка неоплаченного заказа
- Можно вызвать `cancel()` после `deliver()` → Double refund
- Можно вызвать `pay()` после `deliver()` → Повторная оплата
- Нет enforc'а правильной последовательности

#### Exploitation примеры

**Атака 1: Double Refund**:
```cpp
Order order;
order.pay(1000.0);      // State: CREATED → PAID
order.ship();           // State: PAID → SHIPPED
order.deliver();        // State: SHIPPED → DELIVERED

// АТАКА: Ручное изменение state
order.state_ = PAID;    // Вернули в PAID
order.cancel();         // Возврат $1000

order.state_ = PAID;    // Снова в PAID  
order.cancel();         // Еще возврат $1000

// Итого: Получили $2000 вместо $1000
```

**Атака 2: Ship без Payment**:
```cpp
Order order;
// НЕ вызываем pay()!

order.ship();           // Отправка без оплаты
order.deliver();        // Доставка
// Компания потеряла товар без payment
```

**Атака 3: State Rollback**:
```cpp
// Легитимный flow
order.pay(500);
order.ship();
order.deliver();

// АТАКА: Откат в предыдущее состояние
order.state_ = SHIPPED;  // Откат из DELIVERED
order.cancel();          // Возврат средств
// Товар получен + деньги возвращены
```

#### Finite State Machine (FSM) Security

**Правильная FSM**:
```
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
     │ DELIVERED │ (final)
     └───────────┘
```

**Transition Table**:
```cpp
const std::map<State, std::set<State>> ALLOWED_TRANSITIONS = {
    {CREATED,   {PAID, CANCELLED}},
    {PAID,      {SHIPPED, CANCELLED}},
    {SHIPPED,   {DELIVERED, CANCELLED}},
    {DELIVERED, {}},  // Финальное - нет переходов
    {CANCELLED, {}}   // Финальное
};
```

#### Защита

**Enforced Transitions**:
```cpp
class SecureOrder {
private:
    OrderState state_;
    std::mutex mutex_;
    
    bool isTransitionAllowed(OrderState from, OrderState to) const {
        const auto& allowed = ALLOWED_TRANSITIONS.at(from);
        return allowed.find(to) != allowed.end();
    }
    
    void transition(OrderState new_state) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (!isTransitionAllowed(state_, new_state)) {
            throw std::runtime_error(
                "Invalid transition: " + 
                stateToString(state_) + " → " + stateToString(new_state)
            );
        }
        
        // Логируем переход для audit
        logTransition(state_, new_state);
        
        state_ = new_state;
    }
    
public:
    void pay(double amount) {
        transition(OrderState::PAID);
        amount_ = amount;
    }
    
    void ship() {
        transition(OrderState::SHIPPED);
    }
    
    void deliver() {
        transition(OrderState::DELIVERED);
    }
    
    void cancel() {
        transition(OrderState::CANCELLED);
        if (amount_ > 0 && !refunded_) {
            refund(amount_);
            refunded_ = true;
        }
    }
};
```

**Type-safe State Machine (C++17)**:
```cpp
// Используем std::variant для type-safe states
struct Created {};
struct Paid { double amount; };
struct Shipped {};
struct Delivered {};
struct Cancelled {};

using OrderState = std::variant<Created, Paid, Shipped, Delivered, Cancelled>;

class TypeSafeOrder {
private:
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
    
    void deliver() {
        if (std::holds_alternative<Shipped>(state_)) {
            state_ = Delivered{};
        } else {
            throw std::runtime_error("Cannot deliver: not shipped");
        }
    }
};
```

### 3. TOCTOU (Time-Of-Check to Time-Of-Use) (CWE-367)

#### Описание
TOCTOU vulnerability возникает когда проверка состояния и его использование разделены во времени, позволяя изменить состояние между проверкой и использованием.

#### Анатомия TOCTOU

**Классический пример**:
```cpp
// УЯЗВИМЫЙ КОД
void processRequest() {
    // Time of Check
    if (session->getState() == AUTHENTICATED) {
        // ┌─ RACE WINDOW (10-100ms) ─┐
        // │ Thread 2 может logout    │
        // └───────────────────────────┘
        
        // Time of Use
        accessConfidentialData();  // Может выполниться с UNAUTHENTICATED!
    }
}
```

**Размер race window**:
```cpp
// Измерение с помощью RDTSC
uint64_t rdtsc() {
    unsigned int lo, hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}

auto start = rdtsc();
if (state == AUTHENTICATED) {  // Check
    auto check_end = rdtsc();
    
    accessData();  // Use
    
    auto use_end = rdtsc();
    
    uint64_t window = use_end - check_end;  // Race window в CPU cycles
    // Типичное значение: 100-10,000 cycles = 0.03-3 microseconds
}
```

**Факторы увеличивающие window**:
1. **I/O operations**: Disk/Network access между check и use
2. **Memory allocation**: `new`/`malloc` между операциями
3. **System calls**: `syscall()` может вызвать context switch
4. **Logging**: Writing to log files
5. **Virtual calls**: Dynamic dispatch добавляет overhead

#### Exploitation техники

**Технология 1: Race spray**:
```cpp
// Атакующий запускает тысячи попыток
std::atomic<int> successes{0};

for (int i = 0; i < 10000; ++i) {
    std::thread victim([&]() {
        if (exploitTOCTOU()) {
            successes++;
        }
    });
    
    std::thread attacker([&]() {
        raceStateChange();
    });
    
    victim.join();
    attacker.join();
}

std::cout << "Успешных эксплойтов: " << successes << "/10000\n";
// Обычно 1-10% success rate
```

**Технология 2: Timing анализ для синхронизации**:
```cpp
// Замеряем время выполнения check
std::vector<uint64_t> timings;
for (int i = 0; i < 1000; ++i) {
    auto start = rdtsc();
    checkState();
    auto end = rdtsc();
    timings.push_back(end - start);
}

// Вычисляем среднее
uint64_t avg = std::accumulate(timings.begin(), timings.end(), 0ULL) / timings.size();

// Используем для precise timing attack
std::thread attacker([avg]() {
    while (true) {
        auto start = rdtsc();
        while (rdtsc() - start < avg / 2) {
            // Busy wait до середины check
        }
        changeState();  // Максимальная вероятность попадания в window
    }
});
```

**Технология 3: CPU pinning**:
```cpp
// Привязываем потоки к соседним CPU cores для увеличения вероятности
void pinThreadToCPU(std::thread& t, int cpu) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu, &cpuset);
    pthread_setaffinity_np(t.native_handle(), sizeof(cpu_set_t), &cpuset);
}

std::thread victim([]() { checkAndUseState(); });
std::thread attacker([]() { changeState(); });

pinThreadToCPU(victim, 0);    // CPU 0
pinThreadToCPU(attacker, 1);  // CPU 1 (соседний core)

// Соседние cores имеют меньшую латентность для синхронизации
```

#### Защита от TOCTOU

**Решение 1: Атомарная check-and-use**:
```cpp
class SecureAuth {
private:
    State state_;
    std::mutex mutex_;
    
public:
    void accessResource() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // Check и Use под одной блокировкой!
        if (state_ != AUTHENTICATED) {
            throw UnauthorizedException();
        }
        
        // Используем состояние БЕЗ разблокировки
        doAccessResource();
        
        // Mutex освобождается здесь
    }
};
```

**Решение 2: Snapshot pattern**:
```cpp
class SnapshotAuth {
private:
    std::atomic<State> state_;
    
public:
    void accessResource() {
        // Атомарный snapshot состояния
        State snapshot = state_.load(std::memory_order_acquire);
        
        if (snapshot != AUTHENTICATED) {
            throw UnauthorizedException();
        }
        
        // Работаем со snapshot, не с shared state
        doAccessResource(snapshot);
    }
};
```

**Решение 3: Token-based access**:
```cpp
class TokenBasedAuth {
private:
    std::atomic<uint64_t> auth_token_{0};
    
public:
    uint64_t authenticate() {
        uint64_t token = generateSecureToken();
        auth_token_.store(token, std::memory_order_release);
        return token;
    }
    
    void accessResource(uint64_t provided_token) {
        // Проверяем token, а не state
        uint64_t current_token = auth_token_.load(std::memory_order_acquire);
        
        if (provided_token != current_token || current_token == 0) {
            throw UnauthorizedException();
        }
        
        // Используем ресурс
        doAccessResource();
    }
    
    void logout() {
        auth_token_.store(0, std::memory_order_release);
    }
};

// Использование:
uint64_t token = auth.authenticate();
// token immutable - нет TOCTOU!
auth.accessResource(token);
```

### 4. Memory Leak в State Transitions (CWE-401)

#### Описание
Memory leak происходит когда старое состояние не удаляется при переходе в новое.

#### Проблема ручного управления памятью

**Уязвимый код**:
```cpp
class LeakyContext {
private:
    State* state_;  // Сырой указатель!
    
public:
    LeakyContext() : state_(new IdleState()) {}
    
    void transitionTo(State* new_state) {
        state_ = new_state;  // LEAK: старый state не удален!
    }
    
    ~LeakyContext() {
        delete state_;  // Удаляет только последний state
    }
};
```

**Leak accumulation**:
```
Переход 1: IdleState (A) → WorkingState (B)
  - A не удален (leaked)
  - state_ = B

Переход 2: WorkingState (B) → IdleState (C)  
  - B не удален (leaked)
  - state_ = C

После 1000 переходов:
  - 999 states leaked
  - Если каждый state = 1MB → 999MB leaked
```

#### Детектирование

**Valgrind Memcheck**:
```bash
valgrind --leak-check=full --show-leak-kinds=all \
    --track-origins=yes ./state_vulnerabilities

# Вывод:
LEAK SUMMARY:
   definitely lost: 10,485,760 bytes in 10 blocks
   indirectly lost: 0 bytes in 0 blocks
     possibly lost: 0 bytes in 0 blocks
   still reachable: 1,048,576 bytes in 1 blocks

10,485,760 bytes in 10 blocks are definitely lost in loss record 1 of 1
   at 0x4C2FB0F: operator new(unsigned long)
   by 0x400B23: LeakyContext::transitionTo(State*)
   by 0x400C89: demonstrateMemoryLeak()
```

**AddressSanitizer (LeakSanitizer)**:
   ```bash
g++ -fsanitize=address -g state_vulnerabilities.cpp
./a.out

# Вывод при exit:
=================================================================
==12345==ERROR: LeakSanitizer: detected memory leaks

Direct leak of 1048576 byte(s) in 1 object(s) allocated from:
    #0 0x7f8c operator new
    #1 0x400b23 in transitionTo
    #2 0x400c89 in demonstrateMemoryLeak

SUMMARY: AddressSanitizer: 10485760 byte(s) leaked in 10 allocation(s).
```

**Heaptrack (визуализация)**:
   ```bash
heaptrack ./state_vulnerabilities
heaptrack_gui heaptrack.state_vulnerabilities.12345.gz

# Показывает:
# - Flame graph allocation
# - Top allocators
# - Leak locations
```

#### Защита

**Решение 1: std::unique_ptr (Рекомендуется)**:
```cpp
class SafeContext {
private:
    std::unique_ptr<State> state_;
    
public:
    SafeContext() : state_(std::make_unique<IdleState>()) {}
    
    void transitionTo(std::unique_ptr<State> new_state) {
        // Старый state автоматически удалится!
        state_ = std::move(new_state);
    }
    
    // Деструктор автоматически удалит state_
};
```

**Решение 2: RAII State Guard**:
```cpp
class StateGuard {
private:
    Context& context_;
    std::unique_ptr<State> old_state_;
    
public:
    StateGuard(Context& ctx, std::unique_ptr<State> new_state)
        : context_(ctx) {
        
        // Вызываем exit на старом состоянии
        if (auto* current = context_.getCurrentState()) {
            current->exit();
        }
        
        old_state_ = context_.swapState(std::move(new_state));
        
        // Вызываем enter на новом состоянии
        if (auto* current = context_.getCurrentState()) {
            current->enter();
        }
    }
    
    ~StateGuard() {
        // old_state_ автоматически удалится
    }
};
```

**Решение 3: State Pool (для частых переходов)**:
```cpp
class StatePool {
private:
    std::map<std::string, std::unique_ptr<State>> pool_;
    
public:
    template<typename T>
    State* getOrCreate(const std::string& name) {
        auto it = pool_.find(name);
        if (it == pool_.end()) {
            auto state = std::make_unique<T>();
            auto* ptr = state.get();
            pool_[name] = std::move(state);
            return ptr;
        }
        return it->second.get();
    }
};

class PooledContext {
private:
    StatePool pool_;
    State* current_state_;
    
public:
    void transitionTo(const std::string& state_name) {
        if (state_name == "Idle") {
            current_state_ = pool_.getOrCreate<IdleState>("Idle");
        } else if (state_name == "Working") {
            current_state_ = pool_.getOrCreate<WorkingState>("Working");
        }
        // States reused - нет аллокаций при каждом переходе
    }
};
```

### 5. State Pollution (CWE-362)

#### Описание
State pollution происходит когда состояние одного контекста "загрязняет" другой контекст в shared или pooled environments.

#### Проблема Shared Context

**Web Application сценарий**:
```cpp
// Глобальный session context (ОПАСНО!)
SessionContext global_session;

void handleRequest(HttpRequest& req, HttpResponse& res) {
    // УЯЗВИМОСТЬ: Все запросы используют одинаковый context
    
    // Request 1 (User A):
    global_session.setUser("userA");
    global_session.setState(AUTHENTICATED);
    processUserA();
    
    // Request 2 (User B) использует тот же context:
    // global_session.user = "userA"  ← POLLUTION!
    // global_session.state = AUTHENTICATED  ← POLLUTION!
    processUserB();  // Выполняется как UserA!
}
```

**Что происходит**:
```
Time    Request              Context State         Security Impact
------------------------------------------------------------------------
T0      User A login         user=A, auth=true     -
T1      User A reads data    user=A, auth=true     Legitimate
T2      User B request       user=A, auth=true     🚨 Wrong user!
T3      User B reads data    user=A, auth=true     🚨 Data leak!
T4      User C request       user=A, auth=true     🚨 Session hijack!
```

#### Exploitation: Session Fixation через State Pollution

**Сценарий**:
```cpp
// Web server с pooled sessions
std::vector<SessionContext*> session_pool(100);

SessionContext* getSession(const std::string& session_id) {
    size_t index = std::hash<std::string>{}(session_id) % session_pool.size();
    return session_pool[index];  // Может вернуть тот же объект!
}

// АТАКА
// 1. Атакующий создает session
SessionContext* attacker_session = getSession("attacker_session_id");
attacker_session->login("attacker", "password");
attacker_session->setState(PRIVILEGED);

// 2. Атакующий подбирает session_id жертвы который даст тот же pool index
for (std::string candidate : brute_force_session_ids()) {
    if (std::hash<std::string>{}(candidate) % 100 == 
        std::hash<std::string>{}("attacker_session_id") % 100) {
        
        // Найден collision!
        victim_session_id = candidate;
        break;
    }
}

// 3. Жертва использует предсказанный session_id
SessionContext* victim_session = getSession(victim_session_id);
// victim_session == attacker_session (тот же объект!)
// Жертва получает privileged state от атакующего!
```

#### Защита

**Решение 1: Thread-local storage**:
```cpp
class SessionManager {
private:
    thread_local static SessionContext* current_session_;
    
public:
    static SessionContext* getCurrentSession() {
        if (!current_session_) {
            current_session_ = new SessionContext();
        }
        return current_session_;
    }
};

// Каждый thread имеет свой context
void handleRequest() {
    SessionContext* session = SessionManager::getCurrentSession();
    // Нет pollution между threads
}
```

**Решение 2: Request-scoped Context**:
```cpp
class RequestScopedContext {
public:
    void handleRequest(HttpRequest& req, HttpResponse& res) {
        // Новый context для каждого request
        SessionContext session;
        
        session.setUser(req.getUser());
        session.authenticate();
        
        processRequest(session);
        
        // Context удаляется при выходе из scope
    }
};
```

**Решение 3: Immutable State Copy**:
```cpp
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
    
    ImmutableSession withLoggedOut() const {
        return ImmutableSession(user_, State::UNAUTHENTICATED);
    }
    
    // Нет методов изменения - immutable!
};

// Использование:
auto session1 = ImmutableSession::create("userA")
                    .withAuthenticated();

auto session2 = ImmutableSession::create("userB");
// session1 и session2 полностью независимы
```

## Дополнительные уязвимости

### 6. State Injection Attack

Атакующий манипулирует external state (cookies, session storage) для установки недопустимого состояния.

**Пример**:
```cpp
// Сериализация state в cookie
setCookie("state", serializeState(current_state));

// АТАКА: Модификация cookie
// state=ADMIN вместо state=USER
```

**Защита**: Подпись state с помощью HMAC.

### 7. Deadlock в State Transitions

Несколько состояний удерживают блокировки в разном порядке.

**Пример**:
```cpp
void StateA::transitionTo(StateB* b) {
    std::lock_guard l1(mutex_a_);
    std::lock_guard l2(mutex_b_);  // Deadlock risk
}

void StateB::transitionTo(StateA* a) {
    std::lock_guard l2(mutex_b_);
    std::lock_guard l1(mutex_a_);  // Reverse order!
}
```

### 8. Reentrancy Issues

State transition вызывается рекурсивно.

**Пример**:
```cpp
void State::handle() {
    context_->setState(new AnotherState());  // Recursive transition
    // Может вызвать stack overflow
}
```

## Инструменты анализа (расширенный)

### Статический анализ

#### State Machine Verification с Model Checking

**SPIN Model Checker**:
```promela
// state_machine.pml
mtype = { CREATED, PAID, SHIPPED, DELIVERED, CANCELLED };
mtype state = CREATED;

active proctype Order() {
    do
    :: state == CREATED -> state = PAID;
    :: state == PAID -> state = SHIPPED;
    :: state == SHIPPED -> state = DELIVERED;
    :: state == CREATED || state == PAID -> state = CANCELLED;
    od
}

// Проверка свойств
ltl no_double_refund { []!(state == CANCELLED && refunded > 1) }
ltl valid_delivery { [](state == DELIVERED -> previous_state == SHIPPED) }
```

```bash
spin -a state_machine.pml
gcc -o pan pan.c
./pan -a  # Проверка всех свойств
```

#### ThreadSanitizer Suppression для ложных срабатываний

**tsan.supp**:
```
# Suppress benign races in logging
race:Logger::log

# Suppress races in statistics counters
race:Statistics::increment
```

```bash
export TSAN_OPTIONS="suppressions=tsan.supp"
./state_tsan
```

### Динамический анализ

#### Race Fuzzing

```cpp
// race_fuzzer.cpp - Fuzzing для поиска race conditions
#include "state.h"
#include <random>

void fuzz_state_machine(const uint8_t* data, size_t size) {
    if (size < 10) return;
    
    Context context;
    std::vector<std::thread> threads;
    
    // Интерпретируем fuzzing data как последовательность операций
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
}

// Компиляция с libFuzzer + TSan
// clang++ -fsanitize=fuzzer,thread race_fuzzer.cpp -o race_fuzzer
// ./race_fuzzer -workers=8 -max_total_time=300
```

## Практические задания (углубленные)

### Задание 1: Обнаружение Race Condition

**Задача**: Найдите data race в многопоточном State machine.

**Код для анализа**:
```cpp
class VulnerableStateMachine {
    State* state_;
public:
    void transitionTo(State* s) { delete state_; state_ = s; }
    void handle() { state_->action(); }
};
```

**Шаги**:
```bash
# 1. Компиляция с TSan
g++ -fsanitize=thread -g state_vulnerabilities.cpp -o task1_tsan

# 2. Запуск stress test
for i in {1..100}; do ./task1_tsan; done

# 3. Анализ вывода TSan
# Найдите:
# - Адреса конфликтующих операций
# - Stack traces обоих threads
# - Тип race (write-write, read-write)
```

**Вопросы**:
1. Какие две операции конфликтуют?
2. Какой размер race window (в инструкциях)?
3. Как исправить: mutex, atomic, или lock-free?
4. Есть ли риск deadlock в решении с mutex?

### Задание 2: Exploit TOCTOU для Privilege Escalation

**Задача**: Создайте working exploit для обхода проверки прав.

**Базовый код**:
```cpp
if (user->getRole() == ADMIN) {  // Check
    performAdminAction();  // Use
}
```

**Задачи**:
1. Измерьте размер race window (rdtsc)
2. Создайте многопоточный exploit с success rate > 10%
3. Оптимизируйте exploit (CPU pinning, timing)
4. Измерьте impact (сколько unauthorized actions)

**Метрики для отчета**:
- Attempts: ________
- Successes: ________
- Success rate: ________%
- Average race window: ________ CPU cycles
- Impact: ________ unauthorized operations

### Задание 3: Construct Invalid State Machine

**Задача**: Приведите state machine в недопустимое состояние.

**FSM**:
```
CREATED → PAID → SHIPPED → DELIVERED
   ↓         ↓         ↓
   └─────────┴─────────┴──→ CANCELLED
```

**Задачи**:
1. Перейдите из DELIVERED в PAID (недопустимо)
2. Получите double refund
3. Ship без payment
4. Cancel после delivery

**Каждая успешная атака дает баллы**:
- Invalid transition: 10 points
- Double refund: 20 points
- Free shipping: 30 points
- Post-delivery cancel: 40 points

### Задание 4: Memory Leak Hunt

**Задача**: Найдите и измерьте memory leak в State transitions.

**Шаги**:
```bash
# 1. Запуск с Valgrind
valgrind --leak-check=full --log-file=leak.log ./state_vulnerabilities

# 2. Анализ leak summary
grep "definitely lost" leak.log

# 3. Определите leak rate
# Leak rate = leaked_bytes / number_of_transitions

# 4. Проецирование: сколько времени до OOM?
# Time to OOM = (Available RAM) / (Leak rate * Transitions per second)
```

**Отчет**:
- Total leaked: ________ bytes
- Number of transitions: ________
- Leak rate: ________ bytes/transition
- Time to OOM (at 1000 trans/sec): ________ seconds

### Задание 5: Build Exploit Chain

**Задача**: Объедините несколько уязвимостей для полной компрометации.

**Chain**:
1. TOCTOU → Authentication bypass
2. Invalid transition → Privilege escalation
3. State pollution → Session hijacking
4. Race condition → Data corruption

**Сценарий**:
```cpp
// Шаг 1: Bypass auth через TOCTOU
exploitTOCTOU();  // Получить authenticated state

// Шаг 2: Transition в admin state
invalidTransition(USER, ADMIN);

// Шаг 3: Pollute другие sessions
propagateAdminState();

// Шаг 4: Extract credentials
extractDataViaRace();
```

## Архитектурные паттерны для безопасности

### 1. Hierarchical State Machine (HSM)

```cpp
class HierarchicalStateMachine {
private:
    State* current_state_;
    State* parent_state_;  // Иерархия
    
public:
    void transitionTo(State* new_state) {
        // Exit hierarchy: current → parent → root
        exitHierarchy(current_state_);
        
        // Enter hierarchy: root → parent → new
        enterHierarchy(new_state);
        
        current_state_ = new_state;
    }
};
```

**Безопасность**: Гарантирует правильный exit/enter порядок.

### 2. State History Pattern

```cpp
class StateHistory {
private:
    std::stack<std::unique_ptr<State>> history_;
    std::unique_ptr<State> current_;
    
public:
    void transitionTo(std::unique_ptr<State> new_state) {
        history_.push(std::move(current_));
        current_ = std::move(new_state);
    }
    
    void rollback() {
        if (!history_.empty()) {
            current_ = std::move(history_.top());
            history_.pop();
        }
    }
    
    // Audit trail для security analysis
    std::vector<std::string> getTransitionHistory() const;
};
```

**Безопасность**: Audit trail для forensics.

### 3. Guarded State Transitions

```cpp
class GuardedState {
public:
    virtual bool canTransitionTo(const State& target) const = 0;
};

class GuardedContext {
    void transitionTo(std::unique_ptr<GuardedState> new_state) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // Проверка guard condition
        if (!current_state_->canTransitionTo(*new_state)) {
            throw InvalidTransitionException();
        }
        
        current_state_ = std::move(new_state);
    }
};
```

## Связь с другими паттернами

### Strategy Pattern
Strategy можно рассматривать как stateless version of State.

**Security considerations**:
- State: Mutable, требует синхронизацию
- Strategy: Immutable, thread-safe by default

### Memento Pattern  
Memento сохраняет состояние State для undo/redo.

**Уязвимости**:
- Sensitive data в memento snapshots
- Memento tampering для state manipulation

### Observer Pattern
State transitions часто trigger Observer notifications.

**Уязвимости**:
- Observer может изменить state во время transition
- Iterator invalidation в списке observers

### Command Pattern
Commands могут инкапсулировать state transitions.

**Уязвимости**:
- Command replay → повторные переходы
- Command injection → недопустимые переходы

## Compliance и стандарты

### OWASP Top 10 2021

**A01:2021 - Broken Access Control**:
- Invalid state transitions → Permission bypass
- State confusion → Privilege escalation

**A04:2021 - Insecure Design**:
- Missing state transition validation
- No finite state machine enforcement

### CWE Top 25

**CWE-362: Concurrent Execution using Shared Resource with Improper Synchronization** (#11):
- Race conditions в state transitions
- TOCTOU в state checking

**CWE-662: Improper Synchronization** (#18):
- Неатомарные state operations
- Missing mutex protection

### ISO/IEC 27001:2013

**A.14.2.5**: Secure system engineering principles
- State machine должна быть formally verified
- All transitions должны быть validated

### MISRA C++:2008

**Rule 6-4-4**: A switch expression must have explicit handling for all states
**Rule 6-4-5**: An unconditional throw or break statement shall terminate every non-empty switch-clause

## Чек-лист безопасности для State Pattern

### Design Phase
- [ ] FSM formally specified (UML State Diagram)
- [ ] All transitions documented и validated
- [ ] Security properties defined (invariants)
- [ ] Threat model created
- [ ] Sensitive states identified

### Implementation Phase
- [ ] Используется std::unique_ptr для владения states
- [ ] Все state transitions защищены мьютексами или atomic
- [ ] Transition validation выполняется перед actual переходом
- [ ] Нет прямого доступа к state_ (encapsulation)
- [ ] Enter/exit методы вызываются корректно
- [ ] Исключения обрабатываются без corruption state
- [ ] Memory leaks предотвращены (RAII)

### Testing Phase
- [ ] Unit tests для всех transitions
- [ ] Negative tests для invalid transitions
- [ ] Race condition tests (TSan)
- [ ] Memory leak tests (Valgrind)
- [ ] Stress testing (100+ concurrent threads)
- [ ] Fuzzing для поиска corner cases
- [ ] Security penetration testing

### Deployment Phase
- [ ] State transitions логируются для audit
- [ ] Monitoring для обнаружения anomalies
- [ ] Rate limiting для критических transitions
- [ ] Incident response plan для state compromise

## Дополнительные ресурсы

### Книги
- "Concurrent Programming in C++" - Anthony Williams
- "UML State Machine" - Samek Miro
- "Formal Methods" - Jean-François Monin

### Статьи
- [CWE-362: Race Condition](https://cwe.mitre.org/data/definitions/362.html)
- [CWE-367: TOCTOU](https://cwe.mitre.org/data/definitions/367.html)
- [CWE-662: Improper Synchronization](https://cwe.mitre.org/data/definitions/662.html)

### Инструменты
- [ThreadSanitizer](https://github.com/google/sanitizers/wiki/ThreadSanitizerCppManual)
- [SPIN Model Checker](http://spinroot.com/)
- [TLA+](https://lamport.azurewebsites.net/tla/tla.html) для формальной верификации

## Заключение

State паттерн требует особого внимания к безопасности из-за temporal nature состояний и множественных точек перехода. Основные риски:

**🔴 Критические**:
- **Race Conditions** → Data corruption, UAF, Undefined Behavior
- **TOCTOU** → Authentication bypass, Privilege escalation
- **Invalid Transitions** → Business logic bypass, Financial fraud

**🟡 Высокие**:
- **Memory Leaks** → Resource exhaustion, DoS
- **State Pollution** → Session hijacking, Data leakage
- **State Injection** → State manipulation attacks

**🟢 Средние**:
- **Deadlocks** → Denial of Service
- **Reentrancy** → Stack overflow, Logic errors

**Ключевые принципы защиты**:

1. **Atomic Operations** - Check-and-use должны быть атомарны
2. **Transition Validation** - Используйте transition table для проверки допустимости
3. **Synchronization** - Защищайте state мьютексами или atomics
4. **RAII** - Гарантируйте правильный enter/exit и cleanup
5. **Immutability** - Где возможно, используйте immutable states
6. **Logging** - Audit trail всех transitions для forensics
7. **Testing** - Обязательно тестируйте с TSan и stress tests

**Золотое правило State безопасности**:
> "Состояние - это shared mutable data. Относитесь к каждому переходу как к критической секции, требующей синхронизации и валидации."

**Архитектурная рекомендация**:

```cpp
// ПЛОХО: Mutable state в shared context
class SharedContext {
    State* state_;  // Опасно!
public:
    void setState(State* s) { state_ = s; }  // Race!
};

// ХОРОШО: Immutable state transitions
class ImmutableContext {
    const State state_;
    
    ImmutableContext(State s) : state_(s) {}
public:
    ImmutableContext transitionTo(State new_state) const {
        if (!isValidTransition(state_, new_state)) {
            throw InvalidTransitionException();
        }
        return ImmutableContext(new_state);  // Новый объект
    }
};
```

Помните: В многопоточных системах, **любое shared mutable state - это уязвимость по умолчанию**, пока не доказано обратное через правильную синхронизацию!
