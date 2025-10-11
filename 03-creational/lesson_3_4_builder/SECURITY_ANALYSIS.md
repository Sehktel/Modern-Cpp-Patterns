# Анализ безопасности паттерна Builder

## Обзор

Паттерн Builder - один из основных порождающих паттернов проектирования, предназначенный для пошагового конструирования сложных объектов. Однако именно эта пошаговость создает множественные точки входа для атак на каждом этапе построения. В отличие от конструктора, где все параметры валидируются одновременно, Builder позволяет устанавливать поля по отдельности, что открывает широкие возможности для exploitation.

## Почему Builder особенно уязвим

### Множественные точки входа
Каждый setter в Builder - это отдельная точка, где может быть внедрена атака:
- `setHeader()` - buffer overflow
- `setSize()` - integer overflow
- `setPermissions()` - privilege escalation
- `setQuery()` - SQL injection

### Отложенная валидация
Традиционная реализация валидирует данные только в `build()`, что означает:
1. Некорректные данные могут быть установлены
2. State builder может быть частично скомпрометирован
3. Исключения в `build()` оставляют builder в неконсистентном состоянии

### Method Chaining
Fluent interface делает код элегантным, но затрудняет обработку ошибок:
```cpp
builder.setA().setB().setC().build();  // Где обрабатывать ошибки?
```

## Критические уязвимости

### 1. Buffer Overflow (CWE-120/121/122)

#### Описание
Buffer overflow в Builder происходит при копировании пользовательских данных в фиксированные буферы без проверки размера. Это одна из старейших и наиболее опасных уязвимостей в C/C++.

#### Технические детали

**Stack-based Buffer Overflow**:
```cpp
class MessageBuilder {
private:
    char header_[64];   // Фиксированный буфер на стеке
    char body_[256];
    char footer_[32];
    
public:
    MessageBuilder& setHeader(const char* header) {
        strcpy(header_, header);  // КРИТИЧЕСКАЯ УЯЗВИМОСТЬ!
        return *this;
    }
};
```

**Что происходит**:
1. `strcpy()` копирует данные без проверки размера
2. Если `header` > 64 байта, происходит запись за границы `header_[]`
3. Перезаписываются соседние поля (`body_[]`, `footer_[]`)
4. Может быть перезаписан return address на стеке
5. Control flow hijacking → RCE

**Heap-based Buffer Overflow**:
```cpp
class DynamicBuilder {
private:
    char* buffer_;
    size_t allocated_size_;
    size_t used_size_;
    
public:
    DynamicBuilder() : buffer_(new char[1024]), allocated_size_(1024), used_size_(0) {}
    
    void append(const char* data, size_t len) {
        // УЯЗВИМОСТЬ: Нет проверки переполнения
        memcpy(buffer_ + used_size_, data, len);
        used_size_ += len;
    }
};
```

**Exploitation пример**:
```cpp
// Создаем payload для перезаписи return address
char overflow[128];
memset(overflow, 'A', 64);  // Заполняем header_

// Адрес shellcode или ROP gadget
uintptr_t malicious_addr = 0xdeadbeef;
memcpy(overflow + 64, &malicious_addr, sizeof(malicious_addr));

builder.setHeader(overflow);  // Stack corruption!
```

#### Механизм атаки: ROP (Return-Oriented Programming)

**Шаг 1**: Найти gadgets в бинарнике
```bash
ROPgadget --binary ./builder_app
# Находим полезные gadgets:
# 0x400123: pop rdi; ret
# 0x400456: pop rsi; ret
# 0x400789: syscall; ret
```

**Шаг 2**: Построить ROP chain
```cpp
uintptr_t rop_chain[] = {
    0x400123,              // pop rdi; ret
    (uintptr_t)"/bin/sh",  // адрес строки
    0x400456,              // pop rsi; ret
    0,                     // NULL
    0x400789,              // syscall (execve)
};

// Overflow для размещения ROP chain
char payload[256];
memset(payload, 'A', 64);  // Padding до return address
memcpy(payload + 64, rop_chain, sizeof(rop_chain));

builder.setHeader(payload);
```

#### CVE примеры

- **CVE-2021-3156 (Sudo Baron Samedit)**: Heap overflow через buffer overflow в command line parsing
- **CVE-2019-14287 (Sudo)**: Buffer overflow позволял получить root
- **CVE-2020-1938 (Ghostcat)**: Buffer overflow в Apache Tomcat AJP protocol

#### Детектирование

**AddressSanitizer (ASan)**:
```bash
# Компиляция с ASan
g++ -fsanitize=address -fno-omit-frame-pointer -g \
    builder_vulnerabilities.cpp -o builder_asan

# Запуск
./builder_asan

# Вывод при overflow:
=================================================================
==12345==ERROR: AddressSanitizer: stack-buffer-overflow on address 0x7ffc1234
WRITE of size 128 at 0x7ffc1234 thread T0
    #0 0x... in MessageBuilder::setHeader(char const*)
    #1 0x... in main
```

**Valgrind Memcheck**:
```bash
valgrind --tool=memcheck --leak-check=full --track-origins=yes \
    ./builder_vulnerabilities

# Вывод:
Invalid write of size 64
   at 0x...: strcpy
   by 0x...: MessageBuilder::setHeader
```

**GDB для анализа**:
```bash
gdb ./builder_vulnerabilities
(gdb) run
(gdb) x/20x $rsp  # Проверить стек
(gdb) info frame  # Проверить stack frame
```

#### Защита

**Вариант 1: std::string (Рекомендуется)**:
```cpp
class SafeMessageBuilder {
private:
    std::string header_;  // Автоматическое управление памятью
    static constexpr size_t MAX_HEADER = 64;
    
public:
    SafeMessageBuilder& setHeader(const std::string& header) {
        if (header.length() > MAX_HEADER) {
            throw std::length_error("Header exceeds maximum length: " + 
                std::to_string(header.length()) + " > " + std::to_string(MAX_HEADER));
        }
        header_ = header;
        return *this;
    }
};
```

**Вариант 2: Безопасные C-функции**:
```cpp
class SafeCBuilder {
private:
    char header_[64];
    
public:
    SafeCBuilder& setHeader(const char* header) {
        if (!header) {
            throw std::invalid_argument("Null pointer");
        }
        
        size_t len = strnlen(header, sizeof(header_));
        if (len >= sizeof(header_)) {
            throw std::length_error("Header too long");
        }
        
        strncpy(header_, header, sizeof(header_) - 1);
        header_[sizeof(header_) - 1] = '\0';  // Гарантированный null-terminator
        
        return *this;
    }
};
```

**Вариант 3: std::span (C++20)**:
```cpp
class ModernBuilder {
private:
    std::array<char, 64> header_;
    
public:
    ModernBuilder& setHeader(std::span<const char> data) {
        if (data.size() >= header_.size()) {
            throw std::length_error("Header too long");
        }
        
        std::copy(data.begin(), data.end(), header_.begin());
        header_[data.size()] = '\0';
        
        return *this;
    }
};
```

### 2. Integer Overflow (CWE-190/191)

#### Описание
Integer overflow происходит когда результат арифметической операции превышает максимальное значение типа данных, вызывая wrap-around к маленькому значению.

#### Технические детали

**Signed Integer Overflow**:
```cpp
// int32_t max = 2,147,483,647
int32_t a = INT32_MAX;
int32_t b = a + 1;  // Overflow → -2,147,483,648 (undefined behavior!)
```

**Unsigned Integer Overflow**:
```cpp
// size_t на 64-bit = 18,446,744,073,709,551,615
size_t a = SIZE_MAX;
size_t b = a + 1;  // Wrap-around → 0 (defined behavior, но опасно!)
```

**Уязвимость в Builder**:
```cpp
class DataBlockBuilder {
private:
    size_t chunk_size_;
    size_t chunk_count_;
    
public:
    DataBlock build() {
        // КРИТИЧЕСКАЯ УЯЗВИМОСТЬ: Integer overflow
        size_t total_size = chunk_size_ * chunk_count_;
        
        if (total_size == 0) {
            throw std::runtime_error("Invalid size");
        }
        
        // Выделяем МЕНЬШЕ памяти чем планировалось!
        char* data = new char[total_size];
        
        // Последующая запись вызовет heap overflow!
        return DataBlock(data, total_size);
    }
};
```

#### Exploitation сценарий

**Атака**:
```cpp
// SIZE_MAX = 18,446,744,073,709,551,615 (на 64-bit)
// Выбираем: chunk_size = SIZE_MAX / 2 + 1

builder.setChunkSize(SIZE_MAX / 2 + 1);  // 9,223,372,036,854,775,808
builder.setChunkCount(2);

// Overflow:
// (SIZE_MAX / 2 + 1) * 2 = SIZE_MAX + 2 (mod 2^64) = 1

auto block = builder.build();
// Выделено: new char[1] - только 1 байт!
// Ожидалось: ~18 exabytes

// Последующая запись:
memset(block.data, 0, chunk_size * chunk_count);  // HEAP OVERFLOW!
```

**Последствия**:
1. Heap metadata corruption
2. Heap spray attack
3. Use-after-free exploitation
4. Arbitrary write primitive

#### Математика overflow'а

**Формула проверки умножения**:
```
if (a > MAX / b) → overflow
```

**Почему это работает**:
```
a * b > MAX
⟺ a > MAX / b  (деление целое)
```

**Пример**:
```cpp
size_t a = 1000000000;  // 1 billion
size_t b = 20000000000; // 20 billion
size_t max = SIZE_MAX;  // ~18 quintillion

// Проверка: a > max / b?
// 1,000,000,000 > 18,446,744,073,709,551,615 / 20,000,000,000
// 1,000,000,000 > 922
// true → overflow!
```

#### Детектирование

**UndefinedBehaviorSanitizer (UBSan)**:
```bash
g++ -fsanitize=undefined -fno-sanitize-recover=all -g \
    builder_vulnerabilities.cpp -o builder_ubsan

./builder_ubsan

# Вывод:
builder_vulnerabilities.cpp:123:25: runtime error: 
    signed integer overflow: 2147483647 + 1 cannot be represented in type 'int'
```

**Статический анализ (Clang)**:
```bash
clang++ --analyze -Xanalyzer -analyzer-checker=core.UndefinedBinaryOperatorResult \
    builder_vulnerabilities.cpp
```

**CodeQL query**:
```ql
import cpp

from BinaryOperation op
where op.getOperator() = "*" 
  and op.getType().getSize() <= 8
  and not exists(ComparisonOperation cmp |
    cmp.getAnOperand() = op
  )
select op, "Unchecked multiplication may overflow"
```

#### Защита

**Вариант 1: Compiler builtin (GCC/Clang)**:
```cpp
class SafeDataBlockBuilder {
public:
    DataBlock build() {
        size_t total_size;
        
        // Безопасное умножение с проверкой overflow
        if (__builtin_mul_overflow(chunk_size_, chunk_count_, &total_size)) {
            throw std::overflow_error(
                "Size calculation overflow: " + 
                std::to_string(chunk_size_) + " * " + 
                std::to_string(chunk_count_)
            );
        }
        
        if (total_size > MAX_ALLOCATION) {
            throw std::length_error("Allocation too large: " + std::to_string(total_size));
        }
        
        return DataBlock(new char[total_size], total_size);
    }
};
```

**Вариант 2: Manual check**:
```cpp
bool safe_multiply(size_t a, size_t b, size_t* result) {
    if (b != 0 && a > SIZE_MAX / b) {
        return false;  // Overflow
    }
    *result = a * b;
    return true;
}

DataBlock build() {
    size_t total_size;
    if (!safe_multiply(chunk_size_, chunk_count_, &total_size)) {
        throw std::overflow_error("Size calculation overflow");
    }
    // ...
}
```

**Вариант 3: C++20 std::integral (будущее)**:
```cpp
#include <bit>

DataBlock build() {
    auto result = std::mul_overflow(chunk_size_, chunk_count_);
    if (!result) {
        throw std::overflow_error("Overflow");
    }
    return DataBlock(new char[*result], *result);
}
```

### 3. Validation Bypass (CWE-20)

#### Описание
Validation bypass происходит когда валидация данных отделена от их использования во времени или месте, позволяя установить некорректные значения.

#### Проблема традиционного Builder

**Антипаттерн**:
```cpp
class UserBuilder {
private:
    User user_;
    
public:
    // Setters БЕЗ валидации
    UserBuilder& setAge(int age) {
        user_.age = age;  // Может быть отрицательным!
        return *this;
    }
    
    UserBuilder& setRole(Role role) {
        user_.role = role;  // Может быть ADMIN!
        return *this;
    }
    
    // Валидация ТОЛЬКО в build
    User build() {
        if (user_.age < 0 || user_.age > 150) {
            throw std::invalid_argument("Invalid age");
        }
        if (user_.role == Role::ADMIN && !hasAdminPermission()) {
            throw std::runtime_error("Unauthorized admin role");
        }
        return user_;
    }
};
```

**Проблемы**:
1. **Временное окно**: Между `setAge(-1)` и `build()` объект в некорректном состоянии
2. **Partial state**: Можно прочитать `user_.age` до валидации
3. **Exception safety**: Если `build()` бросит исключение, builder в неконсистентном состоянии

#### Exploitation через TOCTOU (Time-Of-Check-Time-Of-Use)

**Сценарий**:
```cpp
class AccountBuilder {
private:
    Account account_;
    
public:
    AccountBuilder& setBalance(double balance) {
        account_.balance = balance;
        return *this;
    }
    
    AccountBuilder& setCreditLimit(double limit) {
        account_.credit_limit = limit;
        return *this;
    }
    
    Account build() {
        // Time of Check
        if (account_.balance < 0) {
            throw std::invalid_argument("Negative balance");
        }
        
        // Time of Use (слишком поздно!)
        return account_;
    }
};

// АТАКА
AccountBuilder builder;
builder.setBalance(-1000000);  // Установили некорректное значение
builder.setCreditLimit(999999999);  // Огромный лимит

// В другом потоке:
std::thread attacker([&builder]() {
    // Меняем balance на валидный ПОСЛЕ проверки но ДО копирования
    std::this_thread::sleep_for(std::chrono::microseconds(50));
    // Прямой доступ к account_.balance (если есть getter)
});

auto account = builder.build();  // Может пройти с некорректными данными
```

#### Method Chaining и пропуск валидации

**Проблема**:
```cpp
auto user = UserBuilder()
    .setName("attacker")
    .setAge(-1)         // НЕТ валидации здесь
    .setRole(ADMIN)     // НЕТ валидации здесь
    .setBalance(999999) // НЕТ валидации здесь
    .build();           // Валидация ЗДЕСЬ - слишком поздно!
```

**Что может пойти не так**:
1. Каждый setter может записать в лог (утечка некорректных данных)
2. Другой поток может прочитать промежуточное состояние
3. Исключение в одном setter оставляет builder в partial state
4. Можно вызвать `build()` несколько раз с разным состоянием

#### CVE примеры

- **CVE-2019-11358 (jQuery)**: Prototype pollution через $.extend без валидации
- **CVE-2021-22893 (Pulse Secure)**: Authentication bypass через parameter pollution
- **CVE-2020-8115 (npm)**: Privilege escalation через validation bypass

#### Защита

**Паттерн 1: Validation в каждом setter**:
```cpp
class SecureUserBuilder {
private:
    std::optional<std::string> name_;
    std::optional<int> age_;
    Role role_ = Role::USER;
    
public:
    SecureUserBuilder& setName(const std::string& name) {
        if (name.empty() || name.length() > 100) {
            throw std::invalid_argument("Invalid name length");
        }
        if (!std::regex_match(name, std::regex("^[a-zA-Z0-9_]+$"))) {
            throw std::invalid_argument("Invalid name characters");
        }
        name_ = name;
        return *this;
    }
    
    SecureUserBuilder& setAge(int age) {
        if (age < 0 || age > 150) {
            throw std::invalid_argument(
                "Invalid age: " + std::to_string(age) + " (must be 0-150)"
            );
        }
        age_ = age;
        return *this;
    }
    
    SecureUserBuilder& setAdminRole(const std::string& authorization_token) {
        if (!verifyAdminToken(authorization_token)) {
            throw std::runtime_error("Unauthorized: invalid admin token");
        }
        role_ = Role::ADMIN;
        return *this;
    }
    
    User build() {
        // Проверка COMPLETENESS
        if (!name_ || !age_) {
            throw std::runtime_error("Missing required fields");
        }
        
        return User(*name_, *age_, role_);
    }
};
```

**Паттерн 2: Step Builder (Compile-time validation)**:
```cpp
// Используем type system для гарантии порядка
class UserBuilderStart {
public:
    UserBuilderWithName setName(const std::string& name);
};

class UserBuilderWithName {
    std::string name_;
public:
    UserBuilderWithAge setAge(int age);
};

class UserBuilderWithAge {
    std::string name_;
    int age_;
public:
    UserBuilderComplete setRole(Role role);
};

class UserBuilderComplete {
    std::string name_;
    int age_;
    Role role_;
public:
    User build() {
        return User(name_, age_, role_);
    }
};

// Использование:
User user = UserBuilderStart()
    .setName("alice")   // Возвращает UserBuilderWithName
    .setAge(25)         // Возвращает UserBuilderWithAge
    .setRole(USER)      // Возвращает UserBuilderComplete
    .build();           // Можно вызвать только здесь!

// НЕЛЬЗЯ:
// UserBuilderStart().build();  // Compile error!
// UserBuilderStart().setAge(25);  // Compile error!
```

**Паттерн 3: Immutable builder state**:
```cpp
class ImmutableUserBuilder {
private:
    const std::string name_;
    const int age_;
    const Role role_;
    
    ImmutableUserBuilder(std::string n, int a, Role r)
        : name_(std::move(n)), age_(a), role_(r) {}
    
public:
    static ImmutableUserBuilder create() {
        return ImmutableUserBuilder("", 0, Role::USER);
    }
    
    ImmutableUserBuilder withName(const std::string& name) const {
        if (name.empty()) throw std::invalid_argument("Empty name");
        return ImmutableUserBuilder(name, age_, role_);
    }
    
    ImmutableUserBuilder withAge(int age) const {
        if (age < 0) throw std::invalid_argument("Invalid age");
        return ImmutableUserBuilder(name_, age, role_);
    }
    
    User build() const {
        if (name_.empty()) throw std::runtime_error("Name required");
        return User(name_, age_, role_);
    }
};

// Использование (functional style):
auto user = ImmutableUserBuilder::create()
    .withName("alice")
    .withAge(25)
    .build();
```

### 4. Incomplete Object Usage (CWE-665)

#### Описание
Incomplete object usage происходит когда частично сконструированный объект используется до завершения построения, что приводит к undefined behavior.

#### Проблема partial state

**Антипаттерн**:
```cpp
class DatabaseConfigBuilder {
private:
    DatabaseConfig* config_;  // Указатель на неполный объект!
    
public:
    DatabaseConfigBuilder() {
        config_ = new DatabaseConfig();
    }
    
    DatabaseConfigBuilder& setHost(const std::string& host) {
        config_->host = host;
        return *this;
    }
    
    DatabaseConfigBuilder& setPort(int port) {
        config_->port = port;
        return *this;
    }
    
    // ОПАСНО: Можно получить неполный объект!
    DatabaseConfig* getPartial() {
        return config_;
    }
    
    DatabaseConfig build() {
        DatabaseConfig result = *config_;
        delete config_;
        config_ = new DatabaseConfig();
        return result;
    }
};
```

**Exploitation**:
```cpp
DatabaseConfigBuilder builder;
builder.setHost("localhost");
// НЕ установили port, credentials, database name...

// Получаем неполный объект
DatabaseConfig* partial = builder.getPartial();

// ОШИБКА: Используем объект с неинициализированными полями
partial->connect();  // port = 0, credentials = nullptr → crash или security bypass
```

#### Последствия incomplete state

**Null pointer dereference**:
```cpp
struct ServiceConfig {
    std::string* endpoint;  // Может быть nullptr!
    int timeout;
};

ServiceConfigBuilder builder;
builder.setTimeout(30);
// Не установили endpoint

auto config = builder.getPartial();
std::string url = *config->endpoint;  // SEGFAULT!
```

**Uninitialized memory**:
```cpp
struct NetworkPacket {
    char header[32];
    uint32_t payload_size;  // Может быть не инициализирован
    char* payload;
};

PacketBuilder builder;
builder.setHeader("DATA");
// Не установили payload_size

auto packet = builder.getPartial();
char* data = new char[packet->payload_size];  // payload_size = garbage!
```

**Security bypass**:
```cpp
struct AuthRequest {
    std::string username;
    std::string password;
    bool is_verified = false;  // По умолчанию false
};

AuthRequestBuilder builder;
builder.setUsername("attacker");
// Не установили password, но is_verified = false

auto request = builder.getPartial();
if (request->is_verified) {  // false
    // Но дальше может быть проверка только username
    grantAccess(request->username);
}
```

#### CVE примеры

- **CVE-2020-13935 (Apache Tomcat)**: Use of uninitialized object leading to information disclosure
- **CVE-2019-5736 (runc)**: Container escape через incomplete initialization

#### Защита

**Вариант 1: std::optional для всех полей**:
```cpp
class SafeDatabaseBuilder {
private:
    std::optional<std::string> host_;
    std::optional<int> port_;
    std::optional<std::string> username_;
    std::optional<std::string> password_;
    
public:
    SafeDatabaseBuilder& setHost(const std::string& host) {
        if (host.empty()) throw std::invalid_argument("Empty host");
        host_ = host;
        return *this;
    }
    
    SafeDatabaseBuilder& setPort(int port) {
        if (port <= 0 || port > 65535) throw std::invalid_argument("Invalid port");
        port_ = port;
        return *this;
    }
    
    // НЕТ метода getPartial()!
    
    DatabaseConfig build() {
        // Проверка COMPLETENESS
        if (!host_) throw std::runtime_error("Host not set");
        if (!port_) throw std::runtime_error("Port not set");
        if (!username_) throw std::runtime_error("Username not set");
        if (!password_) throw std::runtime_error("Password not set");
        
        return DatabaseConfig(*host_, *port_, *username_, *password_);
    }
};
```

**Вариант 2: Required fields в конструкторе**:
```cpp
class HybridBuilder {
private:
    std::string host_;  // Обязательное поле
    int port_;          // Обязательное поле
    std::optional<int> timeout_;  // Опциональное
    std::optional<bool> ssl_;     // Опциональное
    
public:
    // Обязательные поля в конструкторе!
    HybridBuilder(const std::string& host, int port)
        : host_(host), port_(port) {
        
        if (host.empty()) throw std::invalid_argument("Empty host");
        if (port <= 0 || port > 65535) throw std::invalid_argument("Invalid port");
    }
    
    HybridBuilder& setTimeout(int timeout) {
        timeout_ = timeout;
        return *this;
    }
    
    Config build() {
        return Config(host_, port_, timeout_.value_or(30), ssl_.value_or(false));
    }
};

// Использование:
auto config = HybridBuilder("localhost", 5432)
    .setTimeout(60)
    .build();
```

**Вариант 3: Compile-time completeness (C++20 concepts)**:
```cpp
template<typename... Set>
class TypedBuilder {
private:
    std::variant<std::monostate, typename Set::type...> values_;
    
public:
    template<typename T>
    requires (!std::is_same_v<T, typename Set::type> && ...)
    auto set(typename T::type value) {
        return TypedBuilder<Set..., T>(/* ... */);
    }
    
    template<typename... All>
    requires (sizeof...(All) == RequiredFieldCount)
    Object build() {
        return Object(std::get<typename All::type>(values_)...);
    }
};
```

### 5. State Pollution Between Builds (CWE-362/366)

#### Описание
State pollution происходит когда builder не очищает свое состояние после `build()`, что приводит к утечке данных между последовательными построениями.

#### Проблема shared state

**Уязвимый код**:
```cpp
class HttpRequestBuilder {
private:
    std::string method_;
    std::string url_;
    std::vector<std::string> headers_;
    std::string body_;
    
public:
    HttpRequestBuilder& setMethod(const std::string& method) {
        method_ = method;
        return *this;
    }
    
    HttpRequestBuilder& addHeader(const std::string& header) {
        headers_.push_back(header);  // НАКАПЛИВАЮТСЯ!
        return *this;
    }
    
    HttpRequest build() {
        return HttpRequest(method_, url_, headers_, body_);
        // State НЕ очищен!
    }
};
```

**Exploitation - Session Hijacking**:
```cpp
HttpRequestBuilder builder;  // Shared builder

// User 1: Аутентифицированный запрос
auto req1 = builder
    .setMethod("GET")
    .setUrl("/api/user/profile")
    .addHeader("Authorization: Bearer USER1_SECRET_TOKEN_123456")
    .addHeader("X-User-ID: 12345")
    .build();

sendRequest(req1);

// User 2: Использует ТОТ ЖЕ builder
auto req2 = builder
    .setMethod("POST")
    .setUrl("/api/public/data")
    .setBody("{\"data\": \"public\"}")
    .build();

// req2.headers содержит:
// - "Authorization: Bearer USER1_SECRET_TOKEN_123456"  ← УТЕЧКА!
// - "X-User-ID: 12345"  ← УТЕЧКА!

sendRequest(req2);  // Запрос User2 с токеном User1!
```

#### Сценарии атак

**1. Cross-User Authentication Leak**:
```cpp
// Веб-сервер с shared builder
RequestBuilder global_builder;

void handleRequest(User& user, const Request& req) {
    auto internal_req = global_builder
        .setAuth(user.getToken())  // Токен текущего пользователя
        .setUrl(req.url)
        .build();
    
    // Токен остается в builder!
}

// Следующий запрос получит предыдущий токен
```

**2. Data Leakage в Logs**:
```cpp
LogEntryBuilder logger;

// Запись 1: С PII
logger.addField("user_id", "12345")
      .addField("ssn", "123-45-6789")  // Sensitive!
      .write();

// Запись 2: Должна быть без PII
logger.addField("action", "login")
      .write();

// Лог 2 содержит: user_id, ssn, action  ← Утечка PII!
```

**3. SQL Injection через pollution**:
```cpp
QueryBuilder query_builder;

// Запрос 1: С фильтром
auto query1 = query_builder
    .from("users")
    .where("id = 1")
    .build();

// Запрос 2: Без where (должен быть SELECT * FROM products)
auto query2 = query_builder
    .from("products")
    .build();

// query2 = "SELECT * FROM products WHERE id = 1"  ← Pollution!
```

#### Детектирование

**Статический анализ - Custom Clang checker**:
```cpp
// Checker для обнаружения отсутствия reset()
class BuilderStateChecker : public Checker<check::PostCall> {
public:
    void checkPostCall(const CallEvent &Call, CheckerContext &C) const {
        if (Call.getCalleeIdentifier()->getName() == "build") {
            // Проверяем, есть ли reset() после build()
            // ...
        }
    }
};
```

**Динамический анализ - Instrumentation**:
```cpp
#ifdef DEBUG
class InstrumentedBuilder {
    bool built_ = false;
    
public:
    Builder& setSomething(auto value) {
        if (built_) {
            std::cerr << "WARNING: Modifying builder after build()\n";
            std::cerr << "Stack trace:\n";
            printStackTrace();
        }
        // ...
    }
    
    Object build() {
        built_ = true;
        // ...
    }
};
#endif
```

#### Защита

**Вариант 1: Auto-reset в build()**:
```cpp
class CleanBuilder {
private:
    std::string method_;
    std::vector<std::string> headers_;
    
    void reset() {
        method_.clear();
        headers_.clear();
    }
    
public:
    HttpRequest build() {
        HttpRequest req(method_, headers_);
        reset();  // Автоматическая очистка!
        return req;
    }
};
```

**Вариант 2: Move semantics (consume builder)**:
```cpp
class ConsumableBuilder {
private:
    std::string data_;
    
public:
    // build() принимает rvalue (&&)
    HttpRequest build() && {
        return HttpRequest(std::move(data_));
    }
};

// Использование:
auto req = ConsumableBuilder()
    .setData("...")
    .build();  // Builder уничтожен после build()

// НЕЛЬЗЯ:
ConsumableBuilder builder;
builder.setData("...");
auto req1 = builder.build();  // Compile error! (lvalue)
auto req2 = std::move(builder).build();  // OK, но builder больше нельзя использовать
```

**Вариант 3: Stateless builder (factory function)**:
```cpp
// Builder как фабричная функция
class RequestFactory {
public:
    static HttpRequest create(
        const std::string& method,
        const std::string& url,
        const std::vector<std::string>& headers = {}
    ) {
        // Нет state - нет pollution!
        return HttpRequest(method, url, headers);
    }
    
    static HttpRequest createGet(const std::string& url) {
        return create("GET", url);
    }
    
    static HttpRequest createPost(const std::string& url, const std::string& body) {
        return create("POST", url, {"Content-Type: application/json"});
    }
};

// Использование:
auto req1 = RequestFactory::createGet("/api/users");
auto req2 = RequestFactory::createPost("/api/data", "{...}");
// Нет shared state!
```

### 6. Injection через Builder Parameters (CWE-89/77/78)

#### Описание
Injection атаки происходят когда параметры builder'а используются для построения команд (SQL, shell, LDAP) без надлежащей санитизации или параметризации.

#### SQL Injection

**Уязвимый Query Builder**:
```cpp
class VulnerableQueryBuilder {
private:
    std::string table_;
    std::vector<std::string> columns_;
    std::string where_clause_;
    
public:
    VulnerableQueryBuilder& from(const std::string& table) {
        table_ = table;  // Нет валидации!
        return *this;
    }
    
    VulnerableQueryBuilder& select(const std::string& column) {
        columns_.push_back(column);  // Нет валидации!
        return *this;
    }
    
    VulnerableQueryBuilder& where(const std::string& condition) {
        where_clause_ = condition;  // SQL INJECTION!
        return *this;
    }
    
    std::string build() {
        std::string sql = "SELECT ";
        
        if (columns_.empty()) {
            sql += "*";
        } else {
            for (size_t i = 0; i < columns_.size(); ++i) {
                sql += columns_[i];  // Injection здесь!
                if (i < columns_.size() - 1) sql += ", ";
            }
        }
        
        sql += " FROM " + table_;  // Injection здесь!
        
        if (!where_clause_.empty()) {
            sql += " WHERE " + where_clause_;  // Injection здесь!
        }
        
        return sql;
    }
};
```

**Exploitation примеры**:

**1. Union-based SQL Injection**:
```cpp
auto query = VulnerableQueryBuilder()
    .from("users")
    .select("username, password")
    .where("id = 1 UNION SELECT credit_card, cvv FROM payments")
    .build();

// SQL: SELECT username, password FROM users 
//      WHERE id = 1 UNION SELECT credit_card, cvv FROM payments
```

**2. Boolean-based Blind SQL Injection**:
```cpp
auto query = VulnerableQueryBuilder()
    .from("users")
    .where("id = 1 AND (SELECT SUBSTRING(password,1,1) FROM admins LIMIT 1) = 'a'")
    .build();

// Брутфорс пароля администратора по одному символу
```

**3. Time-based Blind SQL Injection**:
```cpp
auto query = VulnerableQueryBuilder()
    .from("users")
    .where("id = 1 AND IF((SELECT password FROM admins LIMIT 1) LIKE 'a%', SLEEP(5), 0)")
    .build();

// Если пароль начинается с 'a', запрос задержится на 5 секунд
```

**4. Stacked Queries**:
```cpp
auto query = VulnerableQueryBuilder()
    .from("users")
    .where("id = 1; DROP TABLE users; --")
    .build();

// SQL: SELECT * FROM users WHERE id = 1; DROP TABLE users; --
```

#### Command Injection

**Уязвимый System Command Builder**:
```cpp
class SystemCommandBuilder {
private:
    std::string command_;
    std::vector<std::string> args_;
    
public:
    SystemCommandBuilder& setCommand(const std::string& cmd) {
        command_ = cmd;
        return *this;
    }
    
    SystemCommandBuilder& addArg(const std::string& arg) {
        args_.push_back(arg);  // Нет экранирования!
        return *this;
    }
    
    void execute() {
        std::string full_cmd = command_;
        for (const auto& arg : args_) {
            full_cmd += " " + arg;  // COMMAND INJECTION!
        }
        
        system(full_cmd.c_str());  // ОПАСНО!
    }
};
```

**Exploitation**:
```cpp
auto cmd = SystemCommandBuilder()
    .setCommand("ls")
    .addArg("-la")
    .addArg("; rm -rf /")  // Command injection!
    .execute();

// Выполняется: ls -la ; rm -rf /
```

#### Защита

**SQL Injection - Prepared Statements**:
```cpp
class SecureQueryBuilder {
private:
    std::string table_;
    std::vector<std::string> columns_;
    std::vector<std::pair<std::string, std::string>> where_params_;
    
    static bool isValidIdentifier(const std::string& name) {
        // Только буквы, цифры, подчеркивания
        return std::regex_match(name, std::regex("^[a-zA-Z_][a-zA-Z0-9_]*$"));
    }
    
public:
    SecureQueryBuilder& from(const std::string& table) {
        if (!isValidIdentifier(table)) {
            throw std::invalid_argument("Invalid table name: " + table);
        }
        table_ = table;
        return *this;
    }
    
    SecureQueryBuilder& select(const std::string& column) {
        if (!isValidIdentifier(column)) {
            throw std::invalid_argument("Invalid column name: " + column);
        }
        columns_.push_back(column);
        return *this;
    }
    
    SecureQueryBuilder& where(const std::string& column, const std::string& value) {
        if (!isValidIdentifier(column)) {
            throw std::invalid_argument("Invalid column name: " + column);
        }
        where_params_.push_back({column, value});  // Параметризовано!
        return *this;
    }
    
    PreparedStatement build() {
        std::string sql = "SELECT ";
        sql += columns_.empty() ? "*" : join(columns_, ", ");
        sql += " FROM " + table_;
        
        if (!where_params_.empty()) {
            sql += " WHERE ";
            for (size_t i = 0; i < where_params_.size(); ++i) {
                sql += where_params_[i].first + " = ?";
                if (i < where_params_.size() - 1) sql += " AND ";
            }
        }
        
        PreparedStatement stmt(sql);
        for (size_t i = 0; i < where_params_.size(); ++i) {
            stmt.bind(i + 1, where_params_[i].second);  // Безопасный bind
        }
        
        return stmt;
    }
};

// Использование:
auto query = SecureQueryBuilder()
    .from("users")
    .select("username")
    .where("id", user_input)  // Безопасно! Параметризовано
    .build();
```

**Command Injection - Whitelist + Escaping**:
```cpp
class SecureSystemCommand {
private:
    std::string command_;
    std::vector<std::string> args_;
    
    static const std::set<std::string> ALLOWED_COMMANDS = {
        "ls", "pwd", "date", "whoami", "echo"
    };
    
    static std::string escapeShellArg(const std::string& arg) {
        std::string escaped;
        for (char c : arg) {
            if (c == '\'' || c == '\\') {
                escaped += '\\';
            }
            escaped += c;
        }
        return "'" + escaped + "'";
    }
    
public:
    SecureSystemCommand& setCommand(const std::string& cmd) {
        if (ALLOWED_COMMANDS.find(cmd) == ALLOWED_COMMANDS.end()) {
            throw std::invalid_argument("Command not allowed: " + cmd);
        }
        command_ = cmd;
        return *this;
    }
    
    SecureSystemCommand& addArg(const std::string& arg) {
        args_.push_back(escapeShellArg(arg));  // Экранирование!
        return *this;
    }
    
    void execute() {
        std::vector<char*> argv;
        argv.push_back(const_cast<char*>(command_.c_str()));
        
        for (const auto& arg : args_) {
            argv.push_back(const_cast<char*>(arg.c_str()));
        }
        argv.push_back(nullptr);
        
        // execv НЕ интерпретирует shell метасимволы!
        execv(command_.c_str(), argv.data());
    }
};
```

## Инструменты анализа (детально)

### Статический анализ

#### 1. Clang Static Analyzer

**Базовое использование**:
```bash
clang++ --analyze \
    -Xanalyzer -analyzer-output=text \
    -Xanalyzer -analyzer-checker=core,cplusplus,security \
    builder_vulnerabilities.cpp
```

**Специализированные checkers**:
```bash
# Buffer overflow checker
clang++ --analyze \
    -Xanalyzer -analyzer-checker=unix.cstring.BufferOverlap \
    -Xanalyzer -analyzer-checker=security.insecureAPI.strcpy \
    builder_vulnerabilities.cpp

# Integer overflow checker  
clang++ --analyze \
    -Xanalyzer -analyzer-checker=core.UndefinedBinaryOperatorResult \
    builder_vulnerabilities.cpp
```

**Custom checker для Builder паттерна**:
```cpp
// BuilderPatternChecker.cpp
class BuilderPatternChecker : public Checker<check::PostCall> {
public:
    void checkPostCall(const CallEvent &Call, CheckerContext &C) const {
        const auto *FD = dyn_cast_or_null<FunctionDecl>(Call.getDecl());
        if (!FD) return;
        
        // Проверяем методы build()
        if (FD->getName() == "build") {
            // Проверяем, были ли установлены обязательные поля
            checkRequiredFields(Call, C);
        }
        
        // Проверяем setters
        if (FD->getName().startswith("set")) {
            checkValidation(Call, C);
        }
    }
    
private:
    void checkValidation(const CallEvent &Call, CheckerContext &C) const {
        // Проверяем, есть ли валидация в setter
        // Если нет - выдаем warning
    }
};
```

#### 2. Cppcheck

**Расширенная конфигурация**:
```bash
cppcheck \
    --enable=all \
    --inconclusive \
    --std=c++17 \
    --platform=unix64 \
    --suppress=missingIncludeSystem \
    --template='{file}:{line}:{severity}:{id}:{message}' \
    --xml \
    builder_vulnerabilities.cpp 2> report.xml
```

**Кастомные правила для Builder**:
```xml
<!-- builder-rules.xml -->
<rule>
    <pattern>strcpy\s*\([^,]+,\s*\w+\)</pattern>
    <message>Unsafe strcpy in builder - use strncpy or std::string</message>
    <severity>error</severity>
</rule>

<rule>
    <pattern>new\s+char\[\s*(\w+)\s*\*\s*(\w+)\s*\]</pattern>
    <message>Potential integer overflow in allocation</message>
    <severity>warning</severity>
</rule>
```

#### 3. PVS-Studio

```bash
pvs-studio-analyzer analyze \
    -o builder_analysis.log \
    -j8

plog-converter \
    -t fullhtml \
    -o pvs_report \
    builder_analysis.log
```

#### 4. CodeQL (Детальный анализ)

**Query для buffer overflow**:
```ql
import cpp
import semmle.code.cpp.security.BufferWrite

from FunctionCall fc, BufferWrite bw
where fc.getTarget().getName() = "strcpy"
  and bw.getAWrite() = fc
  and not exists(Expr sizeCheck |
    sizeCheck = fc.getAnArgument().getAChild*()
    and sizeCheck.toString().matches("%sizeof%")
  )
select fc, "Unchecked strcpy call in builder pattern"
```

**Query для integer overflow**:
```ql
import cpp

class AllocationExpr extends Expr {
    AllocationExpr() {
        this instanceof NewExpr or
        this.(FunctionCall).getTarget().getName() = "malloc"
    }
    
    Expr getSizeExpr() {
        result = this.(NewExpr).getAllocatedType().getSize() or
        result = this.(FunctionCall).getArgument(0)
    }
}

from AllocationExpr alloc, MulExpr mul
where mul = alloc.getSizeExpr().getAChild*()
  and not exists(ComparisonOperation cmp |
    cmp.getAnOperand() = mul
  )
select alloc, mul, "Unchecked multiplication in allocation size"
```

**Query для validation bypass**:
```ql
import cpp

class BuilderSetter extends MemberFunction {
    BuilderSetter() {
        this.getName().matches("set%")
    }
}

from BuilderSetter setter
where not exists(ThrowExpr throw | throw.getEnclosingFunction() = setter)
  and not exists(ReturnStmt ret | 
    ret.getEnclosingFunction() = setter 
    and ret.getExpr().(BoolLiteral).getValue() = "false"
  )
select setter, "Builder setter without validation"
```

### Динамический анализ (детально)

#### AddressSanitizer (ASan)

**Продвинутое использование**:
```bash
# Компиляция с дополнительными опциями
g++ -fsanitize=address \
    -fsanitize-address-use-after-scope \
    -fno-omit-frame-pointer \
    -fno-optimize-sibling-calls \
    -O1 -g \
    builder_vulnerabilities.cpp -o builder_asan

# Runtime опции
export ASAN_OPTIONS=detect_leaks=1:check_initialization_order=1:strict_init_order=1:detect_stack_use_after_return=1:detect_invalid_pointer_pairs=2

./builder_asan
```

**Анализ ASan отчета**:
```
=================================================================
==12345==ERROR: AddressSanitizer: heap-buffer-overflow on address 0x603000000044 at pc 0x000000400b2c bp 0x7fffffffe110 sp 0x7fffffffe100
WRITE of size 8 at 0x603000000044 thread T0
    #0 0x400b2b in MessageBuilder::setHeader(char const*) builder.cpp:45
    #1 0x400c56 in main builder.cpp:123
    #2 0x7ffff7a2d082 in __libc_start_main (/lib/x86_64-linux-gnu/libc.so.6+0x24082)
    #3 0x4009ad in _start (builder_asan+0x4009ad)

0x603000000044 is located 0 bytes to the right of 68-byte region [0x603000000000,0x603000000044)
allocated by thread T0 here:
    #0 0x7ffff7b3c602 in operator new[](unsigned long) (/usr/lib/x86_64-linux-gnu/libasan.so.5+0xe9602)
    #1 0x400a89 in MessageBuilder::MessageBuilder() builder.cpp:12
    #2 0x400c1a in main builder.cpp:120

Shadow bytes around the buggy address:
  0x0c067fff7fb0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x0c067fff7fc0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x0c067fff7fd0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x0c067fff7fe0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x0c067fff7ff0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
=>0x0c067fff8000: 00 00 00 00 00 00 00 00[04]fa fa fa fa fa fa fa
  0x0c067fff8010: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
```

**Расшифровка**:
- `WRITE of size 8` - попытка записи 8 байт
- `0x603000000044` - адрес, куда пытались писать
- `0 bytes to the right` - переполнение СРАЗУ после буфера
- `Shadow bytes: [04]` - 4 байта доступны, остальное - poisoned (`fa`)

#### ThreadSanitizer (TSan)

```bash
g++ -fsanitize=thread -g -O1 builder_vulnerabilities.cpp -o builder_tsan

export TSAN_OPTIONS=second_deadlock_stack=1:history_size=7

./builder_tsan
```

**Пример вывода**:
```
WARNING: ThreadSanitizer: data race (pid=12345)
  Write of size 8 at 0x7b0400000000 by thread T2:
    #0 UserBuilder::setRole(Role) builder.cpp:234
    #1 exploit_toctou() builder.cpp:567

  Previous read of size 8 at 0x7b0400000000 by thread T1:
    #0 UserBuilder::build() builder.cpp:245
    #1 exploit_toctou() builder.cpp:556

  Location is global 'builder' of size 128 at 0x7b0400000000 (builder_tsan+0x000000404000)

  Thread T2 (tid=12347, running) created by main thread at:
    #0 pthread_create (/usr/lib/x86_64-linux-gnu/libtsan.so.0+0x2c6e3)
    #1 std::thread::_M_start_thread
    #2 exploit_toctou() builder.cpp:565
```

#### Valgrind (Детальный анализ)

**Memcheck для memory leaks**:
```bash
valgrind \
    --tool=memcheck \
    --leak-check=full \
    --show-leak-kinds=all \
    --track-origins=yes \
    --verbose \
    --log-file=valgrind-memcheck.log \
    ./builder_vulnerabilities
```

**Пример отчета**:
```
==12345== HEAP SUMMARY:
==12345==     in use at exit: 1,024 bytes in 1 blocks
==12345==   total heap usage: 15 allocs, 14 frees, 73,856 bytes allocated
==12345== 
==12345== 1,024 bytes in 1 blocks are definitely lost in loss record 1 of 1
==12345==    at 0x4C2FB0F: malloc (in /usr/lib/valgrind/vgpreload_memcheck-amd64-linux.so)
==12345==    by 0x400B23: DataBlockBuilder::build() (builder.cpp:123)
==12345==    by 0x400C45: main (builder.cpp:234)
==12345== 
==12345== LEAK SUMMARY:
==12345==    definitely lost: 1,024 bytes in 1 blocks
==12345==    indirectly lost: 0 bytes in 0 blocks
==12345==      possibly lost: 0 bytes in 0 blocks
==12345==    still reachable: 0 bytes in 0 blocks
==12345==         suppressed: 0 bytes in 0 blocks
```

**Helgrind для race conditions**:
```bash
valgrind \
    --tool=helgrind \
    --history-level=full \
    --conflict-cache-size=10000000 \
    ./builder_vulnerabilities
```

#### Fuzzing с libFuzzer

**Создание fuzz target**:
```cpp
// builder_fuzzer.cpp
#include <cstdint>
#include <cstddef>
#include "builder.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size < 4) return 0;
    
    try {
        MessageBuilder builder;
        
        // Интерпретируем данные как строки
        size_t header_size = data[0];
        size_t body_size = data[1];
        
        if (size < 2 + header_size + body_size) return 0;
        
        std::string header(reinterpret_cast<const char*>(data + 2), header_size);
        std::string body(reinterpret_cast<const char*>(data + 2 + header_size), body_size);
        
        builder.setHeader(header);
        builder.setBody(body);
        auto msg = builder.build();
        
    } catch (...) {
        // Ожидаемые исключения
    }
    
    return 0;
}
```

**Компиляция и запуск**:
```bash
# Компиляция с libFuzzer
clang++ -g -O1 -fsanitize=fuzzer,address \
    builder_fuzzer.cpp builder.cpp \
    -o builder_fuzzer

# Fuzzing
./builder_fuzzer \
    -max_len=1024 \
    -timeout=60 \
    -rss_limit_mb=2048 \
    corpus/

# Воспроизведение краша
./builder_fuzzer crash-...
```

## Практические задания (расширенные)

### Задание 1: Обнаружение Buffer Overflow

**Уровень: Начинающий**

**Задача**: Найдите buffer overflow в предоставленном коде.

**Шаги**:
```bash
# 1. Компиляция с ASan
g++ -fsanitize=address -g builder_vulnerabilities.cpp -o task1

# 2. Подготовка payload
echo "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" > payload.txt

# 3. Запуск
./task1 < payload.txt

# 4. Анализ вывода ASan
# Определите:
# - Адрес overflow
# - Размер переполнения
# - Какие данные были перезаписаны
```

**Вопросы**:
1. Какой размер буфера?
2. Сколько байт было перезаписано?
3. Какие структуры данных были повреждены?
4. Как исправить уязвимость?

**Решение** (раскрыть после попытки):
```cpp
// Проблема в:
char header_[64];
strcpy(header_, user_input);  // Нет проверки размера

// Решение:
if (user_input.length() >= sizeof(header_)) {
    throw std::length_error("Header too large");
}
strncpy(header_, user_input.c_str(), sizeof(header_) - 1);
header_[sizeof(header_) - 1] = '\0';
```

### Задание 2: Эксплуатация Integer Overflow

**Уровень: Средний**

**Задача**: Создайте эксплойт для integer overflow в размере аллокации.

**Код**:
```cpp
class AllocatorBuilder {
public:
    char* build() {
        size_t total = element_size_ * element_count_;
        return new char[total];
    }
};
```

**Задачи**:
1. Найдите пару чисел (element_size, element_count), которые вызовут overflow к маленькому значению
2. Вычислите точный размер выделенной памяти
3. Создайте payload, который вызовет heap overflow

**Подсказки**:
```python
# Python скрипт для поиска overflow
SIZE_MAX = 2**64 - 1

def find_overflow_pair(target_size):
    # Находим element_size и element_count такие что:
    # element_size * element_count ≡ target_size (mod 2^64)
    
    for element_size in range(2**32, 2**33):
        if (element_size * 2) % (2**64) == target_size:
            return element_size, 2
    
    return None

# Ищем пару для overflow в 1 байт
pair = find_overflow_pair(1)
print(f"element_size={pair[0]}, element_count={pair[1]}")
```

### Задание 3: SQL Injection через Query Builder

**Уровень: Средний**

**Задача**: Используйте SQL injection для извлечения данных.

**Код**:
```cpp
auto query = QueryBuilder()
    .from("users")
    .where(user_input)  // Инъекция здесь
    .build();
```

**Сценарии**:
1. **Union-based**: Извлеките данные из другой таблицы
2. **Boolean-based blind**: Извлеките пароль администратора по одному биту
3. **Time-based blind**: Используйте SLEEP() для определения правильности условия
4. **Stacked queries**: Создайте backdoor пользователя

**Payloads**:
```sql
-- Union-based
1 UNION SELECT username, password FROM admins

-- Boolean blind (брутфорс 1-го символа)
1 AND SUBSTRING((SELECT password FROM admins LIMIT 1), 1, 1) = 'a'

-- Time-based
1 AND IF(SUBSTRING((SELECT password FROM admins LIMIT 1), 1, 1) = 'a', SLEEP(5), 0)

-- Stacked
1; INSERT INTO users (username, password, role) VALUES ('backdoor', 'pass', 'admin')
```

### Задание 4: TOCTOU Race Condition

**Уровень: Продвинутый**

**Задача**: Эксплуатируйте race condition для privilege escalation.

**Код**:
```cpp
if (current_user->hasPermission(cmd->getRequired())) {
    // [RACE WINDOW]
    cmd->execute();
}
```

**Задачи**:
1. Определите размер race window
2. Создайте многопоточный эксплойт
3. Увеличьте вероятность успеха до >50%

**Подход**:
```cpp
// Эксплойт
std::atomic<bool> success{false};

// Thread 1: Многократные попытки
for (int i = 0; i < 10000; ++i) {
    executor.execute(admin_command);
}

// Thread 2: Rapid permission changes
while (!success) {
    user->setPermission(ADMIN);
    std::this_thread::sleep_for(std::chrono::microseconds(10));
    user->setPermission(USER);
}
```

**Метрики**:
- Race window size: измерьте с помощью `rdtsc` (CPU cycles)
- Success rate: подсчитайте успешные эскалации / попытки
- Optimization: используйте CPU affinity для увеличения вероятности

### Задание 5: Создание Exploit Chain

**Уровень: Эксперт**

**Задача**: Объедините несколько уязвимостей для полной компрометации.

**Цепочка**:
1. **Integer overflow** → маленькая аллокация
2. **Buffer overflow** → heap corruption
3. **Validation bypass** → admin privileges
4. **SQL injection** → data exfiltration

**Сценарий**:
```cpp
// Шаг 1: Integer overflow для создания маленького буфера
auto builder1 = DataBlockBuilder()
    .setSize(SIZE_MAX / 2 + 1)
    .setCount(2);  // Overflow → 1 byte

auto block = builder1.build();  // Выделен 1 байт

// Шаг 2: Heap overflow для corruption
memset(block.data, 'A', 1000);  // Перезапись heap metadata

// Шаг 3: Use-after-free для control flow hijacking
// (детали зависят от heap layout)

// Шаг 4: Получение admin через validation bypass
auto user = UserBuilder()
    .setAdmin(true)  // Bypass
    .build();

// Шаг 5: SQL injection для exfiltration
auto query = QueryBuilder()
    .from("users")
    .where("1=1; SELECT * FROM secrets INTO OUTFILE '/tmp/dump.txt'")
    .build();
```

## Заключение

Builder паттерн - один из наиболее уязвимых паттернов при работе с недоверенными данными. Ключевые выводы:

### Основные риски

**🔴 Критические (RCE/Full Compromise)**:
1. **Buffer Overflow** → Stack/Heap corruption → ROP → Remote Code Execution
2. **Integer Overflow** → Маленькая аллокация → Heap overflow → Arbitrary write
3. **Command/SQL Injection** → Database/System compromise → Data breach

**🟡 Высокие (Privilege Escalation/Data Leak)**:
4. **Validation Bypass** → Business logic bypass → Privilege escalation
5. **TOCTOU** → Race condition → Permission bypass
6. **State Pollution** → Cross-user data leak → Session hijacking

**🟢 Средние (DoS/Information Disclosure)**:
7. **Incomplete Objects** → Null pointer dereference → Crash
8. **Memory Leaks** → Resource exhaustion → Denial of Service

### Защитные принципы

1. **Input Validation** - валидация в КАЖДОМ setter, не только в build()
2. **Bounds Checking** - всегда проверяйте размеры перед операциями с памятью
3. **Overflow Protection** - проверяйте integer overflow перед арифметикой
4. **Safe Types** - используйте std::string, std::vector вместо C-arrays
5. **Completeness Guarantee** - используйте std::optional или Step Builder для гарантии полноты
6. **State Isolation** - очищайте state после build() или используйте move semantics
7. **Parameterization** - используйте prepared statements вместо конкатенации
8. **Principle of Least Privilege** - требуйте authorization для привилегированных операций

### Чек-лист безопасности

**Перед релизом проверьте**:
- [ ] Все строковые операции используют std::string или проверяют границы
- [ ] Все арифметические операции проверяют overflow (__builtin_mul_overflow)
- [ ] Каждый setter выполняет валидацию данных
- [ ] Используется std::optional для обязательных полей
- [ ] State очищается после build() или builder consumable (&&)
- [ ] SQL/Shell команды параметризованы или используют whitelist
- [ ] Нет методов для получения incomplete objects
- [ ] Код протестирован с ASan, TSan, UBSan
- [ ] Проведен статический анализ (Clang, Cppcheck, PVS-Studio)
- [ ] Fuzzing тестирование выполнено (libFuzzer, AFL)
- [ ] Code review проведен с фокусом на security
- [ ] Документация содержит security considerations

### Инструменты для CI/CD

**Минимальный набор**:
```yaml
# .github/workflows/security.yml
security-checks:
  - name: ASan build
    run: g++ -fsanitize=address -g src/**/*.cpp
  
  - name: Static analysis
    run: |
      clang++ --analyze src/**/*.cpp
      cppcheck --enable=all src/
  
  - name: Fuzzing
    run: |
      ./builder_fuzzer -max_total_time=300
```

**Золотое правило Builder безопасности**:
> "Любой Builder, принимающий пользовательский ввод, должен валидировать каждый параметр ДО его сохранения, проверять overflow ДО аллокации, и гарантировать completeness ДО возврата объекта."

Помните: Builder - это не просто удобный API, это **граница доверия** между внешним миром и вашей системой. Относитесь к нему соответственно!
