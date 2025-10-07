# ⛓️ Chain of Responsibility Pattern - Плакат

## 📋 Обзор паттерна

**Chain of Responsibility Pattern** - это поведенческий паттерн проектирования, который позволяет передавать запросы по цепочке обработчиков. При получении запроса каждый обработчик решает, обработать ли запрос или передать его следующему обработчику в цепочке.

## 🎯 Назначение

- **Разделение ответственности**: Разделяет обработку запросов между несколькими объектами
- **Гибкость**: Позволяет динамически изменять цепочку обработчиков
- **Слабая связанность**: Отправитель не знает, какой именно обработчик обработает запрос
- **Расширяемость**: Легко добавлять новые обработчики в цепочку

## 🏗️ Структура паттерна

```
┌─────────────────┐
│   Handler        │
│   (интерфейс)    │
├─────────────────┤
│ + setNext()     │
│ + handle()      │
│ - next          │
└─────────────────┘
         ▲
         │
    ┌────┴────┐
    │         │
┌───▼───┐ ┌──▼───┐
│Handler1│ │Handler2│
├───────┤ ├───────┤
│+handle│ │+handle│
└───────┘ └───────┘
    │         │
    └─────────┘
         │
    ┌────▼────┐
    │ Handler3 │
    ├─────────┤
    │ +handle │
    └─────────┘
```

## 💡 Ключевые компоненты

### 1. Handler (Обработчик)
```cpp
// Базовый класс для всех обработчиков
class Handler {
protected:
    std::shared_ptr<Handler> nextHandler;
    
public:
    virtual ~Handler() = default;
    
    void setNext(std::shared_ptr<Handler> handler) {
        nextHandler = handler;
    }
    
    virtual void handleRequest(const std::string& request) {
        if (nextHandler) {
            nextHandler->handleRequest(request);
        } else {
            std::cout << "Запрос не может быть обработан: " << request << std::endl;
        }
    }
};
```

### 2. ConcreteHandler (Конкретный обработчик)
```cpp
// Конкретные реализации обработчиков
class ConcreteHandler1 : public Handler {
public:
    void handleRequest(const std::string& request) override {
        if (canHandle(request)) {
            std::cout << "Обработчик 1 обрабатывает: " << request << std::endl;
        } else {
            std::cout << "Обработчик 1 передает дальше: " << request << std::endl;
            Handler::handleRequest(request);
        }
    }
    
private:
    bool canHandle(const std::string& request) {
        return request.find("тип1") != std::string::npos;
    }
};

class ConcreteHandler2 : public Handler {
public:
    void handleRequest(const std::string& request) override {
        if (canHandle(request)) {
            std::cout << "Обработчик 2 обрабатывает: " << request << std::endl;
        } else {
            std::cout << "Обработчик 2 передает дальше: " << request << std::endl;
            Handler::handleRequest(request);
        }
    }
    
private:
    bool canHandle(const std::string& request) {
        return request.find("тип2") != std::string::npos;
    }
};
```

## 🔧 Практический пример: Система обработки заявок

```cpp
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <memory>

// Базовый класс для обработчиков заявок
class RequestHandler {
protected:
    std::shared_ptr<RequestHandler> nextHandler;
    
public:
    virtual ~RequestHandler() = default;
    
    void setNext(std::shared_ptr<RequestHandler> handler) {
        nextHandler = handler;
    }
    
    virtual void handleRequest(const std::string& request, int priority) {
        if (nextHandler) {
            nextHandler->handleRequest(request, priority);
        } else {
            std::cout << "❌ Заявка не может быть обработана: " << request << std::endl;
        }
    }
    
    virtual std::string getHandlerName() const = 0;
};

// Обработчик для критических заявок
class CriticalRequestHandler : public RequestHandler {
public:
    void handleRequest(const std::string& request, int priority) override {
        if (priority >= 90) {
            std::cout << "🚨 КРИТИЧЕСКАЯ ЗАЯВКА обработана менеджером: " << request << std::endl;
            std::cout << "   Приоритет: " << priority << " (критический)" << std::endl;
        } else {
            std::cout << "📋 Менеджер передает заявку дальше: " << request << std::endl;
            RequestHandler::handleRequest(request, priority);
        }
    }
    
    std::string getHandlerName() const override {
        return "Менеджер (критические заявки)";
    }
};

// Обработчик для важных заявок
class ImportantRequestHandler : public RequestHandler {
public:
    void handleRequest(const std::string& request, int priority) override {
        if (priority >= 70 && priority < 90) {
            std::cout << "⭐ ВАЖНАЯ ЗАЯВКА обработана старшим специалистом: " << request << std::endl;
            std::cout << "   Приоритет: " << priority << " (важный)" << std::endl;
        } else {
            std::cout << "📋 Старший специалист передает заявку дальше: " << request << std::endl;
            RequestHandler::handleRequest(request, priority);
        }
    }
    
    std::string getHandlerName() const override {
        return "Старший специалист (важные заявки)";
    }
};

// Обработчик для обычных заявок
class NormalRequestHandler : public RequestHandler {
public:
    void handleRequest(const std::string& request, int priority) override {
        if (priority >= 50 && priority < 70) {
            std::cout << "📝 ОБЫЧНАЯ ЗАЯВКА обработана специалистом: " << request << std::endl;
            std::cout << "   Приоритет: " << priority << " (обычный)" << std::endl;
        } else {
            std::cout << "📋 Специалист передает заявку дальше: " << request << std::endl;
            RequestHandler::handleRequest(request, priority);
        }
    }
    
    std::string getHandlerName() const override {
        return "Специалист (обычные заявки)";
    }
};

// Обработчик для низкоприоритетных заявок
class LowPriorityRequestHandler : public RequestHandler {
public:
    void handleRequest(const std::string& request, int priority) override {
        if (priority >= 30 && priority < 50) {
            std::cout << "📄 НИЗКОПРИОРИТЕТНАЯ ЗАЯВКА обработана стажером: " << request << std::endl;
            std::cout << "   Приоритет: " << priority << " (низкий)" << std::endl;
        } else {
            std::cout << "📋 Стажер передает заявку дальше: " << request << std::endl;
            RequestHandler::handleRequest(request, priority);
        }
    }
    
    std::string getHandlerName() const override {
        return "Стажер (низкоприоритетные заявки)";
    }
};

// Обработчик для автоматических заявок
class AutomaticRequestHandler : public RequestHandler {
public:
    void handleRequest(const std::string& request, int priority) override {
        if (priority < 30) {
            std::cout << "🤖 АВТОМАТИЧЕСКАЯ ЗАЯВКА обработана системой: " << request << std::endl;
            std::cout << "   Приоритет: " << priority << " (автоматический)" << std::endl;
            std::cout << "   ✅ Заявка обработана автоматически" << std::endl;
        } else {
            std::cout << "📋 Система передает заявку дальше: " << request << std::endl;
            RequestHandler::handleRequest(request, priority);
        }
    }
    
    std::string getHandlerName() const override {
        return "Автоматическая система (низкоприоритетные заявки)";
    }
};

// Класс для управления цепочкой обработчиков
class RequestProcessor {
private:
    std::shared_ptr<RequestHandler> chain;
    
public:
    RequestProcessor() {
        setupChain();
    }
    
    void processRequest(const std::string& request, int priority) {
        std::cout << "\n🔄 Обработка заявки: " << request << std::endl;
        std::cout << "📊 Приоритет: " << priority << std::endl;
        std::cout << "⛓️ Цепочка обработчиков:" << std::endl;
        
        chain->handleRequest(request, priority);
    }
    
    void printChain() {
        std::cout << "\n📋 Цепочка обработчиков:" << std::endl;
        auto current = chain;
        int index = 1;
        
        while (current) {
            std::cout << index << ". " << current->getHandlerName() << std::endl;
            current = std::dynamic_pointer_cast<RequestHandler>(current);
            if (current) {
                // Получаем следующий обработчик через protected поле
                // В реальном коде это можно сделать через публичный метод
                break;
            }
            index++;
        }
    }
    
private:
    void setupChain() {
        // Создание цепочки обработчиков
        auto critical = std::make_shared<CriticalRequestHandler>();
        auto important = std::make_shared<ImportantRequestHandler>();
        auto normal = std::make_shared<NormalRequestHandler>();
        auto lowPriority = std::make_shared<LowPriorityRequestHandler>();
        auto automatic = std::make_shared<AutomaticRequestHandler>();
        
        // Настройка цепочки: критический -> важный -> обычный -> низкий -> автоматический
        critical->setNext(important);
        important->setNext(normal);
        normal->setNext(lowPriority);
        lowPriority->setNext(automatic);
        
        chain = critical;
    }
};

// Демонстрация использования
int main() {
    std::cout << "=== Демонстрация Chain of Responsibility Pattern ===" << std::endl;
    
    RequestProcessor processor;
    
    // Тестовые заявки с разными приоритетами
    std::vector<std::pair<std::string, int>> requests = {
        {"Система не работает - все пользователи заблокированы", 95},
        {"Важный клиент требует срочного решения", 85},
        {"Обычная заявка на доработку функционала", 60},
        {"Запрос на изменение интерфейса", 45},
        {"Автоматическое обновление данных", 20},
        {"Заявка с очень низким приоритетом", 10},
        {"Заявка, которую никто не может обработать", 5}
    };
    
    std::cout << "\n📊 Обработка заявок:" << std::endl;
    
    for (const auto& [request, priority] : requests) {
        processor.processRequest(request, priority);
        std::cout << "────────────────────────────────────────" << std::endl;
    }
    
    // Демонстрация динамического изменения цепочки
    std::cout << "\n🔧 Динамическое изменение цепочки:" << std::endl;
    
    // Создание новой цепочки без автоматического обработчика
    auto critical = std::make_shared<CriticalRequestHandler>();
    auto important = std::make_shared<ImportantRequestHandler>();
    auto normal = std::make_shared<NormalRequestHandler>();
    
    critical->setNext(important);
    important->setNext(normal);
    
    std::cout << "📋 Новая цепочка (без автоматического обработчика):" << std::endl;
    std::cout << "1. Менеджер (критические заявки)" << std::endl;
    std::cout << "2. Старший специалист (важные заявки)" << std::endl;
    std::cout << "3. Специалист (обычные заявки)" << std::endl;
    
    // Обработка заявки с низким приоритетом
    std::cout << "\n🔄 Обработка заявки с низким приоритетом:" << std::endl;
    critical->handleRequest("Заявка с низким приоритетом", 25);
    
    return 0;
}
```

## 🎯 Применение в реальных проектах

### 1. **Системы логирования**
```cpp
// Цепочка обработчиков логов
class LogHandler {
protected:
    std::shared_ptr<LogHandler> nextHandler;
    
public:
    virtual void handleLog(const LogMessage& message) = 0;
    void setNext(std::shared_ptr<LogHandler> handler) {
        nextHandler = handler;
    }
};

class ConsoleLogHandler : public LogHandler {
public:
    void handleLog(const LogMessage& message) override {
        if (message.level <= LogLevel::INFO) {
            std::cout << "CONSOLE: " << message.text << std::endl;
        } else if (nextHandler) {
            nextHandler->handleLog(message);
        }
    }
};

class FileLogHandler : public LogHandler {
public:
    void handleLog(const LogMessage& message) override {
        if (message.level <= LogLevel::WARNING) {
            // Запись в файл
        } else if (nextHandler) {
            nextHandler->handleLog(message);
        }
    }
};

class EmailLogHandler : public LogHandler {
public:
    void handleLog(const LogMessage& message) override {
        if (message.level <= LogLevel::ERROR) {
            // Отправка email
        } else if (nextHandler) {
            nextHandler->handleLog(message);
        }
    }
};
```

### 2. **Системы аутентификации и авторизации**
```cpp
// Цепочка проверок безопасности
class SecurityHandler {
protected:
    std::shared_ptr<SecurityHandler> nextHandler;
    
public:
    virtual bool handleRequest(const SecurityRequest& request) = 0;
    void setNext(std::shared_ptr<SecurityHandler> handler) {
        nextHandler = handler;
    }
};

class TokenValidationHandler : public SecurityHandler {
public:
    bool handleRequest(const SecurityRequest& request) override {
        if (validateToken(request.token)) {
            return true;
        }
        return nextHandler ? nextHandler->handleRequest(request) : false;
    }
    
private:
    bool validateToken(const std::string& token) {
        // Валидация токена
        return !token.empty();
    }
};

class PermissionCheckHandler : public SecurityHandler {
public:
    bool handleRequest(const SecurityRequest& request) override {
        if (checkPermissions(request.user, request.resource)) {
            return true;
        }
        return nextHandler ? nextHandler->handleRequest(request) : false;
    }
    
private:
    bool checkPermissions(const User& user, const Resource& resource) {
        // Проверка разрешений
        return user.hasPermission(resource);
    }
};
```

### 3. **Системы обработки исключений**
```cpp
// Цепочка обработчиков исключений
class ExceptionHandler {
protected:
    std::shared_ptr<ExceptionHandler> nextHandler;
    
public:
    virtual bool handleException(const std::exception& e) = 0;
    void setNext(std::shared_ptr<ExceptionHandler> handler) {
        nextHandler = handler;
    }
};

class ValidationExceptionHandler : public ExceptionHandler {
public:
    bool handleException(const std::exception& e) override {
        if (dynamic_cast<const ValidationException*>(&e)) {
            // Обработка ошибок валидации
            return true;
        }
        return nextHandler ? nextHandler->handleException(e) : false;
    }
};

class NetworkExceptionHandler : public ExceptionHandler {
public:
    bool handleException(const std::exception& e) override {
        if (dynamic_cast<const NetworkException*>(&e)) {
            // Обработка сетевых ошибок
            return true;
        }
        return nextHandler ? nextHandler->handleException(e) : false;
    }
};
```

## ⚡ Преимущества паттерна

### ✅ **Плюсы:**
- **Разделение ответственности**: Каждый обработчик отвечает за свою область
- **Гибкость**: Легко изменять порядок и состав обработчиков
- **Слабая связанность**: Отправитель не знает о конкретных обработчиках
- **Расширяемость**: Легко добавлять новые обработчики

### ❌ **Минусы:**
- **Гарантии обработки**: Нет гарантии, что запрос будет обработан
- **Производительность**: Может быть медленным для длинных цепочек
- **Отладка**: Сложно отлаживать длинные цепочки
- **Порядок зависимость**: Порядок обработчиков может быть критичен

## 🔄 Альтернативы и связанные паттерны

### **Альтернативы:**
- **Command Pattern**: Для инкапсуляции запросов
- **Strategy Pattern**: Для замены алгоритмов
- **Observer Pattern**: Для уведомлений

### **Связанные паттерны:**
- **Composite**: Chain of Responsibility может использовать Composite
- **Command**: Для инкапсуляции запросов
- **Decorator**: Для добавления функциональности

## 🎨 Современные улучшения в C++

### 1. **Использование std::function (C++11)**
```cpp
#include <functional>
#include <vector>

// Цепочка функций
class FunctionChain {
private:
    std::vector<std::function<bool(const std::string&)>> handlers;
    
public:
    void addHandler(std::function<bool(const std::string&)> handler) {
        handlers.push_back(handler);
    }
    
    bool processRequest(const std::string& request) {
        for (auto& handler : handlers) {
            if (handler(request)) {
                return true;
            }
        }
        return false;
    }
};
```

### 2. **Использование std::variant (C++17)**
```cpp
#include <variant>

// Различные типы запросов
using Request = std::variant<LoginRequest, DataRequest, AdminRequest>;

// Обработчик с std::visit
class VariantHandler {
public:
    bool handleRequest(const Request& request) {
        return std::visit([this](const auto& req) {
            return this->process(req);
        }, request);
    }
    
private:
    template<typename T>
    bool process(const T& request) {
        // Обработка конкретного типа запроса
        return true;
    }
};
```

## 🚀 Лучшие практики

### 1. **Правильная настройка цепочки**
```cpp
// ✅ Хорошо: Четкая настройка цепочки
class ChainBuilder {
public:
    static std::shared_ptr<Handler> buildStandardChain() {
        auto handler1 = std::make_shared<Handler1>();
        auto handler2 = std::make_shared<Handler2>();
        auto handler3 = std::make_shared<Handler3>();
        
        handler1->setNext(handler2);
        handler2->setNext(handler3);
        
        return handler1;
    }
};

// ❌ Плохо: Сложная настройка в конструкторе
class ComplexHandler : public Handler {
public:
    ComplexHandler() {
        // Слишком много логики в конструкторе
    }
};
```

### 2. **Обработка ошибок**
```cpp
// ✅ Хорошо: Четкая обработка ошибок
class SafeHandler : public Handler {
public:
    void handleRequest(const std::string& request) override {
        try {
            if (canHandle(request)) {
                processRequest(request);
            } else {
                Handler::handleRequest(request);
            }
        } catch (const std::exception& e) {
            handleError(e);
        }
    }
    
private:
    void handleError(const std::exception& e) {
        // Логирование и обработка ошибки
    }
};
```

### 3. **Управление состоянием**
```cpp
// ✅ Хорошо: Четкое управление состоянием
class StatefulHandler : public Handler {
private:
    int processedCount = 0;
    std::vector<std::string> processedRequests;
    
public:
    void handleRequest(const std::string& request) override {
        if (canHandle(request)) {
            processedCount++;
            processedRequests.push_back(request);
            processRequest(request);
        } else {
            Handler::handleRequest(request);
        }
    }
    
    void reset() {
        processedCount = 0;
        processedRequests.clear();
    }
};
```

## 📊 Сравнение с другими паттернами

| Аспект | Chain of Responsibility | Command | Strategy |
|--------|-------------------------|---------|----------|
| **Назначение** | Обработка запросов | Инкапсуляция операций | Замена алгоритмов |
| **Сложность** | Средняя | Низкая | Средняя |
| **Гибкость** | Высокая | Средняя | Высокая |
| **Производительность** | Средняя | Низкая | Высокая |
| **Применение** | Логирование, аутентификация | Undo/redo, очереди | Сортировка, валидация |

## 🎯 Заключение

**Chain of Responsibility Pattern** - это мощный инструмент для создания гибких систем обработки запросов. Он особенно полезен в системах, где нужно обрабатывать различные типы запросов с разными уровнями приоритета.

### **Когда использовать:**
- ✅ Нужно обрабатывать запросы разными способами
- ✅ Порядок обработки может изменяться
- ✅ Нужна гибкость в настройке обработки
- ✅ Отправитель не должен знать о конкретных обработчиках

### **Когда НЕ использовать:**
- ❌ Всегда известен единственный обработчик
- ❌ Производительность критична
- ❌ Нужны гарантии обработки запроса
- ❌ Логика обработки простая и не изменяется

**Chain of Responsibility Pattern** идеально подходит для систем логирования, аутентификации, обработки исключений и других приложений, где нужно гибко обрабатывать различные типы запросов! ⛓️✨

