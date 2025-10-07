/**
 * @file cpp23_new_features_examples.cpp
 * @brief Примеры использования новых возможностей C++23 в паттернах
 * 
 * Этот файл демонстрирует практическое применение новых возможностей
 * C++23 в различных паттернах проектирования.
 */

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <thread>
#include <chrono>
#include <random>
#include <algorithm>
#include <functional>

// ============================================================================
// C++23 НОВЫЕ ЗАГОЛОВКИ И ВОЗМОЖНОСТИ
// ============================================================================

// ✅ C++23: Новые заголовки (условная компиляция для демонстрации)
#ifdef __cpp_lib_expected
#include <expected>
#endif

#ifdef __cpp_lib_print
#include <print>
#endif

#ifdef __cpp_lib_generator
#include <generator>
#endif

#ifdef __cpp_lib_flat_map
#include <flat_map>
#endif

#ifdef __cpp_lib_flat_set
#include <flat_set>
#endif

#ifdef __cpp_lib_mdspan
#include <mdspan>
#endif

#ifdef __cpp_lib_stacktrace
#include <stacktrace>
#endif

#ifdef __cpp_lib_jthread
#include <jthread>
#endif

// ============================================================================
// 1. STD::EXPECTED - УЛУЧШЕННАЯ ОБРАБОТКА ОШИБОК
// ============================================================================

/**
 * @brief Демонстрация std::expected для безопасной обработки ошибок
 */
class SafeCalculator {
public:
    // ✅ C++23: std::expected для явной обработки ошибок
#ifdef __cpp_lib_expected
    std::expected<double, std::string> divide(double a, double b) {
        if (b == 0.0) {
            return std::unexpected("Division by zero");
        }
        return a / b;
    }
    
    std::expected<double, std::string> sqrt(double value) {
        if (value < 0.0) {
            return std::unexpected("Negative value for square root");
        }
        return std::sqrt(value);
    }
    
    std::expected<double, std::string> calculate(double a, double b, const std::string& operation) {
        if (operation == "divide") {
            return divide(a, b);
        } else if (operation == "sqrt") {
            return sqrt(a);
        } else {
            return std::unexpected("Unknown operation: " + operation);
        }
    }
#else
    // Fallback для компиляторов без std::expected
    double divide(double a, double b) {
        if (b == 0.0) {
            throw std::runtime_error("Division by zero");
        }
        return a / b;
    }
#endif
};

void demonstrateExpected() {
    std::cout << "\n=== STD::EXPECTED - Обработка ошибок ===" << std::endl;
    
    SafeCalculator calc;
    
#ifdef __cpp_lib_expected
    // ✅ C++23: Безопасные вычисления
    auto result1 = calc.calculate(10, 2, "divide");
    if (result1) {
        std::cout << "✅ Результат деления: " << *result1 << std::endl;
    } else {
        std::cout << "❌ Ошибка деления: " << result1.error() << std::endl;
    }
    
    auto result2 = calc.calculate(10, 0, "divide");
    if (result2) {
        std::cout << "✅ Результат деления: " << *result2 << std::endl;
    } else {
        std::cout << "❌ Ошибка деления: " << result2.error() << std::endl;
    }
    
    auto result3 = calc.calculate(16, 0, "sqrt");
    if (result3) {
        std::cout << "✅ Результат sqrt: " << *result3 << std::endl;
    } else {
        std::cout << "❌ Ошибка sqrt: " << result3.error() << std::endl;
    }
#else
    std::cout << "std::expected не поддерживается в этом компиляторе" << std::endl;
#endif
}

// ============================================================================
// 2. STD::PRINT - УЛУЧШЕННЫЙ ВЫВОД
// ============================================================================

void demonstratePrint() {
    std::cout << "\n=== STD::PRINT - Улучшенный вывод ===" << std::endl;
    
#ifdef __cpp_lib_print
    // ✅ C++23: Красивый и эффективный вывод
    std::print("Привет, {}!\n", "мир");
    std::print("Число: {}, строка: {}\n", 42, "тест");
    std::print("Форматирование: {:.2f}, {:#x}\n", 3.14159, 255);
    
    // Вывод с выравниванием
    std::print("{:<10} {:>10} {:^10}\n", "Лево", "Право", "Центр");
    std::print("{:<10} {:>10} {:^10}\n", "A", "B", "C");
    
    // Вывод с цветами (если поддерживается терминалом)
    std::print("\033[32mЗеленый текст\033[0m\n");
    std::print("\033[31mКрасный текст\033[0m\n");
    std::print("\033[34mСиний текст\033[0m\n");
#else
    std::cout << "std::print не поддерживается в этом компиляторе" << std::endl;
    std::cout << "Используйте std::cout для вывода" << std::endl;
#endif
}

// ============================================================================
// 3. STD::GENERATOR - ЛЕНИВЫЕ ВЫЧИСЛЕНИЯ
// ============================================================================

#ifdef __cpp_lib_generator
/**
 * @brief Генератор чисел Фибоначчи
 */
std::generator<int> fibonacciGenerator(int count) {
    int a = 1, b = 1;
    co_yield a;
    co_yield b;
    
    for (int i = 2; i < count; ++i) {
        int next = a + b;
        co_yield next;
        a = b;
        b = next;
    }
}

/**
 * @brief Генератор простых чисел
 */
std::generator<int> primeGenerator(int count) {
    int num = 2;
    int found = 0;
    
    while (found < count) {
        bool isPrime = true;
        for (int i = 2; i * i <= num; ++i) {
            if (num % i == 0) {
                isPrime = false;
                break;
            }
        }
        
        if (isPrime) {
            co_yield num;
            found++;
        }
        num++;
    }
}

/**
 * @brief Генератор случайных чисел
 */
std::generator<int> randomGenerator(int count, int minVal = 1, int maxVal = 100) {
    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<int> dist(minVal, maxVal);
    
    for (int i = 0; i < count; ++i) {
        co_yield dist(gen);
    }
}
#endif

void demonstrateGenerator() {
    std::cout << "\n=== STD::GENERATOR - Ленивые вычисления ===" << std::endl;
    
#ifdef __cpp_lib_generator
    // ✅ C++23: Ленивая генерация чисел Фибоначчи
    std::cout << "Числа Фибоначчи: ";
    for (int value : fibonacciGenerator(10)) {
        std::cout << value << " ";
    }
    std::cout << std::endl;
    
    // ✅ C++23: Ленивая генерация простых чисел
    std::cout << "Простые числа: ";
    for (int value : primeGenerator(10)) {
        std::cout << value << " ";
    }
    std::cout << std::endl;
    
    // ✅ C++23: Ленивая генерация случайных чисел
    std::cout << "Случайные числа: ";
    for (int value : randomGenerator(10, 1, 50)) {
        std::cout << value << " ";
    }
    std::cout << std::endl;
#else
    std::cout << "std::generator не поддерживается в этом компиляторе" << std::endl;
#endif
}

// ============================================================================
// 4. STD::FLAT_MAP И STD::FLAT_SET - ЭФФЕКТИВНЫЕ КОНТЕЙНЕРЫ
// ============================================================================

void demonstrateFlatContainers() {
    std::cout << "\n=== STD::FLAT_MAP И STD::FLAT_SET - Эффективные контейнеры ===" << std::endl;
    
#ifdef __cpp_lib_flat_map
    // ✅ C++23: std::flat_map для эффективного хранения
    std::flat_map<int, std::string> flatMap;
    
    flatMap[1] = "один";
    flatMap[3] = "три";
    flatMap[2] = "два";
    flatMap[5] = "пять";
    flatMap[4] = "четыре";
    
    std::cout << "flat_map содержимое (автоматически отсортировано):" << std::endl;
    for (const auto& [key, value] : flatMap) {
        std::cout << "  " << key << ": " << value << std::endl;
    }
    
    // Быстрый поиск
    if (flatMap.contains(3)) {
        std::cout << "Найден ключ 3: " << flatMap[3] << std::endl;
    }
    
    // Эффективная вставка
    auto [it, inserted] = flatMap.insert({6, "шесть"});
    if (inserted) {
        std::cout << "Добавлен новый элемент: 6 -> " << flatMap[6] << std::endl;
    }
#else
    std::cout << "std::flat_map не поддерживается в этом компиляторе" << std::endl;
#endif

#ifdef __cpp_lib_flat_set
    // ✅ C++23: std::flat_set для эффективного хранения уникальных элементов
    std::flat_set<int> flatSet;
    
    flatSet.insert({5, 2, 8, 1, 9, 3, 7, 4, 6});
    
    std::cout << "flat_set содержимое (автоматически отсортировано):" << std::endl;
    for (const auto& value : flatSet) {
        std::cout << "  " << value << std::endl;
    }
    
    // Быстрый поиск
    if (flatSet.contains(5)) {
        std::cout << "Найден элемент 5" << std::endl;
    }
#else
    std::cout << "std::flat_set не поддерживается в этом компиляторе" << std::endl;
#endif
}

// ============================================================================
// 5. STD::MDSPAN - МНОГОМЕРНЫЕ ДАННЫЕ
// ============================================================================

void demonstrateMdspan() {
    std::cout << "\n=== STD::MDSPAN - Многомерные данные ===" << std::endl;
    
#ifdef __cpp_lib_mdspan
    // ✅ C++23: Эффективная работа с многомерными массивами
    constexpr size_t rows = 3;
    constexpr size_t cols = 4;
    
    std::vector<double> data(rows * cols);
    
    // Заполняем данными
    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            data[i * cols + j] = i * cols + j + 1;
        }
    }
    
    // Создаем многомерный view
    std::mdspan<double, std::extents<size_t, rows, cols>> matrix(data.data());
    
    std::cout << "Матрица " << rows << "x" << cols << ":" << std::endl;
    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            std::cout << matrix[i, j] << " ";
        }
        std::cout << std::endl;
    }
    
    // Создаем подматрицу
    auto submatrix = std::submdspan(matrix, std::make_pair(1, 3), std::make_pair(1, 3));
    std::cout << "Подматрица 2x2:" << std::endl;
    for (size_t i = 0; i < 2; ++i) {
        for (size_t j = 0; j < 2; ++j) {
            std::cout << submatrix[i, j] << " ";
        }
        std::cout << std::endl;
    }
#else
    std::cout << "std::mdspan не поддерживается в этом компиляторе" << std::endl;
#endif
}

// ============================================================================
// 6. STD::STACKTRACE - ОТЛАДКА
// ============================================================================

void demonstrateStacktrace() {
    std::cout << "\n=== STD::STACKTRACE - Отладка ===" << std::endl;
    
#ifdef __cpp_lib_stacktrace
    // ✅ C++23: Детальная информация о стеке вызовов
    auto trace = std::stacktrace::current();
    
    std::cout << "Текущий стек вызовов:" << std::endl;
    for (const auto& frame : trace) {
        std::cout << "  " << frame.description() << std::endl;
    }
    
    // Функция для демонстрации вложенных вызовов
    auto nestedFunction = []() {
        auto trace = std::stacktrace::current();
        std::cout << "Стек из вложенной функции:" << std::endl;
        for (const auto& frame : trace) {
            std::cout << "    " << frame.description() << std::endl;
        }
    };
    
    nestedFunction();
#else
    std::cout << "std::stacktrace не поддерживается в этом компиляторе" << std::endl;
#endif
}

// ============================================================================
// 7. STD::JTHREAD - УЛУЧШЕННАЯ МНОГОПОТОЧНОСТЬ
// ============================================================================

void demonstrateJthread() {
    std::cout << "\n=== STD::JTHREAD - Улучшенная многопоточность ===" << std::endl;
    
#ifdef __cpp_lib_jthread
    // ✅ C++23: std::jthread с автоматическим управлением
    std::cout << "Запуск jthread с автоматическим join..." << std::endl;
    
    std::jthread worker([](std::stop_token stopToken) {
        std::cout << "Рабочий поток запущен" << std::endl;
        
        for (int i = 0; i < 5; ++i) {
            if (stopToken.stop_requested()) {
                std::cout << "Получен сигнал остановки" << std::endl;
                break;
            }
            
            std::cout << "Работа " << i << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        
        std::cout << "Рабочий поток завершен" << std::endl;
    });
    
    // Работаем некоторое время
    std::this_thread::sleep_for(std::chrono::milliseconds(800));
    
    // Останавливаем поток
    worker.request_stop();
    
    // jthread автоматически join в деструкторе
    std::cout << "jthread будет автоматически остановлен" << std::endl;
#else
    std::cout << "std::jthread не поддерживается в этом компиляторе" << std::endl;
#endif
}

// ============================================================================
// 8. КОМБИНИРОВАННЫЙ ПРИМЕР - СОВРЕМЕННЫЙ ПАТТЕРН
// ============================================================================

#ifdef __cpp_lib_expected
#ifdef __cpp_lib_print
#ifdef __cpp_lib_generator
#ifdef __cpp_lib_flat_map

/**
 * @brief Современный паттерн с использованием всех возможностей C++23
 */
class ModernPattern {
private:
    std::flat_map<int, std::string> data_;
    std::atomic<bool> running_{false};
    
public:
    // ✅ C++23: std::expected для операций
    std::expected<void, std::string> addData(int key, const std::string& value) {
        if (key < 0) {
            return std::unexpected("Negative key not allowed");
        }
        
        data_[key] = value;
        std::print("✅ Добавлены данные: {} -> {}\n", key, value);
        return {};
    }
    
    // ✅ C++23: std::expected для получения данных
    std::expected<std::string, std::string> getData(int key) {
        auto it = data_.find(key);
        if (it == data_.end()) {
            return std::unexpected("Key not found: " + std::to_string(key));
        }
        return it->second;
    }
    
    // ✅ C++23: std::generator для итерации
    std::generator<std::pair<int, std::string>> getAllData() {
        for (const auto& [key, value] : data_) {
            co_yield {key, value};
        }
    }
    
    // ✅ C++23: std::print для вывода
    void printAllData() {
        std::print("=== Все данные ===\n");
        for (const auto& [key, value] : getAllData()) {
            std::print("  {}: {}\n", key, value);
        }
        std::print("==================\n");
    }
    
    // ✅ C++23: std::expected для сложных операций
    std::expected<double, std::string> calculateAverage() {
        if (data_.empty()) {
            return std::unexpected("No data to calculate average");
        }
        
        double sum = 0.0;
        for (const auto& [key, value] : data_) {
            sum += key;
        }
        
        return sum / data_.size();
    }
};

void demonstrateModernPattern() {
    std::cout << "\n=== СОВРЕМЕННЫЙ ПАТТЕРН - Все возможности C++23 ===" << std::endl;
    
    ModernPattern pattern;
    
    // Добавляем данные
    auto result1 = pattern.addData(1, "первый");
    if (!result1) {
        std::cout << "❌ Ошибка: " << result1.error() << std::endl;
    }
    
    auto result2 = pattern.addData(2, "второй");
    if (!result2) {
        std::cout << "❌ Ошибка: " << result2.error() << std::endl;
    }
    
    auto result3 = pattern.addData(3, "третий");
    if (!result3) {
        std::cout << "❌ Ошибка: " << result3.error() << std::endl;
    }
    
    // Получаем данные
    auto data = pattern.getData(2);
    if (data) {
        std::print("✅ Получены данные: {}\n", *data);
    } else {
        std::print("❌ Ошибка получения: {}\n", data.error());
    }
    
    // Выводим все данные
    pattern.printAllData();
    
    // Вычисляем среднее
    auto average = pattern.calculateAverage();
    if (average) {
        std::print("✅ Среднее значение ключей: {:.2f}\n", *average);
    } else {
        std::print("❌ Ошибка вычисления: {}\n", average.error());
    }
}

#endif
#endif
#endif
#endif

// ============================================================================
// ОСНОВНАЯ ФУНКЦИЯ
// ============================================================================

int main() {
    std::cout << "🚀 Примеры новых возможностей C++23 в паттернах" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    
    try {
        demonstrateExpected();
        demonstratePrint();
        demonstrateGenerator();
        demonstrateFlatContainers();
        demonstrateMdspan();
        demonstrateStacktrace();
        demonstrateJthread();
        
#ifdef __cpp_lib_expected
#ifdef __cpp_lib_print
#ifdef __cpp_lib_generator
#ifdef __cpp_lib_flat_map
        demonstrateModernPattern();
#endif
#endif
#endif
#endif
        
        std::cout << "\n✅ Все примеры завершены!" << std::endl;
        
        std::cout << "\n🎯 Ключевые возможности C++23:" << std::endl;
        std::cout << "1. std::expected - явная обработка ошибок" << std::endl;
        std::cout << "2. std::print - эффективный и красивый вывод" << std::endl;
        std::cout << "3. std::generator - ленивые вычисления" << std::endl;
        std::cout << "4. std::flat_map/set - эффективные контейнеры" << std::endl;
        std::cout << "5. std::mdspan - многомерные данные" << std::endl;
        std::cout << "6. std::stacktrace - детальная отладка" << std::endl;
        std::cout << "7. std::jthread - улучшенная многопоточность" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Ошибка: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
