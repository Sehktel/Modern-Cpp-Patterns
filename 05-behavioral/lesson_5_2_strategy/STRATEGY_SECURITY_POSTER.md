# 🛡️ Security Poster: Strategy Pattern

## 🎯 Обзор угроз Strategy Pattern

```
┌────────────────────────────────────────────────────────────────────┐
│             КРИТИЧЕСКИЕ УЯЗВИМОСТИ STRATEGY PATTERN                │
└────────────────────────────────────────────────────────────────────┘

🔴 КРИТИЧНЫЕ УГРОЗЫ:
┌─────────────────────┬──────────────────────────────────────────────┐
│ Type Confusion      │ CWE-843: Неправильное приведение типов       │
│                     │ Impact: Memory corruption, RCE, information  │
│                     │         disclosure                            │
├─────────────────────┼──────────────────────────────────────────────┤
│ Null Pointer        │ CWE-476: Стратегия не инициализирована       │
│ Dereference         │ Impact: Segmentation fault, DoS, crash       │
├─────────────────────┼──────────────────────────────────────────────┤
│ Strategy Injection  │ Malicious strategy замена в runtime          │
│                     │ Impact: Logic bypass, privilege escalation   │
└─────────────────────┴──────────────────────────────────────────────┘

🟡 ВЫСОКИЕ УГРОЗЫ:
┌─────────────────────┬──────────────────────────────────────────────┐
│ Undefined Behavior  │ CWE-758: Virtual function call на bad object │
│ Strategy Leaks      │ CWE-401: Неправильное владение стратегией    │
│ Race Conditions     │ CWE-362: Concurrent strategy change          │
└─────────────────────┴──────────────────────────────────────────────┘
```

## 💣 Критическая уязвимость #1: Type Confusion

### Анатомия атаки

```cpp
// ❌ УЯЗВИМЫЙ КОД - Unsafe downcasting
class VulnerableContext {
private:
    Strategy* strategy_;  // ⚠️ Base pointer
    
public:
    void executeAlgorithm() {
        // ⚠️ ОПАСНО: Unsafe downcast без проверки типа
        auto* concrete = static_cast<ConcreteStrategyA*>(strategy_);
        concrete->specificMethodA();  // 💥 Type confusion!
    }
    
    void processData(void* data) {
        // ⚠️ Предполагаем определенный тип
        auto* concrete = reinterpret_cast<ConcreteStrategyB*>(strategy_);
        concrete->handleData(static_cast<DataType*>(data));  // 💥 Type confusion!
    }
};
```

### 🎯 Exploitation: Type confusion для RCE

```cpp
// ЭКСПЛОЙТ: Type confusion attack
class AttackerStrategy : public Strategy {
public:
    // Эта структура имитирует ConcreteStrategyA
    char padding[64];  // Padding для совпадения layout
    void (*function_ptr)(void*);  // Fake function pointer
    
    void execute() override {
        // Harmless implementation
    }
};

void exploit_type_confusion() {
    VulnerableContext context;
    
    // 1. Устанавливаем AttackerStrategy
    AttackerStrategy* malicious = new AttackerStrategy();
    malicious->function_ptr = (void(*)(void*))0xdeadbeef;  // Shellcode address
    
    context.setStrategy(malicious);
    
    // 2. Вызываем метод, который делает unsafe downcast
    context.executeAlgorithm();
    // ↓
    // static_cast<ConcreteStrategyA*>(strategy_)
    // ↓
    // Интерпретирует AttackerStrategy как ConcreteStrategyA
    // ↓
    // specificMethodA() читает function_ptr как vtable
    // ↓
    // 💥 Переход на 0xdeadbeef → RCE!
}
```

### Memory layout при type confusion

```
┌──────────────────────────────────────────────────────────────────┐
│                  TYPE CONFUSION MEMORY LAYOUT                    │
└──────────────────────────────────────────────────────────────────┘

Normal ConcreteStrategyA layout:
0x00: [vtable ptr]     → ConcreteStrategyA vtable
0x08: [member1]        → int value
0x10: [member2]        → std::string data
0x18: [member3]        → function ptr

AttackerStrategy with same layout:
0x00: [vtable ptr]     → AttackerStrategy vtable  ✅ Legitimate
0x08: [padding]        → Controlled data
0x10: [padding]        → Controlled data
0x18: [function_ptr]   → 0xdeadbeef (shellcode)  ⚠️

После static_cast<ConcreteStrategyA*>:
- Compiler думает что это ConcreteStrategyA
- specificMethodA() читает offset 0x18
- Вместо legitimate function ptr → 0xdeadbeef
- 💥 Call 0xdeadbeef → Remote Code Execution
```

### ✅ Защита: Safe downcasting с dynamic_cast

```cpp
// ✅ БЕЗОПАСНЫЙ КОД - RTTI проверка типа
class SafeContext {
private:
    std::unique_ptr<Strategy> strategy_;
    
public:
    void executeAlgorithm() {
        // ✅ Safe downcast с проверкой типа
        if (auto* concrete = dynamic_cast<ConcreteStrategyA*>(strategy_.get())) {
            concrete->specificMethodA();  // Безопасно
        } else {
            throw std::runtime_error("Invalid strategy type");
        }
    }
    
    // Альтернатива: Visitor pattern для type-safe dispatch
    void executeTypeSafe() {
        strategy_->accept(*this);  // Double dispatch
    }
};

// Еще лучше: Используйте std::variant для type-safe strategies
using SafeStrategy = std::variant<ConcreteStrategyA, ConcreteStrategyB>;

class VariantContext {
    SafeStrategy strategy_;
    
    void execute() {
        std::visit([](auto&& strategy) {
            strategy.execute();  // Type-safe dispatch
        }, strategy_);
    }
};
```

## 💣 Критическая уязвимость #2: Null Pointer Dereference

### Анатомия атаки

```cpp
// ❌ УЯЗВИМЫЙ КОД - Отсутствие проверки на nullptr
class NullableContext {
private:
    Strategy* strategy_ = nullptr;  // ⚠️ Default nullptr
    
public:
    void setStrategy(Strategy* s) {
        strategy_ = s;
    }
    
    void execute() {
        strategy_->execute();  // ⚠️ Нет проверки на nullptr!
    }
    
    void processData(int value) {
        int result = strategy_->calculate(value);  // 💥 NPD!
        useResult(result);
    }
};
```

### 🎯 Exploitation: NPD для DoS

```cpp
// ЭКСПЛОЙТ: Null pointer dereference
void exploit_null_pointer() {
    NullableContext context;
    // НЕ устанавливаем strategy (остается nullptr)
    
    // 1. DoS через crash
    context.execute();  // 💥 Segmentation fault
    
    // 2. В некоторых архитектурах - контроль RIP
    // Если nullptr dereference на конкретном offset:
    //   strategy_->execute() → (*strategy_->vtable[0])()
    //   → Read from 0x0 + vtable_offset
    //   → В некоторых системах 0x0 может быть mapped
    //   → Potential RCE (редко, но возможно)
}

// Более реалистичный exploit: Logic bypass
void exploit_logic_bypass() {
    AuthContext auth_context;
    // auth_context.strategy_ = nullptr
    
    // Предполагаем что есть fallback logic:
    if (auth_context.hasStrategy()) {  // Проверяет != nullptr
        auth_context.execute();  // Строгая аутентификация
    } else {
        // 💥 Fallback path может быть менее secure
        authenticateWithWeakStrategy();
    }
}
```

### Null pointer в виртуальных вызовах

```
┌──────────────────────────────────────────────────────────────────┐
│                NULL POINTER VIRTUAL CALL CRASH                   │
└──────────────────────────────────────────────────────────────────┘

Нормальный вызов:
strategy_->execute()
↓
1. Read strategy_ → 0x7f8c00001000 (valid object)
2. Read vtable from object → 0x7f8c00002000
3. Read function ptr from vtable → 0x400b12
4. Call 0x400b12 ✅

Null pointer dereference:
strategy_->execute()
↓
1. Read strategy_ → 0x0 (NULL)
2. Read vtable from 0x0 → 💥 SEGFAULT
   или
   Read from 0x0 + vtable_offset → 💥 SEGFAULT

Assembly:
mov    rax, QWORD PTR [rbp-0x18]    ; Load strategy_
mov    rax, QWORD PTR [rax]         ; Load vtable (crash if rax=0)
mov    rdx, QWORD PTR [rax+0x10]    ; Load function pointer
call   rdx                          ; Call function
```

### ✅ Защита: Null-safe strategy pattern

```cpp
// ✅ БЕЗОПАСНЫЙ КОД - Null Object Pattern
class NullStrategy : public Strategy {
public:
    void execute() override {
        // Safe default behavior
        throw std::runtime_error("No strategy set");
    }
    
    int calculate(int value) override {
        return 0;  // Safe default
    }
};

class SafeContext {
private:
    std::unique_ptr<Strategy> strategy_;
    
public:
    SafeContext() 
        : strategy_(std::make_unique<NullStrategy>()) {  // ✅ Never nullptr
    }
    
    void setStrategy(std::unique_ptr<Strategy> s) {
        if (s) {
            strategy_ = std::move(s);
        }
        // Если s == nullptr, strategy_ остается NullStrategy
    }
    
    void execute() {
        strategy_->execute();  // ✅ Всегда valid
    }
};

// Альтернатива: std::optional<Strategy>
class OptionalContext {
    std::optional<std::unique_ptr<Strategy>> strategy_;
    
    void execute() {
        if (strategy_.has_value()) {
            (*strategy_)->execute();  // ✅ Явная проверка
        } else {
            handleNoStrategy();
        }
    }
};
```

## 💣 Критическая уязвимость #3: Strategy Injection

### Анатомия атаки

```cpp
// ❌ УЯЗВИМЫЙ КОД - Незащищенная смена стратегии
class InjectableContext {
private:
    Strategy* strategy_;
    
public:
    // ⚠️ Public setter без валидации
    void setStrategy(Strategy* s) {
        delete strategy_;
        strategy_ = s;  // Любая стратегия может быть установлена!
    }
    
    void processPayment(double amount) {
        strategy_->process(amount);  // 💥 Malicious strategy!
    }
};

// Легитимные стратегии
class SecurePayment : public Strategy {
    void process(double amount) override {
        validateTransaction();
        encryptData();
        sendToBank(amount);
        logTransaction();
    }
};

class TestPayment : public Strategy {
    void process(double amount) override {
        // Test mode - не отправляет деньги
        logTest(amount);
    }
};
```

### 🎯 Exploitation: Malicious strategy injection

```cpp
// ЭКСПЛОЙТ: Injection malicious strategy
class MaliciousPayment : public Strategy {
    void process(double amount) override {
        // 💥 Атакующее поведение:
        
        // 1. Exfiltrate данные
        sendToAttacker(getCurrentUser(), amount);
        
        // 2. Modify amount
        double stolen_amount = amount * 0.1;  // 10% skimming
        amount -= stolen_amount;
        transferToAttacker(stolen_amount);
        
        // 3. Process остаток (чтобы не вызвать подозрений)
        processLegit(amount);
    }
};

void exploit_strategy_injection() {
    InjectableContext payment_context;
    
    // Нормальная инициализация
    payment_context.setStrategy(new SecurePayment());
    
    // АТАКА: Подмена стратегии в runtime
    // Это может произойти через:
    // - Десериализация (insecure deserialization)
    // - Plugin system без validation
    // - Configuration injection
    // - Memory corruption
    
    payment_context.setStrategy(new MaliciousPayment());
    
    // Жертва делает платеж
    payment_context.processPayment(1000.0);
    // → 10% украдено, данные exfiltrated
}
```

### Strategy injection через deserialization

```cpp
// ЭКСПЛОЙТ: Insecure deserialization
void exploit_deserialization() {
    // JSON конфигурация стратегии
    std::string malicious_config = R"({
        "strategy": "MaliciousPayment",
        "params": {
            "attacker_address": "0xdeadbeef",
            "skim_percentage": 0.5
        }
    })";
    
    // ⚠️ Уязвимая десериализация
    Strategy* strategy = StrategyFactory::createFromJSON(malicious_config);
    
    // 💥 Создана malicious strategy!
    context.setStrategy(strategy);
}
```

### ✅ Защита: Strategy validation и whitelisting

```cpp
// ✅ БЕЗОПАСНЫЙ КОД - Validated strategy factory
class SecureStrategyFactory {
private:
    // Whitelist разрешенных стратегий
    static const std::set<std::string> ALLOWED_STRATEGIES;
    
    // Registry для контроля
    std::map<std::string, std::function<std::unique_ptr<Strategy>()>> registry_;
    
public:
    SecureStrategyFactory() {
        // Регистрация ТОЛЬКО безопасных стратегий
        registry_["secure_payment"] = []() {
            return std::make_unique<SecurePayment>();
        };
        registry_["test_payment"] = []() {
            return std::make_unique<TestPayment>();
        };
        // MaliciousPayment НЕ в registry!
    }
    
    std::unique_ptr<Strategy> create(const std::string& name) {
        // Валидация против whitelist
        if (ALLOWED_STRATEGIES.find(name) == ALLOWED_STRATEGIES.end()) {
            throw std::runtime_error("Strategy not whitelisted: " + name);
        }
        
        // Проверка в registry
        auto it = registry_.find(name);
        if (it == registry_.end()) {
            throw std::runtime_error("Unknown strategy: " + name);
        }
        
        // Audit log
        logStrategyCreation(name);
        
        return it->second();
    }
};

class ProtectedContext {
private:
    std::unique_ptr<Strategy> strategy_;
    SecureStrategyFactory& factory_;
    
public:
    void setStrategy(const std::string& strategy_name) {
        // ✅ Создание через validated factory
        strategy_ = factory_.create(strategy_name);
        
        // ✅ Signature verification (если из конфигурации)
        if (!verifySignature(strategy_name)) {
            throw std::runtime_error("Strategy signature invalid");
        }
    }
};
```

## 🛠️ Инструменты детектирования

### 1. UndefinedBehaviorSanitizer - Type Confusion Detection

```bash
# Компиляция с UBSan
g++ -fsanitize=undefined -g strategy_vulnerabilities.cpp -o strategy_ubsan

./strategy_ubsan
```

**Типичный вывод UBSan**:
```
strategy.cpp:45:12: runtime error: downcast of address 0x602000000010 
which does not point to an object of type 'ConcreteStrategyA'
0x602000000010: note: object is of type 'AttackerStrategy'
 00 00 00 00  vptr for 'AttackerStrategy'
              ^~~~~~~~~~~~~~~~~~~~~~~
```

### 2. AddressSanitizer - NPD Detection

```bash
g++ -fsanitize=address -g strategy_vulnerabilities.cpp -o strategy_asan

./strategy_asan
```

**Типичный вывод ASan**:
```
SUMMARY: AddressSanitizer: SEGV on unknown address 0x000000000000
    #0 0x400b12 in Context::execute() strategy.cpp:67
    #1 0x400c34 in main strategy.cpp:142

AddressSanitizer can not describe address in more detail 
(wild memory access suspected).
```

### 3. Static Analysis - Clang-Tidy

```bash
# Проверка с clang-tidy
clang-tidy strategy_vulnerabilities.cpp \
    -checks='cppcoreguidelines-*,clang-analyzer-*' \
    -- -std=c++17

# Warnings:
# strategy.cpp:45:5: warning: do not use static_cast to downcast from 
# a base to a derived class; use dynamic_cast instead [cppcoreguidelines-pro-type-static-cast-downcast]
#     auto* concrete = static_cast<ConcreteStrategyA*>(strategy_);
#     ^
```

### 4. Fuzzing для Strategy Injection

```cpp
// strategy_fuzzer.cpp
#include <libfuzzer/FuzzerInterface.h>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if (size < 4) return 0;
    
    // Интерпретируем fuzzing data как strategy selection
    uint32_t strategy_id = *reinterpret_cast<const uint32_t*>(data);
    
    Context context;
    
    try {
        // Попытка создать стратегию по ID
        Strategy* strategy = StrategyFactory::create(strategy_id);
        context.setStrategy(strategy);
        context.execute();
    } catch (const std::exception&) {
        // Expected для invalid IDs
    }
    
    return 0;
}
```

```bash
# Компиляция и запуск fuzzer
clang++ -fsanitize=fuzzer,address,undefined \
        strategy_fuzzer.cpp strategy.cpp -o strategy_fuzzer

./strategy_fuzzer -max_total_time=300
```

## 📋 Security Checklist для Strategy Pattern

### Design Phase
```
[ ] Определены все допустимые стратегии (whitelist)
[ ] Документирован контракт Strategy interface
[ ] Определена ownership policy (кто владеет strategy)
[ ] Рассмотрены scenarios для strategy injection
[ ] Threat model создана
```

### Implementation Phase
```
[ ] Используется std::unique_ptr для владения strategy
[ ] Null Object Pattern или validation для nullptr
[ ] Нет unsafe downcasting (static_cast/reinterpret_cast)
[ ] Strategy factory с whitelist validation
[ ] Signature verification для external strategies
[ ] Нет сырых указателей на strategy
[ ] Thread-safe strategy change (если multithreading)
```

### Testing Phase
```
[ ] Unit tests для всех strategies
[ ] Negative tests (nullptr, invalid types)
[ ] Type confusion tests с UBSan
[ ] NPD tests с ASan
[ ] Fuzzing для strategy selection logic
[ ] Penetration testing для injection attacks
```

### Deployment Phase
```
[ ] Logging для strategy changes (audit trail)
[ ] Monitoring для abnormal strategy usage
[ ] Rate limiting для strategy changes
[ ] Configuration validation перед загрузкой
```

## 🎓 Best Practices

### 1. Type-Safe Strategy с std::variant (C++17)

```cpp
// ✅ Type-safe strategies без virtual functions
class StrategyA {
public:
    void execute() { /* ... */ }
};

class StrategyB {
public:
    void execute() { /* ... */ }
};

using Strategy = std::variant<StrategyA, StrategyB>;

class TypeSafeContext {
    Strategy strategy_;
    
public:
    void execute() {
        std::visit([](auto&& s) {
            s.execute();  // ✅ Type-safe dispatch
        }, strategy_);
    }
};
```

### 2. Policy-Based Design для Compile-Time Strategies

```cpp
// ✅ Compile-time strategy selection (zero runtime overhead)
template<typename SortStrategy>
class Container {
    SortStrategy strategy_;
    
public:
    void sort(std::vector<int>& data) {
        strategy_.sort(data);  // ✅ Compile-time dispatch
    }
};

// Использование
Container<QuickSort> quick_container;
Container<MergeSort> merge_container;
// Type safety гарантирована на compile time
```

### 3. Immutable Strategy Pattern

```cpp
// ✅ Immutable strategies для thread-safety
class ImmutableContext {
private:
    const std::unique_ptr<const Strategy> strategy_;
    
public:
    ImmutableContext(std::unique_ptr<Strategy> s)
        : strategy_(std::move(s)) {}
    
    // Нет setStrategy() - immutable после конструкции
    
    void execute() const {
        strategy_->execute();
    }
};
```

## 🚨 CVE Examples - Real-World Impact

```
┌────────────────────────────────────────────────────────────────┐
│              ИЗВЕСТНЫЕ CVE С STRATEGY VULNERABILITIES          │
└────────────────────────────────────────────────────────────────┘

CVE-2019-12384 (Jackson Databind)
├─ Тип: Polymorphic deserialization → RCE
├─ CVSS: 9.8 (CRITICAL)
├─ Impact: Remote Code Execution через strategy injection
└─ Причина: Deserialization создает malicious strategies

CVE-2020-9484 (Apache Tomcat)
├─ Тип: Insecure deserialization в session strategy
├─ CVSS: 7.0 (HIGH)
├─ Impact: RCE через session persistence strategy
└─ Причина: Session strategy из untrusted source

CVE-2021-44228 (Log4Shell)
├─ Тип: JNDI Lookup strategy injection
├─ CVSS: 10.0 (CRITICAL)
├─ Impact: RCE через lookup strategy injection
└─ Причина: Dynamic strategy loading без validation
```

## 📚 Дополнительные ресурсы

### Стандарты
- **CWE-843**: Access of Resource Using Incompatible Type
- **CWE-476**: NULL Pointer Dereference
- **CWE-502**: Deserialization of Untrusted Data
- **CERT C++**: EXP50-CPP, OOP51-CPP

### Инструменты
- UndefinedBehaviorSanitizer: https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html
- Clang-Tidy: https://clang.llvm.org/extra/clang-tidy/
- PVS-Studio: https://pvs-studio.com/

### Книги
- "Modern C++ Design" - Andrei Alexandrescu (Policy-based design)
- "Secure Coding in C and C++" - Robert Seacord
- "Effective C++" - Scott Meyers (Item 38: Model "has-a" or "is-implemented-in-terms-of")

## 💡 Золотое правило Strategy Security

```
┌────────────────────────────────────────────────────────────────┐
│                                                                │
│  "Strategy - это executable policy.                            │
│   Валидируйте каждую стратегию как если бы это был            │
│   external code, потому что так оно и есть."                   │
│                                                                │
│  Никогда не доверяйте типу strategy без RTTI проверки.         │
│  Никогда не загружайте strategy без whitelist validation.      │
│  Всегда используйте Null Object Pattern или std::optional.     │
│                                                                │
└────────────────────────────────────────────────────────────────┘
```

---

**Версия**: 1.0  
**Дата**: 2025-10-11  
**Курс**: Patterns (Security Integration Track)  
**Лицензия**: MIT

