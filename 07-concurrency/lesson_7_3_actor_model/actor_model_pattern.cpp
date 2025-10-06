#include <iostream>
#include <memory>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <functional>
#include <string>
#include <any>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <atomic>
#include <random>

/**
 * @file actor_model_pattern.cpp
 * @brief Демонстрация Actor Model Pattern
 * 
 * Этот файл показывает полную реализацию Actor Model с системой управления актерами,
 * примерами различных типов актеров и демонстрацией взаимодействия между ними.
 */

// ============================================================================
// БАЗОВЫЕ ТИПЫ ДАННЫХ
// ============================================================================

/**
 * @brief Базовый тип сообщения для Actor Model
 */
struct Message {
    std::string type;
    std::any data;
    std::string sender;
    std::chrono::system_clock::time_point timestamp;
    
    Message(const std::string& msgType, const std::any& msgData, const std::string& msgSender = "")
        : type(msgType), data(msgData), sender(msgSender), 
          timestamp(std::chrono::system_clock::now()) {}
    
    std::string toString() const {
        auto time_t = std::chrono::system_clock::to_time_t(timestamp);
        std::string timeStr = std::ctime(&time_t);
        timeStr.pop_back(); // Убираем \n
        
        return "Message{type='" + type + "', sender='" + sender + "', time=" + timeStr + "}";
    }
};

/**
 * @brief Интерфейс актера
 */
class Actor {
public:
    virtual ~Actor() = default;
    virtual void receive(const Message& message) = 0;
    virtual std::string getName() const = 0;
    virtual void start() {}
    virtual void stop() {}
};

/**
 * @brief Базовый актер с mailbox и обработкой сообщений
 */
class BaseActor : public Actor {
private:
    std::string name_;
    std::queue<Message> mailbox_;
    std::mutex mailboxMutex_;
    std::condition_variable condition_;
    std::thread workerThread_;
    std::atomic<bool> running_{false};
    std::atomic<bool> shouldStop_{false};
    
    // Статистика
    std::atomic<size_t> messagesProcessed_{0};
    std::atomic<size_t> messagesReceived_{0};
    
protected:
    std::unordered_map<std::string, std::function<void(const Message&)>> messageHandlers_;
    
public:
    explicit BaseActor(const std::string& name) : name_(name) {
        std::cout << "[" << name_ << "] Конструктор актера" << std::endl;
    }
    
    virtual ~BaseActor() {
        stop();
    }
    
    std::string getName() const override {
        return name_;
    }
    
    void start() override {
        if (running_.exchange(true)) {
            std::cout << "[" << name_ << "] Актер уже запущен" << std::endl;
            return;
        }
        
        shouldStop_.store(false);
        workerThread_ = std::thread(&BaseActor::messageLoop, this);
        
        std::cout << "[" << name_ << "] ✅ Актер запущен (ID потока: " 
                  << std::this_thread::get_id() << ")" << std::endl;
    }
    
    void stop() override {
        if (!running_.exchange(false)) {
            std::cout << "[" << name_ << "] Актер уже остановлен" << std::endl;
            return;
        }
        
        std::cout << "[" << name_ << "] Останавливаю актера..." << std::endl;
        
        shouldStop_.store(true);
        condition_.notify_all();
        
        if (workerThread_.joinable()) {
            workerThread_.join();
        }
        
        std::cout << "[" << name_ << "] ❌ Актер остановлен" << std::endl;
        std::cout << "[" << name_ << "] Статистика: получено=" << messagesReceived_.load() 
                  << ", обработано=" << messagesProcessed_.load() << std::endl;
    }
    
    // Отправка сообщения актеру
    void send(const Message& message) {
        messagesReceived_.fetch_add(1);
        
        {
            std::lock_guard<std::mutex> lock(mailboxMutex_);
            mailbox_.push(message);
        }
        condition_.notify_one();
        
        std::cout << "[" << name_ << "] 📨 Получено сообщение: " << message.type 
                  << " от " << message.sender << std::endl;
    }
    
    // Отправка сообщения другому актеру
    template<typename ActorType>
    void sendTo(ActorType& target, const std::string& messageType, const std::any& data) {
        Message msg(messageType, data, name_);
        target.send(msg);
    }
    
    // Получение статистики
    size_t getMessagesProcessed() const { return messagesProcessed_.load(); }
    size_t getMessagesReceived() const { return messagesReceived_.load(); }
    size_t getQueueSize() const {
        std::lock_guard<std::mutex> lock(mailboxMutex_);
        return mailbox_.size();
    }
    
protected:
    // Обработка сообщения (должна быть реализована в наследниках)
    virtual void receive(const Message& message) override {
        messagesProcessed_.fetch_add(1);
        
        auto handler = messageHandlers_.find(message.type);
        if (handler != messageHandlers_.end()) {
            std::cout << "[" << name_ << "] 🔄 Обрабатываю сообщение: " << message.type << std::endl;
            handler->second(message);
        } else {
            std::cout << "[" << name_ << "] ❓ Неизвестный тип сообщения: " << message.type << std::endl;
        }
    }
    
    // Регистрация обработчика сообщений
    void registerHandler(const std::string& messageType, 
                        std::function<void(const Message&)> handler) {
        messageHandlers_[messageType] = handler;
        std::cout << "[" << name_ << "] 📝 Зарегистрирован обработчик для: " << messageType << std::endl;
    }
    
private:
    void messageLoop() {
        std::cout << "[" << name_ << "] 🔄 Начинаю обработку сообщений" << std::endl;
        
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
                    std::cerr << "[" << name_ << "] ❌ Ошибка обработки сообщения: " << e.what() << std::endl;
                }
            }
        }
        
        std::cout << "[" << name_ << "] 🔄 Завершил обработку сообщений" << std::endl;
    }
};

// ============================================================================
// ПРИМЕРЫ АКТЕРОВ
// ============================================================================

/**
 * @brief Актер-калькулятор для демонстрации базовой функциональности
 */
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
        registerHandler("STATUS", [this](const Message& msg) { handleStatus(msg); });
    }
    
private:
    void handleAdd(const Message& message) {
        try {
            double value = std::any_cast<double>(message.data);
            result_ += value;
            std::cout << "[" << name_ << "] ➕ Добавил " << value << ", результат: " << result_ << std::endl;
        } catch (const std::bad_any_cast& e) {
            std::cerr << "[" << name_ << "] ❌ Ошибка типа данных в ADD: " << e.what() << std::endl;
        }
    }
    
    void handleSubtract(const Message& message) {
        try {
            double value = std::any_cast<double>(message.data);
            result_ -= value;
            std::cout << "[" << name_ << "] ➖ Вычел " << value << ", результат: " << result_ << std::endl;
        } catch (const std::bad_any_cast& e) {
            std::cerr << "[" << name_ << "] ❌ Ошибка типа данных в SUBTRACT: " << e.what() << std::endl;
        }
    }
    
    void handleMultiply(const Message& message) {
        try {
            double value = std::any_cast<double>(message.data);
            result_ *= value;
            std::cout << "[" << name_ << "] ✖️ Умножил на " << value << ", результат: " << result_ << std::endl;
        } catch (const std::bad_any_cast& e) {
            std::cerr << "[" << name_ << "] ❌ Ошибка типа данных в MULTIPLY: " << e.what() << std::endl;
        }
    }
    
    void handleDivide(const Message& message) {
        try {
            double value = std::any_cast<double>(message.data);
            if (value != 0.0) {
                result_ /= value;
                std::cout << "[" << name_ << "] ➗ Разделил на " << value << ", результат: " << result_ << std::endl;
            } else {
                std::cout << "[" << name_ << "] ❌ Ошибка: деление на ноль!" << std::endl;
            }
        } catch (const std::bad_any_cast& e) {
            std::cerr << "[" << name_ << "] ❌ Ошибка типа данных в DIVIDE: " << e.what() << std::endl;
        }
    }
    
    void handleGetResult(const Message& message) {
        std::cout << "[" << name_ << "] 📊 Текущий результат: " << result_ << std::endl;
    }
    
    void handleReset(const Message& message) {
        result_ = 0.0;
        std::cout << "[" << name_ << "] 🔄 Сбросил результат на 0" << std::endl;
    }
    
    void handleStatus(const Message& message) {
        std::cout << "[" << name_ << "] 📈 Статус: результат=" << result_ 
                  << ", сообщений обработано=" << getMessagesProcessed() << std::endl;
    }
};

/**
 * @brief Актер для управления пользователями
 */
struct User {
    std::string id;
    std::string name;
    std::string email;
    bool isActive{true};
    std::chrono::system_clock::time_point createdAt;
    
    User(const std::string& userId, const std::string& userName, const std::string& userEmail)
        : id(userId), name(userName), email(userEmail), 
          createdAt(std::chrono::system_clock::now()) {}
    
    std::string toString() const {
        return "User{id='" + id + "', name='" + name + "', email='" + email + 
               "', active=" + (isActive ? "true" : "false") + "}";
    }
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
        registerHandler("USER_COUNT", [this](const Message& msg) { handleUserCount(msg); });
    }
    
private:
    void handleCreateUser(const Message& message) {
        try {
            auto userData = std::any_cast<std::tuple<std::string, std::string, std::string>>(message.data);
            auto [id, name, email] = userData;
            
            if (users_.find(id) == users_.end()) {
                users_[id] = User(id, name, email);
                std::cout << "[" << name_ << "] 👤 Создан пользователь: " << id << " (" << name << ")" << std::endl;
            } else {
                std::cout << "[" << name_ << "] ⚠️ Пользователь " << id << " уже существует" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "[" << name_ << "] ❌ Ошибка создания пользователя: " << e.what() << std::endl;
        }
    }
    
    void handleGetUser(const Message& message) {
        try {
            std::string userId = std::any_cast<std::string>(message.data);
            auto it = users_.find(userId);
            
            if (it != users_.end()) {
                const User& user = it->second;
                std::cout << "[" << name_ << "] 👤 Пользователь " << userId << ": " 
                          << user.toString() << std::endl;
            } else {
                std::cout << "[" << name_ << "] ❓ Пользователь " << userId << " не найден" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "[" << name_ << "] ❌ Ошибка получения пользователя: " << e.what() << std::endl;
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
                std::cout << "[" << name_ << "] ✏️ Обновлен пользователь: " << id << std::endl;
            } else {
                std::cout << "[" << name_ << "] ❓ Пользователь " << id << " не найден для обновления" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "[" << name_ << "] ❌ Ошибка обновления пользователя: " << e.what() << std::endl;
        }
    }
    
    void handleDeleteUser(const Message& message) {
        try {
            std::string userId = std::any_cast<std::string>(message.data);
            
            auto it = users_.find(userId);
            if (it != users_.end()) {
                users_.erase(it);
                std::cout << "[" << name_ << "] 🗑️ Удален пользователь: " << userId << std::endl;
            } else {
                std::cout << "[" << name_ << "] ❓ Пользователь " << userId << " не найден для удаления" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "[" << name_ << "] ❌ Ошибка удаления пользователя: " << e.what() << std::endl;
        }
    }
    
    void handleListUsers(const Message& message) {
        std::cout << "[" << name_ << "] 📋 Список пользователей (" << users_.size() << "):" << std::endl;
        for (const auto& pair : users_) {
            const User& user = pair.second;
            std::cout << "  - " << user.toString() << std::endl;
        }
    }
    
    void handleUserCount(const Message& message) {
        std::cout << "[" << name_ << "] 📊 Количество пользователей: " << users_.size() << std::endl;
    }
};

/**
 * @brief Актер-координатор для демонстрации взаимодействия между актерами
 */
class CoordinatorActor : public BaseActor {
private:
    std::vector<BaseActor*> managedActors_;
    
public:
    CoordinatorActor() : BaseActor("Coordinator") {
        registerHandler("ADD_ACTOR", [this](const Message& msg) { handleAddActor(msg); });
        registerHandler("SEND_TO_ALL", [this](const Message& msg) { handleSendToAll(msg); });
        registerHandler("BROADCAST", [this](const Message& msg) { handleBroadcast(msg); });
        registerHandler("LIST_ACTORS", [this](const Message& msg) { handleListActors(msg); });
    }
    
    void addActor(BaseActor* actor) {
        managedActors_.push_back(actor);
        std::cout << "[" << name_ << "] ➕ Добавлен актер: " << actor->getName() << std::endl;
    }
    
private:
    void handleAddActor(const Message& message) {
        // В реальной реализации здесь была бы логика добавления актера по имени
        std::cout << "[" << name_ << "] ➕ Запрос на добавление актера" << std::endl;
    }
    
    void handleSendToAll(const Message& message) {
        std::cout << "[" << name_ << "] 📢 Отправляю сообщение всем актерам" << std::endl;
        
        for (auto* actor : managedActors_) {
            if (actor && actor != this) {
                sendTo(*actor, "COORDINATOR_MESSAGE", message.data);
            }
        }
    }
    
    void handleBroadcast(const Message& message) {
        std::cout << "[" << name_ << "] 📡 Трансляция сообщения: " 
                  << std::any_cast<std::string>(message.data) << std::endl;
        
        for (auto* actor : managedActors_) {
            if (actor && actor != this) {
                sendTo(*actor, "BROADCAST", message.data);
            }
        }
    }
    
    void handleListActors(const Message& message) {
        std::cout << "[" << name_ << "] 📋 Управляемые актеры (" << managedActors_.size() << "):" << std::endl;
        for (auto* actor : managedActors_) {
            if (actor) {
                std::cout << "  - " << actor->getName() 
                          << " (сообщений: " << actor->getMessagesProcessed() << ")" << std::endl;
            }
        }
    }
};

// ============================================================================
// СИСТЕМА УПРАВЛЕНИЯ АКТЕРАМИ
// ============================================================================

/**
 * @brief Система управления актерами
 */
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
        
        std::cout << "[ActorSystem] ✅ Создан актер: " << name << std::endl;
        return actorRef;
    }
    
    Actor* getActor(const std::string& name) {
        std::lock_guard<std::mutex> lock(actorsMutex_);
        auto it = actors_.find(name);
        return (it != actors_.end()) ? it->second.get() : nullptr;
    }
    
    BaseActor* getBaseActor(const std::string& name) {
        return dynamic_cast<BaseActor*>(getActor(name));
    }
    
    void sendMessage(const std::string& actorName, const Message& message) {
        BaseActor* actor = getBaseActor(actorName);
        if (actor) {
            actor->send(message);
        } else {
            std::cout << "[ActorSystem] ❓ Актер " << actorName << " не найден" << std::endl;
        }
    }
    
    template<typename... Args>
    void sendMessage(const std::string& actorName, const std::string& messageType, Args&&... args) {
        auto data = std::make_tuple(std::forward<Args>(args)...);
        Message msg(messageType, data);
        sendMessage(actorName, msg);
    }
    
    void shutdown() {
        std::cout << "[ActorSystem] 🔄 Останавливаю всех актеров..." << std::endl;
        
        std::lock_guard<std::mutex> lock(actorsMutex_);
        for (auto& pair : actors_) {
            pair.second->stop();
        }
        
        actors_.clear();
        std::cout << "[ActorSystem] ✅ Все актеры остановлены" << std::endl;
    }
    
    size_t getActorCount() const {
        std::lock_guard<std::mutex> lock(actorsMutex_);
        return actors_.size();
    }
    
    void printSystemStatus() const {
        std::lock_guard<std::mutex> lock(actorsMutex_);
        std::cout << "\n=== СТАТУС СИСТЕМЫ АКТЕРОВ ===" << std::endl;
        std::cout << "Всего актеров: " << actors_.size() << std::endl;
        
        for (const auto& pair : actors_) {
            auto* baseActor = dynamic_cast<BaseActor*>(pair.second.get());
            if (baseActor) {
                std::cout << "  - " << pair.first 
                          << ": обработано=" << baseActor->getMessagesProcessed()
                          << ", в очереди=" << baseActor->getQueueSize() << std::endl;
            }
        }
        std::cout << "===============================" << std::endl;
    }
};

// ============================================================================
// ДЕМОНСТРАЦИОННЫЕ ФУНКЦИИ
// ============================================================================

/**
 * @brief Демонстрация базового Actor Model
 */
void demonstrateBasicActorModel() {
    std::cout << "\n=== БАЗОВЫЙ ACTOR MODEL ===" << std::endl;
    
    ActorSystem system;
    
    // Создаем актер-калькулятор
    CalculatorActor& calculator = system.createActor<CalculatorActor>("Calculator");
    
    // Отправляем различные сообщения
    system.sendMessage("Calculator", "ADD", 10.0);
    system.sendMessage("Calculator", "MULTIPLY", 2.0);
    system.sendMessage("Calculator", "GET_RESULT", "");
    system.sendMessage("Calculator", "SUBTRACT", 5.0);
    system.sendMessage("Calculator", "GET_RESULT", "");
    system.sendMessage("Calculator", "STATUS", "");
    
    // Ждем обработки сообщений
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    system.printSystemStatus();
}

/**
 * @brief Демонстрация системы управления пользователями
 */
void demonstrateUserManagement() {
    std::cout << "\n=== УПРАВЛЕНИЕ ПОЛЬЗОВАТЕЛЯМИ ===" << std::endl;
    
    ActorSystem system;
    
    // Создаем менеджер пользователей
    UserManagerActor& userManager = system.createActor<UserManagerActor>("UserManager");
    
    // Создаем пользователей
    system.sendMessage("UserManager", "CREATE_USER", 
                      std::make_tuple(std::string("user1"), std::string("Alice"), std::string("alice@example.com")));
    system.sendMessage("UserManager", "CREATE_USER", 
                      std::make_tuple(std::string("user2"), std::string("Bob"), std::string("bob@example.com")));
    system.sendMessage("UserManager", "CREATE_USER", 
                      std::make_tuple(std::string("user3"), std::string("Charlie"), std::string("charlie@example.com")));
    
    // Получаем информацию о пользователях
    system.sendMessage("UserManager", "GET_USER", std::string("user1"));
    system.sendMessage("UserManager", "GET_USER", std::string("user2"));
    
    // Обновляем пользователя
    system.sendMessage("UserManager", "UPDATE_USER", 
                      std::make_tuple(std::string("user1"), std::string("Alice Updated"), std::string("alice.new@example.com")));
    
    // Список пользователей
    system.sendMessage("UserManager", "LIST_USERS", "");
    system.sendMessage("UserManager", "USER_COUNT", "");
    
    // Удаляем пользователя
    system.sendMessage("UserManager", "DELETE_USER", std::string("user3"));
    
    // Финальный список
    system.sendMessage("UserManager", "LIST_USERS", "");
    
    // Ждем обработки
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    system.printSystemStatus();
}

/**
 * @brief Демонстрация взаимодействия между актерами
 */
void demonstrateActorInteraction() {
    std::cout << "\n=== ВЗАИМОДЕЙСТВИЕ МЕЖДУ АКТЕРАМИ ===" << std::endl;
    
    ActorSystem system;
    
    // Создаем актеры
    CalculatorActor& calculator = system.createActor<CalculatorActor>("Calculator");
    UserManagerActor& userManager = system.createActor<UserManagerActor>("UserManager");
    CoordinatorActor& coordinator = system.createActor<CoordinatorActor>("Coordinator");
    
    // Добавляем актеры в координатор
    coordinator.addActor(&calculator);
    coordinator.addActor(&userManager);
    
    // Координатор отправляет сообщения всем актерам
    system.sendMessage("Coordinator", "BROADCAST", std::string("Привет от координатора!"));
    system.sendMessage("Coordinator", "LIST_ACTORS", "");
    
    // Актеры взаимодействуют между собой
    calculator.sendTo(userManager, "CREATE_USER", 
                     std::make_tuple(std::string("calc_user"), std::string("Calculator User"), std::string("calc@example.com")));
    
    userManager.sendTo(calculator, "ADD", 100.0);
    userManager.sendTo(calculator, "GET_RESULT", "");
    
    // Ждем обработки
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    system.printSystemStatus();
}

/**
 * @brief Демонстрация производительности Actor Model
 */
void demonstratePerformance() {
    std::cout << "\n=== ТЕСТ ПРОИЗВОДИТЕЛЬНОСТИ ===" << std::endl;
    
    ActorSystem system;
    CalculatorActor& calculator = system.createActor<CalculatorActor>("Calculator");
    
    const int NUM_MESSAGES = 1000;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Отправляем много сообщений
    for (int i = 0; i < NUM_MESSAGES; ++i) {
        system.sendMessage("Calculator", "ADD", 1.0);
    }
    
    // Ждем обработки всех сообщений
    while (calculator.getQueueSize() > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Обработано " << NUM_MESSAGES << " сообщений за " 
              << duration.count() << " мс" << std::endl;
    std::cout << "Производительность: " 
              << (NUM_MESSAGES * 1000.0 / duration.count()) << " сообщений/сек" << std::endl;
    
    system.sendMessage("Calculator", "GET_RESULT", "");
    system.sendMessage("Calculator", "STATUS", "");
    
    // Ждем финальной обработки
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    system.printSystemStatus();
}

// ============================================================================
// ОСНОВНАЯ ФУНКЦИЯ
// ============================================================================

int main() {
    std::cout << "🎭 Демонстрация Actor Model Pattern" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    
    try {
        demonstrateBasicActorModel();
        demonstrateUserManagement();
        demonstrateActorInteraction();
        demonstratePerformance();
        
        std::cout << "\n✅ Все демонстрации завершены успешно!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Ошибка: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\n🎯 Рекомендации:" << std::endl;
    std::cout << "1. Используйте Actor Model для изоляции состояния" << std::endl;
    std::cout << "2. Все общение между актерами только через сообщения" << std::endl;
    std::cout << "3. Один актер обрабатывает одно сообщение за раз" << std::endl;
    std::cout << "4. Правильно управляйте жизненным циклом актеров" << std::endl;
    std::cout << "5. Используйте неизменяемые сообщения" << std::endl;
    
    return 0;
}
