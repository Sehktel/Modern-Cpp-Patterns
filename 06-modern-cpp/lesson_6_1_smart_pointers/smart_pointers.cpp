#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <chrono>
#include <thread>
#include <cstdio>
#include <stdexcept>

/**
 * @file smart_pointers.cpp
 * @brief Демонстрация умных указателей C++11/14/17
 * 
 * Этот файл показывает различные типы умных указателей
 * и их практическое применение в современном C++.
 */

// ============================================================================
// БАЗОВЫЕ ПРИМЕРЫ SMART POINTERS
// ============================================================================

/**
 * @brief Пример класса для демонстрации
 */
class MyClass {
private:
    std::string name_;
    int value_;
    
public:
    explicit MyClass(const std::string& name, int value = 0) 
        : name_(name), value_(value) {
        std::cout << "🏗️ Создан объект: " << name_ << " (значение: " << value_ << ")" << std::endl;
    }
    
    ~MyClass() {
        std::cout << "🗑️ Уничтожен объект: " << name_ << std::endl;
    }
    
    void setValue(int value) {
        value_ = value;
        std::cout << "📝 " << name_ << ": значение изменено на " << value_ << std::endl;
    }
    
    int getValue() const {
        return value_;
    }
    
    const std::string& getName() const {
        return name_;
    }
    
    void doSomething() {
        std::cout << "⚡ " << name_ << " выполняет действие" << std::endl;
    }
};

/**
 * @brief Демонстрация std::unique_ptr
 */
void demonstrateUniquePtr() {
    std::cout << "\n=== std::unique_ptr ===" << std::endl;
    
    // Создание unique_ptr
    std::unique_ptr<MyClass> ptr1 = std::make_unique<MyClass>("Объект 1", 42);
    std::cout << "Создан unique_ptr для: " << ptr1->getName() << std::endl;
    
    // Передача владения
    std::cout << "\nПередача владения..." << std::endl;
    std::unique_ptr<MyClass> ptr2 = std::move(ptr1);
    
    std::cout << "ptr1 после move: " << (ptr1 ? "не null" : "null") << std::endl;
    std::cout << "ptr2 владеет: " << (ptr2 ? ptr2->getName() : "ничем") << std::endl;
    
    // Использование объекта
    if (ptr2) {
        ptr2->doSomething();
        ptr2->setValue(100);
    }
    
    // Автоматическая очистка при выходе из области видимости
    std::cout << "\nВыход из области видимости..." << std::endl;
}

/**
 * @brief Демонстрация std::shared_ptr
 */
void demonstrateSharedPtr() {
    std::cout << "\n=== std::shared_ptr ===" << std::endl;
    
    // Создание shared_ptr
    std::shared_ptr<MyClass> ptr1 = std::make_shared<MyClass>("Разделяемый объект", 200);
    std::cout << "Создан shared_ptr. Счетчик ссылок: " << ptr1.use_count() << std::endl;
    
    // Разделение владения
    std::cout << "\nРазделение владения..." << std::endl;
    std::shared_ptr<MyClass> ptr2 = ptr1;
    std::shared_ptr<MyClass> ptr3 = ptr1;
    
    std::cout << "Счетчик ссылок после создания ptr2 и ptr3: " << ptr1.use_count() << std::endl;
    
    // Использование через разные указатели
    ptr1->doSomething();
    ptr2->setValue(300);
    std::cout << "Значение через ptr3: " << ptr3->getValue() << std::endl;
    
    // Сброс одной ссылки
    std::cout << "\nСброс ptr2..." << std::endl;
    ptr2.reset();
    std::cout << "Счетчик ссылок после сброса ptr2: " << ptr1.use_count() << std::endl;
    
    // Автоматическая очистка при уничтожении последней ссылки
    std::cout << "\nВыход из области видимости..." << std::endl;
}

/**
 * @brief Демонстрация std::weak_ptr
 */
void demonstrateWeakPtr() {
    std::cout << "\n=== std::weak_ptr ===" << std::endl;
    
    // Создаем shared_ptr
    std::shared_ptr<MyClass> shared = std::make_shared<MyClass>("Слабая ссылка объект", 500);
    std::cout << "Создан shared_ptr. Счетчик ссылок: " << shared.use_count() << std::endl;
    
    // Создаем weak_ptr
    std::weak_ptr<MyClass> weak = shared;
    std::cout << "Создан weak_ptr. Счетчик ссылок shared_ptr: " << shared.use_count() << std::endl;
    
    // Проверяем, жив ли объект
    if (auto locked = weak.lock()) {
        std::cout << "Объект жив! Имя: " << locked->getName() << std::endl;
        locked->doSomething();
    } else {
        std::cout << "Объект уже уничтожен" << std::endl;
    }
    
    // Сбрасываем shared_ptr
    std::cout << "\nСброс shared_ptr..." << std::endl;
    shared.reset();
    
    // Проверяем weak_ptr после сброса
    if (auto locked = weak.lock()) {
        std::cout << "Объект все еще жив: " << locked->getName() << std::endl;
    } else {
        std::cout << "Объект уничтожен, weak_ptr не может заблокировать" << std::endl;
    }
    
    std::cout << "weak_ptr истек: " << (weak.expired() ? "Да" : "Нет") << std::endl;
}

// ============================================================================
// ПОЛЬЗОВАТЕЛЬСКИЕ УДАЛИТЕЛИ
// ============================================================================

/**
 * @brief Пользовательский удалитель для файлов
 */
class FileDeleter {
private:
    std::string filename_;
    
public:
    explicit FileDeleter(const std::string& filename) : filename_(filename) {}
    
    void operator()(FILE* file) {
        if (file) {
            fclose(file);
            std::cout << "📁 Файл '" << filename_ << "' закрыт" << std::endl;
        }
    }
};

/**
 * @brief Демонстрация пользовательских удалителей
 */
void demonstrateCustomDeleters() {
    std::cout << "\n=== Пользовательские удалители ===" << std::endl;
    
    // Удалитель для файла
    {
        std::unique_ptr<FILE, FileDeleter> file(
            fopen("test.txt", "w"), 
            FileDeleter("test.txt")
        );
        
        if (file) {
            fprintf(file.get(), "Тестовые данные\n");
            std::cout << "📝 Данные записаны в файл" << std::endl;
        }
        // Файл автоматически закроется здесь
    }
    
    // Лямбда-удалитель
    {
        auto lambdaDeleter = [](MyClass* obj) {
            std::cout << "🗑️ Лямбда удаляет: " << obj->getName() << std::endl;
            delete obj;
        };
        
        std::unique_ptr<MyClass, decltype(lambdaDeleter)> ptr(
            new MyClass("Лямбда объект", 999),
            lambdaDeleter
        );
        
        ptr->doSomething();
        // Объект будет удален лямбда-функцией
    }
}

// ============================================================================
// УПРАВЛЕНИЕ МАССИВАМИ
// ============================================================================

/**
 * @brief Демонстрация умных указателей для массивов
 */
void demonstrateArrayPointers() {
    std::cout << "\n=== Умные указатели для массивов ===" << std::endl;
    
    // unique_ptr для массива
    std::unique_ptr<int[]> array = std::make_unique<int[]>(5);
    
    // Заполняем массив
    for (int i = 0; i < 5; ++i) {
        array[i] = i * i;
    }
    
    std::cout << "Массив: ";
    for (int i = 0; i < 5; ++i) {
        std::cout << array[i] << " ";
    }
    std::cout << std::endl;
    
    // shared_ptr для массива (менее распространено)
    std::shared_ptr<int[]> sharedArray(new int[3]{1, 4, 9}, std::default_delete<int[]>());
    
    std::cout << "Shared массив: ";
    for (int i = 0; i < 3; ++i) {
        std::cout << sharedArray[i] << " ";
    }
    std::cout << std::endl;
}

// ============================================================================
// enable_shared_from_this
// ============================================================================

/**
 * @brief Класс с поддержкой shared_from_this
 */
class SharedObject : public std::enable_shared_from_this<SharedObject> {
private:
    std::string name_;
    int value_;
    
public:
    explicit SharedObject(const std::string& name, int value = 0) 
        : name_(name), value_(value) {
        std::cout << "🏗️ SharedObject создан: " << name_ << std::endl;
    }
    
    ~SharedObject() {
        std::cout << "🗑️ SharedObject уничтожен: " << name_ << std::endl;
    }
    
    std::shared_ptr<SharedObject> getSharedPtr() {
        return shared_from_this();
    }
    
    void simulateAsyncOperation() {
        // Имитируем асинхронную операцию
        auto self = shared_from_this();
        std::cout << "🔄 Запуск асинхронной операции для " << name_ << std::endl;
        
        // В реальном коде здесь был бы std::async или другой механизм
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        std::cout << "✅ Асинхронная операция завершена для " << name_ << std::endl;
    }
    
    const std::string& getName() const { return name_; }
    int getValue() const { return value_; }
    void setValue(int value) { value_ = value; }
};

/**
 * @brief Демонстрация enable_shared_from_this
 */
void demonstrateSharedFromThis() {
    std::cout << "\n=== enable_shared_from_this ===" << std::endl;
    
    // Создаем объект через make_shared (обязательно!)
    auto obj = std::make_shared<SharedObject>("Асинхронный объект", 777);
    
    std::cout << "Счетчик ссылок: " << obj.use_count() << std::endl;
    
    // Получаем shared_ptr из самого объекта
    auto selfPtr = obj->getSharedPtr();
    std::cout << "Счетчик ссылок после getSharedPtr: " << obj.use_count() << std::endl;
    
    // Имитируем асинхронную операцию
    obj->simulateAsyncOperation();
    
    std::cout << "Значение объекта: " << obj->getValue() << std::endl;
}

// ============================================================================
// АЛИАСИНГ КОНСТРУКТОРЫ
// ============================================================================

/**
 * @brief Контейнер с алиасинг конструкторами
 */
class Container {
private:
    std::shared_ptr<int[]> data_;
    size_t size_;
    
public:
    explicit Container(size_t size) : size_(size) {
        data_ = std::shared_ptr<int[]>(new int[size], std::default_delete<int[]>());
        std::cout << "📦 Контейнер создан с " << size << " элементами" << std::endl;
    }
    
    // Алиасинг конструктор - возвращает shared_ptr на элемент массива
    std::shared_ptr<int> getElement(size_t index) {
        if (index < size_) {
            // Создаем shared_ptr, который разделяет владение с data_
            return std::shared_ptr<int>(data_, &data_[index]);
        }
        return nullptr;
    }
    
    void setElement(size_t index, int value) {
        if (index < size_) {
            data_[index] = value;
        }
    }
    
    size_t getSize() const { return size_; }
};

/**
 * @brief Демонстрация алиасинг конструкторов
 */
void demonstrateAliasingConstructors() {
    std::cout << "\n=== Алиасинг конструкторы ===" << std::endl;
    
    Container container(5);
    
    // Заполняем контейнер
    for (size_t i = 0; i < container.getSize(); ++i) {
        container.setElement(i, static_cast<int>(i * 10));
    }
    
    // Получаем shared_ptr на отдельные элементы
    std::vector<std::shared_ptr<int>> elementPtrs;
    for (size_t i = 0; i < container.getSize(); ++i) {
        elementPtrs.push_back(container.getElement(i));
    }
    
    std::cout << "Элементы через shared_ptr: ";
    for (const auto& ptr : elementPtrs) {
        if (ptr) {
            std::cout << *ptr << " ";
        }
    }
    std::cout << std::endl;
    
    // Изменяем значение через один из shared_ptr
    if (!elementPtrs.empty()) {
        *elementPtrs[2] = 999;
        std::cout << "Изменен элемент с индексом 2" << std::endl;
    }
    
    // Проверяем, что изменение отразилось в контейнере
    std::cout << "Элемент 2 в контейнере: " << *container.getElement(2) << std::endl;
}

// ============================================================================
// ПРОИЗВОДИТЕЛЬНОСТЬ И СРАВНЕНИЕ
// ============================================================================

/**
 * @brief Демонстрация производительности
 */
void demonstratePerformance() {
    std::cout << "\n=== Производительность ===" << std::endl;
    
    const size_t iterations = 100000;
    
    // Тест unique_ptr
    auto start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < iterations; ++i) {
        auto ptr = std::make_unique<MyClass>("Perf test " + std::to_string(i));
        ptr->setValue(static_cast<int>(i));
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto uniquePtrTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // Тест shared_ptr
    start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < iterations; ++i) {
        auto ptr = std::make_shared<MyClass>("Perf test " + std::to_string(i));
        ptr->setValue(static_cast<int>(i));
    }
    end = std::chrono::high_resolution_clock::now();
    auto sharedPtrTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "unique_ptr время: " << uniquePtrTime.count() << " мкс" << std::endl;
    std::cout << "shared_ptr время: " << sharedPtrTime.count() << " мкс" << std::endl;
    std::cout << "shared_ptr медленнее в " 
              << (double)sharedPtrTime.count() / uniquePtrTime.count() << " раз" << std::endl;
}

// ============================================================================
// ОСНОВНАЯ ФУНКЦИЯ
// ============================================================================

int main() {
    std::cout << "🧠 Демонстрация умных указателей C++" << std::endl;
    std::cout << std::string(50, '=') << std::endl;
    
    demonstrateUniquePtr();
    demonstrateSharedPtr();
    demonstrateWeakPtr();
    demonstrateCustomDeleters();
    demonstrateArrayPointers();
    demonstrateSharedFromThis();
    demonstrateAliasingConstructors();
    demonstratePerformance();
    
    std::cout << "\n✅ Демонстрация умных указателей завершена!" << std::endl;
    std::cout << "\n🎯 Ключевые выводы:" << std::endl;
    std::cout << "• unique_ptr обеспечивает единоличное владение и максимальную производительность" << std::endl;
    std::cout << "• shared_ptr позволяет разделять владение, но имеет накладные расходы" << std::endl;
    std::cout << "• weak_ptr разрывает циклические ссылки и обеспечивает безопасные ссылки" << std::endl;
    std::cout << "• Пользовательские удалители позволяют управлять любыми ресурсами" << std::endl;
    std::cout << "• enable_shared_from_this необходим для получения shared_ptr из this" << std::endl;
    std::cout << "• Алиасинг конструкторы позволяют разделять владение частями объектов" << std::endl;
    
    return 0;
}
