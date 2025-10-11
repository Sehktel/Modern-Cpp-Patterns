# 🛡️ Security Poster: Decorator Pattern

## 🎯 Обзор угроз Decorator Pattern

```
┌────────────────────────────────────────────────────────────────────┐
│              КРИТИЧЕСКИЕ УЯЗВИМОСТИ DECORATOR PATTERN              │
└────────────────────────────────────────────────────────────────────┘

🔴 КРИТИЧНЫЕ УГРОЗЫ:
├─ Stack Overflow (CWE-674): Deep decorator chains
├─ Memory Leak (CWE-401): Circular decorator references
├─ Use-After-Free (CWE-416): Decorator outlives component
└─ Decorator Bypass: Skip security-critical decorators

🟡 ВЫСОКИЕ УГРОЗЫ:
├─ Decorator Injection: Malicious decorator in chain
├─ Order-Dependent Vulnerabilities: Incorrect decorator ordering
└─ Resource Exhaustion: Unbounded decorator chains
```

## 💣 Ключевые уязвимости

### 1. Stack Overflow от Deep Nesting

```cpp
// ❌ УЯЗВИМЫЙ КОД
class Decorator : public Component {
    Component* component_;
public:
    void operation() override {
        component_->operation();  // ⚠️ Deep recursion
        doAdditionalWork();
    }
};

// ЭКСПЛОЙТ: Stack overflow
Component* c = new ConcreteComponent();
for (int i = 0; i < 100000; ++i) {
    c = new Decorator(c);  // 💥 Deep chain
}
c->operation();  // Stack overflow!

// ✅ БЕЗОПАСНЫЙ КОД
class SafeDecorator : public Component {
    std::unique_ptr<Component> component_;
    static constexpr size_t MAX_DEPTH = 100;
    size_t depth_ = 0;
    
public:
    SafeDecorator(std::unique_ptr<Component> comp, size_t depth) 
        : component_(std::move(comp)), depth_(depth) {
        if (depth_ > MAX_DEPTH) {
            throw std::runtime_error("Decorator chain too deep");
        }
    }
};
```

### 2. Decorator Bypass Attack

```cpp
// ❌ УЯЗВИМЫЙ КОД - Security decorator can be bypassed
class EncryptionDecorator : public DataWriter {
    DataWriter* writer_;
public:
    void write(const Data& data) override {
        auto encrypted = encrypt(data);
        writer_->write(encrypted);  // ✅ Encrypted
    }
};

// АТАКА: Direct access to underlying writer
DataWriter* writer = new FileWriter();
DataWriter* encrypted = new EncryptionDecorator(writer);

// Bypass encryption:
writer->write(sensitive_data);  // 💥 Writes unencrypted!

// ✅ БЕЗОПАСНЫЙ КОД - Hide underlying component
class SecureEncryptionDecorator : public DataWriter {
    std::unique_ptr<DataWriter> writer_;  // ✅ Exclusive ownership
public:
    explicit SecureEncryptionDecorator(std::unique_ptr<DataWriter> w) 
        : writer_(std::move(w)) {}
    
    void write(const Data& data) override {
        auto encrypted = encrypt(data);
        writer_->write(encrypted);
    }
    // ✅ writer_ недоступен извне
};
```

### 3. Decorator Injection

```cpp
// ❌ УЯЗВИМЫЙ КОД - Unvalidated decorator chain
Component* buildChain(const std::vector<std::string>& decorator_names) {
    Component* c = new BaseComponent();
    for (const auto& name : decorator_names) {
        c = DecoratorFactory::create(name, c);  // ⚠️ No validation!
    }
    return c;
}

// АТАКА: Inject malicious decorator
std::vector<std::string> malicious_chain = {
    "LoggingDecorator",
    "MaliciousDecorator",  // 💥 Injected!
    "CompressionDecorator"
};

// ✅ БЕЗОПАСНЫЙ КОД - Whitelist validation
Component* buildSecureChain(const std::vector<std::string>& names) {
    static const std::set<std::string> ALLOWED = {
        "LoggingDecorator",
        "CompressionDecorator",
        "EncryptionDecorator"
    };
    
    auto component = std::make_unique<BaseComponent>();
    for (const auto& name : names) {
        if (ALLOWED.find(name) == ALLOWED.end()) {
            throw SecurityException("Decorator not whitelisted: " + name);
        }
        component = DecoratorFactory::create(name, std::move(component));
    }
    return component.release();
}
```

### 4. Order-Dependent Security Bug

```cpp
// ❌ ОПАСНО: Неправильный порядок decorators
auto writer = std::make_unique<FileWriter>();

// WRONG ORDER:
writer = std::make_unique<CompressionDecorator>(std::move(writer));
writer = std::make_unique<EncryptionDecorator>(std::move(writer));
// → Compress → Encrypt
// Проблема: Compression ослабляет encryption (patterns in compressed data)

// ✅ ПРАВИЛЬНЫЙ ORDER:
writer = std::make_unique<EncryptionDecorator>(std::move(writer));
writer = std::make_unique<CompressionDecorator>(std::move(writer));
// → Encrypt → Compress
// ✅ Сначала encrypt (скрывает patterns), затем compress

// ✅ Enforce правильный порядок
class DecoratorChainBuilder {
public:
    enum class DecoratorType { ENCRYPT, COMPRESS, LOG };
    
    std::unique_ptr<Component> build(std::vector<DecoratorType> types) {
        // ✅ Validate ordering
        if (!isValidOrder(types)) {
            throw std::invalid_argument("Invalid decorator order");
        }
        
        auto component = std::make_unique<BaseComponent>();
        for (auto type : types) {
            component = createDecorator(type, std::move(component));
        }
        return component;
    }
    
private:
    bool isValidOrder(const std::vector<DecoratorType>& types) {
        // ✅ Rule: ENCRYPT must come before COMPRESS
        auto encrypt_pos = std::find(types.begin(), types.end(), 
                                     DecoratorType::ENCRYPT);
        auto compress_pos = std::find(types.begin(), types.end(), 
                                      DecoratorType::COMPRESS);
        
        if (encrypt_pos != types.end() && compress_pos != types.end()) {
            return encrypt_pos < compress_pos;  // Encrypt first
        }
        
        return true;
    }
};
```

## 🛠️ Инструменты

```bash
# Stack overflow detection
ulimit -s 8192  # Set stack limit
./decorator_test

# Memory leak detection
g++ -fsanitize=address -g decorator_vulnerabilities.cpp
valgrind --leak-check=full ./decorator_test

# Circular reference detection
valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all
```

## 📋 Security Checklist

```
[ ] Max depth limit для decorator chains
[ ] Smart pointers (exclusive ownership)
[ ] Whitelist validation для decorator types
[ ] Enforced ordering rules
[ ] No raw pointers to underlying components
[ ] Circular reference prevention
[ ] Stack size monitoring
```

## 🎓 Best Practices

### Depth-Limited Decorator Chain
```cpp
template<size_t MaxDepth>
class BoundedDecorator : public Component {
    std::unique_ptr<Component> component_;
    
public:
    template<typename... Args>
    static auto create(std::unique_ptr<Component> comp, Args&&... args) {
        if (getDepth(comp.get()) >= MaxDepth) {
            throw std::runtime_error("Max decorator depth exceeded");
        }
        return std::make_unique<BoundedDecorator>(
            std::move(comp), std::forward<Args>(args)...
        );
    }
    
private:
    static size_t getDepth(Component* c) {
        size_t depth = 0;
        while (auto* dec = dynamic_cast<BoundedDecorator*>(c)) {
            ++depth;
            c = dec->component_.get();
        }
        return depth;
    }
};
```

## 💡 Золотое правило

```
"Decorator chains - это call stacks в runtime.
 Limit depth, enforce ordering, validate types.
 Never expose underlying components.
 Always use smart pointers!"
```

---

**Версия**: 1.0 | **Дата**: 2025-10-11 | **Лицензия**: MIT

