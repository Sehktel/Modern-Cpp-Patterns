/**
 * @file hello_world.cpp
 * @brief Демонстрация эволюции Hello World от классического C++ до современного
 * 
 * Этот файл показывает, как одни и те же задачи решались в разных версиях C++,
 * демонстрируя прогресс языка и его возможностей.
 * 
 * @author Sehktel
 * @license MIT License
 * @copyright Copyright (c) 2025 Sehktel
 * @version 1.0
 */

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <optional>
#include <tuple>
#include <algorithm>
#include <numeric>

// ============================================================================
// ЭВОЛЮЦИЯ HELLO WORLD
// ============================================================================

/**
 * @brief Классический Hello World (C++98 стиль)
 * 
 * Проблемы этого подхода:
 * - Явное управление памятью
 * - Вероятность утечек памяти
 * - Многословный синтаксис
 */
void helloWorldClassic() {
    std::cout << "=== Классический C++ (C++98) ===" << std::endl;
    
    // Создание и инициализация контейнера
    std::vector<std::string>* messages = new std::vector<std::string>();
    messages->push_back("Hello");
    messages->push_back("World");
    messages->push_back("from");
    messages->push_back("Classic");
    messages->push_back("C++!");
    
    // Итерация по элементам
    for (std::vector<std::string>::iterator it = messages->begin(); 
         it != messages->end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
    
    // Обязательное освобождение памяти
    delete messages;
}

/**
 * @brief Современный Hello World (C++11 стиль)
 * 
 * Улучшения:
 * - Автоматическое управление памятью (RAII)
 * - Auto для вывода типов
 * - Range-based for loops
 * - nullptr вместо NULL
 */
void helloWorldModern() {
    std::cout << "\n=== Современный C++ (C++11) ===" << std::endl;
    
    // Автоматическое управление памятью
    auto messages = std::make_unique<std::vector<std::string>>();
    
    // Добавление элементов
    messages->push_back("Hello");
    messages->push_back("World");
    messages->push_back("from");
    messages->push_back("Modern");
    messages->push_back("C++!");
    
    // Современная итерация
    for (const auto& message : *messages) {
        std::cout << message << " ";
    }
    std::cout << std::endl;
    
    // Автоматическое освобождение памяти при выходе из области видимости
}

/**
 * @brief Продвинутый Hello World (C++17 стиль)
 * 
 * Новые возможности:
 * - Structured bindings
 * - std::optional для безопасной работы с отсутствующими значениями
 * - std::string_view для эффективной работы со строками
 * - Constexpr для вычислений на этапе компиляции
 */
void helloWorldAdvanced() {
    std::cout << "\n=== Продвинутый C++ (C++17) ===" << std::endl;
    
    // Structured bindings для работы с парами и кортежами
    auto [greeting, target] = std::make_tuple("Hello", "Modern C++");
    std::cout << greeting << " " << target << "!" << std::endl;
    
    // std::optional для безопасной работы с отсутствующими значениями
    std::optional<std::string> maybeMessage = getMessageIfAvailable();
    if (maybeMessage.has_value()) {
        std::cout << "Получено сообщение: " << maybeMessage.value() << std::endl;
    } else {
        std::cout << "Сообщение недоступно" << std::endl;
    }
    
    // std::string_view для эффективной работы со строками
    printMessage(std::string_view("Привет от C++17!"));
    
    // Constexpr вычисления
    constexpr int result = calculateAtCompileTime(5, 3);
    std::cout << "Результат вычислений на этапе компиляции: " << result << std::endl;
}

// ============================================================================
// ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ
// ============================================================================

/**
 * @brief Демонстрирует использование std::optional
 * @return std::optional<std::string> Может содержать или не содержать сообщение
 */
std::optional<std::string> getMessageIfAvailable() {
    static int callCount = 0;
    ++callCount;
    
    // Возвращаем сообщение только при четных вызовах
    if (callCount % 2 == 0) {
        return "Сообщение доступно!";
    }
    return std::nullopt; // Отсутствующее значение
}

/**
 * @brief Демонстрирует использование std::string_view
 * @param message Ссылка на строку (без копирования)
 */
void printMessage(std::string_view message) {
    std::cout << "string_view сообщение: " << message << std::endl;
}

/**
 * @brief Constexpr функция для вычислений на этапе компиляции
 * @param a Первое число
 * @param b Второе число
 * @return Результат вычислений
 */
constexpr int calculateAtCompileTime(int a, int b) {
    return a * a + b * b;
}

// ============================================================================
// ДЕМОНСТРАЦИЯ СОВРЕМЕННЫХ ИДИОМ
// ============================================================================

/**
 * @brief Демонстрирует современные идиомы C++
 */
void demonstrateModernIdioms() {
    std::cout << "\n=== Современные идиомы C++ ===" << std::endl;
    
    // 1. Auto с lambda функциями
    auto add = [](int a, int b) { return a + b; };
    std::cout << "Lambda результат: " << add(5, 3) << std::endl;
    
    // 2. Range-based for с structured bindings
    std::map<std::string, int> wordCounts = {
        {"hello", 5},
        {"world", 3},
        {"modern", 2},
        {"c++", 1}
    };
    
    std::cout << "Частота слов:" << std::endl;
    for (const auto& [word, count] : wordCounts) {
        std::cout << "  " << word << ": " << count << std::endl;
    }
    
    // 3. Алгоритмы STL с современным синтаксисом
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    // Фильтрация четных чисел
    std::vector<int> evenNumbers;
    std::copy_if(numbers.begin(), numbers.end(), 
                 std::back_inserter(evenNumbers),
                 [](int n) { return n % 2 == 0; });
    
    std::cout << "Четные числа: ";
    for (const auto& num : evenNumbers) {
        std::cout << num << " ";
    }
    std::cout << std::endl;
    
    // 4. Преобразование элементов
    std::vector<int> squares;
    std::transform(numbers.begin(), numbers.end(), 
                   std::back_inserter(squares),
                   [](int n) { return n * n; });
    
    std::cout << "Квадраты чисел: ";
    for (const auto& square : squares) {
        std::cout << square << " ";
    }
    std::cout << std::endl;
}

// ============================================================================
// ОСНОВНАЯ ФУНКЦИЯ
// ============================================================================

int main() {
    std::cout << "🚀 Добро пожаловать в курс по паттернам программирования в современном C++!" << std::endl;
    std::cout << "📚 Урок 1.1: Hello World в современном C++" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    
    // Демонстрация эволюции Hello World
    helloWorldClassic();
    helloWorldModern();
    helloWorldAdvanced();
    demonstrateModernIdioms();
    
    std::cout << "\n✅ Урок завершен! Изучите код и попробуйте выполнить упражнения из README.md" << std::endl;
    
    return 0;
}
