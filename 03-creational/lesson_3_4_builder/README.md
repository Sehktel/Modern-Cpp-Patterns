# Урок 3.4: Builder Pattern (Строитель)

## 🎯 Цель урока
Изучить паттерн Builder - креационный паттерн для пошагового построения сложных объектов. Понять, как отделить конструирование объекта от его представления, и как создавать различные представления используя один и тот же процесс построения.

## 📚 Что изучаем

### 1. Паттерн Builder
- **Определение**: Отделяет конструирование сложного объекта от его представления
- **Назначение**: Пошаговое создание объектов с множеством параметров
- **Применение**: Построение сложных объектов, конфигурация, парсинг, query builders
- **Решаемая проблема**: Telescoping constructor anti-pattern

### 2. Структура паттерна
- **Builder**: Интерфейс для пошагового построения
- **ConcreteBuilder**: Конкретная реализация построения
- **Director**: Управляет процессом построения (опционально)
- **Product**: Создаваемый сложный объект

### 3. Виды Builder
- **Fluent Interface Builder**: Цепочка вызовов с return *this
- **Step Builder**: Гарантирует порядок вызовов через типы
- **Telescoping Constructor**: Анти-паттерн, который решает Builder

## 🔍 Ключевые концепции

### Проблема: Telescoping Constructor
```cpp
// ❌ ПРОБЛЕМА: Множество конструкторов
class Pizza {
public:
    Pizza(int size) { }
    Pizza(int size, bool cheese) { }
    Pizza(int size, bool cheese, bool pepperoni) { }
    Pizza(int size, bool cheese, bool pepperoni, bool bacon) { }
    Pizza(int size, bool cheese, bool pepperoni, bool bacon, bool onion) { }
    // И так далее... Комбинаторный взрыв!
    
    // Проблемы:
    // - Слишком много конструкторов
    // - Непонятный порядок параметров
    // - Сложно добавлять новые параметры
    // - Невозможно сделать некоторые комбинации
};

// Использование неудобно
Pizza pizza(12, true, false, true, false);  // Что это значит?
```

**Теоретическая база**: Проблема telescoping constructor возникает из-за экспоненциального роста числа конструкторов: C(n,k) = 2^n для n опциональных параметров. Builder решает это через O(n) методов вместо O(2^n) конструкторов.

### Решение 1: Classic Builder Pattern
```cpp
// Продукт
class Pizza {
private:
    int size_;
    bool cheese_;
    bool pepperoni_;
    bool bacon_;
    bool onion_;
    bool mushrooms_;
    
    // Приватный конструктор - только Builder может создать
    Pizza(int size) : size_(size), 
                      cheese_(false),
                      pepperoni_(false),
                      bacon_(false),
                      onion_(false),
                      mushrooms_(false) {}
    
    friend class PizzaBuilder;
    
public:
    void describe() const {
        std::cout << "Пицца " << size_ << " дюймов";
        if (cheese_) std::cout << " + сыр";
        if (pepperoni_) std::cout << " + пепперони";
        if (bacon_) std::cout << " + бекон";
        if (onion_) std::cout << " + лук";
        if (mushrooms_) std::cout << " + грибы";
        std::cout << std::endl;
    }
};

// Builder
class PizzaBuilder {
private:
    std::unique_ptr<Pizza> pizza_;
    
public:
    explicit PizzaBuilder(int size) {
        pizza_ = std::unique_ptr<Pizza>(new Pizza(size));
    }
    
    PizzaBuilder& addCheese() {
        pizza_->cheese_ = true;
        return *this;
    }
    
    PizzaBuilder& addPepperoni() {
        pizza_->pepperoni_ = true;
        return *this;
    }
    
    PizzaBuilder& addBacon() {
        pizza_->bacon_ = true;
        return *this;
    }
    
    PizzaBuilder& addOnion() {
        pizza_->onion_ = true;
        return *this;
    }
    
    PizzaBuilder& addMushrooms() {
        pizza_->mushrooms_ = true;
        return *this;
    }
    
    std::unique_ptr<Pizza> build() {
        return std::move(pizza_);
    }
};

// Использование
void orderPizza() {
    auto pizza = PizzaBuilder(12)
                    .addCheese()
                    .addPepperoni()
                    .addMushrooms()
                    .build();
    
    pizza->describe();  // Ясно что мы заказали!
}
```

**Теоретическая база**: Fluent interface (цепочка вызовов) основана на монадическом паттерне из функционального программирования. Каждый метод возвращает Builder, формируя цепочку: Builder → Builder → Builder → Product.

### Решение 2: Fluent Builder (Method Chaining)
```cpp
// Более продвинутый Builder с валидацией
class HttpRequest {
private:
    std::string url_;
    std::string method_;
    std::map<std::string, std::string> headers_;
    std::string body_;
    int timeout_;
    
    HttpRequest() : method_("GET"), timeout_(30) {}
    
    friend class HttpRequestBuilder;
    
public:
    void send() const {
        std::cout << method_ << " " << url_ << std::endl;
        std::cout << "Headers: " << headers_.size() << std::endl;
        std::cout << "Timeout: " << timeout_ << "s" << std::endl;
        if (!body_.empty()) {
            std::cout << "Body: " << body_ << std::endl;
        }
    }
};

class HttpRequestBuilder {
private:
    std::unique_ptr<HttpRequest> request_;
    
public:
    HttpRequestBuilder() : request_(std::make_unique<HttpRequest>()) {}
    
    // Обязательный параметр
    HttpRequestBuilder& url(const std::string& url) {
        request_->url_ = url;
        return *this;
    }
    
    HttpRequestBuilder& method(const std::string& method) {
        request_->method_ = method;
        return *this;
    }
    
    HttpRequestBuilder& header(const std::string& key, const std::string& value) {
        request_->headers_[key] = value;
        return *this;
    }
    
    HttpRequestBuilder& body(const std::string& body) {
        request_->body_ = body;
        return *this;
    }
    
    HttpRequestBuilder& timeout(int seconds) {
        if (seconds <= 0) {
            throw std::invalid_argument("Timeout должен быть положительным");
        }
        request_->timeout_ = seconds;
        return *this;
    }
    
    std::unique_ptr<HttpRequest> build() {
        // Валидация перед построением
        if (request_->url_.empty()) {
            throw std::logic_error("URL обязателен");
        }
        
        return std::move(request_);
    }
};

// Использование
void makeHttpRequest() {
    auto request = HttpRequestBuilder()
                      .url("https://api.example.com/users")
                      .method("POST")
                      .header("Content-Type", "application/json")
                      .header("Authorization", "Bearer token123")
                      .body(R"({"name": "John", "age": 30})")
                      .timeout(60)
                      .build();
    
    request->send();
}
```

### Решение 3: Step Builder (Type-Safe)
```cpp
// Step Builder гарантирует порядок вызовов через типы
class Account {
private:
    std::string username_;
    std::string email_;
    std::string password_;
    bool verified_;
    
    Account() : verified_(false) {}
    
    template<typename... Steps>
    friend class AccountBuilderBase;
    
public:
    void display() const {
        std::cout << "Account: " << username_ 
                  << " (" << email_ << ")" 
                  << " verified: " << (verified_ ? "yes" : "no")
                  << std::endl;
    }
};

// Маркеры для шагов
struct UsernameStep {};
struct EmailStep {};
struct PasswordStep {};
struct BuildStep {};

// Базовый класс билдера
template<typename CurrentStep>
class AccountBuilderBase {
protected:
    std::unique_ptr<Account> account_;
    
    AccountBuilderBase() : account_(std::make_unique<Account>()) {}
    AccountBuilderBase(std::unique_ptr<Account> acc) : account_(std::move(acc)) {}
};

// Специализация для каждого шага
template<>
class AccountBuilderBase<UsernameStep> {
protected:
    std::unique_ptr<Account> account_;
    
public:
    AccountBuilderBase() : account_(std::make_unique<Account>()) {}
    
    AccountBuilderBase<EmailStep> username(const std::string& username) {
        account_->username_ = username;
        return AccountBuilderBase<EmailStep>(std::move(account_));
    }
};

template<>
class AccountBuilderBase<EmailStep> {
protected:
    std::unique_ptr<Account> account_;
    
public:
    AccountBuilderBase(std::unique_ptr<Account> acc) : account_(std::move(acc)) {}
    
    AccountBuilderBase<PasswordStep> email(const std::string& email) {
        account_->email_ = email;
        return AccountBuilderBase<PasswordStep>(std::move(account_));
    }
};

template<>
class AccountBuilderBase<PasswordStep> {
protected:
    std::unique_ptr<Account> account_;
    
public:
    AccountBuilderBase(std::unique_ptr<Account> acc) : account_(std::move(acc)) {}
    
    AccountBuilderBase<BuildStep> password(const std::string& password) {
        account_->password_ = password;
        return AccountBuilderBase<BuildStep>(std::move(account_));
    }
};

template<>
class AccountBuilderBase<BuildStep> {
protected:
    std::unique_ptr<Account> account_;
    
public:
    AccountBuilderBase(std::unique_ptr<Account> acc) : account_(std::move(acc)) {}
    
    AccountBuilderBase<BuildStep>& verified(bool v) {
        account_->verified_ = v;
        return *this;
    }
    
    std::unique_ptr<Account> build() {
        return std::move(account_);
    }
};

using AccountBuilder = AccountBuilderBase<UsernameStep>;

// Использование - компилятор гарантирует правильный порядок!
void createAccount() {
    auto account = AccountBuilder()
                      .username("john_doe")      // Должно быть первым
                      .email("john@example.com") // Должно быть вторым
                      .password("secret123")     // Должно быть третьим
                      .verified(true)            // Опционально
                      .build();
    
    // Это не скомпилируется:
    // AccountBuilder().email("...").username("...");  // ОШИБКА!
    
    account->display();
}
```

**Теоретическая база**: Step Builder использует phantom types из теории типов для compile-time гарантий порядка вызовов. Каждый шаг возвращает Builder с новым типом, и только финальный тип имеет метод build().

## 🤔 Вопросы для размышления

### 1. Когда использовать Builder?
**✅ Хорошие случаи:**
- Объект имеет много параметров (>4-5)
- Многие параметры опциональны
- Нужна валидация параметров
- Сложный процесс построения с шагами
- Нужны различные представления объекта

**Примеры применения:**
```cpp
// 1. Query Builder для БД
auto query = QueryBuilder()
                .select("name", "age")
                .from("users")
                .where("age > 18")
                .orderBy("name")
                .limit(10)
                .build();

// 2. Configuration Builder
auto config = ConfigBuilder()
                .setHost("localhost")
                .setPort(8080)
                .enableSSL(true)
                .setTimeout(30)
                .build();

// 3. Test Data Builder
auto user = UserBuilder()
               .withName("John")
               .withAge(30)
               .withEmail("john@test.com")
               .build();

// 4. Document Builder
auto doc = DocumentBuilder()
              .addTitle("Report")
              .addParagraph("Introduction")
              .addTable(data)
              .addFooter("Page 1")
              .build();
```

**❌ Плохие случаи:**
- Мало параметров (<3-4)
- Все параметры обязательны
- Объект прост и не требует валидации
- Не нужны различные представления

### 2. Преимущества Builder
```cpp
// ✅ Читаемый код
auto pizza = PizzaBuilder(12)
                .addCheese()
                .addPepperoni()
                .build();
// Ясно что делаем!

// ✅ Гибкость
// Можем строить разные комбинации
auto veggie = PizzaBuilder(10)
                 .addCheese()
                 .addMushrooms()
                 .addOnion()
                 .build();

auto meaty = PizzaBuilder(14)
                .addCheese()
                .addPepperoni()
                .addBacon()
                .build();

// ✅ Валидация
try {
    auto invalid = HttpRequestBuilder()
                      .method("POST")
                      .body("data")
                      .build();  // ОШИБКА: нет URL
} catch (const std::logic_error& e) {
    std::cerr << "Ошибка построения: " << e.what() << std::endl;
}

// ✅ Изоляция сложности
// Клиент не знает о внутренней структуре
// Билдер скрывает детали конструирования
```

### 3. Недостатки Builder
```cpp
// ❌ Больше кода
// Нужен отдельный класс Builder

// ❌ Overhead для простых объектов
struct SimplePoint {
    int x, y;
    
    // Не нужен Builder! Просто используйте struct
    SimplePoint(int x, int y) : x(x), y(y) {}
};

// ❌ Может быть избыточным
// Для 2-3 параметров достаточно обычного конструктора
class User {
public:
    User(const std::string& name, int age)  // Достаточно простого конструктора
        : name_(name), age_(age) {}
};
```

## 🛠️ Практические примеры

### Пример 1: SQL Query Builder
```cpp
class SqlQuery {
private:
    std::string select_;
    std::string from_;
    std::string where_;
    std::string orderBy_;
    int limit_;
    
    SqlQuery() : limit_(-1) {}
    
    friend class QueryBuilder;
    
public:
    std::string toString() const {
        std::ostringstream sql;
        sql << "SELECT " << (select_.empty() ? "*" : select_);
        sql << " FROM " << from_;
        
        if (!where_.empty()) {
            sql << " WHERE " << where_;
        }
        
        if (!orderBy_.empty()) {
            sql << " ORDER BY " << orderBy_;
        }
        
        if (limit_ > 0) {
            sql << " LIMIT " << limit_;
        }
        
        return sql.str();
    }
};

class QueryBuilder {
private:
    std::unique_ptr<SqlQuery> query_;
    
public:
    QueryBuilder() : query_(std::make_unique<SqlQuery>()) {}
    
    QueryBuilder& select(const std::string& columns) {
        query_->select_ = columns;
        return *this;
    }
    
    QueryBuilder& from(const std::string& table) {
        query_->from_ = table;
        return *this;
    }
    
    QueryBuilder& where(const std::string& condition) {
        if (!query_->where_.empty()) {
            query_->where_ += " AND ";
        }
        query_->where_ += condition;
        return *this;
    }
    
    QueryBuilder& orderBy(const std::string& column, bool ascending = true) {
        if (!query_->orderBy_.empty()) {
            query_->orderBy_ += ", ";
        }
        query_->orderBy_ += column + (ascending ? " ASC" : " DESC");
        return *this;
    }
    
    QueryBuilder& limit(int count) {
        query_->limit_ = count;
        return *this;
    }
    
    std::unique_ptr<SqlQuery> build() {
        if (query_->from_.empty()) {
            throw std::logic_error("FROM clause is required");
        }
        return std::move(query_);
    }
};

// Использование
void buildQueries() {
    // Простой запрос
    auto query1 = QueryBuilder()
                     .from("users")
                     .build();
    std::cout << query1->toString() << std::endl;
    // SELECT * FROM users
    
    // Сложный запрос
    auto query2 = QueryBuilder()
                     .select("name, age, email")
                     .from("users")
                     .where("age >= 18")
                     .where("active = true")
                     .orderBy("name")
                     .orderBy("age", false)
                     .limit(10)
                     .build();
    std::cout << query2->toString() << std::endl;
    // SELECT name, age, email FROM users 
    // WHERE age >= 18 AND active = true 
    // ORDER BY name ASC, age DESC LIMIT 10
}
```

### Пример 2: Email Builder
```cpp
class Email {
private:
    std::string from_;
    std::vector<std::string> to_;
    std::vector<std::string> cc_;
    std::vector<std::string> bcc_;
    std::string subject_;
    std::string body_;
    std::vector<std::string> attachments_;
    
    Email() {}
    
    friend class EmailBuilder;
    
public:
    void send() const {
        std::cout << "Отправка email:" << std::endl;
        std::cout << "От: " << from_ << std::endl;
        std::cout << "Кому: ";
        for (const auto& recipient : to_) {
            std::cout << recipient << " ";
        }
        std::cout << std::endl;
        std::cout << "Тема: " << subject_ << std::endl;
        std::cout << "Тело: " << body_ << std::endl;
        
        if (!cc_.empty()) {
            std::cout << "Копия: ";
            for (const auto& recipient : cc_) {
                std::cout << recipient << " ";
            }
            std::cout << std::endl;
        }
        
        if (!attachments_.empty()) {
            std::cout << "Вложения: " << attachments_.size() << std::endl;
        }
    }
};

class EmailBuilder {
private:
    std::unique_ptr<Email> email_;
    
public:
    EmailBuilder() : email_(std::make_unique<Email>()) {}
    
    EmailBuilder& from(const std::string& sender) {
        email_->from_ = sender;
        return *this;
    }
    
    EmailBuilder& to(const std::string& recipient) {
        email_->to_.push_back(recipient);
        return *this;
    }
    
    EmailBuilder& cc(const std::string& recipient) {
        email_->cc_.push_back(recipient);
        return *this;
    }
    
    EmailBuilder& bcc(const std::string& recipient) {
        email_->bcc_.push_back(recipient);
        return *this;
    }
    
    EmailBuilder& subject(const std::string& subj) {
        email_->subject_ = subj;
        return *this;
    }
    
    EmailBuilder& body(const std::string& text) {
        email_->body_ = text;
        return *this;
    }
    
    EmailBuilder& attach(const std::string& filename) {
        email_->attachments_.push_back(filename);
        return *this;
    }
    
    std::unique_ptr<Email> build() {
        // Валидация
        if (email_->from_.empty()) {
            throw std::logic_error("Sender (from) is required");
        }
        if (email_->to_.empty()) {
            throw std::logic_error("At least one recipient is required");
        }
        if (email_->subject_.empty()) {
            throw std::logic_error("Subject is required");
        }
        
        return std::move(email_);
    }
};

// Использование
void sendEmail() {
    auto email = EmailBuilder()
                    .from("sender@example.com")
                    .to("recipient1@example.com")
                    .to("recipient2@example.com")
                    .cc("manager@example.com")
                    .subject("Quarterly Report")
                    .body("Please find the attached report.")
                    .attach("report_q4.pdf")
                    .attach("summary.xlsx")
                    .build();
    
    email->send();
}
```

### Пример 3: Builder с Director
```cpp
// Продукт - Компьютер
class Computer {
private:
    std::string cpu_;
    std::string gpu_;
    int ramGB_;
    int storageGB_;
    bool hasSSD_;
    
    friend class ComputerBuilder;
    
public:
    void display() const {
        std::cout << "Computer Configuration:" << std::endl;
        std::cout << "  CPU: " << cpu_ << std::endl;
        std::cout << "  GPU: " << gpu_ << std::endl;
        std::cout << "  RAM: " << ramGB_ << "GB" << std::endl;
        std::cout << "  Storage: " << storageGB_ << "GB " 
                  << (hasSSD_ ? "SSD" : "HDD") << std::endl;
    }
};

// Builder
class ComputerBuilder {
private:
    std::unique_ptr<Computer> computer_;
    
public:
    ComputerBuilder() : computer_(std::make_unique<Computer>()) {}
    
    ComputerBuilder& setCPU(const std::string& cpu) {
        computer_->cpu_ = cpu;
        return *this;
    }
    
    ComputerBuilder& setGPU(const std::string& gpu) {
        computer_->gpu_ = gpu;
        return *this;
    }
    
    ComputerBuilder& setRAM(int gb) {
        computer_->ramGB_ = gb;
        return *this;
    }
    
    ComputerBuilder& setStorage(int gb, bool ssd = false) {
        computer_->storageGB_ = gb;
        computer_->hasSSD_ = ssd;
        return *this;
    }
    
    std::unique_ptr<Computer> build() {
        return std::move(computer_);
    }
};

// Director - управляет процессом построения
class ComputerDirector {
private:
    ComputerBuilder builder_;
    
public:
    std::unique_ptr<Computer> buildGamingPC() {
        return ComputerBuilder()
                  .setCPU("Intel Core i9-13900K")
                  .setGPU("NVIDIA RTX 4090")
                  .setRAM(64)
                  .setStorage(2000, true)
                  .build();
    }
    
    std::unique_ptr<Computer> buildOfficePC() {
        return ComputerBuilder()
                  .setCPU("Intel Core i5-13400")
                  .setGPU("Intel UHD Graphics")
                  .setRAM(16)
                  .setStorage(512, true)
                  .build();
    }
    
    std::unique_ptr<Computer> buildBudgetPC() {
        return ComputerBuilder()
                  .setCPU("AMD Ryzen 3 4100")
                  .setGPU("AMD Radeon Graphics")
                  .setRAM(8)
                  .setStorage(256, false)
                  .build();
    }
};

// Использование
void buildComputers() {
    ComputerDirector director;
    
    auto gaming = director.buildGamingPC();
    gaming->display();
    
    auto office = director.buildOfficePC();
    office->display();
    
    // Или строим кастомную конфигурацию
    auto custom = ComputerBuilder()
                     .setCPU("AMD Ryzen 7 5800X")
                     .setGPU("NVIDIA RTX 3070")
                     .setRAM(32)
                     .setStorage(1000, true)
                     .build();
    custom->display();
}
```

## 🎨 Современные альтернативы

### Named Parameters через designated initializers (C++20)
```cpp
// C++20 designated initializers как альтернатива Builder
struct Config {
    std::string host = "localhost";
    int port = 8080;
    bool ssl = false;
    int timeout = 30;
};

// Использование
Config config {
    .host = "example.com",
    .port = 443,
    .ssl = true,
    .timeout = 60
};
```

### std::optional для опциональных параметров
```cpp
#include <optional>

class ModernBuilder {
private:
    std::string required_;
    std::optional<std::string> optional1_;
    std::optional<int> optional2_;
    
public:
    explicit ModernBuilder(std::string required) 
        : required_(std::move(required)) {}
    
    ModernBuilder& setOptional1(const std::string& value) {
        optional1_ = value;
        return *this;
    }
    
    ModernBuilder& setOptional2(int value) {
        optional2_ = value;
        return *this;
    }
    
    void process() const {
        std::cout << "Required: " << required_ << std::endl;
        
        if (optional1_) {
            std::cout << "Optional1: " << *optional1_ << std::endl;
        }
        
        if (optional2_) {
            std::cout << "Optional2: " << *optional2_ << std::endl;
        }
    }
};
```

## 🚀 Современный C++

### Builder с Concepts (C++20)
```cpp
#include <concepts>

// Concept для buildable типа
template<typename T>
concept Buildable = requires(T t) {
    { t.build() } -> std::convertible_to<typename T::ProductType>;
};

// Generic функция работает с любым Builder
template<Buildable Builder>
auto constructProduct(Builder&& builder) {
    return std::forward<Builder>(builder).build();
}
```

### Variadic Builder
```cpp
// Builder с variadic templates для гибкости
template<typename Product>
class VariadicBuilder {
private:
    Product product_;
    
public:
    template<typename Func>
    VariadicBuilder& with(Func&& func) {
        func(product_);
        return *this;
    }
    
    Product build() {
        return std::move(product_);
    }
};

// Использование
auto product = VariadicBuilder<MyProduct>()
                  .with([](auto& p) { p.setName("Product"); })
                  .with([](auto& p) { p.setPrice(99.99); })
                  .build();
```

## 🎯 Практические упражнения

### Упражнение 1: HTTP Request Builder
Создайте Builder для HTTP запросов с поддержкой headers, body, timeouts.

### Упражнение 2: Step Builder
Реализуйте type-safe Step Builder для создания пользователей.

### Упражнение 3: Document Builder
Создайте Builder для построения документов (PDF/Word/HTML).

### Упражнение 4: Test Data Builder
Реализуйте Builder для генерации тестовых данных.

## 📈 Связь с другими паттернами

### Builder + Abstract Factory
```cpp
// Builder может использовать фабрику для создания частей
class ComplexObjectBuilder {
private:
    AbstractFactory& factory_;
    
public:
    ComplexObjectBuilder& addPart() {
        auto part = factory_.createPart();
        // добавляем часть
        return *this;
    }
};
```

### Builder + Composite
```cpp
// Builder для построения составных структур
class TreeBuilder {
public:
    TreeBuilder& addNode(const std::string& value) {
        // Добавляем узел в дерево
        return *this;
    }
    
    TreeBuilder& beginChildren() {
        // Начинаем добавлять детей
        return *this;
    }
    
    TreeBuilder& endChildren() {
        // Заканчиваем добавлять детей
        return *this;
    }
};
```

## 📈 Следующие шаги
После изучения Builder вы будете готовы к:
- Модулю 4: Структурные паттерны
- Пониманию других креационных паттернов
- Проектированию fluent APIs
- Написанию читаемого кода

## ⚠️ Важные предупреждения

1. **Не переусложняйте**: Для 2-3 параметров достаточно простого конструктора
2. **Валидируйте**: Проверяйте обязательные параметры в build()
3. **Используйте smart pointers**: Управляйте памятью через unique_ptr
4. **Документируйте**: Укажите какие параметры обязательны

---

*"The Builder pattern separates the construction of a complex object from its representation."* - Gang of Four

