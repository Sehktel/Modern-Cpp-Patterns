#include <iostream>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <algorithm>
#include <functional>
#include <shared_mutex>

/**
 * @file secure_observer_alternatives.cpp
 * @brief Безопасные реализации паттерна Observer
 * 
 * Этот файл демонстрирует безопасные подходы к реализации Observer,
 * которые предотвращают распространенные уязвимости.
 */

// ============================================================================
// БЕЗОПАСНАЯ РЕАЛИЗАЦИЯ 1: ИСПОЛЬЗОВАНИЕ WEAK_PTR
// Решает: Use-After-Free, Memory Leaks
// ============================================================================

class IObserver {
public:
    virtual ~IObserver() = default;
    virtual void update(const std::string& message) = 0;
    virtual std::string getName() const = 0;
};

/**
 * @brief Безопасный Subject с использованием weak_ptr
 * 
 * Использует weak_ptr для предотвращения циклических ссылок
 * и автоматического удаления "мертвых" observers
 */
class SafeSubject {
private:
    std::vector<std::weak_ptr<IObserver>> observers_;
    std::mutex mutex_;  // Защита от race conditions
    std::string name_;
    
    // Очистка expired weak_ptr
    void cleanupExpiredObservers() {
        observers_.erase(
            std::remove_if(observers_.begin(), observers_.end(),
                [](const std::weak_ptr<IObserver>& wp) {
                    return wp.expired();
                }),
            observers_.end());
    }
    
public:
    explicit SafeSubject(const std::string& name) : name_(name) {}
    
    void attach(std::shared_ptr<IObserver> observer) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // Проверяем, не подписан ли уже этот observer
        auto it = std::find_if(observers_.begin(), observers_.end(),
            [&observer](const std::weak_ptr<IObserver>& wp) {
                return wp.lock() == observer;
            });
        
        if (it == observers_.end()) {
            observers_.push_back(observer);
            std::cout << "[SafeSubject " << name_ << "] Observer '" 
                      << observer->getName() << "' подписан\n";
        }
        
        cleanupExpiredObservers();
    }
    
    void detach(std::shared_ptr<IObserver> observer) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        observers_.erase(
            std::remove_if(observers_.begin(), observers_.end(),
                [&observer](const std::weak_ptr<IObserver>& wp) {
                    return wp.lock() == observer;
                }),
            observers_.end());
        
        std::cout << "[SafeSubject " << name_ << "] Observer '" 
                  << observer->getName() << "' отписан\n";
    }
    
    void notify(const std::string& message) {
        // Создаем копию observers для итерации без блокировки
        std::vector<std::shared_ptr<IObserver>> active_observers;
        
        {
            std::lock_guard<std::mutex> lock(mutex_);
            
            for (auto& weak_obs : observers_) {
                if (auto observer = weak_obs.lock()) {
                    active_observers.push_back(observer);
                }
            }
        }
        
        // Уведомляем без удержания мьютекса
        std::cout << "[SafeSubject " << name_ << "] Уведомление " 
                  << active_observers.size() << " observers\n";
        
        for (auto& observer : active_observers) {
            try {
                observer->update(message);
            } catch (const std::exception& e) {
                std::cerr << "Exception in observer: " << e.what() << "\n";
            }
        }
        
        // Очистка после уведомления
        {
            std::lock_guard<std::mutex> lock(mutex_);
            cleanupExpiredObservers();
        }
    }
    
    size_t getActiveObserverCount() const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        return std::count_if(observers_.begin(), observers_.end(),
            [](const std::weak_ptr<IObserver>& wp) {
                return !wp.expired();
            });
    }
};

class SafeObserver : public IObserver {
private:
    std::string name_;
    
public:
    explicit SafeObserver(const std::string& name) : name_(name) {
        std::cout << "[SafeObserver " << name_ << "] Создан\n";
    }
    
    ~SafeObserver() override {
        std::cout << "[SafeObserver " << name_ << "] Удален\n";
    }
    
    void update(const std::string& message) override {
        std::cout << "[SafeObserver " << name_ << "] Получено: " << message << "\n";
    }
    
    std::string getName() const override {
        return name_;
    }
};

// Демонстрация безопасного Subject
void demonstrateSafeSubject() {
    std::cout << "\n=== БЕЗОПАСНАЯ РЕАЛИЗАЦИЯ 1: SafeSubject с weak_ptr ===\n";
    
    auto subject = std::make_shared<SafeSubject>("Main");
    
    {
        auto obs1 = std::make_shared<SafeObserver>("Observer1");
        auto obs2 = std::make_shared<SafeObserver>("Observer2");
        
        subject->attach(obs1);
        subject->attach(obs2);
        
        subject->notify("Первое сообщение");
        
        std::cout << "Активных observers: " << subject->getActiveObserverCount() << "\n";
        
        // obs1 и obs2 будут удалены здесь
    }
    
    std::cout << "\nОбъекты observers удалены\n";
    std::cout << "Активных observers: " << subject->getActiveObserverCount() << "\n";
    
    subject->notify("Сообщение после удаления observers");
}

// ============================================================================
// БЕЗОПАСНАЯ РЕАЛИЗАЦИЯ 2: RAII SUBSCRIPTION
// Решает: Забытая отписка, Resource leaks
// ============================================================================

/**
 * @brief RAII класс для автоматической подписки/отписки
 */
class Subscription {
private:
    std::function<void()> unsubscribe_;
    bool active_ = true;
    
public:
    Subscription(std::function<void()> unsubscribe)
        : unsubscribe_(std::move(unsubscribe)) {}
    
    ~Subscription() {
        unsubscribe();
    }
    
    // Запрещаем копирование
    Subscription(const Subscription&) = delete;
    Subscription& operator=(const Subscription&) = delete;
    
    // Разрешаем перемещение
    Subscription(Subscription&& other) noexcept
        : unsubscribe_(std::move(other.unsubscribe_)),
          active_(other.active_) {
        other.active_ = false;
    }
    
    void unsubscribe() {
        if (active_ && unsubscribe_) {
            unsubscribe_();
            active_ = false;
        }
    }
};

/**
 * @brief Subject с RAII подписками
 */
class RAIISubject {
private:
    std::vector<std::weak_ptr<IObserver>> observers_;
    mutable std::mutex mutex_;
    
public:
    Subscription subscribe(std::shared_ptr<IObserver> observer) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            observers_.push_back(observer);
        }
        
        std::cout << "[RAIISubject] Observer '" << observer->getName() << "' подписан\n";
        
        // Возвращаем RAII объект для автоматической отписки
        return Subscription([this, observer]() {
            std::lock_guard<std::mutex> lock(mutex_);
            
            observers_.erase(
                std::remove_if(observers_.begin(), observers_.end(),
                    [&observer](const std::weak_ptr<IObserver>& wp) {
                        return wp.lock() == observer;
                    }),
                observers_.end());
            
            std::cout << "[RAIISubject] Observer '" << observer->getName() << "' отписан (RAII)\n";
        });
    }
    
    void notify(const std::string& message) {
        std::vector<std::shared_ptr<IObserver>> active_observers;
        
        {
            std::lock_guard<std::mutex> lock(mutex_);
            for (auto& weak_obs : observers_) {
                if (auto obs = weak_obs.lock()) {
                    active_observers.push_back(obs);
                }
            }
        }
        
        for (auto& obs : active_observers) {
            obs->update(message);
        }
    }
};

// Демонстрация RAII подписок
void demonstrateRAIISubscription() {
    std::cout << "\n=== БЕЗОПАСНАЯ РЕАЛИЗАЦИЯ 2: RAII Subscription ===\n";
    
    RAIISubject subject;
    auto observer = std::make_shared<SafeObserver>("RAII Observer");
    
    {
        auto subscription = subject.subscribe(observer);
        subject.notify("Сообщение с активной подпиской");
        
        // subscription автоматически отпишется здесь
    }
    
    std::cout << "\nПосле выхода из scope:\n";
    subject.notify("Сообщение после автоматической отписки");
}

// ============================================================================
// БЕЗОПАСНАЯ РЕАЛИЗАЦИЯ 3: THREAD-SAFE С SHARED_MUTEX
// Решает: Race conditions, одновременное чтение/запись
// ============================================================================

/**
 * @brief Потокобезопасный Subject с read-write lock
 */
class ThreadSafeSubject {
private:
    std::vector<std::weak_ptr<IObserver>> observers_;
    mutable std::shared_mutex mutex_;  // Позволяет множественное чтение
    
public:
    void attach(std::shared_ptr<IObserver> observer) {
        std::unique_lock<std::shared_mutex> lock(mutex_);  // Эксклюзивная блокировка для записи
        
        observers_.push_back(observer);
        std::cout << "[ThreadSafeSubject] Observer '" << observer->getName() << "' подписан\n";
    }
    
    void detach(std::shared_ptr<IObserver> observer) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        
        observers_.erase(
            std::remove_if(observers_.begin(), observers_.end(),
                [&observer](const std::weak_ptr<IObserver>& wp) {
                    return wp.lock() == observer;
                }),
            observers_.end());
    }
    
    void notify(const std::string& message) {
        std::vector<std::shared_ptr<IObserver>> active_observers;
        
        {
            std::shared_lock<std::shared_mutex> lock(mutex_);  // Shared lock для чтения
            
            for (auto& weak_obs : observers_) {
                if (auto obs = weak_obs.lock()) {
                    active_observers.push_back(obs);
                }
            }
        }
        
        // Уведомляем без блокировки
        for (auto& obs : active_observers) {
            obs->update(message);
        }
    }
    
    size_t getObserverCount() const {
        std::shared_lock<std::shared_mutex> lock(mutex_);  // Shared lock для чтения
        
        return std::count_if(observers_.begin(), observers_.end(),
            [](const std::weak_ptr<IObserver>& wp) {
                return !wp.expired();
            });
    }
};

// Демонстрация thread-safe Subject
void demonstrateThreadSafeSubject() {
    std::cout << "\n=== БЕЗОПАСНАЯ РЕАЛИЗАЦИЯ 3: Thread-Safe Subject ===\n";
    
    auto subject = std::make_shared<ThreadSafeSubject>();
    
    // Создаем observers
    std::vector<std::shared_ptr<IObserver>> observers;
    for (int i = 0; i < 5; ++i) {
        observers.push_back(std::make_shared<SafeObserver>("ThreadObserver" + std::to_string(i)));
    }
    
    // Несколько потоков подписывают observers
    std::vector<std::thread> threads;
    
    for (auto& obs : observers) {
        threads.emplace_back([subject, obs]() {
            subject->attach(obs);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        });
    }
    
    // Поток уведомлений
    threads.emplace_back([subject]() {
        for (int i = 0; i < 3; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            subject->notify("Сообщение #" + std::to_string(i) + " из потока");
        }
    });
    
    // Ждем завершения всех потоков
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Финальное количество observers: " << subject->getObserverCount() << "\n";
}

// ============================================================================
// БЕЗОПАСНАЯ РЕАЛИЗАЦИЯ 4: SIGNAL-SLOT С TYPE SAFETY
// Решает: Type confusion, неправильные сигнатуры
// ============================================================================

/**
 * @brief Type-safe сигнал с compile-time проверками
 */
template<typename... Args>
class Signal {
private:
    struct Slot {
        std::function<void(Args...)> callback;
        uint64_t id;
    };
    
    std::vector<Slot> slots_;
    mutable std::mutex mutex_;
    uint64_t next_id_ = 0;
    
public:
    // Подписка с возвратом ID для отписки
    uint64_t connect(std::function<void(Args...)> callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        uint64_t id = next_id_++;
        slots_.push_back({std::move(callback), id});
        
        return id;
    }
    
    void disconnect(uint64_t id) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        slots_.erase(
            std::remove_if(slots_.begin(), slots_.end(),
                [id](const Slot& slot) {
                    return slot.id == id;
                }),
            slots_.end());
    }
    
    void emit(Args... args) const {
        std::vector<Slot> slots_copy;
        
        {
            std::lock_guard<std::mutex> lock(mutex_);
            slots_copy = slots_;
        }
        
        for (const auto& slot : slots_copy) {
            try {
                slot.callback(args...);
            } catch (const std::exception& e) {
                std::cerr << "Exception in slot: " << e.what() << "\n";
            }
        }
    }
};

// Демонстрация type-safe Signal
void demonstrateTypeSafeSignal() {
    std::cout << "\n=== БЕЗОПАСНАЯ РЕАЛИЗАЦИЯ 4: Type-Safe Signal ===\n";
    
    Signal<int, std::string> dataChanged;
    Signal<std::string> userLogin;
    
    // Type-safe подписки
    auto id1 = dataChanged.connect([](int value, const std::string& name) {
        std::cout << "[Signal] Данные изменились: " << name << " = " << value << "\n";
    });
    
    auto id2 = userLogin.connect([](const std::string& username) {
        std::cout << "[Signal] Пользователь вошел: " << username << "\n";
    });
    
    // Compile-time проверка типов
    dataChanged.emit(42, "температура");
    dataChanged.emit(100, "давление");
    
    userLogin.emit("john_doe");
    
    // Отписка
    dataChanged.disconnect(id1);
    userLogin.disconnect(id2);
    
    dataChanged.emit(999, "после отписки");  // Никто не получит
}

// ============================================================================
// MAIN
// ============================================================================

int main() {
    std::cout << "=== БЕЗОПАСНЫЕ РЕАЛИЗАЦИИ OBSERVER PATTERN ===\n\n";
    
    demonstrateSafeSubject();
    demonstrateRAIISubscription();
    demonstrateThreadSafeSubject();
    demonstrateTypeSafeSignal();
    
    std::cout << "\n=== РЕКОМЕНДАЦИИ ПО БЕЗОПАСНОСТИ ===\n";
    std::cout << "✅ Используйте weak_ptr для хранения observers\n";
    std::cout << "✅ Применяйте RAII для автоматической отписки\n";
    std::cout << "✅ Защищайте shared state с помощью мьютексов\n";
    std::cout << "✅ Используйте shared_mutex для read-write lock\n";
    std::cout << "✅ Делайте копии для итерации без блокировки\n";
    std::cout << "✅ Обрабатывайте исключения в callbacks\n";
    std::cout << "✅ Используйте type-safe подходы (templates)\n";
    
    return 0;
}
