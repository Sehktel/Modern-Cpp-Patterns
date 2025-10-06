#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <optional>
#include <variant>
#include <any>
#include <chrono>
#include <thread>

/**
 * @file modern_features.cpp
 * @brief Демонстрация современных возможностей C++17/20
 * 
 * Этот файл показывает продвинутые возможности современного C++,
 * которые будут использоваться в паттернах проектирования.
 */

// ============================================================================
// VARIANT И TYPE-SAFE ALTERNATIVES
// ============================================================================

/**
 * @brief Демонстрация std::variant для type-safe unions
 */
void demonstrateVariant() {
    std::cout << "\n=== std::variant - Type-Safe Unions ===" << std::endl;
    
    // variant может содержать один из нескольких типов
    std::variant<int, std::string, double> value;
    
    // Присваиваем разные типы
    value = 42;
    std::cout << "Значение: " << std::get<int>(value) << std::endl;
    
    value = std::string("Hello Variant");
    std::cout << "Значение: " << std::get<std::string>(value) << std::endl;
    
    value = 3.14;
    std::cout << "Значение: " << std::get<double>(value) << std::endl;
    
    // Безопасное получение значения с проверкой типа
    if (std::holds_alternative<std::string>(value)) {
        std::cout << "Строковое значение: " << std::get<std::string>(value) << std::endl;
    }
}

/**
 * @brief Демонстрация std::any для type erasure
 */
void demonstrateAny() {
    std::cout << "\n=== std::any - Type Erasure ===" << std::endl;
    
    std::any value;
    
    // Можем хранить любой тип
    value = 42;
    value = std::string("Hello Any");
    value = std::vector<int>{1, 2, 3, 4, 5};
    
    // Проверяем тип и получаем значение
    if (value.type() == typeid(std::vector<int>)) {
        auto vec = std::any_cast<std::vector<int>>(value);
        std::cout << "Вектор содержит " << vec.size() << " элементов" << std::endl;
    }
}

// ============================================================================
// FUNCTIONAL PROGRAMMING FEATURES
// ============================================================================

/**
 * @brief Демонстрация функционального программирования в C++
 */
void demonstrateFunctionalProgramming() {
    std::cout << "\n=== Функциональное программирование ===" << std::endl;
    
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    // 1. Map (transform)
    std::vector<int> doubled;
    std::transform(numbers.begin(), numbers.end(), 
                   std::back_inserter(doubled),
                   [](int x) { return x * 2; });
    
    std::cout << "Удвоенные числа: ";
    for (const auto& num : doubled) {
        std::cout << num << " ";
    }
    std::cout << std::endl;
    
    // 2. Filter (copy_if)
    std::vector<int> evens;
    std::copy_if(numbers.begin(), numbers.end(), 
                 std::back_inserter(evens),
                 [](int x) { return x % 2 == 0; });
    
    std::cout << "Четные числа: ";
    for (const auto& num : evens) {
        std::cout << num << " ";
    }
    std::cout << std::endl;
    
    // 3. Reduce (accumulate)
    int sum = std::accumulate(numbers.begin(), numbers.end(), 0);
    std::cout << "Сумма всех чисел: " << sum << std::endl;
    
    // 4. Compose functions
    auto compose = [](auto f, auto g) {
        return [f, g](auto x) { return f(g(x)); };
    };
    
    auto square = [](int x) { return x * x; };
    auto addOne = [](int x) { return x + 1; };
    
    auto squareThenAddOne = compose(addOne, square);
    std::cout << "5^2 + 1 = " << squareThenAddOne(5) << std::endl;
}

// ============================================================================
// SMART POINTERS AND MEMORY MANAGEMENT
// ============================================================================

/**
 * @brief Демонстрация различных типов smart pointers
 */
void demonstrateSmartPointers() {
    std::cout << "\n=== Smart Pointers ===" << std::endl;
    
    // 1. unique_ptr - единоличное владение
    auto uniqueData = std::make_unique<std::vector<int>>();
    uniqueData->push_back(1);
    uniqueData->push_back(2);
    uniqueData->push_back(3);
    
    std::cout << "unique_ptr содержит " << uniqueData->size() << " элементов" << std::endl;
    
    // 2. shared_ptr - разделяемое владение
    auto sharedData = std::make_shared<std::string>("Shared String");
    std::cout << "shared_ptr счетчик ссылок: " << sharedData.use_count() << std::endl;
    
    {
        auto anotherReference = sharedData;
        std::cout << "После создания копии счетчик: " << sharedData.use_count() << std::endl;
    } // anotherReference уничтожается здесь
    
    std::cout << "После уничтожения копии счетчик: " << sharedData.use_count() << std::endl;
    
    // 3. weak_ptr - слабая ссылка (не увеличивает счетчик)
    std::weak_ptr<std::string> weakRef = sharedData;
    std::cout << "weak_ptr не увеличивает счетчик: " << sharedData.use_count() << std::endl;
    
    // Проверяем, что объект еще существует
    if (auto locked = weakRef.lock()) {
        std::cout << "Объект через weak_ptr: " << *locked << std::endl;
    }
}

// ============================================================================
// ASYNC PROGRAMMING
// ============================================================================

/**
 * @brief Демонстрация асинхронного программирования
 */
void demonstrateAsyncProgramming() {
    std::cout << "\n=== Асинхронное программирование ===" << std::endl;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Имитация асинхронной работы
    std::cout << "Начинаем асинхронную задачу..." << std::endl;
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Задача завершена за " << duration.count() << " мс" << std::endl;
}

// ============================================================================
// TYPE TRAITS AND METAPROGRAMMING
// ============================================================================

/**
 * @brief Демонстрация type traits
 */
template<typename T>
void demonstrateTypeTraits(T value) {
    std::cout << "\n=== Type Traits для типа " << typeid(T).name() << " ===" << std::endl;
    
    std::cout << "Является целым числом: " << std::boolalpha << std::is_integral_v<T> << std::endl;
    std::cout << "Является указателем: " << std::boolalpha << std::is_pointer_v<T> << std::endl;
    std::cout << "Является ссылкой: " << std::boolalpha << std::is_reference_v<T> << std::endl;
    std::cout << "Константный: " << std::boolalpha << std::is_const_v<T> << std::endl;
}

// ============================================================================
// CONSTEXPR И COMPILE-TIME COMPUTATIONS
// ============================================================================

/**
 * @brief Constexpr функция для вычислений на этапе компиляции
 */
constexpr int fibonacci(int n) {
    if (n <= 1) return n;
    return fibonacci(n - 1) + fibonacci(n - 2);
}

/**
 * @brief Демонстрация constexpr вычислений
 */
void demonstrateConstexpr() {
    std::cout << "\n=== Constexpr вычисления ===" << std::endl;
    
    // Эти вычисления происходят на этапе компиляции
    constexpr int fib10 = fibonacci(10);
    constexpr int fib15 = fibonacci(15);
    
    std::cout << "10-е число Фибоначчи: " << fib10 << std::endl;
    std::cout << "15-е число Фибоначчи: " << fib15 << std::endl;
    
    // Можно использовать в качестве размера массива
    int array[fib10];
    std::cout << "Размер массива: " << sizeof(array) / sizeof(array[0]) << std::endl;
}

// ============================================================================
// ОСНОВНАЯ ФУНКЦИЯ ДЕМОНСТРАЦИИ
// ============================================================================

int main() {
    std::cout << "🔧 Демонстрация современных возможностей C++" << std::endl;
    std::cout << std::string(50, '=') << std::endl;
    
    demonstrateVariant();
    demonstrateAny();
    demonstrateFunctionalProgramming();
    demonstrateSmartPointers();
    demonstrateAsyncProgramming();
    demonstrateConstexpr();
    
    // Демонстрация type traits с разными типами
    demonstrateTypeTraits(42);
    demonstrateTypeTraits(std::string("Hello"));
    demonstrateTypeTraits(3.14);
    
    std::cout << "\n✅ Демонстрация завершена!" << std::endl;
    std::cout << "Эти возможности будут использоваться в паттернах проектирования." << std::endl;
    
    return 0;
}
