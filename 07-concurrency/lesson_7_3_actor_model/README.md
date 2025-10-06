# Урок 7.3: Actor Model Pattern (Модель актеров)

## 🎯 Цель урока
Изучить Actor Model - один из самых элегантных паттернов для многопоточного программирования. Понять принципы изоляции состояния, асинхронного обмена сообщениями и создания масштабируемых распределенных систем.

## 📚 Что изучаем

### 1. Actor Model
- **Определение**: Модель параллельных вычислений, где каждый актер изолирован и общается только через сообщения
- **Назначение**: Упрощение многопоточного программирования, изоляция состояния, масштабируемость
- **Применение**: Распределенные системы, игровые движки, микросервисы, системы реального времени

### 2. Основные принципы
- **Изоляция состояния**: Каждый актер имеет собственное изолированное состояние
- **Асинхронные сообщения**: Общение только через асинхронную передачу сообщений
- **Локальность**: Актер обрабатывает только одно сообщение за раз
- **Без совместного состояния**: Нет разделяемых переменных между актерами

### 3. Ключевые компоненты
- **Actor**: Актер с состоянием и поведением
- **Message**: Сообщение для общения между актерами
- **Mailbox**: Очередь сообщений для актера
- **Actor System**: Система управления актерами

## 🔍 Ключевые концепции

### Базовая архитектура Actor
```cpp
#include <memory>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <functional>
#include <string>
#include <any>
#include <unordered_map>

// Базовый тип сообщения
struct Message {
    std::string type;
    std::any data;
    std::string sender;
    std::chrono::system_clock::time_point timestamp;
    
    Message(const std::string& msgType, const std::any& msgData, const std::string& msgSender = "")
        : type(msgType), data(msgData), sender(msgSender), 
          timestamp(std::chrono::system_clock::now()) {}
};

// Интерфейс актера
class Actor {
public:
    virtual ~Actor() = default;
    virtual void receive(const Message& message) = 0;
    virtual std::string getName() const = 0;
    virtual void start() {}
    virtual void stop() {}
};

// Базовый актер с mailbox
class BaseActor : public Actor {
private:
    std::string name_;
    std::queue<Message> mailbox_;
    std::mutex mailboxMutex_;
    std::condition_variable condition_;
    std::thread workerThread_;
    std::atomic<bool> running_{false};
    std::atomic<bool> shouldStop_{false};
    
protected:
    std::unordered_map<std::string, std::function<void(const Message&)>> messageHandlers_;
    
public:
    explicit BaseActor(const std::string& name) : name_(name) {}
    
    virtual ~BaseActor() {
        stop();
    }
    
    std::string getName() const override {
        return name_;
    }
    
    void start() override {
        if (running_.exchange(true)) {
            return; // Уже запущен
        }
        
        shouldStop_.store(false);
        workerThread_ = std::thread(&BaseActor::messageLoop, this);
        
        std::cout << "[" << name_ << "] Актер запущен" << std::endl;
    }
    
    void stop() override {
        if (!running_.exchange(false)) {
            return; // Уже остановлен
        }
        
        shouldStop_.store(true);
        condition_.notify_all();
        
        if (workerThread_.joinable()) {
            workerThread_.join();
        }
        
        std::cout << "[" << name_ << "] Актер остановлен" << std::endl;
    }
    
    // Отправка сообщения актеру
    void send(const Message& message) {
        {
            std::lock_guard<std::mutex> lock(mailboxMutex_);
            mailbox_.push(message);
        }
        condition_.notify_one();
    }
    
    // Отправка сообщения другому актеру
    template<typename ActorType>
    void sendTo(ActorType& target, const std::string& messageType, const std::any& data) {
        Message msg(messageType, data, name_);
        target.send(msg);
    }
    
protected:
    // Обработка сообщения (должна быть реализована в наследниках)
    virtual void receive(const Message& message) override {
        auto handler = messageHandlers_.find(message.type);
        if (handler != messageHandlers_.end()) {
            handler->second(message);
        } else {
            std::cout << "[" << name_ << "] Неизвестный тип сообщения: " << message.type << std::endl;
        }
    }
    
    // Регистрация обработчика сообщений
    void registerHandler(const std::string& messageType, 
                        std::function<void(const Message&)> handler) {
        messageHandlers_[messageType] = handler;
    }
    
private:
    void messageLoop() {
        std::cout << "[" << name_ << "] Начинаю обработку сообщений" << std::endl;
        
        while (!shouldStop_.load()) {
            Message message("", std::any{}, "");
            bool hasMessage = false;
            
            {
                std::unique_lock<std::mutex> lock(mailboxMutex_);
                condition_.wait(lock, [this] { return !mailbox_.empty() || shouldStop_.load(); });
                
                if (!mailbox_.empty()) {
                    message = std::move(mailbox_.front());
                    mailbox_.pop();
                    hasMessage = true;
                }
            }
            
            if (hasMessage) {
                try {
                    receive(message);
                } catch (const std::exception& e) {
                    std::cerr << "[" << name_ << "] Ошибка обработки сообщения: " << e.what() << std::endl;
                }
            }
        }
        
        std::cout << "[" << name_ << "] Завершил обработку сообщений" << std::endl;
    }
};
```

## 🛠️ Практические примеры

### Простой актер-калькулятор
```cpp
class CalculatorActor : public BaseActor {
private:
    double result_{0.0};
    
public:
    CalculatorActor() : BaseActor("Calculator") {
        // Регистрируем обработчики сообщений
        registerHandler("ADD", [this](const Message& msg) { handleAdd(msg); });
        registerHandler("SUBTRACT", [this](const Message& msg) { handleSubtract(msg); });
        registerHandler("MULTIPLY", [this](const Message& msg) { handleMultiply(msg); });
        registerHandler("DIVIDE", [this](const Message& msg) { handleDivide(msg); });
        registerHandler("GET_RESULT", [this](const Message& msg) { handleGetResult(msg); });
        registerHandler("RESET", [this](const Message& msg) { handleReset(msg); });
    }
    
private:
    void handleAdd(const Message& message) {
        try {
            double value = std::any_cast<double>(message.data);
            result_ += value;
            std::cout << "[" << name_ << "] Добавил " << value << ", результат: " << result_ << std::endl;
        } catch (const std::bad_any_cast& e) {
            std::cerr << "[" << name_ << "] Ошибка типа данных в ADD: " << e.what() << std::endl;
        }
    }
    
    void handleSubtract(const Message& message) {
        try {
            double value = std::any_cast<double>(message.data);
            result_ -= value;
            std::cout << "[" << name_ << "] Вычел " << value << ", результат: " << result_ << std::endl;
        } catch (const std::bad_any_cast& e) {
            std::cerr << "[" << name_ << "] Ошибка типа данных в SUBTRACT: " << e.what() << std::endl;
        }
    }
    
    void handleMultiply(const Message& message) {
        try {
            double value = std::any_cast<double>(message.data);
            result_ *= value;
            std::cout << "[" << name_ << "] Умножил на " << value << ", результат: " << result_ << std::endl;
        } catch (const std::bad_any_cast& e) {
            std::cerr << "[" << name_ << "] Ошибка типа данных в MULTIPLY: " << e.what() << std::endl;
        }
    }
    
    void handleDivide(const Message& message) {
        try {
            double value = std::any_cast<double>(message.data);
            if (value != 0.0) {
                result_ /= value;
                std::cout << "[" << name_ << "] Разделил на " << value << ", результат: " << result_ << std::endl;
            } else {
                std::cout << "[" << name_ << "] Ошибка: деление на ноль!" << std::endl;
            }
        } catch (const std::bad_any_cast& e) {
            std::cerr << "[" << name_ << "] Ошибка типа данных в DIVIDE: " << e.what() << std::endl;
        }
    }
    
    void handleGetResult(const Message& message) {
        std::cout << "[" << name_ << "] Текущий результат: " << result_ << std::endl;
    }
    
    void handleReset(const Message& message) {
        result_ = 0.0;
        std::cout << "[" << name_ << "] Сбросил результат на 0" << std::endl;
    }
};
```

### Система управления пользователями
```cpp
struct User {
    std::string id;
    std::string name;
    std::string email;
    bool isActive{true};
    
    User(const std::string& userId, const std::string& userName, const std::string& userEmail)
        : id(userId), name(userName), email(userEmail) {}
};

class UserManagerActor : public BaseActor {
private:
    std::unordered_map<std::string, User> users_;
    
public:
    UserManagerActor() : BaseActor("UserManager") {
        registerHandler("CREATE_USER", [this](const Message& msg) { handleCreateUser(msg); });
        registerHandler("GET_USER", [this](const Message& msg) { handleGetUser(msg); });
        registerHandler("UPDATE_USER", [this](const Message& msg) { handleUpdateUser(msg); });
        registerHandler("DELETE_USER", [this](const Message& msg) { handleDeleteUser(msg); });
        registerHandler("LIST_USERS", [this](const Message& msg) { handleListUsers(msg); });
    }
    
private:
    void handleCreateUser(const Message& message) {
        try {
            auto userData = std::any_cast<std::tuple<std::string, std::string, std::string>>(message.data);
            auto [id, name, email] = userData;
            
            if (users_.find(id) == users_.end()) {
                users_[id] = User(id, name, email);
                std::cout << "[" << name_ << "] Создан пользователь: " << id << " (" << name << ")" << std::endl;
            } else {
                std::cout << "[" << name_ << "] Пользователь " << id << " уже существует" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "[" << name_ << "] Ошибка создания пользователя: " << e.what() << std::endl;
        }
    }
    
    void handleGetUser(const Message& message) {
        try {
            std::string userId = std::any_cast<std::string>(message.data);
            auto it = users_.find(userId);
            
            if (it != users_.end()) {
                const User& user = it->second;
                std::cout << "[" << name_ << "] Пользователь " << userId << ": " 
                          << user.name << " (" << user.email << ")" << std::endl;
            } else {
                std::cout << "[" << name_ << "] Пользователь " << userId << " не найден" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "[" << name_ << "] Ошибка получения пользователя: " << e.what() << std::endl;
        }
    }
    
    void handleUpdateUser(const Message& message) {
        try {
            auto updateData = std::any_cast<std::tuple<std::string, std::string, std::string>>(message.data);
            auto [id, newName, newEmail] = updateData;
            
            auto it = users_.find(id);
            if (it != users_.end()) {
                it->second.name = newName;
                it->second.email = newEmail;
                std::cout << "[" << name_ << "] Обновлен пользователь: " << id << std::endl;
            } else {
                std::cout << "[" << name_ << "] Пользователь " << id << " не найден для обновления" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "[" << name_ << "] Ошибка обновления пользователя: " << e.what() << std::endl;
        }
    }
    
    void handleDeleteUser(const Message& message) {
        try {
            std::string userId = std::any_cast<std::string>(message.data);
            
            auto it = users_.find(userId);
            if (it != users_.end()) {
                users_.erase(it);
                std::cout << "[" << name_ << "] Удален пользователь: " << userId << std::endl;
            } else {
                std::cout << "[" << name_ << "] Пользователь " << userId << " не найден для удаления" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "[" << name_ << "] Ошибка удаления пользователя: " << e.what() << std::endl;
        }
    }
    
    void handleListUsers(const Message& message) {
        std::cout << "[" << name_ << "] Список пользователей (" << users_.size() << "):" << std::endl;
        for (const auto& pair : users_) {
            const User& user = pair.second;
            std::cout << "  - " << user.id << ": " << user.name << " (" << user.email << ")" << std::endl;
        }
    }
};
```

### Система управления актерами
```cpp
class ActorSystem {
private:
    std::unordered_map<std::string, std::unique_ptr<Actor>> actors_;
    std::mutex actorsMutex_;
    
public:
    template<typename ActorType, typename... Args>
    ActorType& createActor(const std::string& name, Args&&... args) {
        std::lock_guard<std::mutex> lock(actorsMutex_);
        
        auto actor = std::make_unique<ActorType>(std::forward<Args>(args)...);
        ActorType& actorRef = *actor;
        
        actors_[name] = std::move(actor);
        actorRef.start();
        
        std::cout << "[ActorSystem] Создан актер: " << name << std::endl;
        return actorRef;
    }
    
    Actor* getActor(const std::string& name) {
        std::lock_guard<std::mutex> lock(actorsMutex_);
        auto it = actors_.find(name);
        return (it != actors_.end()) ? it->second.get() : nullptr;
    }
    
    void sendMessage(const std::string& actorName, const Message& message) {
        Actor* actor = getActor(actorName);
        if (actor) {
            auto* baseActor = dynamic_cast<BaseActor*>(actor);
            if (baseActor) {
                baseActor->send(message);
            }
        } else {
            std::cout << "[ActorSystem] Актер " << actorName << " не найден" << std::endl;
        }
    }
    
    void shutdown() {
        std::cout << "[ActorSystem] Останавливаю всех актеров..." << std::endl;
        
        std::lock_guard<std::mutex> lock(actorsMutex_);
        for (auto& pair : actors_) {
            pair.second->stop();
        }
        
        actors_.clear();
        std::cout << "[ActorSystem] Все актеры остановлены" << std::endl;
    }
    
    size_t getActorCount() const {
        std::lock_guard<std::mutex> lock(actorsMutex_);
        return actors_.size();
    }
};
```

## 🎯 Практические упражнения

### Упражнение 1: Игровой движок
Создайте систему актеров для простого игрового движка с игроками, врагами и предметами.

### Упражнение 2: Система заказов
Реализуйте систему обработки заказов с актерами для клиентов, склада и доставки.

### Упражнение 3: Чат-система
Создайте многопользовательскую чат-систему с использованием Actor Model.

### Упражнение 4: Система мониторинга
Реализуйте систему мониторинга с актерами для сбора метрик и алертов.

## 💡 Важные принципы

1. **Изоляция состояния**: Каждый актер имеет собственное изолированное состояние
2. **Асинхронные сообщения**: Все общение через асинхронные сообщения
3. **Один поток на актера**: Актер обрабатывает одно сообщение за раз
4. **Неизменяемость**: Сообщения должны быть неизменяемыми
5. **Управление жизненным циклом**: Правильный запуск и остановка актеров
