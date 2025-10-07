# Анализ безопасности паттерна Factory Method

## Обзор уязвимостей

### Основные типы уязвимостей в Factory Method:
- **Buffer Overflow** - переполнение буфера при создании объектов
- **Memory Leak** - утечки памяти при исключениях в конструкторе
- **Type Confusion** - проблемы с небезопасным приведением типов
- **Integer Overflow** - переполнение при вычислении размеров объектов

## Инструменты анализа

### Статический анализ
- **Clang Static Analyzer**: `clang --analyze factory_method_vulnerabilities.cpp`
- **Cppcheck**: `cppcheck --enable=all factory_method_vulnerabilities.cpp`
- **PVS-Studio**: Анализ на предмет buffer overflow и memory leaks

### Динамический анализ
- **AddressSanitizer**: Обнаружение buffer overflow и use-after-free
- **MemorySanitizer**: Обнаружение неинициализированной памяти
- **Valgrind**: Анализ памяти и обнаружение leaks

## Практические задания

1. **Обнаружение buffer overflow**:
   ```bash
   g++ -fsanitize=address -g -std=c++17 factory_method_vulnerabilities.cpp -o factory_asan
   ./factory_asan
   
   # AddressSanitizer обнаружит:
   # - Heap-buffer-overflow
   # - Use-after-free
   # - Memory leaks
   ```

2. **Анализ memory leaks**:
   ```bash
   g++ -g -std=c++17 factory_method_vulnerabilities.cpp -o factory_debug
   valgrind --tool=memcheck --leak-check=full ./factory_debug
   
   # Valgrind покажет утечки памяти
   ```

3. **Обнаружение type confusion**:
   ```bash
   g++ -fsanitize=undefined -g -std=c++17 factory_method_vulnerabilities.cpp -o factory_ubsan
   ./factory_ubsan
   
   # UBSanitizer обнаружит undefined behavior
   ```

4. **Запуск эксплоитов**:
   ```bash
   cd exploits
   g++ -std=c++17 factory_method_exploits.cpp -o factory_exploits
   ./factory_exploits
   ```

## Безопасные альтернативы

### 1. Factory с RAII и smart pointers
```cpp
// БЕЗОПАСНАЯ РЕАЛИЗАЦИЯ: Factory со std::unique_ptr
class Product {
public:
    virtual ~Product() = default;
    virtual void use() = 0;
};

class ConcreteProductA : public Product {
public:
    void use() override {
        std::cout << "Using Product A" << std::endl;
    }
};

class SafeFactory {
public:
    // Возвращаем std::unique_ptr - автоматическое управление памятью
    static std::unique_ptr<Product> createProduct(const std::string& type) {
        if (type == "A") {
            return std::make_unique<ConcreteProductA>();
        }
        
        throw std::invalid_argument("Unknown product type");
    }
    
    // БЕЗОПАСНО: Нет manual delete, RAII гарантирует освобождение
};

// Использование:
auto product = SafeFactory::createProduct("A");
product->use();
// Автоматическое освобождение памяти
```

### 2. Factory с проверкой размеров
```cpp
// БЕЗОПАСНАЯ РЕАЛИЗАЦИЯ: Factory с валидацией размеров
class SafeSizeFactory {
private:
    static constexpr size_t MAX_OBJECT_SIZE = 1024 * 1024;  // 1 MB
    
public:
    static std::unique_ptr<char[]> createBuffer(size_t size) {
        // ЗАЩИТА: Проверка размера перед аллокацией
        if (size == 0 || size > MAX_OBJECT_SIZE) {
            throw std::invalid_argument(
                "Invalid size: must be in range [1, " + 
                std::to_string(MAX_OBJECT_SIZE) + "]"
            );
        }
        
        // ЗАЩИТА: Проверка overflow при вычислениях
        if (size > SIZE_MAX / 2) {
            throw std::overflow_error("Size too large");
        }
        
        // Безопасное создание
        return std::make_unique<char[]>(size);
    }
};
```

### 3. Factory с type-safe приведением типов
```cpp
// БЕЗОПАСНАЯ РЕАЛИЗАЦИЯ: Factory с std::variant
class SafeTypedFactory {
public:
    using ProductVariant = std::variant<ConcreteProductA, ConcreteProductB>;
    
    static ProductVariant createProduct(const std::string& type) {
        if (type == "A") {
            return ConcreteProductA{};
        } else if (type == "B") {
            return ConcreteProductB{};
        }
        
        throw std::invalid_argument("Unknown type");
    }
    
    // БЕЗОПАСНО: Compile-time type checking через std::visit
    template<typename Visitor>
    static auto visit(ProductVariant& product, Visitor&& visitor) {
        return std::visit(std::forward<Visitor>(visitor), product);
    }
};

// Использование:
auto product = SafeTypedFactory::createProduct("A");
SafeTypedFactory::visit(product, [](auto& p) {
    p.use();  // Type-safe!
});
```

## Эксплоиты и атаки

### 1. Buffer Overflow Exploit
```cpp
// ЭКСПЛОИТ: Buffer overflow для перезаписи return address

class ExploitFactory {
public:
    static void* createVulnerableBuffer(size_t user_size) {
        // УЯЗВИМОСТЬ: Нет проверки размера
        char* buffer = new char[user_size];
        
        // Атакующий передает огромный size
        // size = 0xFFFFFFFF → integer overflow
        // Фактически выделится маленький буфер
        
        return buffer;
    }
};

// АТАКА:
size_t malicious_size = static_cast<size_t>(-1);  // MAX SIZE_T
auto* buffer = ExploitFactory::createVulnerableBuffer(malicious_size);

// Переполнение буфера
memset(buffer, 'A', 1000000);  // HEAP OVERFLOW!

// Результат:
// - Heap corruption
// - Перезапись соседних объектов
// - Возможность ROP chain
// - Arbitrary code execution
```

**Детали атаки**:
1. Integer overflow в вычислении размера
2. Heap buffer overflow
3. Перезапись metadata heap allocator
4. Control flow hijacking

### 2. Type Confusion Attack
```cpp
// ЭКСПЛОИТ: Type confusion через неправильное приведение типов

class BaseProduct {
public:
    virtual ~BaseProduct() = default;
    virtual void execute() = 0;
};

class UserProduct : public BaseProduct {
private:
    char username[64];
public:
    void execute() override {
        std::cout << "User: " << username << std::endl;
    }
};

class AdminProduct : public BaseProduct {
private:
    char admin_key[64];
    bool has_privileges = true;
public:
    void execute() override {
        if (has_privileges) {
            performAdminAction();
        }
    }
};

class VulnerableFactory {
public:
    static BaseProduct* create(const std::string& type) {
        if (type == "user") {
            return new UserProduct();
        }
        return new UserProduct();  // УЯЗВИМОСТЬ: всегда возвращает User
    }
};

// АТАКА: Type confusion
auto* product = VulnerableFactory::create("user");

// Атакующий делает reinterpret_cast
auto* admin = reinterpret_cast<AdminProduct*>(product);

// TYPE CONFUSION! Обращение к has_privileges UserProduct как к AdminProduct
admin->execute();  // PRIVILEGE ESCALATION!
```

### 3. Heap Spraying Attack
```cpp
// АТАКА: Heap spraying для повышения надежности эксплоита

class SprayableFactory {
public:
    static void* createObject(size_t size) {
        return new char[size];
    }
};

// HEAP SPRAYING:
std::vector<void*> spray_objects;

// Распыляем heap с контролируемыми данными
for (int i = 0; i < 10000; ++i) {
    void* obj = SprayableFactory::createObject(1024);
    
    // Заполняем контролируемым payload
    memset(obj, 0x41, 1024);  // ROP gadgets
    
    spray_objects.push_back(obj);
}

// Теперь большая часть heap содержит наш payload
// Эксплоит более надежен
```

## Защитные меры

### 1. Использование smart pointers
```cpp
std::unique_ptr<Product> product = factory.create();
// Автоматическое освобождение, нет memory leaks
```

### 2. Проверка размеров
```cpp
if (size == 0 || size > MAX_SIZE) {
    throw std::invalid_argument("Invalid size");
}
```

### 3. Безопасное приведение типов
```cpp
// Используйте dynamic_cast, не reinterpret_cast
auto* admin = dynamic_cast<AdminProduct*>(product);
if (admin == nullptr) {
    throw std::bad_cast("Invalid cast");
}
```

### 4. RAII для всех ресурсов
```cpp
class RAIIFactory {
public:
    std::unique_ptr<Resource> create() {
        return std::make_unique<Resource>();
    }
};
```

### 5. Валидация входных данных
```cpp
if (type.empty() || type.length() > MAX_TYPE_LENGTH) {
    throw std::invalid_argument("Invalid type");
}
```

### 6. Static analysis в CI/CD
```yaml
- name: Clang Static Analyzer
  run: clang --analyze src/factory*.cpp

- name: Cppcheck
  run: cppcheck --enable=all src/
```

## Связь с другими паттернами

### Abstract Factory
- **Проблема**: Множественные Factory методы → больше точек атаки
- **Решение**: Централизованная валидация, единые проверки безопасности

### Builder
- **Проблема**: Builder может создавать объекты через уязвимый Factory
- **Решение**: Валидация на каждом этапе построения

### Prototype
- **Проблема**: Clone может копировать уязвимые объекты
- **Решение**: Deep copy с валидацией

### Singleton
- **Проблема**: Singleton Factory - комбинация уязвимостей
- **Решение**: Thread-safe Singleton + Safe Factory

## Чек-лист безопасности Factory Method

- [ ] ✅ Используются smart pointers (std::unique_ptr/shared_ptr)
- [ ] ✅ Проверка размеров перед аллокацией
- [ ] ✅ Проверка типов (std::variant или dynamic_cast)
- [ ] ✅ Обработка исключений (exception-safe)
- [ ] ✅ Нет integer overflow в вычислениях
- [ ] ✅ Валидация входных параметров
- [ ] ✅ RAII для всех ресурсов
- [ ] ✅ Тестирование с AddressSanitizer
- [ ] ✅ Code review на buffer overflow

## Дополнительные ресурсы

### CWE (Common Weakness Enumeration)
- [CWE-120: Buffer Overflow](https://cwe.mitre.org/data/definitions/120.html)
- [CWE-401: Missing Release of Memory](https://cwe.mitre.org/data/definitions/401.html)
- [CWE-843: Type Confusion](https://cwe.mitre.org/data/definitions/843.html)
- [CWE-190: Integer Overflow](https://cwe.mitre.org/data/definitions/190.html)

### Инструменты
- [AddressSanitizer](https://github.com/google/sanitizers/wiki/AddressSanitizer)
- [Valgrind Memcheck](https://valgrind.org/docs/manual/mc-manual.html)
- [Cppcheck](http://cppcheck.sourceforge.net/)

### Best Practices
- C++ Core Guidelines: R.1 (Manage resources automatically using RAII)
- C++ Core Guidelines: ES.106 (Don't try to avoid negative values by using unsigned)
- Secure Coding in C and C++ (Robert Seacord)