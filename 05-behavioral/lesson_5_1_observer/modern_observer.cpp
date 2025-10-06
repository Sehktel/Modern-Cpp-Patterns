#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <map>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <chrono>
#include <queue>
#include <atomic>

/**
 * @file modern_observer.cpp
 * @brief Современные подходы к реализации Observer паттерна в C++
 * 
 * Этот файл демонстрирует продвинутые техники реализации Observer
 * с использованием современных возможностей C++17/20.
 */

// ============================================================================
// SIGNAL-SLOT SYSTEM (ВДОХНОВЛЕНО QT)
// ============================================================================

/**
 * @brief Система сигналов и слотов
 */
class SignalSlotSystem {
private:
    std::unordered_map<std::string, std::vector<std::function<void()>>> slots_;
    mutable std::mutex mutex_;
    
public:
    void connect(const std::string& signal, std::function<void()> slot) {
        std::lock_guard<std::mutex> lock(mutex_);
        slots_[signal].push_back(slot);
        std::cout << "Слот подключен к сигналу '" << signal << "'" << std::endl;
    }
    
    void emit(const std::string& signal) {
        std::vector<std::function<void()>> slots_copy;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = slots_.find(signal);
            if (it != slots_.end()) {
                slots_copy = it->second;
            }
        }
        
        std::cout << "\n--- Сигнал '" << signal << "' ---" << std::endl;
        for (const auto& slot : slots_copy) {
            slot();
        }
    }
    
    void disconnect(const std::string& signal, std::function<void()> slot) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = slots_.find(signal);
        if (it != slots_.end()) {
            auto& slots = it->second;
            slots.erase(std::remove(slots.begin(), slots.end(), slot), slots.end());
            std::cout << "Слот отключен от сигнала '" << signal << "'" << std::endl;
        }
    }
    
    size_t getSlotCount(const std::string& signal) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = slots_.find(signal);
        return (it != slots_.end()) ? it->second.size() : 0;
    }
};

// ============================================================================
// REACTIVE STREAMS
// ============================================================================

/**
 * @brief Простая реализация реактивных потоков
 */
template<typename T>
class Observable {
private:
    std::vector<std::function<void(const T&)>> observers_;
    mutable std::mutex mutex_;
    std::string name_;
    
public:
    explicit Observable(const std::string& name) : name_(name) {}
    
    void subscribe(std::function<void(const T&)> observer) {
        std::lock_guard<std::mutex> lock(mutex_);
        observers_.push_back(observer);
        std::cout << "Подписка на Observable '" << name_ << "'" << std::endl;
    }
    
    void emit(const T& value) {
        std::vector<std::function<void(const T&)>> observers_copy;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            observers_copy = observers_;
        }
        
        std::cout << "\n--- Observable '" << name_ << "' испускает значение ---" << std::endl;
        for (const auto& observer : observers_copy) {
            observer(value);
        }
    }
    
    size_t getObserverCount() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return observers_.size();
    }
};

/**
 * @brief Операторы для Observable
 */
template<typename T>
class ObservableOperators {
public:
    // Фильтрация
    static Observable<T> filter(Observable<T>& source, std::function<bool(const T&)> predicate) {
        Observable<T> filtered("Filtered_" + std::to_string(reinterpret_cast<uintptr_t>(&source)));
        
        source.subscribe([&filtered, predicate](const T& value) {
            if (predicate(value)) {
                filtered.emit(value);
            }
        });
        
        return filtered;
    }
    
    // Преобразование
    template<typename U>
    static Observable<U> map(Observable<T>& source, std::function<U(const T&)> transformer) {
        Observable<U> mapped("Mapped_" + std::to_string(reinterpret_cast<uintptr_t>(&source)));
        
        source.subscribe([&mapped, transformer](const T& value) {
            mapped.emit(transformer(value));
        });
        
        return mapped;
    }
};

// ============================================================================
// EVENT BUS
// ============================================================================

/**
 * @brief Централизованная система событий
 */
class EventBus {
private:
    std::unordered_map<std::string, std::vector<std::function<void(const std::string&)>>> handlers_;
    mutable std::mutex mutex_;
    std::queue<std::pair<std::string, std::string>> eventQueue_;
    std::atomic<bool> processing_;
    
public:
    EventBus() : processing_(false) {}
    
    void subscribe(const std::string& eventType, std::function<void(const std::string&)> handler) {
        std::lock_guard<std::mutex> lock(mutex_);
        handlers_[eventType].push_back(handler);
        std::cout << "Обработчик подписан на событие '" << eventType << "'" << std::endl;
    }
    
    void publish(const std::string& eventType, const std::string& data) {
        std::lock_guard<std::mutex> lock(mutex_);
        eventQueue_.push({eventType, data});
        
        if (!processing_) {
            processing_ = true;
            processEvents();
            processing_ = false;
        }
    }
    
    void publishAsync(const std::string& eventType, const std::string& data) {
        std::lock_guard<std::mutex> lock(mutex_);
        eventQueue_.push({eventType, data});
    }
    
    void processEvents() {
        while (!eventQueue_.empty()) {
            auto event = eventQueue_.front();
            eventQueue_.pop();
            
            auto it = handlers_.find(event.first);
            if (it != handlers_.end()) {
                std::cout << "\n--- Событие '" << event.first << "' ---" << std::endl;
                for (const auto& handler : it->second) {
                    handler(event.second);
                }
            }
        }
    }
    
    size_t getHandlerCount(const std::string& eventType) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = handlers_.find(eventType);
        return (it != handlers_.end()) ? it->second.size() : 0;
    }
    
    size_t getQueueSize() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return eventQueue_.size();
    }
};

// ============================================================================
// PUBLISH-SUBSCRIBE WITH TOPICS
// ============================================================================

/**
 * @brief Система публикации-подписки с топиками
 */
class PubSubSystem {
private:
    struct Subscription {
        std::string subscriberId_;
        std::function<void(const std::string&)> handler_;
        
        Subscription(const std::string& id, std::function<void(const std::string&)> handler)
            : subscriberId_(id), handler_(handler) {}
    };
    
    std::unordered_map<std::string, std::vector<Subscription>> topics_;
    mutable std::mutex mutex_;
    
public:
    void subscribe(const std::string& topic, const std::string& subscriberId, 
                  std::function<void(const std::string&)> handler) {
        std::lock_guard<std::mutex> lock(mutex_);
        topics_[topic].emplace_back(subscriberId, handler);
        std::cout << "Подписчик '" << subscriberId << "' подписан на топик '" << topic << "'" << std::endl;
    }
    
    void unsubscribe(const std::string& topic, const std::string& subscriberId) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = topics_.find(topic);
        if (it != topics_.end()) {
            auto& subscriptions = it->second;
            subscriptions.erase(
                std::remove_if(subscriptions.begin(), subscriptions.end(),
                    [&subscriberId](const Subscription& sub) {
                        return sub.subscriberId_ == subscriberId;
                    }),
                subscriptions.end());
            std::cout << "Подписчик '" << subscriberId << "' отписан от топика '" << topic << "'" << std::endl;
        }
    }
    
    void publish(const std::string& topic, const std::string& message) {
        std::vector<Subscription> subscriptions_copy;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = topics_.find(topic);
            if (it != topics_.end()) {
                subscriptions_copy = it->second;
            }
        }
        
        std::cout << "\n--- Публикация в топик '" << topic << "' ---" << std::endl;
        for (const auto& subscription : subscriptions_copy) {
            std::cout << "Отправка подписчику '" << subscription.subscriberId_ << "': ";
            subscription.handler_(message);
        }
    }
    
    size_t getSubscriberCount(const std::string& topic) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = topics_.find(topic);
        return (it != topics_.end()) ? it->second.size() : 0;
    }
    
    std::vector<std::string> getTopics() const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<std::string> topics;
        for (const auto& pair : topics_) {
            topics.push_back(pair.first);
        }
        return topics;
    }
};

// ============================================================================
// OBSERVER WITH PRIORITY
// ============================================================================

/**
 * @brief Наблюдатель с приоритетами
 */
class PriorityObserver {
private:
    struct PriorityHandler {
        int priority_;
        std::function<void(const std::string&)> handler_;
        std::string name_;
        
        PriorityHandler(int priority, std::function<void(const std::string&)> handler, const std::string& name)
            : priority_(priority), handler_(handler), name_(name) {}
        
        bool operator<(const PriorityHandler& other) const {
            return priority_ < other.priority_; // Меньший приоритет = выше в очереди
        }
    };
    
    std::vector<PriorityHandler> handlers_;
    mutable std::mutex mutex_;
    std::string subjectName_;
    
public:
    explicit PriorityObserver(const std::string& name) : subjectName_(name) {}
    
    void attach(int priority, std::function<void(const std::string&)> handler, const std::string& name) {
        std::lock_guard<std::mutex> lock(mutex_);
        handlers_.emplace_back(priority, handler, name);
        std::sort(handlers_.begin(), handlers_.end());
        std::cout << "Обработчик '" << name << "' с приоритетом " << priority 
                  << " добавлен к '" << subjectName_ << "'" << std::endl;
    }
    
    void notify(const std::string& message) {
        std::vector<PriorityHandler> handlers_copy;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            handlers_copy = handlers_;
        }
        
        std::cout << "\n--- Уведомление от '" << subjectName_ << "' (с приоритетами) ---" << std::endl;
        for (const auto& handler : handlers_copy) {
            std::cout << "[" << handler.priority_ << "] " << handler.name_ << ": ";
            handler.handler_(message);
        }
    }
    
    size_t getHandlerCount() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return handlers_.size();
    }
};

// ============================================================================
// ДЕМОНСТРАЦИОННЫЕ ФУНКЦИИ
// ============================================================================

/**
 * @brief Демонстрация системы сигналов и слотов
 */
void demonstrateSignalSlotSystem() {
    std::cout << "\n=== Система сигналов и слотов ===" << std::endl;
    
    SignalSlotSystem signalSlot;
    
    // Подключаем слоты к сигналам
    signalSlot.connect("button_clicked", []() {
        std::cout << "🔘 Кнопка нажата!" << std::endl;
    });
    
    signalSlot.connect("button_clicked", []() {
        std::cout << "📊 Обновляем статистику" << std::endl;
    });
    
    signalSlot.connect("window_closed", []() {
        std::cout << "🪟 Окно закрыто" << std::endl;
    });
    
    // Испускаем сигналы
    signalSlot.emit("button_clicked");
    signalSlot.emit("window_closed");
    
    std::cout << "Слотов на button_clicked: " << signalSlot.getSlotCount("button_clicked") << std::endl;
    std::cout << "Слотов на window_closed: " << signalSlot.getSlotCount("window_closed") << std::endl;
}

/**
 * @brief Демонстрация реактивных потоков
 */
void demonstrateReactiveStreams() {
    std::cout << "\n=== Реактивные потоки ===" << std::endl;
    
    Observable<int> numbers("Numbers");
    
    // Подписываемся на числа
    numbers.subscribe([](int value) {
        std::cout << "📊 Получено число: " << value << std::endl;
    });
    
    numbers.subscribe([](int value) {
        std::cout << "🔢 Квадрат числа: " << (value * value) << std::endl;
    });
    
    // Создаем фильтрованный поток
    auto evenNumbers = ObservableOperators<int>::filter(numbers, [](int value) {
        return value % 2 == 0;
    });
    
    evenNumbers.subscribe([](int value) {
        std::cout << "✨ Четное число: " << value << std::endl;
    });
    
    // Создаем преобразованный поток
    auto strings = ObservableOperators<int>::map(numbers, [](int value) {
        return "Number: " + std::to_string(value);
    });
    
    strings.subscribe([](const std::string& str) {
        std::cout << "📝 Строка: " << str << std::endl;
    });
    
    // Испускаем значения
    for (int i = 1; i <= 5; ++i) {
        numbers.emit(i);
    }
    
    std::cout << "Наблюдателей в основном потоке: " << numbers.getObserverCount() << std::endl;
}

/**
 * @brief Демонстрация Event Bus
 */
void demonstrateEventBus() {
    std::cout << "\n=== Event Bus ===" << std::endl;
    
    EventBus eventBus;
    
    // Подписываемся на события
    eventBus.subscribe("user_login", [](const std::string& data) {
        std::cout << "👤 Пользователь вошел: " << data << std::endl;
    });
    
    eventBus.subscribe("user_login", [](const std::string& data) {
        std::cout << "📊 Обновляем статистику входа" << std::endl;
    });
    
    eventBus.subscribe("data_updated", [](const std::string& data) {
        std::cout << "💾 Данные обновлены: " << data << std::endl;
    });
    
    // Публикуем события
    eventBus.publish("user_login", "john_doe");
    eventBus.publish("data_updated", "user_profile");
    eventBus.publish("user_login", "jane_smith");
    
    std::cout << "Обработчиков user_login: " << eventBus.getHandlerCount("user_login") << std::endl;
    std::cout << "Обработчиков data_updated: " << eventBus.getHandlerCount("data_updated") << std::endl;
}

/**
 * @brief Демонстрация Pub-Sub системы
 */
void demonstratePubSubSystem() {
    std::cout << "\n=== Pub-Sub система с топиками ===" << std::endl;
    
    PubSubSystem pubSub;
    
    // Подписываемся на топики
    pubSub.subscribe("news", "subscriber1", [](const std::string& message) {
        std::cout << "📰 Новости: " << message << std::endl;
    });
    
    pubSub.subscribe("news", "subscriber2", [](const std::string& message) {
        std::cout << "📺 ТВ: " << message << std::endl;
    });
    
    pubSub.subscribe("weather", "subscriber1", [](const std::string& message) {
        std::cout << "🌤️ Погода: " << message << std::endl;
    });
    
    pubSub.subscribe("sports", "subscriber3", [](const std::string& message) {
        std::cout << "⚽ Спорт: " << message << std::endl;
    });
    
    // Публикуем сообщения
    pubSub.publish("news", "Важные новости дня");
    pubSub.publish("weather", "Солнечно, +25°C");
    pubSub.publish("sports", "Футбольный матч завершен");
    
    // Отписываемся от одного топика
    pubSub.unsubscribe("news", "subscriber2");
    
    // Публикуем еще раз
    pubSub.publish("news", "Обновленные новости");
    
    std::cout << "Подписчиков на news: " << pubSub.getSubscriberCount("news") << std::endl;
    std::cout << "Подписчиков на weather: " << pubSub.getSubscriberCount("weather") << std::endl;
    std::cout << "Подписчиков на sports: " << pubSub.getSubscriberCount("sports") << std::endl;
}

/**
 * @brief Демонстрация наблюдателя с приоритетами
 */
void demonstratePriorityObserver() {
    std::cout << "\n=== Наблюдатель с приоритетами ===" << std::endl;
    
    PriorityObserver priorityObserver("Priority System");
    
    // Добавляем обработчики с разными приоритетами
    priorityObserver.attach(10, [](const std::string& msg) {
        std::cout << "Низкий приоритет: " << msg << std::endl;
    }, "LowPriority");
    
    priorityObserver.attach(1, [](const std::string& msg) {
        std::cout << "Высокий приоритет: " << msg << std::endl;
    }, "HighPriority");
    
    priorityObserver.attach(5, [](const std::string& msg) {
        std::cout << "Средний приоритет: " << msg << std::endl;
    }, "MediumPriority");
    
    priorityObserver.attach(2, [](const std::string& msg) {
        std::cout << "Критический приоритет: " << msg << std::endl;
    }, "CriticalPriority");
    
    // Отправляем уведомление
    priorityObserver.notify("Важное сообщение");
    
    std::cout << "Всего обработчиков: " << priorityObserver.getHandlerCount() << std::endl;
}

// ============================================================================
// ОСНОВНАЯ ФУНКЦИЯ
// ============================================================================

int main() {
    std::cout << "🚀 Современные подходы к Observer Pattern" << std::endl;
    std::cout << std::string(50, '=') << std::endl;
    
    demonstrateSignalSlotSystem();
    demonstrateReactiveStreams();
    demonstrateEventBus();
    demonstratePubSubSystem();
    demonstratePriorityObserver();
    
    std::cout << "\n✅ Демонстрация современных подходов завершена!" << std::endl;
    std::cout << "\n🎯 Ключевые выводы:" << std::endl;
    std::cout << "• Signal-Slot система упрощает работу с событиями" << std::endl;
    std::cout << "• Реактивные потоки позволяют создавать цепочки обработки" << std::endl;
    std::cout << "• Event Bus централизует управление событиями" << std::endl;
    std::cout << "• Pub-Sub система поддерживает топики и подписчиков" << std::endl;
    std::cout << "• Приоритеты позволяют контролировать порядок обработки" << std::endl;
    
    return 0;
}
