# 🏋️ Упражнения: Hello World в современном C++

## 📋 Задание 1: Рефакторинг Legacy кода

### Описание
Перепишите следующий legacy код, используя современные возможности C++17:

```cpp
// Исходный код (C++98 стиль)
#include <iostream>
#include <vector>
#include <string>
#include <map>

class WordCounter {
private:
    std::map<std::string, int>* wordCounts;
    
public:
    WordCounter() {
        wordCounts = new std::map<std::string, int>();
    }
    
    ~WordCounter() {
        delete wordCounts;
    }
    
    void addWord(const std::string& word) {
        std::map<std::string, int>::iterator it = wordCounts->find(word);
        if (it != wordCounts->end()) {
            it->second++;
        } else {
            wordCounts->insert(std::make_pair(word, 1));
        }
    }
    
    void printFrequentWords(int threshold) {
        std::map<std::string, int>::iterator it = wordCounts->begin();
        while (it != wordCounts->end()) {
            if (it->second >= threshold) {
                std::cout << it->first << ": " << it->second << std::endl;
            }
            ++it;
        }
    }
};
```

### Требования к решению
1. ✅ Используйте smart pointers вместо raw pointers
2. ✅ Примените range-based for loops
3. ✅ Используйте auto для вывода типов
4. ✅ Примените structured bindings где возможно
5. ✅ Добавьте const-correctness
6. ✅ Используйте std::optional для обработки ошибок

### Ожидаемый результат
```cpp
// Современная версия с использованием C++17
class ModernWordCounter {
    // Ваша реализация здесь
};
```

---

## 📋 Задание 2: Функциональное программирование

### Описание
Создайте набор функций для обработки коллекций чисел, используя функциональный подход:

### Требования
1. **Map функция**: Применяет функцию к каждому элементу
2. **Filter функция**: Фильтрует элементы по условию
3. **Reduce функция**: Сворачивает коллекцию в одно значение
4. **Compose функция**: Композирует две функции

### Шаблон для начала
```cpp
#include <vector>
#include <functional>
#include <algorithm>

template<typename T, typename F>
auto map(const std::vector<T>& vec, F func) {
    // Ваша реализация здесь
}

template<typename T, typename P>
auto filter(const std::vector<T>& vec, P predicate) {
    // Ваша реализация здесь
}

template<typename T, typename F, typename R>
R reduce(const std::vector<T>& vec, R initial, F func) {
    // Ваша реализация здесь
}

template<typename F, typename G>
auto compose(F f, G g) {
    // Ваша реализация здесь
}
```

### Тестовый случай
```cpp
int main() {
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    // 1. Удвоить все числа
    auto doubled = map(numbers, [](int x) { return x * 2; });
    
    // 2. Найти четные числа
    auto evens = filter(numbers, [](int x) { return x % 2 == 0; });
    
    // 3. Вычислить сумму
    int sum = reduce(numbers, 0, [](int acc, int x) { return acc + x; });
    
    // 4. Композиция: квадрат + 1
    auto square = [](int x) { return x * x; };
    auto addOne = [](int x) { return x + 1; };
    auto squareThenAddOne = compose(addOne, square);
    
    std::cout << "5^2 + 1 = " << squareThenAddOne(5) << std::endl;
    
    return 0;
}
```

---

## 📋 Задание 3: Type-Safe Configuration

### Описание
Создайте систему конфигурации, которая может безопасно хранить значения разных типов:

### Требования
1. Используйте `std::variant` для хранения значений разных типов
2. Создайте класс `Configuration` с методами:
   - `setValue(key, value)` - установка значения
   - `getValue<T>(key)` - получение значения с проверкой типа
   - `hasKey(key)` - проверка существования ключа
3. Поддержите типы: `int`, `double`, `std::string`, `bool`

### Интерфейс
```cpp
class Configuration {
public:
    template<typename T>
    void setValue(const std::string& key, T value);
    
    template<typename T>
    std::optional<T> getValue(const std::string& key) const;
    
    bool hasKey(const std::string& key) const;
    
    void printAll() const;
};
```

### Тестовый случай
```cpp
int main() {
    Configuration config;
    
    config.setValue("server.port", 8080);
    config.setValue("server.host", std::string("localhost"));
    config.setValue("debug.enabled", true);
    config.setValue("timeout", 30.5);
    
    // Безопасное получение значений
    if (auto port = config.getValue<int>("server.port")) {
        std::cout << "Port: " << *port << std::endl;
    }
    
    if (auto host = config.getValue<std::string>("server.host")) {
        std::cout << "Host: " << *host << std::endl;
    }
    
    config.printAll();
    
    return 0;
}
```

---

## 📋 Задание 4: Smart Pointer Management

### Описание
Создайте систему управления ресурсами, демонстрирующую различные типы владения:

### Требования
1. **Resource класс**: Представляет некоторый ресурс (файл, сетевое соединение, etc.)
2. **ResourceManager класс**: Управляет коллекцией ресурсов
3. Продемонстрируйте:
   - `std::unique_ptr` для единоличного владения
   - `std::shared_ptr` для разделяемого владения
   - `std::weak_ptr` для слабых ссылок

### Шаблон Resource
```cpp
class Resource {
private:
    std::string name_;
    bool isOpen_;
    
public:
    explicit Resource(const std::string& name);
    ~Resource();
    
    void open();
    void close();
    bool isOpen() const;
    std::string getName() const;
    
    // Запрещаем копирование (для демонстрации unique_ptr)
    Resource(const Resource&) = delete;
    Resource& operator=(const Resource&) = delete;
    
    // Разрешаем перемещение
    Resource(Resource&& other) noexcept;
    Resource& operator=(Resource&& other) noexcept;
};
```

### Тестовый сценарий
```cpp
int main() {
    ResourceManager manager;
    
    // Добавляем ресурсы с разными типами владения
    manager.addUniqueResource("database_connection");
    manager.addSharedResource("cache");
    manager.addSharedResource("logger");
    
    // Демонстрируем слабые ссылки
    manager.demonstrateWeakReferences();
    
    // Показываем автоматическое управление памятью
    manager.printResourceStatus();
    
    return 0;
}
```

---

## 📋 Задание 5: Constexpr вычисления

### Описание
Создайте набор constexpr функций для вычислений на этапе компиляции:

### Требования
1. **Математические функции**: факториал, числа Фибоначчи, простые числа
2. **Строковые функции**: длина строки, поиск подстроки
3. **Алгоритмы**: сортировка, поиск максимума/минимума

### Функции для реализации
```cpp
// Математические функции
constexpr int factorial(int n);
constexpr int fibonacci(int n);
constexpr bool isPrime(int n);
constexpr int gcd(int a, int b);

// Строковые функции
constexpr int stringLength(const char* str);
constexpr const char* findSubstring(const char* str, const char* substr);

// Алгоритмы
template<typename T, size_t N>
constexpr T findMax(const T (&arr)[N]);

template<typename T, size_t N>
constexpr void bubbleSort(T (&arr)[N]);
```

### Тестовый случай
```cpp
int main() {
    // Проверяем, что вычисления происходят на этапе компиляции
    static_assert(factorial(5) == 120);
    static_assert(fibonacci(10) == 55);
    static_assert(isPrime(17));
    
    // Используем в качестве размера массива
    int array[fibonacci(10)]; // Размер массива вычислен на этапе компиляции
    
    std::cout << "Все вычисления выполнены на этапе компиляции!" << std::endl;
    
    return 0;
}
```

---

## 🎯 Критерии оценки

### Отлично (5 баллов)
- ✅ Все требования выполнены
- ✅ Код следует современным стандартам C++17
- ✅ Используются все рекомендованные возможности
- ✅ Код читаемый и хорошо документирован
- ✅ Нет утечек памяти и undefined behavior

### Хорошо (4 балла)
- ✅ Большинство требований выполнено
- ✅ Код в целом современный
- ✅ Есть небольшие недочеты в использовании возможностей
- ✅ Код работает корректно

### Удовлетворительно (3 балла)
- ✅ Основные требования выполнены
- ✅ Код компилируется и работает
- ❌ Устаревшие подходы в некоторых местах
- ❌ Не все современные возможности использованы

### Неудовлетворительно (2 балла)
- ❌ Код не компилируется или работает неправильно
- ❌ Использованы устаревшие подходы
- ❌ Не выполнены основные требования

---

## 💡 Подсказки

1. **Начните с простого**: Сначала сделайте код работающим, затем оптимизируйте
2. **Используйте компилятор**: Включайте все предупреждения (`-Wall -Wextra`)
3. **Тестируйте**: Создавайте простые тесты для проверки функциональности
4. **Изучайте документацию**: cppreference.com - ваш лучший друг
5. **Экспериментируйте**: Пробуйте разные подходы и сравнивайте результаты

---

*Удачи в выполнении упражнений! Помните: цель не просто написать код, а понять принципы современного C++.*
