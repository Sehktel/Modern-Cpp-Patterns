# 🛡️ Security Poster: Adapter Pattern

## 🎯 Обзор угроз Adapter Pattern

```
┌────────────────────────────────────────────────────────────────────┐
│               КРИТИЧЕСКИЕ УЯЗВИМОСТИ ADAPTER PATTERN               │
└────────────────────────────────────────────────────────────────────┘

🔴 КРИТИЧНЫЕ УГРОЗЫ:
├─ Type Confusion (CWE-843): Неправильное приведение между интерфейсами
├─ Buffer Overflow (CWE-120): Несоответствие размеров буферов
├─ Use-After-Free (CWE-416): Dangling references между adaptee и adapter
└─ Data Truncation: Потеря данных при conversion между типами

🟡 ВЫСОКИЕ УГРОЗЫ:
├─ Information Disclosure: Утечка внутренних данных через adapter
├─ Privilege Escalation: Bypass restrictions через adapter
└─ Interface Injection: Malicious adaptee injection
```

## 💣 Ключевые уязвимости

### 1. Type Confusion в двусторонних адаптерах

```cpp
// ❌ УЯЗВИМЫЙ КОД
class UnsafeAdapter : public Target {
    Adaptee* adaptee_;
public:
    void request() override {
        // ⚠️ Unsafe downcast
        auto* concrete = static_cast<ConcreteAdaptee*>(adaptee_);
        concrete->specificRequest();  // 💥 Type confusion!
    }
};

// ✅ БЕЗОПАСНЫЙ КОД
class SafeAdapter : public Target {
    std::unique_ptr<Adaptee> adaptee_;
public:
    void request() override {
        if (auto* concrete = dynamic_cast<ConcreteAdaptee*>(adaptee_.get())) {
            concrete->specificRequest();
        } else {
            throw std::runtime_error("Invalid adaptee type");
        }
    }
};
```

### 2. Buffer Overflow при Data Conversion

```cpp
// ❌ УЯЗВИМЫЙ КОД
class BufferAdapter {
public:
    void convert(const LargeData& input, SmallData& output) {
        memcpy(&output, &input, sizeof(input));  // 💥 Buffer overflow!
    }
};

// ✅ БЕЗОПАСНЫЙ КОД
class SafeBufferAdapter {
public:
    void convert(const LargeData& input, SmallData& output) {
        if (sizeof(input) > sizeof(output)) {
            throw std::invalid_argument("Data too large for conversion");
        }
        
        std::memcpy(&output, &input, std::min(sizeof(input), sizeof(output)));
    }
};
```

### 3. Use-After-Free с Adaptee

```cpp
// ❌ УЯЗВИМЫЙ КОД
Adapter* adapter;
{
    Adaptee adaptee;
    adapter = new Adapter(&adaptee);  // ⚠️ Stores raw pointer
}
// adaptee destroyed
adapter->request();  // 💥 UAF!

// ✅ БЕЗОПАСНЫЙ КОД
auto adapter = std::make_unique<Adapter>(
    std::make_shared<Adaptee>()  // ✅ Shared ownership
);
```

## 🛠️ Инструменты

```bash
# Type confusion detection
g++ -fsanitize=undefined -g adapter_vulnerabilities.cpp

# Buffer overflow detection
g++ -fsanitize=address -g adapter_vulnerabilities.cpp

# UAF detection
valgrind --tool=memcheck ./adapter_test
```

## 📋 Security Checklist

```
[ ] Используется dynamic_cast для type checking
[ ] Bounds checking при data conversion
[ ] Smart pointers для lifetime management
[ ] Validation всех converted данных
[ ] Нет unsafe downcasting
[ ] Error handling для incompatible types
```

## 🎓 Best Practices

### Type-Safe Adapter с шаблонами
```cpp
template<typename Target, typename Adaptee>
class TypeSafeAdapter : public Target {
    std::shared_ptr<Adaptee> adaptee_;
public:
    explicit TypeSafeAdapter(std::shared_ptr<Adaptee> adaptee) 
        : adaptee_(std::move(adaptee)) {}
    
    void request() override {
        adaptee_->specificRequest();  // ✅ Compile-time type safety
    }
};
```

## 💡 Золотое правило

```
"Adapter преобразует интерфейсы - validate ALL conversions.
 Use dynamic_cast для runtime checks.
 Use smart pointers для lifetime safety.
 Never assume size compatibility!"
```

---

**Версия**: 1.0 | **Дата**: 2025-10-11 | **Лицензия**: MIT

