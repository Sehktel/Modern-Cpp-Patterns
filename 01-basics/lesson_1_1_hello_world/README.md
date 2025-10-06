# Урок 1.1: Hello World в современном C++

## 🎯 Цель урока
Познакомиться с современными возможностями C++17/20 на примере простой программы "Hello World", демонстрируя эволюцию языка от классического C++ до современных стандартов.

## 📚 Что изучаем

### 1. Эволюция Hello World
- **C++98**: Классический подход
- **C++11**: Auto, nullptr, range-based for
- **C++17**: Structured bindings, optional, string_view
- **C++20**: Concepts, ranges, modules (обзор)

### 2. Современные идиомы
- **RAII**: Resource Acquisition Is Initialization
- **Smart Pointers**: Автоматическое управление памятью
- **Move Semantics**: Эффективная передача данных
- **Type Safety**: Предотвращение ошибок на этапе компиляции

## 🔍 Ключевые концепции

### Auto и Type Deduction
```cpp
// Старый способ
std::vector<std::string>::iterator it = vec.begin();

// Современный способ
auto it = vec.begin();  // Компилятор выводит тип автоматически
```

### Range-based for loops
```cpp
// Старый способ
for (size_t i = 0; i < vec.size(); ++i) {
    std::cout << vec[i] << std::endl;
}

// Современный способ
for (const auto& item : vec) {
    std::cout << item << std::endl;
}
```

### Structured Bindings (C++17)
```cpp
// Возвращаем пару значений
auto [key, value] = getKeyValuePair();

// Работаем с элементами структуры
auto [name, age, salary] = getEmployee();
```

## 🤔 Вопросы для размышления

1. **Почему auto предпочтительнее явного указания типов?**
   - Меньше дублирования кода
   - Автоматическое обновление при изменении типов
   - Предотвращение ошибок при рефакторинге

2. **Когда НЕ стоит использовать auto?**
   - Когда тип неочевиден из контекста
   - В публичных интерфейсах библиотек
   - При работе с целочисленными типами (предпочесть explicit типы)

3. **Какие проблемы решают structured bindings?**
   - Упрощение работы с std::pair и std::tuple
   - Более читаемый код при работе со структурами
   - Предотвращение ошибок с индексами

## 🛠️ Практические упражнения

### Упражнение 1: Рефакторинг legacy кода
Перепишите старый C++ код, используя современные возможности:

```cpp
// Исходный код
std::map<std::string, int> wordCount;
// ... заполнение map ...

std::map<std::string, int>::iterator it = wordCount.begin();
while (it != wordCount.end()) {
    if (it->second > 5) {
        std::cout << it->first << ": " << it->second << std::endl;
    }
    ++it;
}
```

### Упражнение 2: Использование structured bindings
Создайте функцию, возвращающую несколько значений, и используйте structured bindings для их получения.

### Упражнение 3: Optional и error handling
Продемонстрируйте использование std::optional для безопасной работы с потенциально отсутствующими значениями.

## 🎨 Альтернативные подходы

### Вместо raw pointers
```cpp
// Плохо
int* data = new int[100];
// ... использование ...
delete[] data;  // Легко забыть!

// Хорошо
auto data = std::make_unique<int[]>(100);
// Автоматическое освобождение при выходе из области видимости
```

### Вместо массивов
```cpp
// Плохо
int arr[100];
for (int i = 0; i < 100; ++i) {
    arr[i] = i * 2;
}

// Хорошо
std::vector<int> arr(100);
std::iota(arr.begin(), arr.end(), 0);
std::transform(arr.begin(), arr.end(), arr.begin(), 
               [](int x) { return x * 2; });
```

## 📈 Следующие шаги
После изучения этого урока вы будете готовы к:
- Уроку 1.2: RAII и управление ресурсами
- Пониманию принципов современного C++
- Изучению паттернов проектирования с современными возможностями языка
