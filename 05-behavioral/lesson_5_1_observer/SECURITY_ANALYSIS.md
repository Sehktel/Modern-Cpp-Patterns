# Анализ безопасности паттерна Observer

## Обзор

Паттерн Observer является одним из наиболее распространенных поведенческих паттернов, но при неправильной реализации может стать источником критических уязвимостей. Этот документ анализирует основные проблемы безопасности и предлагает решения.

## Критические уязвимости

### 1. Use-After-Free (UAF)

**Описание**: Subject хранит сырые указатели на Observers, которые могут быть удалены без уведомления Subject.

**Механизм атаки**:
```cpp
VulnerableSubject subject;
{
    Observer* obs = new ConcreteObserver();
    subject.attach(obs);
    delete obs;  // Subject все еще хранит указатель!
}
subject.notify();  // UAF - обращение к удаленному объекту
```

**Последствия**:
- Segmentation fault / Access Violation
- Выполнение произвольного кода (через перезапись vtable)
- Утечка конфиденциальных данных
- Нарушение контроля потока выполнения

**CVE примеры**: CVE-2019-11358 (jQuery event handlers UAF)

**Детектирование**:
```bash
# AddressSanitizer
g++ -fsanitize=address -g observer_vulnerabilities.cpp
./a.out
# Output: heap-use-after-free

# Valgrind
valgrind --tool=memcheck --leak-check=full ./observer_vulnerabilities
```

**Решение**:
```cpp
// Используйте weak_ptr
class SafeSubject {
    std::vector<std::weak_ptr<Observer>> observers_;
    
    void notify() {
        for (auto& weak_obs : observers_) {
            if (auto obs = weak_obs.lock()) {  // Безопасная проверка
                obs->update();
            }
        }
    }
};
```

### 2. Race Condition

**Описание**: Неатомарный доступ к списку observers в многопоточной среде.

**Механизм атаки**:
```cpp
// Поток 1: добавляет observer
subject.attach(observer);

// Поток 2: одновременно уведомляет
subject.notify();  // RACE - может читать некорректное состояние вектора
```

**Последствия**:
- Data race (undefined behavior по стандарту C++)
- Segmentation fault
- Iterator invalidation
- Потеря уведомлений
- Двойное уведомление

**Детектирование**:
```bash
# ThreadSanitizer
g++ -fsanitize=thread -g observer_vulnerabilities.cpp
./a.out
# Output: WARNING: ThreadSanitizer: data race
```

**Решение**:
```cpp
class ThreadSafeSubject {
    std::vector<std::weak_ptr<Observer>> observers_;
    mutable std::shared_mutex mutex_;
    
    void attach(std::shared_ptr<Observer> obs) {
        std::unique_lock lock(mutex_);  // Эксклюзивная блокировка
        observers_.push_back(obs);
    }
    
    void notify() {
        std::vector<std::shared_ptr<Observer>> active;
        {
            std::shared_lock lock(mutex_);  // Разделяемая блокировка
            for (auto& wp : observers_) {
                if (auto obs = wp.lock()) active.push_back(obs);
            }
        }
        // Уведомляем БЕЗ блокировки
        for (auto& obs : active) obs->update();
    }
};
```

### 3. Memory Leak (Циклические ссылки)

**Описание**: Observer и Subject владеют друг другом через shared_ptr, создавая циклическую ссылку.

**Механизм**:
```cpp
class Observer {
    std::shared_ptr<Subject> subject_;  // Observer владеет Subject
};

class Subject {
    std::vector<std::shared_ptr<Observer>> observers_;  // Subject владеет Observer
};
// Циклическая ссылка → memory leak!
```

**Последствия**:
- Постоянная утечка памяти
- Resource exhaustion
- Denial of Service (OOM)

**Детектирование**:
```bash
# Valgrind
valgrind --leak-check=full --show-leak-kinds=all ./observer
# Output: definitely lost: X bytes in Y blocks

# Heaptrack
heaptrack ./observer
heaptrack_gui heaptrack.observer.PID.gz
```

**Решение**:
```cpp
// Observer хранит weak_ptr к Subject
class Observer {
    std::weak_ptr<Subject> subject_;
};

// Subject хранит weak_ptr к Observers
class Subject {
    std::vector<std::weak_ptr<Observer>> observers_;
    
    void cleanupExpired() {
        observers_.erase(
            std::remove_if(observers_.begin(), observers_.end(),
                [](auto& wp) { return wp.expired(); }),
            observers_.end());
    }
};
```

### 4. Iterator Invalidation

**Описание**: Observer отписывается от Subject во время notify, инвалидируя итератор.

**Механизм**:
```cpp
void Subject::notify() {
    for (auto* obs : observers_) {  // Итератор
        obs->update();
        // Если update() вызывает detach(obs), итератор становится invalid!
    }
}
```

**Последствия**:
- Undefined behavior
- Segmentation fault
- Пропуск observers
- Двойная обработка

**Решение**:
```cpp
void Subject::notify() {
    // Копируем список для безопасной итерации
    auto observers_copy = observers_;
    for (auto& obs : observers_copy) {
        obs->update();
    }
}
```

### 5. Dangling Weak Pointer Accumulation

**Описание**: expired weak_ptr не очищаются из вектора, вызывая утечку ресурсов.

**Механизм**:
```cpp
std::vector<std::weak_ptr<Observer>> observers_;

void notify() {
    for (auto& wp : observers_) {
        if (auto obs = wp.lock()) {
            obs->update();
        }
        // Expired weak_ptr остается в векторе!
    }
}
// Вектор растет, производительность падает
```

**Последствия**:
- O(n) проверки expired указателей
- Деградация производительности
- Resource leak (память для weak_ptr control block)

**Решение**:
```cpp
void notify() {
    // Периодическая очистка
    observers_.erase(
        std::remove_if(observers_.begin(), observers_.end(),
            [](auto& wp) { return wp.expired(); }),
        observers_.end());
    
    for (auto& wp : observers_) {
        if (auto obs = wp.lock()) {
            obs->update();
        }
    }
}
```

## Дополнительные угрозы

### 6. TOCTOU (Time-of-Check to Time-of-Use)

**Описание**: Проверка состояния observer и его использование не атомарны.

```cpp
if (observer->isValid()) {  // Time of Check
    // Другой поток может изменить состояние здесь
    observer->update();     // Time of Use - может быть уже invalid
}
```

### 7. Timing Attack

**Описание**: Утечка информации через анализ времени обработки уведомлений.

```cpp
void notify(const std::string& secretData) {
    for (auto& obs : observers_) {
        // Разное время обработки может выдать информацию о secretData
        obs->update(secretData);
    }
}
```

**Защита**: Constant-time операции для секретных данных.

### 8. Unbounded Growth (DoS)

**Описание**: Отсутствие лимита на количество observers.

```cpp
// Атакующий может подписать миллионы observers
for (int i = 0; i < 1000000; ++i) {
    subject.attach(createHeavyObserver());
}
// Память исчерпана, производительность нулевая
```

**Защита**: Ограничение количества подписок, rate limiting.

## Инструменты анализа

### Статический анализ

#### Clang Static Analyzer
```bash
clang++ --analyze -Xanalyzer -analyzer-output=text \
    observer_vulnerabilities.cpp
```
Находит: use-after-free, memory leaks, null dereference

#### Cppcheck
```bash
cppcheck --enable=all --inconclusive --std=c++17 \
    observer_vulnerabilities.cpp
```
Находит: memory leaks, uninitialized variables, resource leaks

#### PVS-Studio
```bash
pvs-studio-analyzer analyze -o project.log
plog-converter -t fullhtml project.log -o report
```
Находит: data races, memory issues, logic errors

### Динамический анализ

#### AddressSanitizer (ASan)
```bash
g++ -fsanitize=address -fno-omit-frame-pointer -g \
    observer_vulnerabilities.cpp -o observer_asan
./observer_asan
```
Обнаруживает:
- Use-after-free
- Heap buffer overflow
- Stack buffer overflow
- Memory leaks

#### ThreadSanitizer (TSan)
```bash
g++ -fsanitize=thread -g observer_vulnerabilities.cpp -o observer_tsan
./observer_tsan
```
Обнаруживает:
- Data races
- Deadlocks
- Mutex misuse

#### Valgrind Memcheck
```bash
valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all \
    --track-origins=yes ./observer_vulnerabilities
```

#### Valgrind Helgrind
```bash
valgrind --tool=helgrind ./observer_vulnerabilities
```
Обнаруживает data races и lock order inversions.

## Практические задания

### Задание 1: Обнаружение Use-After-Free

```bash
# Скомпилируйте с AddressSanitizer
g++ -fsanitize=address -g observer_vulnerabilities.cpp -o observer_asan

# Запустите
./observer_asan

# Ожидаемый вывод:
# ==XXXXX==ERROR: AddressSanitizer: heap-use-after-free
```

### Задание 2: Обнаружение Race Condition

```bash
# Скомпилируйте с ThreadSanitizer
g++ -fsanitize=thread -g observer_vulnerabilities.cpp -o observer_tsan

# Запустите многократно
for i in {1..10}; do ./observer_tsan; done

# Ожидаемый вывод:
# WARNING: ThreadSanitizer: data race
```

### Задание 3: Анализ Memory Leaks

```bash
# Запустите Valgrind
valgrind --leak-check=full --show-leak-kinds=all ./observer_vulnerabilities

# Найдите циклические ссылки
# Hint: ищите "still reachable" blocks
```

### Задание 4: Создание эксплойта

Используйте `observer_exploits.cpp`:
```bash
g++ -std=c++17 exploits/observer_exploits.cpp -o exploits
./exploits
```

Попробуйте:
1. Эксплуатировать use-after-free для контроля vtable
2. Вызвать race condition для обхода проверки permissions
3. Исчерпать память через подписку множества observers

## Защитные меры

### Best Practices

1. **Используйте умные указатели**
   ```cpp
   std::vector<std::weak_ptr<Observer>> observers_;  // НЕ std::vector<Observer*>
   ```

2. **RAII для подписок**
   ```cpp
   class Subscription {
       std::function<void()> unsubscribe_;
   public:
       Subscription(auto unsub) : unsubscribe_(unsub) {}
       ~Subscription() { unsubscribe_(); }
   };
   ```

3. **Thread-safe доступ**
   ```cpp
   std::shared_mutex mutex_;  // Позволяет concurrent reads
   ```

4. **Копируйте для итерации**
   ```cpp
   auto observers_copy = observers_;  // Копируйте перед notify
   ```

5. **Ограничивайте количество**
   ```cpp
   static constexpr size_t MAX_OBSERVERS = 1000;
   if (observers_.size() >= MAX_OBSERVERS) {
       throw std::runtime_error("Too many observers");
   }
   ```

6. **Обрабатывайте исключения**
   ```cpp
   for (auto& obs : observers_copy) {
       try {
           obs->update();
       } catch (const std::exception& e) {
           // Логируйте, но продолжайте
       }
   }
   ```

7. **Периодическая очистка**
   ```cpp
   void cleanup() {
       observers_.erase(
           std::remove_if(observers_.begin(), observers_.end(),
               [](auto& wp) { return wp.expired(); }),
           observers_.end());
   }
   ```

## Связь с другими паттернами

### Mediator
- Observer может быть реализован через Mediator
- Уязвимости: централизованная точка отказа

### Command
- Command objects могут быть observers
- Уязвимости: command injection, replay attacks

### Strategy
- Strategies могут использовать Observer для уведомлений
- Уязвимости: наследуют проблемы обоих паттернов

### Singleton
- Subject часто является Singleton
- Уязвимости: глобальное состояние, race conditions при инициализации

## Чек-лист безопасности

- [ ] Используются weak_ptr вместо сырых указателей
- [ ] Есть защита от race conditions (мьютексы)
- [ ] Реализована очистка expired weak_ptr
- [ ] Копируется список observers перед notify
- [ ] Есть лимит на количество подписок
- [ ] Обрабатываются исключения в callbacks
- [ ] Применяется RAII для автоматической отписки
- [ ] Протестировано с ASan/TSan
- [ ] Нет циклических ссылок shared_ptr
- [ ] Документированы потокобезопасные методы

## Дополнительные ресурсы

### Книги
- "Secure Coding in C and C++" - Robert Seacord
- "The Art of Software Security Assessment" - Mark Dowd et al.

### Статьи
- CWE-416: Use After Free
- CWE-362: Concurrent Execution using Shared Resource with Improper Synchronization
- CWE-401: Memory Leak

### Инструменты
- [AddressSanitizer](https://github.com/google/sanitizers)
- [Valgrind](https://valgrind.org/)
- [Clang Static Analyzer](https://clang-analyzer.llvm.org/)

## Заключение

Паттерн Observer, несмотря на свою полезность, требует особого внимания к безопасности. Основные риски связаны с управлением памятью и многопоточностью. Использование современных C++ возможностей (умные указатели, RAII, мьютексы) значительно снижает вероятность уязвимостей.

**Ключевые выводы**:
- ✅ Всегда используйте weak_ptr для хранения observers
- ✅ Защищайте shared state с помощью мьютексов
- ✅ Применяйте RAII для управления подписками
- ✅ Регулярно тестируйте с санитайзерами
- ✅ Ограничивайте количество подписок
