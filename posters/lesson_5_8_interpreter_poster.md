# 🧮 Interpreter Pattern - Плакат

## 📋 Обзор паттерна

**Interpreter Pattern** - это поведенческий паттерн проектирования, который определяет грамматику языка и интерпретирует предложения этого языка. Паттерн использует рекурсивную композицию для представления грамматических правил.

## 🎯 Назначение

- **Определение грамматики**: Представляет грамматику языка в виде иерархии классов
- **Интерпретация выражений**: Выполняет интерпретацию предложений языка
- **Расширяемость**: Легко добавлять новые правила грамматики
- **Декларативность**: Грамматика описывается декларативно через классы

## 🏗️ Структура паттерна

```
┌─────────────────┐
│   Expression    │
│   (интерфейс)   │
├─────────────────┤
│ + interpret()   │
└─────────────────┘
         ▲
         │
    ┌────┴────┐
    │         │
┌───▼───┐ ┌──▼───┐
│Terminal│ │NonTerminal│
│Expression│ │Expression│
├───────┤ ├───────┤
│+interpret│ │+interpret│
└───────┘ └───────┘
                ▲
                │
        ┌───────┴───────┐
        │               │
    ┌───▼───┐       ┌───▼───┐
    │Expression│     │Expression│
    │    A    │     │    B    │
    └─────────┘     └─────────┘
```

## 💡 Ключевые компоненты

### 1. AbstractExpression (Абстрактное выражение)
```cpp
// Базовый интерфейс для всех выражений
class Expression {
public:
    virtual ~Expression() = default;
    virtual int interpret() = 0;
};
```

### 2. TerminalExpression (Терминальное выражение)
```cpp
// Конкретные выражения, которые не содержат других выражений
class NumberExpression : public Expression {
private:
    int value;
    
public:
    NumberExpression(int val) : value(val) {}
    
    int interpret() override {
        return value;
    }
};

class VariableExpression : public Expression {
private:
    std::string name;
    std::unordered_map<std::string, int>& context;
    
public:
    VariableExpression(const std::string& varName, 
                      std::unordered_map<std::string, int>& ctx) 
        : name(varName), context(ctx) {}
    
    int interpret() override {
        return context[name];
    }
};
```

### 3. NonTerminalExpression (Нетерминальное выражение)
```cpp
// Выражения, содержащие другие выражения
class AddExpression : public Expression {
private:
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;
    
public:
    AddExpression(std::unique_ptr<Expression> l, std::unique_ptr<Expression> r)
        : left(std::move(l)), right(std::move(r)) {}
    
    int interpret() override {
        return left->interpret() + right->interpret();
    }
};

class SubtractExpression : public Expression {
private:
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;
    
public:
    SubtractExpression(std::unique_ptr<Expression> l, std::unique_ptr<Expression> r)
        : left(std::move(l)), right(std::move(r)) {}
    
    int interpret() override {
        return left->interpret() - right->interpret();
    }
};

class MultiplyExpression : public Expression {
private:
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;
    
public:
    MultiplyExpression(std::unique_ptr<Expression> l, std::unique_ptr<Expression> r)
        : left(std::move(l)), right(std::move(r)) {}
    
    int interpret() override {
        return left->interpret() * right->interpret();
    }
};
```

## 🔧 Практический пример: Калькулятор выражений

```cpp
#include <iostream>
#include <memory>
#include <unordered_map>
#include <string>
#include <stack>
#include <vector>
#include <sstream>

// Базовый класс для всех выражений
class Expression {
public:
    virtual ~Expression() = default;
    virtual int interpret() = 0;
    virtual std::string toString() const = 0;
};

// Терминальные выражения
class NumberExpression : public Expression {
private:
    int value;
    
public:
    NumberExpression(int val) : value(val) {}
    
    int interpret() override {
        return value;
    }
    
    std::string toString() const override {
        return std::to_string(value);
    }
};

class VariableExpression : public Expression {
private:
    std::string name;
    std::unordered_map<std::string, int>& context;
    
public:
    VariableExpression(const std::string& varName, 
                      std::unordered_map<std::string, int>& ctx) 
        : name(varName), context(ctx) {}
    
    int interpret() override {
        auto it = context.find(name);
        if (it != context.end()) {
            return it->second;
        }
        throw std::runtime_error("Переменная '" + name + "' не определена");
    }
    
    std::string toString() const override {
        return name;
    }
};

// Нетерминальные выражения
class AddExpression : public Expression {
private:
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;
    
public:
    AddExpression(std::unique_ptr<Expression> l, std::unique_ptr<Expression> r)
        : left(std::move(l)), right(std::move(r)) {}
    
    int interpret() override {
        return left->interpret() + right->interpret();
    }
    
    std::string toString() const override {
        return "(" + left->toString() + " + " + right->toString() + ")";
    }
};

class SubtractExpression : public Expression {
private:
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;
    
public:
    SubtractExpression(std::unique_ptr<Expression> l, std::unique_ptr<Expression> r)
        : left(std::move(l)), right(std::move(r)) {}
    
    int interpret() override {
        return left->interpret() - right->interpret();
    }
    
    std::string toString() const override {
        return "(" + left->toString() + " - " + right->toString() + ")";
    }
};

class MultiplyExpression : public Expression {
private:
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;
    
public:
    MultiplyExpression(std::unique_ptr<Expression> l, std::unique_ptr<Expression> r)
        : left(std::move(l)), right(std::move(r)) {}
    
    int interpret() override {
        return left->interpret() * right->interpret();
    }
    
    std::string toString() const override {
        return "(" + left->toString() + " * " + right->toString() + ")";
    }
};

class DivideExpression : public Expression {
private:
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;
    
public:
    DivideExpression(std::unique_ptr<Expression> l, std::unique_ptr<Expression> r)
        : left(std::move(l)), right(std::move(r)) {}
    
    int interpret() override {
        int rightValue = right->interpret();
        if (rightValue == 0) {
            throw std::runtime_error("Деление на ноль");
        }
        return left->interpret() / rightValue;
    }
    
    std::string toString() const override {
        return "(" + left->toString() + " / " + right->toString() + ")";
    }
};

// Парсер для построения дерева выражений
class ExpressionParser {
private:
    std::unordered_map<std::string, int>& context;
    
public:
    ExpressionParser(std::unordered_map<std::string, int>& ctx) : context(ctx) {}
    
    std::unique_ptr<Expression> parse(const std::string& expression) {
        std::vector<std::string> tokens = tokenize(expression);
        return parseExpression(tokens, 0).first;
    }
    
private:
    std::vector<std::string> tokenize(const std::string& expression) {
        std::vector<std::string> tokens;
        std::stringstream ss(expression);
        std::string token;
        
        while (ss >> token) {
            tokens.push_back(token);
        }
        return tokens;
    }
    
    std::pair<std::unique_ptr<Expression>, int> parseExpression(
        const std::vector<std::string>& tokens, int start) {
        
        auto [left, nextPos] = parseTerm(tokens, start);
        
        while (nextPos < tokens.size() && 
               (tokens[nextPos] == "+" || tokens[nextPos] == "-")) {
            std::string op = tokens[nextPos];
            auto [right, pos] = parseTerm(tokens, nextPos + 1);
            
            if (op == "+") {
                left = std::make_unique<AddExpression>(std::move(left), std::move(right));
            } else {
                left = std::make_unique<SubtractExpression>(std::move(left), std::move(right));
            }
            nextPos = pos;
        }
        
        return {std::move(left), nextPos};
    }
    
    std::pair<std::unique_ptr<Expression>, int> parseTerm(
        const std::vector<std::string>& tokens, int start) {
        
        auto [left, nextPos] = parseFactor(tokens, start);
        
        while (nextPos < tokens.size() && 
               (tokens[nextPos] == "*" || tokens[nextPos] == "/")) {
            std::string op = tokens[nextPos];
            auto [right, pos] = parseFactor(tokens, nextPos + 1);
            
            if (op == "*") {
                left = std::make_unique<MultiplyExpression>(std::move(left), std::move(right));
            } else {
                left = std::make_unique<DivideExpression>(std::move(left), std::move(right));
            }
            nextPos = pos;
        }
        
        return {std::move(left), nextPos};
    }
    
    std::pair<std::unique_ptr<Expression>, int> parseFactor(
        const std::vector<std::string>& tokens, int start) {
        
        if (start >= tokens.size()) {
            throw std::runtime_error("Неожиданный конец выражения");
        }
        
        std::string token = tokens[start];
        
        // Число
        if (std::isdigit(token[0]) || (token[0] == '-' && token.length() > 1)) {
            int value = std::stoi(token);
            return {std::make_unique<NumberExpression>(value), start + 1};
        }
        
        // Переменная
        if (std::isalpha(token[0])) {
            return {std::make_unique<VariableExpression>(token, context), start + 1};
        }
        
        // Скобки
        if (token == "(") {
            auto [expr, pos] = parseExpression(tokens, start + 1);
            if (pos >= tokens.size() || tokens[pos] != ")") {
                throw std::runtime_error("Ожидается закрывающая скобка");
            }
            return {std::move(expr), pos + 1};
        }
        
        throw std::runtime_error("Неожиданный токен: " + token);
    }
};

// Контекст для переменных
class Context {
private:
    std::unordered_map<std::string, int> variables;
    
public:
    void setVariable(const std::string& name, int value) {
        variables[name] = value;
    }
    
    int getVariable(const std::string& name) const {
        auto it = variables.find(name);
        if (it != variables.end()) {
            return it->second;
        }
        throw std::runtime_error("Переменная '" + name + "' не определена");
    }
    
    std::unordered_map<std::string, int>& getVariables() {
        return variables;
    }
    
    void printVariables() const {
        std::cout << "Переменные: ";
        for (const auto& [name, value] : variables) {
            std::cout << name << "=" << value << " ";
        }
        std::cout << std::endl;
    }
};

// Демонстрация использования
int main() {
    std::cout << "=== Демонстрация Interpreter Pattern ===" << std::endl;
    
    // Создание контекста с переменными
    Context context;
    context.setVariable("x", 10);
    context.setVariable("y", 5);
    context.setVariable("z", 3);
    
    // Создание парсера
    ExpressionParser parser(context.getVariables());
    
    // Тестовые выражения
    std::vector<std::string> expressions = {
        "x + y",
        "x * y - z",
        "(x + y) * z",
        "x / y + z",
        "10 + 20 * 3",
        "x + y * z"
    };
    
    std::cout << "\n📊 Вычисление выражений:" << std::endl;
    context.printVariables();
    
    for (const auto& exprStr : expressions) {
        try {
            auto expression = parser.parse(exprStr);
            int result = expression->interpret();
            
            std::cout << "Выражение: " << exprStr << std::endl;
            std::cout << "Дерево: " << expression->toString() << std::endl;
            std::cout << "Результат: " << result << std::endl;
            std::cout << "---" << std::endl;
            
        } catch (const std::exception& e) {
            std::cout << "Ошибка в выражении '" << exprStr << "': " << e.what() << std::endl;
        }
    }
    
    // Интерактивный режим
    std::cout << "\n🎮 Интерактивный режим (введите 'quit' для выхода):" << std::endl;
    std::string input;
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, input);
        
        if (input == "quit") {
            break;
        }
        
        if (input.empty()) {
            continue;
        }
        
        try {
            auto expression = parser.parse(input);
            int result = expression->interpret();
            std::cout << "Результат: " << result << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Ошибка: " << e.what() << std::endl;
        }
    }
    
    return 0;
}
```

## 🎯 Применение в реальных проектах

### 1. **SQL-подобные запросы**
```cpp
// Грамматика для простых запросов
class QueryExpression {
public:
    virtual ~QueryExpression() = default;
    virtual std::vector<Record> execute(Database& db) = 0;
};

class SelectExpression : public QueryExpression {
private:
    std::vector<std::string> columns;
    std::unique_ptr<WhereExpression> whereClause;
    
public:
    std::vector<Record> execute(Database& db) override {
        // Выполнение SELECT запроса
    }
};

class WhereExpression : public QueryExpression {
private:
    std::unique_ptr<ConditionExpression> condition;
    
public:
    std::vector<Record> execute(Database& db) override {
        // Выполнение WHERE условия
    }
};
```

### 2. **Регулярные выражения**
```cpp
// Простой интерпретатор регулярных выражений
class RegexExpression {
public:
    virtual ~RegexExpression() = default;
    virtual bool match(const std::string& text) = 0;
};

class LiteralExpression : public RegexExpression {
private:
    std::string literal;
    
public:
    bool match(const std::string& text) override {
        return text.find(literal) != std::string::npos;
    }
};

class StarExpression : public RegexExpression {
private:
    std::unique_ptr<RegexExpression> expression;
    
public:
    bool match(const std::string& text) override {
        // Реализация операции *
    }
};
```

### 3. **Конфигурационные файлы**
```cpp
// Интерпретатор конфигурационных выражений
class ConfigExpression {
public:
    virtual ~ConfigExpression() = default;
    virtual std::string evaluate(const ConfigContext& context) = 0;
};

class PropertyExpression : public ConfigExpression {
private:
    std::string propertyName;
    
public:
    std::string evaluate(const ConfigContext& context) override {
        return context.getProperty(propertyName);
    }
};

class ConditionalExpression : public ConfigExpression {
private:
    std::unique_ptr<ConfigExpression> condition;
    std::unique_ptr<ConfigExpression> trueExpr;
    std::unique_ptr<ConfigExpression> falseExpr;
    
public:
    std::string evaluate(const ConfigContext& context) override {
        // Условное выражение
    }
};
```

## ⚡ Преимущества паттерна

### ✅ **Плюсы:**
- **Простота грамматики**: Легко изменять и расширять грамматику
- **Интерпретация**: Позволяет интерпретировать выражения во время выполнения
- **Декларативность**: Грамматика описывается декларативно
- **Расширяемость**: Легко добавлять новые правила

### ❌ **Минусы:**
- **Сложность**: Может быть сложен для понимания
- **Производительность**: Интерпретация медленнее компиляции
- **Ограниченность**: Подходит только для простых грамматик
- **Отладка**: Сложно отлаживать интерпретируемый код

## 🔄 Альтернативы и связанные паттерны

### **Альтернативы:**
- **Visitor Pattern**: Для обхода AST
- **Strategy Pattern**: Для различных алгоритмов интерпретации
- **Command Pattern**: Для инкапсуляции операций

### **Связанные паттерны:**
- **Composite**: Interpreter часто использует Composite для построения AST
- **Visitor**: Для обхода и обработки AST
- **Strategy**: Для различных стратегий интерпретации

## 🎨 Современные улучшения в C++

### 1. **Использование std::variant (C++17)**
```cpp
#include <variant>

// Выражения как variant
using Expression = std::variant<
    NumberExpression,
    VariableExpression,
    AddExpression,
    SubtractExpression
>;

// Посетитель для интерпретации
class InterpreterVisitor {
public:
    int interpret(const Expression& expr) {
        return std::visit([this](const auto& expression) {
            return this->visit(expression);
        }, expr);
    }
    
private:
    template<typename T>
    int visit(const T& expression) {
        return expression.interpret();
    }
};
```

### 2. **Концепты и шаблоны (C++20)**
```cpp
#include <concepts>

// Концепт для выражений
template<typename T>
concept ExpressionType = requires(T t) {
    t.interpret();
    t.toString();
};

// Шаблонный интерпретатор
template<ExpressionType... Expressions>
class TemplateInterpreter {
public:
    template<typename T>
    int interpret(const T& expression) {
        static_assert((std::is_same_v<T, Expressions> || ...));
        return expression.interpret();
    }
};
```

## 🚀 Лучшие практики

### 1. **Простая грамматика**
```cpp
// ✅ Хорошо: Простая и понятная грамматика
class SimpleCalculator {
    // Поддерживает только базовые операции: +, -, *, /
};

// ❌ Плохо: Слишком сложная грамматика
class ComplexLanguage {
    // Поддерживает все возможные конструкции языка
};
```

### 2. **Обработка ошибок**
```cpp
// ✅ Хорошо: Четкая обработка ошибок
class SafeInterpreter {
public:
    int interpret(const Expression& expr) {
        try {
            return expr.interpret();
        } catch (const std::exception& e) {
            handleError(e);
            return 0; // или другое значение по умолчанию
        }
    }
    
private:
    void handleError(const std::exception& e) {
        // Логирование и обработка ошибки
    }
};
```

### 3. **Кэширование результатов**
```cpp
// ✅ Хорошо: Кэширование для повторных вычислений
class CachedInterpreter {
private:
    std::unordered_map<std::string, int> cache;
    
public:
    int interpret(const std::string& expression) {
        auto it = cache.find(expression);
        if (it != cache.end()) {
            return it->second;
        }
        
        int result = doInterpret(expression);
        cache[expression] = result;
        return result;
    }
};
```

## 📊 Сравнение с другими паттернами

| Аспект | Interpreter | Visitor | Strategy |
|--------|-------------|---------|----------|
| **Назначение** | Интерпретация языка | Операции над объектами | Замена алгоритмов |
| **Сложность** | Высокая | Высокая | Средняя |
| **Производительность** | Низкая | Средняя | Высокая |
| **Применение** | Языки, выражения | AST, графические редакторы | Сортировка, валидация |
| **Расширяемость** | Легко добавлять правила | Легко добавлять операции | Легко добавлять алгоритмы |

## 🎯 Заключение

**Interpreter Pattern** - это мощный инструмент для создания интерпретаторов простых языков и выражений. Он особенно полезен для систем, где нужно обрабатывать пользовательский ввод в виде выражений.

### **Когда использовать:**
- ✅ Нужно интерпретировать простые языки или выражения
- ✅ Грамматика языка относительно простая
- ✅ Производительность не критична
- ✅ Нужна гибкость в изменении грамматики

### **Когда НЕ использовать:**
- ❌ Грамматика языка очень сложная
- ❌ Производительность критична
- ❌ Есть готовые библиотеки для парсинга
- ❌ Нужна компиляция, а не интерпретация

**Interpreter Pattern** идеально подходит для калькуляторов, простых языков запросов, конфигурационных систем и других приложений, где нужно интерпретировать пользовательский ввод! 🧮✨
