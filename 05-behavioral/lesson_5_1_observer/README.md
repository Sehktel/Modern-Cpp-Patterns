# Урок 5.1: Observer Pattern (Наблюдатель)

## 🎯 Цель урока
Изучить паттерн Observer - один из наиболее важных поведенческих паттернов. Понять, как реализовать механизм уведомлений и как создать слабосвязанные системы с автоматическим обновлением.

## 📚 Что изучаем

### 1. Паттерн Observer
- **Определение**: Определяет зависимость "один ко многим" между объектами
- **Назначение**: Автоматическое уведомление зависимых объектов об изменениях
- **Применение**: MVC архитектура, события UI, системы уведомлений, модели данных

### 2. Компоненты паттерна
- **Subject (Субъект)**: Объект, за которым наблюдают
- **Observer (Наблюдатель)**: Объект, который получает уведомления
- **ConcreteSubject**: Конкретная реализация субъекта
- **ConcreteObserver**: Конкретная реализация наблюдателя

### 3. Современные подходы
- **std::function**: Современная замена виртуальных функций
- **Lambda expressions**: Функциональный подход
- **Signals and Slots**: Механизм Qt
- **Event-driven programming**: Программирование, управляемое событиями

## 🔍 Ключевые концепции

### Классический Observer
```cpp
// Интерфейс наблюдателя
class Observer {
public:
    virtual ~Observer() = default;
    virtual void update(const std::string& message) = 0;
};

// Интерфейс субъекта
class Subject {
public:
    virtual ~Subject() = default;
    virtual void attach(std::shared_ptr<Observer> observer) = 0;
    virtual void detach(std::shared_ptr<Observer> observer) = 0;
    virtual void notify(const std::string& message) = 0;
};

// Конкретный субъект
class ConcreteSubject : public Subject {
private:
    std::vector<std::weak_ptr<Observer>> observers_;
    std::string state_;
    
public:
    void attach(std::shared_ptr<Observer> observer) override {
        observers_.push_back(observer);
    }
    
    void detach(std::shared_ptr<Observer> observer) override {
        observers_.erase(
            std::remove_if(observers_.begin(), observers_.end(),
                [&observer](const std::weak_ptr<Observer>& weak_obs) {
                    return weak_obs.lock() == observer;
                }),
            observers_.end());
    }
    
    void notify(const std::string& message) override {
        for (auto it = observers_.begin(); it != observers_.end();) {
            if (auto observer = it->lock()) {
                observer->update(message);
                ++it;
            } else {
                it = observers_.erase(it);
            }
        }
    }
    
    void setState(const std::string& state) {
        state_ = state;
        notify("State changed to: " + state);
    }
};
```

### Современный Observer с std::function
```cpp
class ModernSubject {
private:
    std::vector<std::function<void(const std::string&)>> observers_;
    std::string state_;
    
public:
    void attach(std::function<void(const std::string&)> observer) {
        observers_.push_back(observer);
    }
    
    void notify(const std::string& message) {
        for (const auto& observer : observers_) {
            observer(message);
        }
    }
    
    void setState(const std::string& state) {
        state_ = state;
        notify("State changed to: " + state);
    }
};
```

## 🤔 Вопросы для размышления

### 1. Когда использовать Observer?
**✅ Хорошие случаи:**
- Изменение одного объекта требует изменения других
- Объекты должны быть слабо связаны
- Количество зависимых объектов неизвестно заранее
- Нужна поддержка MVC архитектуры
- Системы событий и уведомлений

**❌ Плохие случаи:**
- Когда есть циклические зависимости
- Когда производительность критична
- Когда порядок уведомлений важен
- Когда нужна гарантия доставки

### 2. Преимущества Observer
- **Слабая связанность**: Субъект не знает о конкретных наблюдателях
- **Динамические отношения**: Можно добавлять/удалять наблюдателей во время выполнения
- **Открытость для расширения**: Легко добавлять новые типы наблюдателей
- **Поддержка MVC**: Идеально подходит для архитектуры Model-View-Controller

### 3. Недостатки Observer
- **Производительность**: Накладные расходы на уведомления
- **Порядок уведомлений**: Не гарантирован порядок
- **Утечки памяти**: При неправильном использовании weak_ptr
- **Сложность отладки**: Может быть сложно отследить цепочку уведомлений

## 🛠️ Практические примеры

### Система уведомлений
```cpp
class NotificationSystem {
private:
    std::vector<std::function<void(const std::string&)>> subscribers_;
    
public:
    void subscribe(std::function<void(const std::string&)> callback) {
        subscribers_.push_back(callback);
    }
    
    void publish(const std::string& message) {
        for (const auto& subscriber : subscribers_) {
            subscriber(message);
        }
    }
};

// Использование
NotificationSystem notifications;

// Подписываемся на уведомления
notifications.subscribe([](const std::string& msg) {
    std::cout << "Email: " << msg << std::endl;
});

notifications.subscribe([](const std::string& msg) {
    std::cout << "SMS: " << msg << std::endl;
});

// Отправляем уведомление
notifications.publish("Новое сообщение!");
```

### Модель данных с представлениями
```cpp
class DataModel {
private:
    std::vector<std::function<void(int)>> views_;
    int data_;
    
public:
    void attachView(std::function<void(int)> view) {
        views_.push_back(view);
    }
    
    void setData(int newData) {
        data_ = newData;
        notifyViews();
    }
    
    int getData() const { return data_; }
    
private:
    void notifyViews() {
        for (const auto& view : views_) {
            view(data_);
        }
    }
};

// Представления
class ChartView {
public:
    void update(int data) {
        std::cout << "Chart updated with data: " << data << std::endl;
    }
};

class TableView {
public:
    void update(int data) {
        std::cout << "Table updated with data: " << data << std::endl;
    }
};
```

## 🎨 Современные подходы в C++

### Event-driven Observer
```cpp
template<typename... Args>
class Event {
private:
    std::vector<std::function<void(Args...)>> handlers_;
    
public:
    void subscribe(std::function<void(Args...)> handler) {
        handlers_.push_back(handler);
    }
    
    void emit(Args... args) {
        for (const auto& handler : handlers_) {
            handler(args...);
        }
    }
    
    size_t getSubscriberCount() const {
        return handlers_.size();
    }
};

// Использование
Event<int, std::string> dataChanged;
Event<> systemShutdown;

dataChanged.subscribe([](int value, const std::string& name) {
    std::cout << "Data changed: " << name << " = " << value << std::endl;
});

dataChanged.emit(42, "temperature");
```

### RAII Observer с автоматической отпиской
```cpp
class RAIIObserver {
private:
    std::function<void()> unsubscribe_;
    
public:
    template<typename Subject, typename Observer>
    RAIIObserver(Subject& subject, Observer observer) {
        subject.attach(observer);
        unsubscribe_ = [&subject, observer]() {
            subject.detach(observer);
        };
    }
    
    ~RAIIObserver() {
        if (unsubscribe_) {
            unsubscribe_();
        }
    }
    
    // Запрещаем копирование
    RAIIObserver(const RAIIObserver&) = delete;
    RAIIObserver& operator=(const RAIIObserver&) = delete;
    
    // Разрешаем перемещение
    RAIIObserver(RAIIObserver&& other) noexcept 
        : unsubscribe_(std::move(other.unsubscribe_)) {
        other.unsubscribe_ = nullptr;
    }
};
```

### Thread-safe Observer
```cpp
class ThreadSafeSubject {
private:
    std::vector<std::function<void(const std::string&)>> observers_;
    mutable std::mutex mutex_;
    
public:
    void attach(std::function<void(const std::string&)> observer) {
        std::lock_guard<std::mutex> lock(mutex_);
        observers_.push_back(observer);
    }
    
    void detach(std::function<void(const std::string&)> observer) {
        std::lock_guard<std::mutex> lock(mutex_);
        observers_.erase(
            std::remove(observers_.begin(), observers_.end(), observer),
            observers_.end());
    }
    
    void notify(const std::string& message) {
        std::vector<std::function<void(const std::string&)>> observers_copy;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            observers_copy = observers_;
        }
        
        for (const auto& observer : observers_copy) {
            observer(message);
        }
    }
};
```

## 🧪 Тестирование Observer

### Создание моков для тестирования
```cpp
class MockObserver {
public:
    MOCK_METHOD(void, update, (const std::string&), ());
};

// Тест
TEST(ObserverTest, Notification) {
    ConcreteSubject subject;
    auto mockObserver = std::make_shared<MockObserver>();
    
    subject.attach(mockObserver);
    
    EXPECT_CALL(*mockObserver, update("State changed to: new_state"))
        .Times(1);
    
    subject.setState("new_state");
}
```

## 🎯 Практические упражнения

### Упражнение 1: Система чата
Создайте систему чата с использованием Observer для уведомления пользователей о новых сообщениях.

### Упражнение 2: Модель-Представление
Реализуйте простую MVC архитектуру с моделью данных и несколькими представлениями.

### Упражнение 3: Event System
Создайте систему событий для игрового движка с различными типами событий.

### Упражнение 4: Configuration Observer
Реализуйте систему наблюдения за изменениями конфигурации приложения.

## 📈 Связь с другими паттернами

### Observer vs Mediator
- **Observer**: Один субъект уведомляет многих наблюдателей
- **Mediator**: Посредник координирует взаимодействие между объектами

### Observer vs Command
- **Observer**: Реакция на изменения состояния
- **Command**: Инкапсуляция запросов как объектов

### Observer vs MVC
- **Observer**: Основа архитектуры MVC
- **Model**: Субъект, **View**: Наблюдатель, **Controller**: Координатор

## 📈 Следующие шаги
После изучения Observer вы будете готовы к:
- Уроку 5.2: Strategy Pattern
- Пониманию поведенческих паттернов
- Созданию MVC архитектуры
- Разработке event-driven систем

## 💡 Важные принципы

1. **Слабая связанность**: Субъект не должен знать о конкретных наблюдателях
2. **RAII**: Используйте RAII для автоматической отписки
3. **Thread Safety**: Обеспечьте безопасность в многопоточном коде
4. **Производительность**: Учитывайте накладные расходы на уведомления
5. **Отладка**: Логируйте уведомления для упрощения отладки
