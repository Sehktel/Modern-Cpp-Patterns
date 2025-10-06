#include <iostream>
#include <thread>
#include <mutex>
#include <memory>
#include <vector>
#include <chrono>
#include <cstring>

// ============================================================================
// УЯЗВИМЫЕ РЕАЛИЗАЦИИ FACTORY METHOD ДЛЯ АНАЛИЗА БЕЗОПАСНОСТИ
// ============================================================================

// TODO: Добавить комментарии на русском языке
// TODO: Создать эксплоиты для каждой уязвимости
// TODO: Добавить инструменты анализа

// ----------------------------------------------------------------------------
// УЯЗВИМОСТЬ 1: Buffer Overflow при создании объектов
// ----------------------------------------------------------------------------
class VulnerableFactory {
private:
    char* buffer;
    size_t bufferSize;
    
public:
    VulnerableFactory() : buffer(nullptr), bufferSize(0) {
        std::cout << "VulnerableFactory создан" << std::endl;
    }
    
    // УЯЗВИМОСТЬ: Buffer overflow при создании больших объектов
    void* createObject(size_t size) {
        // УЯЗВИМОСТЬ: Нет проверки размера
        buffer = new char[size];
        bufferSize = size;
        
        // УЯЗВИМОСТЬ: Может произойти переполнение
        memset(buffer, 'A', size);
        
        return buffer;
    }
    
    // УЯЗВИМОСТЬ: Возврат указателя на внутренние данные
    char* getBuffer() {
        return buffer;
    }
    
    ~VulnerableFactory() {
        delete[] buffer;
        std::cout << "VulnerableFactory уничтожен" << std::endl;
    }
};

// ----------------------------------------------------------------------------
// УЯЗВИМОСТЬ 2: Memory Leak при исключениях
// ----------------------------------------------------------------------------
class LeakyFactory {
private:
    std::vector<void*> objects;
    
public:
    LeakyFactory() {
        std::cout << "LeakyFactory создан" << std::endl;
    }
    
    // УЯЗВИМОСТЬ: Memory leak при исключении
    void* createObject(size_t size) {
        void* obj = new char[size];
        objects.push_back(obj);
        
        // УЯЗВИМОСТЬ: Если здесь произойдет исключение,
        // память не будет освобождена
        if (size > 1000000) {
            throw std::runtime_error("Слишком большой объект");
        }
        
        return obj;
    }
    
    ~LeakyFactory() {
        // УЯЗВИМОСТЬ: Может быть вызван дважды
        for (void* obj : objects) {
            delete[] static_cast<char*>(obj);
        }
        std::cout << "LeakyFactory уничтожен" << std::endl;
    }
};

// ----------------------------------------------------------------------------
// УЯЗВИМОСТЬ 3: Type Confusion
// ----------------------------------------------------------------------------
class TypeConfusionFactory {
private:
    void* lastObject;
    
public:
    TypeConfusionFactory() : lastObject(nullptr) {
        std::cout << "TypeConfusionFactory создан" << std::endl;
    }
    
    // УЯЗВИМОСТЬ: Небезопасное приведение типов
    template<typename T>
    T* createObject(size_t size) {
        lastObject = new char[size];
        
        // УЯЗВИМОСТЬ: Небезопасное приведение типа
        return static_cast<T*>(lastObject);
    }
    
    // УЯЗВИМОСТЬ: Возврат объекта неправильного типа
    void* getLastObject() {
        return lastObject;
    }
    
    ~TypeConfusionFactory() {
        delete[] static_cast<char*>(lastObject);
        std::cout << "TypeConfusionFactory уничтожен" << std::endl;
    }
};

// ----------------------------------------------------------------------------
// УЯЗВИМОСТЬ 4: Integer Overflow
// ----------------------------------------------------------------------------
class IntegerOverflowFactory {
private:
    size_t totalSize;
    
public:
    IntegerOverflowFactory() : totalSize(0) {
        std::cout << "IntegerOverflowFactory создан" << std::endl;
    }
    
    // УЯЗВИМОСТЬ: Integer overflow при вычислении размера
    void* createObject(size_t count, size_t elementSize) {
        // УЯЗВИМОСТЬ: Может произойти переполнение
        size_t totalSize = count * elementSize;
        
        // УЯЗВИМОСТЬ: Проверка может быть обойдена при overflow
        if (totalSize < 1000000) {
            return new char[totalSize];
        }
        
        return nullptr;
    }
    
    size_t getTotalSize() const {
        return totalSize;
    }
    
    ~IntegerOverflowFactory() {
        std::cout << "IntegerOverflowFactory уничтожен" << std::endl;
    }
};

// ----------------------------------------------------------------------------
// ДЕМОНСТРАЦИЯ УЯЗВИМОСТЕЙ
// ----------------------------------------------------------------------------

void demonstrateBufferOverflow() {
    std::cout << "\n=== Демонстрация Buffer Overflow ===" << std::endl;
    
    VulnerableFactory factory;
    
    // Нормальное использование
    void* obj1 = factory.createObject(100);
    std::cout << "Создан объект размером 100 байт" << std::endl;
    
    // УЯЗВИМОСТЬ: Попытка создать очень большой объект
    void* obj2 = factory.createObject(SIZE_MAX);
    std::cout << "Попытка создать объект максимального размера..." << std::endl;
}

void demonstrateMemoryLeak() {
    std::cout << "\n=== Демонстрация Memory Leak ===" << std::endl;
    
    LeakyFactory factory;
    
    try {
        // Нормальное использование
        void* obj1 = factory.createObject(100);
        std::cout << "Создан объект размером 100 байт" << std::endl;
        
        // УЯЗВИМОСТЬ: Исключение приведет к утечке памяти
        void* obj2 = factory.createObject(2000000);
    } catch (const std::exception& e) {
        std::cout << "Исключение: " << e.what() << std::endl;
        std::cout << "Память может быть утеряна!" << std::endl;
    }
}

void demonstrateTypeConfusion() {
    std::cout << "\n=== Демонстрация Type Confusion ===" << std::endl;
    
    TypeConfusionFactory factory;
    
    // Создаем объект одного типа
    int* intObj = factory.createObject<int>(sizeof(int));
    *intObj = 42;
    
    // УЯЗВИМОСТЬ: Получаем объект как другой тип
    char* charObj = static_cast<char*>(factory.getLastObject());
    
    std::cout << "Int значение: " << *intObj << std::endl;
    std::cout << "Char значение: " << *charObj << std::endl;
}

void demonstrateIntegerOverflow() {
    std::cout << "\n=== Демонстрация Integer Overflow ===" << std::endl;
    
    IntegerOverflowFactory factory;
    
    // Нормальное использование
    void* obj1 = factory.createObject(100, 10);
    std::cout << "Создан объект: 100 * 10 = 1000 байт" << std::endl;
    
    // УЯЗВИМОСТЬ: Integer overflow
    size_t largeCount = SIZE_MAX / 2;
    size_t largeSize = SIZE_MAX / 2;
    
    std::cout << "Попытка создать объект: " << largeCount << " * " << largeSize << std::endl;
    void* obj2 = factory.createObject(largeCount, largeSize);
    
    if (obj2) {
        std::cout << "Объект создан (возможно из-за overflow)!" << std::endl;
    } else {
        std::cout << "Объект не создан" << std::endl;
    }
}

// ----------------------------------------------------------------------------
// ИНСТРУМЕНТЫ АНАЛИЗА
// ----------------------------------------------------------------------------

void runStaticAnalysis() {
    std::cout << "\n=== Инструкции для статического анализа ===" << std::endl;
    std::cout << "1. Clang Static Analyzer:" << std::endl;
    std::cout << "   clang --analyze factory_method_vulnerabilities.cpp" << std::endl;
    std::cout << "2. Cppcheck:" << std::endl;
    std::cout << "   cppcheck --enable=all factory_method_vulnerabilities.cpp" << std::endl;
    std::cout << "3. PVS-Studio:" << std::endl;
    std::cout << "   pvs-studio-analyzer trace -- make" << std::endl;
}

void runDynamicAnalysis() {
    std::cout << "\n=== Инструкции для динамического анализа ===" << std::endl;
    std::cout << "1. AddressSanitizer:" << std::endl;
    std::cout << "   g++ -fsanitize=address -g factory_method_vulnerabilities.cpp -o factory_method_asan" << std::endl;
    std::cout << "   ./factory_method_asan" << std::endl;
    std::cout << "2. MemorySanitizer:" << std::endl;
    std::cout << "   g++ -fsanitize=memory -g factory_method_vulnerabilities.cpp -o factory_method_msan" << std::endl;
    std::cout << "   ./factory_method_msan" << std::endl;
    std::cout << "3. Valgrind:" << std::endl;
    std::cout << "   valgrind --tool=memcheck ./factory_method_vulnerabilities" << std::endl;
}

// ----------------------------------------------------------------------------
// ОСНОВНАЯ ФУНКЦИЯ
// ----------------------------------------------------------------------------

int main() {
    std::cout << "=== АНАЛИЗ УЯЗВИМОСТЕЙ В ПАТТЕРНЕ FACTORY METHOD ===" << std::endl;
    
    // Демонстрация различных уязвимостей
    demonstrateBufferOverflow();
    demonstrateMemoryLeak();
    demonstrateTypeConfusion();
    demonstrateIntegerOverflow();
    
    // Инструкции по анализу
    runStaticAnalysis();
    runDynamicAnalysis();
    
    std::cout << "\n=== ВНИМАНИЕ: Этот код содержит уязвимости! ===" << std::endl;
    std::cout << "Используйте только для обучения и анализа безопасности." << std::endl;
    
    return 0;
}
