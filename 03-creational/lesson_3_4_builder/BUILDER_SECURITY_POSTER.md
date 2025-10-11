# 🛡️ Security Poster: Builder Pattern

## 🎯 Обзор угроз Builder Pattern

```
┌────────────────────────────────────────────────────────────────────┐
│               КРИТИЧЕСКИЕ УЯЗВИМОСТИ BUILDER PATTERN               │
└────────────────────────────────────────────────────────────────────┘

🔴 КРИТИЧНЫЕ УГРОЗЫ:
┌─────────────────────┬──────────────────────────────────────────────┐
│ Buffer Overflow     │ CWE-120: Unchecked string building           │
│                     │ Impact: Heap overflow, RCE                   │
├─────────────────────┼──────────────────────────────────────────────┤
│ Integer Overflow    │ CWE-190: Size calculation overflow          │
│                     │ Impact: Buffer overflow, memory corruption   │
├─────────────────────┼──────────────────────────────────────────────┤
│ State Corruption    │ Invalid build sequence                       │
│                     │ Impact: Inconsistent object state, crashes   │
└─────────────────────┴──────────────────────────────────────────────┘

🟡 ВЫСОКИЕ УГРОЗЫ:
┌─────────────────────┬──────────────────────────────────────────────┐
│ Memory Leaks        │ CWE-401: Exception during build              │
│ Incomplete Objects  │ Missing required parameters                  │
│ SQL Injection       │ Query builder без sanitization               │
└─────────────────────┴──────────────────────────────────────────────┘
```

## 💣 Критическая уязвимость #1: Buffer Overflow в String Building

### Анатомия атаки

```cpp
// ❌ УЯЗВИМЫЙ КОД - Unchecked string concatenation
class VulnerableBuilder {
private:
    char buffer_[1024];  // ⚠️ Fixed size buffer
    size_t pos_ = 0;
    
public:
    VulnerableBuilder& addString(const std::string& str) {
        // ⚠️ No bounds checking!
        strcpy(buffer_ + pos_, str.c_str());  // 💥 Buffer overflow!
        pos_ += str.length();
        return *this;
    }
    
    std::string build() {
        return std::string(buffer_, pos_);
    }
};
```

### 🎯 Exploitation: Heap overflow

```cpp
// ЭКСПЛОЙТ: Buffer overflow attack
void exploit_buffer_overflow() {
    VulnerableBuilder builder;
    
    // АТАКА: Overflow buffer с large input
    std::string payload(2000, 'A');  // 2000 > 1024
    payload += "\xef\xbe\xad\xde";   // Overwrite return address
    
    builder.addString(payload);  // 💥 Buffer overflow!
    
    // Последствия:
    // 1. buffer_ overflows на stack
    // 2. Перезаписывает return address
    // 3. При return → переход на 0xdeadbeef
    // 4. RCE
}

// Альтернативный exploit: Heap overflow
class HeapBuilder {
    char* buffer_;  // ⚠️ Heap allocated но fixed size
    size_t capacity_ = 1024;
    size_t pos_ = 0;
    
public:
    HeapBuilder() { buffer_ = new char[capacity_]; }
    
    HeapBuilder& addString(const std::string& str) {
        memcpy(buffer_ + pos_, str.data(), str.length());  // 💥 No check!
        pos_ += str.length();
        return *this;
    }
};

void exploit_heap_overflow() {
    HeapBuilder builder;
    
    // Overflow heap buffer
    builder.addString(std::string(2000, 'A'));
    // 💥 Overflows на adjacent heap chunks
    // → Heap metadata corruption
    // → Control over allocator
    // → RCE
}
```

### ✅ Защита: Dynamic buffer с bounds checking

```cpp
// ✅ БЕЗОПАСНЫЙ КОД - Dynamic growth с validation
class SecureBuilder {
private:
    std::vector<char> buffer_;  // ✅ Dynamic size
    const size_t MAX_SIZE = 10 * 1024 * 1024;  // 10MB limit
    
public:
    SecureBuilder& addString(const std::string& str) {
        // ✅ Validate size
        if (str.length() > MAX_SIZE) {
            throw std::invalid_argument("String too large");
        }
        
        // ✅ Check total size
        if (buffer_.size() + str.length() > MAX_SIZE) {
            throw std::runtime_error("Builder size limit exceeded");
        }
        
        // ✅ Check for overflow
        if (buffer_.size() > SIZE_MAX - str.length()) {
            throw std::overflow_error("Size calculation overflow");
        }
        
        // ✅ Safe append
        buffer_.insert(buffer_.end(), str.begin(), str.end());
        return *this;
    }
    
    std::string build() {
        return std::string(buffer_.begin(), buffer_.end());
    }
};

// Альтернатива: std::stringstream (еще проще)
class SimpleBuilder {
    std::stringstream ss_;
    size_t size_ = 0;
    const size_t MAX_SIZE = 10 * 1024 * 1024;
    
public:
    SimpleBuilder& add(const std::string& str) {
        if (size_ + str.length() > MAX_SIZE) {
            throw std::runtime_error("Size limit exceeded");
        }
        
        ss_ << str;
        size_ += str.length();
        return *this;
    }
    
    std::string build() { return ss_.str(); }
};
```

## 💣 Критическая уязвимость #2: Integer Overflow в Size Calculation

### Анатомия атаки

```cpp
// ❌ УЯЗВИМЫЙ КОД - Integer overflow в allocation
class AllocatingBuilder {
public:
    struct Config {
        size_t item_count;
        size_t item_size;
    };
    
    char* build(const Config& config) {
        // ⚠️ Integer overflow!
        size_t total_size = config.item_count * config.item_size;
        
        char* buffer = new char[total_size];  // 💥 May allocate small buffer!
        // Fill buffer...
        return buffer;
    }
};
```

### 🎯 Exploitation: Integer overflow → heap overflow

```cpp
// ЭКСПЛОЙТ: Integer overflow attack
void exploit_integer_overflow() {
    AllocatingBuilder builder;
    
    // АТАКА: Trigger integer overflow
    AllocatingBuilder::Config malicious_config;
    malicious_config.item_count = SIZE_MAX / 2 + 1;  // Large value
    malicious_config.item_size = 4;                  // Small multiplier
    
    // Вычисление:
    // total_size = (SIZE_MAX/2 + 1) * 4
    //            = SIZE_MAX/2*4 + 4
    //            = SIZE_MAX*2 + 4
    //            → Overflow! Результат: очень маленькое число
    
    char* buffer = builder.build(malicious_config);
    // 💥 Allocated small buffer (например, 8 bytes)
    // Но code предполагает что buffer большой
    // → Heap overflow при filling
}
```

### ✅ Защита: Overflow-safe arithmetic

```cpp
// ✅ БЕЗОПАСНЫЙ КОД - Overflow checking
class SecureAllocatingBuilder {
public:
    struct Config {
        size_t item_count;
        size_t item_size;
    };
    
    std::unique_ptr<char[]> build(const Config& config) {
        // ✅ Validate inputs
        if (config.item_count == 0 || config.item_size == 0) {
            throw std::invalid_argument("Invalid config");
        }
        
        // ✅ Check for multiplication overflow
        if (config.item_count > SIZE_MAX / config.item_size) {
            throw std::overflow_error(
                "Size calculation would overflow: " +
                std::to_string(config.item_count) + " * " +
                std::to_string(config.item_size)
            );
        }
        
        size_t total_size = config.item_count * config.item_size;
        
        // ✅ Additional limit check
        const size_t MAX_ALLOCATION = 100 * 1024 * 1024;  // 100MB
        if (total_size > MAX_ALLOCATION) {
            throw std::runtime_error("Allocation size exceeds limit");
        }
        
        // ✅ Safe allocation
        return std::make_unique<char[]>(total_size);
    }
};

// C++20: std::cmp_* для safe comparisons
bool safe_multiply(size_t a, size_t b, size_t& result) {
    if (a == 0 || b == 0) {
        result = 0;
        return true;
    }
    
    if (a > SIZE_MAX / b) {
        return false;  // Overflow would occur
    }
    
    result = a * b;
    return true;
}
```

## 💣 Критическая уязвимость #3: SQL Injection в Query Builder

### Анатомия атаки

```cpp
// ❌ УЯЗВИМЫЙ КОД - SQL query builder без sanitization
class VulnerableSQLBuilder {
private:
    std::string query_;
    
public:
    VulnerableSQLBuilder& select(const std::string& columns) {
        query_ = "SELECT " + columns + " ";  // ⚠️ No sanitization!
        return *this;
    }
    
    VulnerableSQLBuilder& from(const std::string& table) {
        query_ += "FROM " + table + " ";  // ⚠️ No sanitization!
        return *this;
    }
    
    VulnerableSQLBuilder& where(const std::string& condition) {
        query_ += "WHERE " + condition;  // ⚠️ No sanitization!
        return *this;
    }
    
    std::string build() { return query_; }
};
```

### 🎯 Exploitation: SQL injection

```cpp
// ЭКСПЛОЙТ: SQL injection attack
void exploit_sql_injection() {
    VulnerableSQLBuilder builder;
    
    // Легитимное использование:
    // SELECT * FROM users WHERE id = 1
    
    // АТАКА: Malicious input
    std::string malicious_condition = "1=1 OR '1'='1"; // Always true
    
    std::string query = builder
        .select("*")
        .from("users")
        .where(malicious_condition)
        .build();
    
    // Результат: "SELECT * FROM users WHERE 1=1 OR '1'='1"
    // → Returns ALL users (auth bypass!)
    
    // Более опасная атака:
    std::string drop_table = "1=1; DROP TABLE users; --";
    
    query = builder
        .select("*")
        .from("users")
        .where(drop_table)
        .build();
    
    // Результат: "SELECT * FROM users WHERE 1=1; DROP TABLE users; --"
    // → Executes TWO queries:
    //   1. SELECT * FROM users WHERE 1=1
    //   2. DROP TABLE users
    // 💥 Database destroyed!
}
```

### ✅ Защита: Parameterized queries

```cpp
// ✅ БЕЗОПАСНЫЙ КОД - Parameterized query builder
class SecureSQLBuilder {
private:
    std::string query_;
    std::vector<std::string> params_;
    int param_index_ = 0;
    
public:
    SecureSQLBuilder& select(const std::vector<std::string>& columns) {
        // ✅ Whitelist validation для column names
        for (const auto& col : columns) {
            if (!isValidIdentifier(col)) {
                throw std::invalid_argument("Invalid column name: " + col);
            }
        }
        
        query_ = "SELECT " + join(columns, ", ") + " ";
        return *this;
    }
    
    SecureSQLBuilder& from(const std::string& table) {
        // ✅ Whitelist validation
        if (!isValidIdentifier(table)) {
            throw std::invalid_argument("Invalid table name: " + table);
        }
        
        query_ += "FROM " + table + " ";
        return *this;
    }
    
    SecureSQLBuilder& where(const std::string& column, 
                           const std::string& value) {
        // ✅ Column name validation
        if (!isValidIdentifier(column)) {
            throw std::invalid_argument("Invalid column name: " + column);
        }
        
        // ✅ Value как parameter (НЕ concatenated!)
        query_ += "WHERE " + column + " = $" + std::to_string(++param_index_) + " ";
        params_.push_back(value);
        
        return *this;
    }
    
    std::pair<std::string, std::vector<std::string>> build() {
        return {query_, params_};
    }
    
private:
    static bool isValidIdentifier(const std::string& id) {
        // ✅ Allow только alphanumeric + underscore
        static const std::regex valid_id_regex("^[a-zA-Z_][a-zA-Z0-9_]*$");
        return std::regex_match(id, valid_id_regex);
    }
};

// Использование:
auto [query, params] = SecureSQLBuilder()
    .select({"id", "name", "email"})
    .from("users")
    .where("id", user_input)  // ✅ user_input как parameter
    .build();

// Execute с parameterized query:
// query = "SELECT id, name, email FROM users WHERE id = $1"
// params = {user_input}
// → SQL engine treats user_input as DATA, не CODE
// → No injection possible!
```

## 🛠️ Инструменты детектирования

### 1. ASan - Buffer Overflow Detection

```bash
g++ -fsanitize=address -g builder_vulnerabilities.cpp -o builder_asan

./builder_asan
```

**Вывод ASan**:
```
=================================================================
==12345==ERROR: AddressSanitizer: heap-buffer-overflow
WRITE of size 2000 at 0x602000000010 thread T0
    #0 0x400b12 in VulnerableBuilder::addString() builder.cpp:45
```

### 2. Static Analysis - Buffer Overflow

```bash
clang-tidy builder_vulnerabilities.cpp \
    -checks='clang-analyzer-security.insecureAPI.strcpy'

# Warning: Call to function 'strcpy' is insecure
```

### 3. UBSan - Integer Overflow Detection

```bash
g++ -fsanitize=undefined -g builder_vulnerabilities.cpp -o builder_ubsan

./builder_ubsan
```

**Вывод UBSan**:
```
builder.cpp:67:23: runtime error: unsigned integer overflow:
18446744073709551615 * 4 cannot be represented in type 'unsigned long'
```

## 📋 Security Checklist для Builder Pattern

### Design Phase
```
[ ] Определен maximum size для built objects
[ ] Документированы required vs optional parameters
[ ] Рассмотрены injection risks (SQL, command, etc.)
[ ] Определена validation strategy
```

### Implementation Phase
```
[ ] Dynamic buffers (std::vector, std::string) вместо fixed
[ ] Overflow checking для size calculations
[ ] Bounds checking для all append operations
[ ] Input sanitization (whitelist для identifiers)
[ ] Parameterized output (для SQL, commands)
[ ] Exception-safe building
```

### Testing Phase
```
[ ] Buffer overflow tests с ASan
[ ] Integer overflow tests с UBSan
[ ] Injection tests (malformed input)
[ ] Size limit tests
[ ] Incomplete object tests
[ ] Exception safety tests
```

### Deployment Phase
```
[ ] Monitoring для builder size anomalies
[ ] Logging для large builds
[ ] Rate limiting для builder usage
```

## 🎓 Best Practices

### 1. Fluent Builder с Validation

```cpp
// ✅ Builder с step-by-step validation
class ValidatedBuilder {
    std::stringstream ss_;
    bool header_set_ = false;
    bool body_set_ = false;
    
public:
    ValidatedBuilder& setHeader(const std::string& header) {
        if (header.empty()) {
            throw std::invalid_argument("Header cannot be empty");
        }
        ss_ << "Header: " << header << "\n";
        header_set_ = true;
        return *this;
    }
    
    ValidatedBuilder& setBody(const std::string& body) {
        if (!header_set_) {
            throw std::logic_error("Must set header before body");
        }
        ss_ << "Body: " << body << "\n";
        body_set_ = true;
        return *this;
    }
    
    std::string build() {
        if (!header_set_ || !body_set_) {
            throw std::logic_error("Incomplete object");
        }
        return ss_.str();
    }
};
```

### 2. Type-Safe Builder (Builder Pattern + Type State)

```cpp
// ✅ Compile-time validation с type states
template<bool HasHeader, bool HasBody>
class TypedBuilder;

template<>
class TypedBuilder<false, false> {
    std::string data_;
    
public:
    TypedBuilder<true, false> setHeader(const std::string& h) {
        TypedBuilder<true, false> next;
        next.data_ = "Header: " + h + "\n";
        return next;
    }
    
    // ❌ setBody() недоступен (compile error)
    // ❌ build() недоступен (compile error)
};

template<>
class TypedBuilder<true, false> {
    std::string data_;
    
public:
    TypedBuilder<true, true> setBody(const std::string& b) {
        TypedBuilder<true, true> next;
        next.data_ = data_ + "Body: " + b + "\n";
        return next;
    }
    
    // ❌ build() еще недоступен
};

template<>
class TypedBuilder<true, true> {
    std::string data_;
    
public:
    std::string build() {  // ✅ Только здесь доступен build()
        return data_;
    }
};

// Использование:
auto result = TypedBuilder<false, false>()
    .setHeader("Hello")  // ✅ OK
    .setBody("World")    // ✅ OK
    .build();            // ✅ OK

// auto bad = TypedBuilder<false, false>().build();  // ❌ Compile error!
```

## 💡 Золотое правило Builder Security

```
┌────────────────────────────────────────────────────────────────┐
│                                                                │
│  "Builder accumulates data - validate EVERY input.            │
│   Use dynamic buffers с bounds checking.                      │
│   Check for integer overflow в size calculations.             │
│   Sanitize/parameterize output (SQL, commands)."              │
│                                                                │
│  Никогда не используйте fixed-size buffers.                  │
│  Всегда validate accumulated size.                           │
│  Parameterize, don't concatenate (SQL, shell commands).       │
│                                                                │
└────────────────────────────────────────────────────────────────┘
```

---

**Версия**: 1.0  
**Дата**: 2025-10-11  
**Курс**: Patterns (Security Integration Track)  
**Лицензия**: MIT

