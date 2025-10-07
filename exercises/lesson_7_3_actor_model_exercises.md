# 🏋️ Упражнения: Actor Model Pattern

## 📋 Задание 1: Базовый Actor с типизированными сообщениями

### Описание
Реализуйте базовую систему акторов с поддержкой типизированных сообщений и message routing.

### Требования
1. ✅ Базовый класс Actor с mailbox
2. ✅ Типизированные сообщения (std::variant или наследование)
3. ✅ Message Router для маршрутизации между акторами
4. ✅ Асинхронная обработка сообщений
5. ✅ Graceful shutdown

### Шаблон для реализации
```cpp
#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <variant>
#include <string>
#include <unordered_map>

// Типизированные сообщения через std::variant
using Message = std::variant<
    struct PingMessage { std::string sender; int sequence; },
    struct PongMessage { std::string sender; int sequence; },
    struct WorkMessage { int id; std::string data; },
    struct ResultMessage { int id; std::string result; },
    struct ShutdownMessage { }
>;

class Actor {
protected:
    std::string name_;
    std::queue<Message> mailbox_;
    std::mutex mailbox_mutex_;
    std::condition_variable cv_;
    std::atomic<bool> running_{true};
    std::thread worker_;
    
public:
    // TODO: Реализуйте конструктор
    explicit Actor(const std::string& name) {
        // Ваш код здесь
    }
    
    // TODO: Реализуйте деструктор
    virtual ~Actor() {
        // Ваш код здесь
    }
    
    // TODO: Реализуйте отправку сообщения
    void send(Message msg) {
        // Ваш код здесь
    }
    
    const std::string& getName() const { return name_; }
    
protected:
    // TODO: Реализуйте message loop
    virtual void messageLoop() {
        // Ваш код здесь
    }
    
    // TODO: Реализуйте обработку сообщений
    virtual void handleMessage(const Message& msg) = 0;
};

class MessageRouter {
private:
    std::unordered_map<std::string, std::shared_ptr<Actor>> actors_;
    std::mutex mutex_;
    
public:
    // TODO: Реализуйте регистрацию актора
    void registerActor(std::shared_ptr<Actor> actor) {
        // Ваш код здесь
    }
    
    // TODO: Реализуйте отправку сообщения
    void sendMessage(const std::string& target, Message msg) {
        // Ваш код здесь
    }
    
    // TODO: Реализуйте broadcast
    void broadcast(Message msg) {
        // Ваш код здесь
    }
};
```

### Тесты
```cpp
class PingPongActor : public Actor {
public:
    PingPongActor(const std::string& name) : Actor(name) {}
    
protected:
    void handleMessage(const Message& msg) override {
        std::visit([this](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            
            if constexpr (std::is_same_v<T, PingMessage>) {
                std::cout << name_ << " received Ping from " 
                          << arg.sender << std::endl;
            } else if constexpr (std::is_same_v<T, PongMessage>) {
                std::cout << name_ << " received Pong from " 
                          << arg.sender << std::endl;
            }
        }, msg);
    }
};

void testPingPong() {
    MessageRouter router;
    
    auto actor1 = std::make_shared<PingPongActor>("Actor1");
    auto actor2 = std::make_shared<PingPongActor>("Actor2");
    
    router.registerActor(actor1);
    router.registerActor(actor2);
    
    router.sendMessage("Actor2", PingMessage{"Actor1", 1});
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
}
```

---

## 📋 Задание 2: Supervisor Pattern

### Описание
Реализуйте Supervisor Actor для управления worker акторами с fault tolerance и restart стратегиями.

### Требования
1. ✅ Supervisor Actor управляет несколькими Workers
2. ✅ Restart стратегии: one-for-one, one-for-all
3. ✅ Мониторинг здоровья workers
4. ✅ Автоматический перезапуск при сбоях
5. ✅ Статистика по сбоям и перезапускам

### Шаблон для реализации
```cpp
enum class RestartStrategy {
    ONE_FOR_ONE,  // Перезапускаем только упавший актор
    ONE_FOR_ALL   // Перезапускаем всех акторов
};

struct SupervisorConfig {
    RestartStrategy strategy;
    size_t max_restarts;  // Максимум перезапусков за период
    std::chrono::seconds restart_period;
};

class SupervisorActor : public Actor {
private:
    std::vector<std::shared_ptr<Actor>> children_;
    SupervisorConfig config_;
    std::unordered_map<std::string, size_t> restart_counts_;
    
public:
    // TODO: Реализуйте конструктор
    SupervisorActor(const std::string& name, const SupervisorConfig& config) {
        // Ваш код здесь
    }
    
    // TODO: Реализуйте добавление child actor
    void addChild(std::shared_ptr<Actor> child) {
        // Ваш код здесь
    }
    
    // TODO: Реализуйте обработку сбоев
    void handleChildFailure(const std::string& child_name) {
        // Ваш код здесь
    }
    
protected:
    void handleMessage(const Message& msg) override {
        // TODO: Обработка сообщений для supervisor
    }
};
```

---

## 📋 Задание 3: Actor Pool для параллельной обработки

### Описание
Реализуйте пул акторов для параллельной обработки задач с балансировкой нагрузки.

### Требования
1. ✅ Пул из N worker акторов
2. ✅ Round-robin или random балансировка
3. ✅ Отслеживание загрузки каждого актора
4. ✅ Динамическая балансировка по загрузке
5. ✅ Метрики производительности

### Шаблон
```cpp
class WorkerActor : public Actor {
private:
    std::atomic<size_t> tasks_processed_{0};
    std::atomic<size_t> mailbox_size_{0};
    
public:
    // TODO: Реализуйте worker actor
    
    size_t getLoad() const {
        return mailbox_size_.load();
    }
    
    size_t getProcessedCount() const {
        return tasks_processed_.load();
    }
};

class ActorPool {
private:
    std::vector<std::shared_ptr<WorkerActor>> workers_;
    std::atomic<size_t> next_worker_{0};
    
public:
    // TODO: Реализуйте пул акторов
    
    // TODO: Балансировка нагрузки
    std::shared_ptr<WorkerActor> getNextWorker() {
        // Round-robin или least-loaded
    }
};
```

---

## 📋 Задание 4: Typed Actor System

### Описание
Реализуйте систему акторов с compile-time типизацией сообщений (без `std::variant`).

### Требования
1. ✅ Template-based типизация
2. ✅ Compile-time проверка типов сообщений
3. ✅ Type-safe message sending
4. ✅ Поддержка разных типов акторов
5. ✅ Zero-cost abstractions

### Шаблон
```cpp
template<typename... MessageTypes>
class TypedActor {
protected:
    std::queue<std::variant<MessageTypes...>> mailbox_;
    // TODO: Остальная реализация
    
public:
    template<typename MsgType>
    void send(MsgType msg) {
        static_assert((std::is_same_v<MsgType, MessageTypes> || ...),
                     "Message type not supported by this actor");
        // TODO: Реализация
    }
};

// Пример использования
using PingPongActor = TypedActor<PingMessage, PongMessage, ShutdownMessage>;
```

---

## 📋 Задание 5: Benchmarking Actor Model

### Описание
Создайте benchmark для сравнения Actor Model с прямым вызовом методов и message queue.

### Требования
1. ✅ Бенчмарк latency отправки сообщений
2. ✅ Бенчмарк throughput (сообщений/сек)
3. ✅ Сравнение с прямыми вызовами
4. ✅ Сравнение с Thread Pool + Queue
5. ✅ Анализ overhead

### Тесты
```cpp
void benchmarkActorModel() {
    const size_t NUM_MESSAGES = 10000;
    
    // 1. Actor Model
    auto start = std::chrono::high_resolution_clock::now();
    
    MessageRouter router;
    auto actor = std::make_shared<WorkerActor>("Worker");
    router.registerActor(actor);
    
    for (size_t i = 0; i < NUM_MESSAGES; ++i) {
        router.sendMessage("Worker", WorkMessage{static_cast<int>(i), "data"});
    }
    
    // Ждем обработки
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Actor Model: " << NUM_MESSAGES << " messages in " 
              << duration.count() << " ms" << std::endl;
    std::cout << "Throughput: " << (NUM_MESSAGES * 1000.0 / duration.count()) 
              << " msgs/sec" << std::endl;
}
```

---

**Автор**: Senior C++ Developer  
**Дата**: 2025-10-07  
**Сложность**: ⭐⭐⭐⭐⭐ (Экспертный уровень)
