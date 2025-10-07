#include <iostream>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <chrono>
#include <algorithm>

/**
 * @file observer_vulnerabilities.cpp
 * @brief Демонстрация уязвимостей в паттерне Observer
 * 
 * Этот файл содержит примеры уязвимых реализаций паттерна Observer
 * для образовательных целей. НЕ ИСПОЛЬЗОВАТЬ В PRODUCTION!
 */

// ============================================================================
// УЯЗВИМОСТЬ 1: USE-AFTER-FREE
// Проблема: Subject хранит сырые указатели на observers, которые могут быть удалены
// ============================================================================

class IObserver {
public:
    virtual ~IObserver() = default;
    virtual void update(const std::string& message) = 0;
};

// УЯЗВИМАЯ РЕАЛИЗАЦИЯ: Использует сырые указатели
class VulnerableSubject {
private:
    std::vector<IObserver*> observers_;  // ОПАСНО: Сырые указатели!
    
public:
    void attach(IObserver* observer) {
        observers_.push_back(observer);
        std::cout << "[Уязвимый Subject] Observer подписан\n";
    }
    
    void detach(IObserver* observer) {
        observers_.erase(
            std::remove(observers_.begin(), observers_.end(), observer),
            observers_.end());
        std::cout << "[Уязвимый Subject] Observer отписан\n";
    }
    
    void notify(const std::string& message) {
        std::cout << "[Уязвимый Subject] Отправка уведомления...\n";
        for (auto* observer : observers_) {
            // ОПАСНО: observer может быть уже удален!
            observer->update(message);  // Potential use-after-free
        }
    }
};

class SimpleObserver : public IObserver {
private:
    std::string name_;
    
public:
    explicit SimpleObserver(const std::string& name) : name_(name) {}
    
    void update(const std::string& message) override {
        std::cout << "[Observer " << name_ << "] Получено: " << message << "\n";
    }
};

// Демонстрация use-after-free
void demonstrateUseAfterFree() {
    std::cout << "\n=== УЯЗВИМОСТЬ 1: Use-After-Free ===\n";
    
    VulnerableSubject subject;
    
    {
        SimpleObserver observer("Temporary");
        subject.attach(&observer);
        subject.notify("Первое сообщение");
        
        // observer выходит из области видимости и удаляется
    }
    
    // ОПАСНО: Subject все еще хранит указатель на удаленный объект
    std::cout << "\nПопытка отправить уведомление после удаления observer...\n";
    subject.notify("Второе сообщение");  // USE-AFTER-FREE!
}

// ============================================================================
// УЯЗВИМОСТЬ 2: RACE CONDITION
// Проблема: Неатомарный доступ к списку observers в многопоточной среде
// ============================================================================

class UnsafeSubject {
private:
    std::vector<std::function<void(int)>> observers_;
    // НЕТ МЬЮТЕКСА!
    
public:
    void attach(std::function<void(int)> observer) {
        observers_.push_back(observer);  // RACE CONDITION
    }
    
    void detach(std::function<void(int)> observer) {
        // Упрощенно: просто очищаем (тоже race condition)
        observers_.clear();  // RACE CONDITION
    }
    
    void notify(int value) {
        for (const auto& observer : observers_) {  // RACE CONDITION
            observer(value);
        }
    }
};

// Демонстрация race condition
void demonstrateRaceCondition() {
    std::cout << "\n=== УЯЗВИМОСТЬ 2: Race Condition ===\n";
    
    UnsafeSubject subject;
    
    // Поток 1: Постоянно добавляет observers
    std::thread producer([&subject]() {
        for (int i = 0; i < 100; ++i) {
            subject.attach([i](int val) {
                // Наблюдатель
            });
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
    });
    
    // Поток 2: Постоянно уведомляет observers
    std::thread notifier([&subject]() {
        for (int i = 0; i < 100; ++i) {
            subject.notify(i);  // RACE CONDITION: может произойти segfault
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
    });
    
    producer.join();
    notifier.join();
    
    std::cout << "Race condition test завершен (возможен segfault)\n";
}

// ============================================================================
// УЯЗВИМОСТЬ 3: MEMORY LEAK
// Проблема: Observer не отписывается от Subject
// ============================================================================

class LeakySubject {
private:
    std::vector<std::shared_ptr<IObserver>> observers_;
    
public:
    void attach(std::shared_ptr<IObserver> observer) {
        observers_.push_back(observer);  // Создает circular reference!
    }
    
    // Нет метода detach!
    
    void notify(const std::string& message) {
        for (auto& observer : observers_) {
            observer->update(message);
        }
    }
};

class SelfReferencingObserver : public IObserver {
private:
    std::shared_ptr<LeakySubject> subject_;  // ОПАСНО: Циклическая ссылка!
    std::string name_;
    
public:
    SelfReferencingObserver(std::shared_ptr<LeakySubject> subject, const std::string& name)
        : subject_(subject), name_(name) {}
    
    void update(const std::string& message) override {
        std::cout << "[Self-Referencing Observer " << name_ << "] " << message << "\n";
    }
};

// Демонстрация memory leak
void demonstrateMemoryLeak() {
    std::cout << "\n=== УЯЗВИМОСТЬ 3: Memory Leak (Circular Reference) ===\n";
    
    auto subject = std::make_shared<LeakySubject>();
    
    for (int i = 0; i < 5; ++i) {
        // ОПАСНО: Создаем циклическую ссылку
        auto observer = std::make_shared<SelfReferencingObserver>(
            subject, "Observer_" + std::to_string(i));
        subject->attach(observer);
    }
    
    subject->notify("Test message");
    
    std::cout << "Subject use_count: " << subject.use_count() << " (должен быть 1, но больше из-за циклических ссылок)\n";
    
    // subject и observers НЕ БУДУТ УДАЛЕНЫ из-за циклических ссылок!
}

// ============================================================================
// УЯЗВИМОСТЬ 4: ITERATOR INVALIDATION
// Проблема: Удаление observer во время итерации по списку
// ============================================================================

class InvalidatingSubject {
private:
    std::vector<IObserver*> observers_;
    
public:
    void attach(IObserver* observer) {
        observers_.push_back(observer);
    }
    
    void detach(IObserver* observer) {
        observers_.erase(
            std::remove(observers_.begin(), observers_.end(), observer),
            observers_.end());
    }
    
    void notify(const std::string& message) {
        // ОПАСНО: Observer может отписаться во время notify
        for (auto* observer : observers_) {  // Iterator может быть invalidated!
            observer->update(message);
        }
    }
};

class UnsubscribingObserver : public IObserver {
private:
    InvalidatingSubject& subject_;
    std::string name_;
    int callCount_ = 0;
    
public:
    UnsubscribingObserver(InvalidatingSubject& subject, const std::string& name)
        : subject_(subject), name_(name) {}
    
    void update(const std::string& message) override {
        std::cout << "[Unsubscribing Observer " << name_ << "] " << message << "\n";
        
        callCount_++;
        if (callCount_ >= 2) {
            std::cout << "[Observer " << name_ << "] Отписываюсь во время notify!\n";
            subject_.detach(this);  // ОПАСНО: Iterator invalidation!
        }
    }
};

// Демонстрация iterator invalidation
void demonstrateIteratorInvalidation() {
    std::cout << "\n=== УЯЗВИМОСТЬ 4: Iterator Invalidation ===\n";
    
    InvalidatingSubject subject;
    
    UnsubscribingObserver obs1(subject, "A");
    UnsubscribingObserver obs2(subject, "B");
    UnsubscribingObserver obs3(subject, "C");
    
    subject.attach(&obs1);
    subject.attach(&obs2);
    subject.attach(&obs3);
    
    std::cout << "Первое уведомление:\n";
    subject.notify("Сообщение 1");
    
    std::cout << "\nВторое уведомление (observers будут отписываться):\n";
    subject.notify("Сообщение 2");  // Iterator может быть invalidated!
}

// ============================================================================
// УЯЗВИМОСТЬ 5: DANGLING WEAK_PTR
// Проблема: Неправильная проверка weak_ptr перед использованием
// ============================================================================

class DanglingWeakSubject {
private:
    std::vector<std::weak_ptr<IObserver>> observers_;
    
public:
    void attach(std::weak_ptr<IObserver> observer) {
        observers_.push_back(observer);
    }
    
    void notify(const std::string& message) {
        for (auto& weak_obs : observers_) {
            auto observer = weak_obs.lock();
            if (observer) {
                observer->update(message);
            }
            // Проблема: НЕ удаляем expired weak_ptr
            // Со временем вектор заполняется мертвыми указателями
        }
    }
    
    size_t getObserverCount() const {
        return observers_.size();  // НЕПРАВИЛЬНО: включает expired указатели!
    }
};

// Демонстрация dangling weak_ptr
void demonstrateDanglingWeakPtr() {
    std::cout << "\n=== УЯЗВИМОСТЬ 5: Dangling Weak Pointers ===\n";
    
    DanglingWeakSubject subject;
    
    for (int i = 0; i < 10; ++i) {
        auto observer = std::make_shared<SimpleObserver>("Temp_" + std::to_string(i));
        subject.attach(observer);
        // observer удаляется сразу после выхода из итерации
    }
    
    std::cout << "Количество observers (включая мертвые): " << subject.getObserverCount() << "\n";
    
    subject.notify("Сообщение для несуществующих observers");
    
    std::cout << "Все observers мертвы, но weak_ptr остаются в векторе!\n";
}

// ============================================================================
// MAIN
// ============================================================================

int main() {
    std::cout << "=== ДЕМОНСТРАЦИЯ УЯЗВИМОСТЕЙ OBSERVER PATTERN ===\n";
    std::cout << "⚠️  ВНИМАНИЕ: Этот код содержит уязвимости для образовательных целей!\n\n";
    
    try {
        demonstrateUseAfterFree();          // Может вызвать segfault
    } catch (...) {
        std::cout << "Caught exception in use-after-free demo\n";
    }
    
    try {
        demonstrateRaceCondition();         // Может вызвать data race
    } catch (...) {
        std::cout << "Caught exception in race condition demo\n";
    }
    
    demonstrateMemoryLeak();                // Memory leak
    demonstrateIteratorInvalidation();      // Может вызвать undefined behavior
    demonstrateDanglingWeakPtr();           // Resource leak
    
    std::cout << "\n=== АНАЛИЗ ЗАВЕРШЕН ===\n";
    std::cout << "Используйте:\n";
    std::cout << "• AddressSanitizer: g++ -fsanitize=address -g observer_vulnerabilities.cpp\n";
    std::cout << "• ThreadSanitizer: g++ -fsanitize=thread -g observer_vulnerabilities.cpp\n";
    std::cout << "• Valgrind: valgrind --leak-check=full ./observer_vulnerabilities\n";
    
    return 0;
}
