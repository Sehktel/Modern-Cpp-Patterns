#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <mutex>
#include <algorithm>
#include <thread>
#include <chrono>

/**
 * @file observer_pattern.cpp
 * @brief Демонстрация паттерна Observer
 * 
 * Этот файл показывает различные реализации паттерна Observer,
 * от классической до современных подходов с использованием C++17/20.
 */

// ============================================================================
// КЛАССИЧЕСКИЙ OBSERVER PATTERN
// ============================================================================

/**
 * @brief Интерфейс наблюдателя
 */
class Observer {
public:
    virtual ~Observer() = default;
    virtual void update(const std::string& message) = 0;
    virtual std::string getName() const = 0;
};

/**
 * @brief Интерфейс субъекта
 */
class Subject {
public:
    virtual ~Subject() = default;
    virtual void attach(std::shared_ptr<Observer> observer) = 0;
    virtual void detach(std::shared_ptr<Observer> observer) = 0;
    virtual void notify(const std::string& message) = 0;
};

/**
 * @brief Конкретный субъект - система уведомлений
 */
class NotificationSystem : public Subject {
private:
    std::vector<std::weak_ptr<Observer>> observers_;
    std::string systemName_;
    
public:
    explicit NotificationSystem(const std::string& name) : systemName_(name) {}
    
    void attach(std::shared_ptr<Observer> observer) override {
        observers_.push_back(observer);
        std::cout << "Observer '" << observer->getName() 
                  << "' подписан на систему '" << systemName_ << "'" << std::endl;
    }
    
    void detach(std::shared_ptr<Observer> observer) override {
        observers_.erase(
            std::remove_if(observers_.begin(), observers_.end(),
                [&observer](const std::weak_ptr<Observer>& weak_obs) {
                    return weak_obs.lock() == observer;
                }),
            observers_.end());
        std::cout << "Observer '" << observer->getName() 
                  << "' отписан от системы '" << systemName_ << "'" << std::endl;
    }
    
    void notify(const std::string& message) override {
        std::cout << "\n--- Уведомление от системы '" << systemName_ << "' ---" << std::endl;
        
        for (auto it = observers_.begin(); it != observers_.end();) {
            if (auto observer = it->lock()) {
                observer->update(message);
                ++it;
            } else {
                it = observers_.erase(it);
            }
        }
    }
    
    void publishMessage(const std::string& message) {
        notify(message);
    }
    
    size_t getObserverCount() const {
        return observers_.size();
    }
};

/**
 * @brief Конкретный наблюдатель - Email уведомления
 */
class EmailObserver : public Observer {
private:
    std::string email_;
    
public:
    explicit EmailObserver(const std::string& email) : email_(email) {}
    
    void update(const std::string& message) override {
        std::cout << "📧 Email отправлен на " << email_ 
                  << ": " << message << std::endl;
    }
    
    std::string getName() const override {
        return "EmailObserver(" + email_ + ")";
    }
};

/**
 * @brief Конкретный наблюдатель - SMS уведомления
 */
class SMSObserver : public Observer {
private:
    std::string phoneNumber_;
    
public:
    explicit SMSObserver(const std::string& phone) : phoneNumber_(phone) {}
    
    void update(const std::string& message) override {
        std::cout << "📱 SMS отправлен на " << phoneNumber_ 
                  << ": " << message << std::endl;
    }
    
    std::string getName() const override {
        return "SMSObserver(" + phoneNumber_ + ")";
    }
};

/**
 * @brief Конкретный наблюдатель - Логирование
 */
class LoggingObserver : public Observer {
private:
    std::string logFile_;
    
public:
    explicit LoggingObserver(const std::string& logFile) : logFile_(logFile) {}
    
    void update(const std::string& message) override {
        std::cout << "📝 Запись в лог " << logFile_ 
                  << ": " << message << std::endl;
    }
    
    std::string getName() const override {
        return "LoggingObserver(" + logFile_ + ")";
    }
};

// ============================================================================
// СОВРЕМЕННЫЙ OBSERVER С std::function
// ============================================================================

/**
 * @brief Современный субъект с использованием std::function
 */
class ModernSubject {
private:
    std::vector<std::function<void(const std::string&)>> observers_;
    std::string subjectName_;
    
public:
    explicit ModernSubject(const std::string& name) : subjectName_(name) {}
    
    void attach(std::function<void(const std::string&)> observer) {
        observers_.push_back(observer);
        std::cout << "Функциональный наблюдатель подписан на '" << subjectName_ << "'" << std::endl;
    }
    
    void notify(const std::string& message) {
        std::cout << "\n--- Уведомление от современного субъекта '" << subjectName_ << "' ---" << std::endl;
        for (const auto& observer : observers_) {
            observer(message);
        }
    }
    
    void publishMessage(const std::string& message) {
        notify(message);
    }
    
    size_t getObserverCount() const {
        return observers_.size();
    }
};

// ============================================================================
// EVENT-DRIVEN OBSERVER
// ============================================================================

/**
 * @brief Шаблонный класс для событий
 */
template<typename... Args>
class Event {
private:
    std::vector<std::function<void(Args...)>> handlers_;
    std::string eventName_;
    
public:
    explicit Event(const std::string& name) : eventName_(name) {}
    
    void subscribe(std::function<void(Args...)> handler) {
        handlers_.push_back(handler);
        std::cout << "Обработчик подписан на событие '" << eventName_ << "'" << std::endl;
    }
    
    void emit(Args... args) {
        std::cout << "\n--- Событие '" << eventName_ << "' ---" << std::endl;
        for (const auto& handler : handlers_) {
            handler(args...);
        }
    }
    
    size_t getSubscriberCount() const {
        return handlers_.size();
    }
    
    const std::string& getName() const {
        return eventName_;
    }
};

// ============================================================================
// RAII OBSERVER С АВТОМАТИЧЕСКОЙ ОТПИСКОЙ
// ============================================================================

/**
 * @brief RAII наблюдатель с автоматической отпиской
 */
class RAIIObserver {
private:
    std::function<void()> unsubscribe_;
    std::string observerName_;
    
public:
    template<typename Subject, typename Observer>
    RAIIObserver(Subject& subject, Observer observer, const std::string& name)
        : observerName_(name) {
        subject.attach(observer);
        unsubscribe_ = [&subject, observer]() {
            subject.detach(observer);
        };
        std::cout << "RAII Observer '" << name << "' создан" << std::endl;
    }
    
    ~RAIIObserver() {
        if (unsubscribe_) {
            unsubscribe_();
            std::cout << "RAII Observer '" << observerName_ << "' автоматически отписан" << std::endl;
        }
    }
    
    // Запрещаем копирование
    RAIIObserver(const RAIIObserver&) = delete;
    RAIIObserver& operator=(const RAIIObserver&) = delete;
    
    // Разрешаем перемещение
    RAIIObserver(RAIIObserver&& other) noexcept 
        : unsubscribe_(std::move(other.unsubscribe_)), observerName_(std::move(other.observerName_)) {
        other.unsubscribe_ = nullptr;
    }
    
    const std::string& getName() const {
        return observerName_;
    }
};

// ============================================================================
// THREAD-SAFE OBSERVER
// ============================================================================

/**
 * @brief Потокобезопасный субъект
 */
class ThreadSafeSubject {
private:
    std::vector<std::function<void(const std::string&)>> observers_;
    mutable std::mutex mutex_;
    std::string subjectName_;
    
public:
    explicit ThreadSafeSubject(const std::string& name) : subjectName_(name) {}
    
    void attach(std::function<void(const std::string&)> observer) {
        std::lock_guard<std::mutex> lock(mutex_);
        observers_.push_back(observer);
        std::cout << "Потокобезопасный наблюдатель подписан на '" << subjectName_ << "'" << std::endl;
    }
    
    void detach(std::function<void(const std::string&)> observer) {
        std::lock_guard<std::mutex> lock(mutex_);
        observers_.erase(
            std::remove(observers_.begin(), observers_.end(), observer),
            observers_.end());
        std::cout << "Потокобезопасный наблюдатель отписан от '" << subjectName_ << "'" << std::endl;
    }
    
    void notify(const std::string& message) {
        std::vector<std::function<void(const std::string&)>> observers_copy;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            observers_copy = observers_;
        }
        
        std::cout << "\n--- Потокобезопасное уведомление от '" << subjectName_ << "' ---" << std::endl;
        for (const auto& observer : observers_copy) {
            observer(message);
        }
    }
    
    void publishMessage(const std::string& message) {
        notify(message);
    }
    
    size_t getObserverCount() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return observers_.size();
    }
};

// ============================================================================
// ДЕМОНСТРАЦИОННЫЕ ФУНКЦИИ
// ============================================================================

/**
 * @brief Демонстрация классического Observer
 */
void demonstrateClassicObserver() {
    std::cout << "\n=== Классический Observer Pattern ===" << std::endl;
    
    // Создаем систему уведомлений
    auto notificationSystem = std::make_unique<NotificationSystem>("Main System");
    
    // Создаем наблюдателей
    auto emailObserver = std::make_shared<EmailObserver>("user@example.com");
    auto smsObserver = std::make_shared<SMSObserver>("+1234567890");
    auto logObserver = std::make_shared<LoggingObserver>("system.log");
    
    // Подписываем наблюдателей
    notificationSystem->attach(emailObserver);
    notificationSystem->attach(smsObserver);
    notificationSystem->attach(logObserver);
    
    std::cout << "Количество наблюдателей: " << notificationSystem->getObserverCount() << std::endl;
    
    // Отправляем уведомления
    notificationSystem->publishMessage("Система запущена");
    notificationSystem->publishMessage("Новое обновление доступно");
    
    // Отписываем одного наблюдателя
    notificationSystem->detach(smsObserver);
    std::cout << "Количество наблюдателей после отписки: " << notificationSystem->getObserverCount() << std::endl;
    
    // Отправляем еще одно уведомление
    notificationSystem->publishMessage("SMS больше не получает уведомления");
}

/**
 * @brief Демонстрация современного Observer
 */
void demonstrateModernObserver() {
    std::cout << "\n=== Современный Observer с std::function ===" << std::endl;
    
    ModernSubject subject("Modern System");
    
    // Подписываемся с помощью lambda функций
    subject.attach([](const std::string& msg) {
        std::cout << "🔔 Lambda 1: " << msg << std::endl;
    });
    
    subject.attach([](const std::string& msg) {
        std::cout << "🔔 Lambda 2: " << msg << std::endl;
    });
    
    // Подписываемся с помощью функтора
    struct MessagePrinter {
        std::string prefix_;
        explicit MessagePrinter(const std::string& prefix) : prefix_(prefix) {}
        
        void operator()(const std::string& msg) {
            std::cout << prefix_ << ": " << msg << std::endl;
        }
    };
    
    MessagePrinter printer("🎯 Functor");
    subject.attach([printer](const std::string& msg) {
        printer(msg);
    });
    
    std::cout << "Количество функциональных наблюдателей: " << subject.getObserverCount() << std::endl;
    
    // Отправляем сообщения
    subject.publishMessage("Современный подход к Observer");
    subject.publishMessage("Использование std::function");
}

/**
 * @brief Демонстрация Event-driven Observer
 */
void demonstrateEventDrivenObserver() {
    std::cout << "\n=== Event-driven Observer ===" << std::endl;
    
    // Создаем различные типы событий
    Event<int, std::string> dataChanged("DataChanged");
    Event<std::string> userLogin("UserLogin");
    Event<> systemShutdown("SystemShutdown");
    
    // Подписываемся на события
    dataChanged.subscribe([](int value, const std::string& name) {
        std::cout << "📊 Данные изменились: " << name << " = " << value << std::endl;
    });
    
    userLogin.subscribe([](const std::string& username) {
        std::cout << "👤 Пользователь вошел: " << username << std::endl;
    });
    
    systemShutdown.subscribe([]() {
        std::cout << "🛑 Система завершает работу" << std::endl;
    });
    
    // Генерируем события
    dataChanged.emit(42, "temperature");
    dataChanged.emit(100, "pressure");
    
    userLogin.emit("john_doe");
    userLogin.emit("jane_smith");
    
    systemShutdown.emit();
    
    std::cout << "Подписчиков на DataChanged: " << dataChanged.getSubscriberCount() << std::endl;
    std::cout << "Подписчиков на UserLogin: " << userLogin.getSubscriberCount() << std::endl;
    std::cout << "Подписчиков на SystemShutdown: " << systemShutdown.getSubscriberCount() << std::endl;
}

/**
 * @brief Демонстрация RAII Observer
 */
void demonstrateRAIIObserver() {
    std::cout << "\n=== RAII Observer с автоматической отпиской ===" << std::endl;
    
    auto notificationSystem = std::make_unique<NotificationSystem>("RAII System");
    
    // Создаем RAII наблюдателей
    {
        auto emailObserver = std::make_shared<EmailObserver>("raii@example.com");
        RAIIObserver raiiEmail(*notificationSystem, emailObserver, "RAII Email");
        
        auto smsObserver = std::make_shared<SMSObserver>("+9876543210");
        RAIIObserver raiiSMS(*notificationSystem, smsObserver, "RAII SMS");
        
        std::cout << "Количество наблюдателей: " << notificationSystem->getObserverCount() << std::endl;
        
        // Отправляем сообщение
        notificationSystem->publishMessage("RAII Observer работает");
        
        std::cout << "Выходим из области видимости..." << std::endl;
    } // RAII наблюдатели автоматически отписываются здесь
    
    std::cout << "Количество наблюдателей после выхода из области видимости: " 
              << notificationSystem->getObserverCount() << std::endl;
    
    // Отправляем еще одно сообщение
    notificationSystem->publishMessage("RAII наблюдатели автоматически отписались");
}

/**
 * @brief Демонстрация потокобезопасного Observer
 */
void demonstrateThreadSafeObserver() {
    std::cout << "\n=== Потокобезопасный Observer ===" << std::endl;
    
    ThreadSafeSubject subject("Thread Safe System");
    
    // Подписываемся на уведомления
    subject.attach([](const std::string& msg) {
        std::cout << "🧵 Thread 1: " << msg << std::endl;
    });
    
    subject.attach([](const std::string& msg) {
        std::cout << "🧵 Thread 2: " << msg << std::endl;
    });
    
    // Создаем несколько потоков, которые отправляют сообщения
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back([&subject, i]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(100 * i));
            subject.publishMessage("Сообщение из потока " + std::to_string(i));
        });
    }
    
    // Ждем завершения всех потоков
    for (auto& thread : threads) {
        thread.join();
    }
    
    std::cout << "Количество потокобезопасных наблюдателей: " << subject.getObserverCount() << std::endl;
}

// ============================================================================
// ОСНОВНАЯ ФУНКЦИЯ
// ============================================================================

int main() {
    std::cout << "👁️ Демонстрация паттерна Observer" << std::endl;
    std::cout << std::string(50, '=') << std::endl;
    
    demonstrateClassicObserver();
    demonstrateModernObserver();
    demonstrateEventDrivenObserver();
    demonstrateRAIIObserver();
    demonstrateThreadSafeObserver();
    
    std::cout << "\n✅ Демонстрация Observer Pattern завершена!" << std::endl;
    std::cout << "\n🎯 Ключевые выводы:" << std::endl;
    std::cout << "• Observer обеспечивает слабую связанность между объектами" << std::endl;
    std::cout << "• std::function позволяет использовать современный подход" << std::endl;
    std::cout << "• RAII обеспечивает автоматическую отписку" << std::endl;
    std::cout << "• Thread-safe версии необходимы в многопоточном коде" << std::endl;
    std::cout << "• Event-driven подход упрощает работу с событиями" << std::endl;
    
    return 0;
}
