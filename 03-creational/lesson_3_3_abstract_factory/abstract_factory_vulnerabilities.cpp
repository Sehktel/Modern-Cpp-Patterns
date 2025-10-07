#include <iostream>
#include <thread>
#include <mutex>
#include <memory>
#include <vector>
#include <chrono>
#include <cstring>

// ============================================================================
// УЯЗВИМЫЕ РЕАЛИЗАЦИИ ABSTRACT FACTORY ДЛЯ АНАЛИЗА БЕЗОПАСНОСТИ
// ============================================================================
//
// Этот файл демонстрирует типичные уязвимости в паттерне Abstract Factory:
// 1. Memory Leaks при создании семейств объектов
// 2. Use-After-Free при уничтожении связанных объектов
// 3. Type Confusion между семействами продуктов
// 4. Lifetime management issues для связанных объектов
//
// ИНСТРУМЕНТЫ ДЛЯ АНАЛИЗА:
// - AddressSanitizer: g++ -fsanitize=address -g abstract_factory_vulnerabilities.cpp
// - Valgrind: valgrind --tool=memcheck --leak-check=full ./abstract_factory
//
// ⚠️ ВНИМАНИЕ: Этот код содержит НАМЕРЕННЫЕ уязвимости для обучения!
//              НЕ используйте в production!
// ============================================================================

// ----------------------------------------------------------------------------
// УЯЗВИМОСТЬ 1: Type Confusion при создании объектов
// ----------------------------------------------------------------------------
class VulnerableAbstractFactory {
private:
    std::vector<void*> createdObjects;
    
public:
    VulnerableAbstractFactory() {
        std::cout << "VulnerableAbstractFactory создан" << std::endl;
    }
    
    // УЯЗВИМОСТЬ: Небезопасное создание объектов
    template<typename T>
    T* createObject(size_t size) {
        // УЯЗВИМОСТЬ: Небезопасное приведение типа
        void* obj = new char[size];
        createdObjects.push_back(obj);
        
        // УЯЗВИМОСТЬ: Небезопасное приведение к типу T
        return static_cast<T*>(obj);
    }
    
    // УЯЗВИМОСТЬ: Возврат объекта неправильного типа
    void* getObject(size_t index) {
        if (index < createdObjects.size()) {
            return createdObjects[index];
        }
        return nullptr;
    }
    
    size_t getObjectCount() const {
        return createdObjects.size();
    }
    
    ~VulnerableAbstractFactory() {
        for (void* obj : createdObjects) {
            delete[] static_cast<char*>(obj);
        }
        std::cout << "VulnerableAbstractFactory уничтожен" << std::endl;
    }
};

// ----------------------------------------------------------------------------
// УЯЗВИМОСТЬ 2: Memory Leak при исключениях
// ----------------------------------------------------------------------------
class LeakyAbstractFactory {
private:
    std::vector<void*> objects;
    std::vector<size_t> sizes;
    
public:
    LeakyAbstractFactory() {
        std::cout << "LeakyAbstractFactory создан" << std::endl;
    }
    
    // УЯЗВИМОСТЬ: Memory leak при исключении
    void* createObject(size_t size, const char* type) {
        // УЯЗВИМОСТЬ: Если здесь произойдет исключение,
        // память не будет освобождена
        void* obj = new char[size];
        objects.push_back(obj);
        sizes.push_back(size);
        
        // УЯЗВИМОСТЬ: Исключение может произойти здесь
        if (size > 1000000) {
            throw std::runtime_error("Слишком большой объект");
        }
        
        // Инициализация объекта
        memset(obj, 0, size);
        
        return obj;
    }
    
    ~LeakyAbstractFactory() {
        // УЯЗВИМОСТЬ: Может быть вызван дважды
        for (size_t i = 0; i < objects.size(); ++i) {
            delete[] static_cast<char*>(objects[i]);
        }
        std::cout << "LeakyAbstractFactory уничтожен" << std::endl;
    }
};

// ----------------------------------------------------------------------------
// УЯЗВИМОСТЬ 3: Buffer Overflow при создании массивов
// ----------------------------------------------------------------------------
class BufferOverflowAbstractFactory {
private:
    char* buffer;
    size_t bufferSize;
    size_t currentSize;
    
public:
    BufferOverflowAbstractFactory() : buffer(nullptr), bufferSize(0), currentSize(0) {
        std::cout << "BufferOverflowAbstractFactory создан" << std::endl;
    }
    
    // УЯЗВИМОСТЬ: Buffer overflow при создании объектов
    void* createObjectArray(size_t count, size_t elementSize) {
        // УЯЗВИМОСТЬ: Может произойти integer overflow
        size_t totalSize = count * elementSize;
        
        // УЯЗВИМОСТЬ: Нет проверки переполнения буфера
        if (currentSize + totalSize > bufferSize) {
            bufferSize = currentSize + totalSize;
            char* newBuffer = new char[bufferSize];
            if (buffer) {
                memcpy(newBuffer, buffer, currentSize);
                delete[] buffer;
            }
            buffer = newBuffer;
        }
        
        // УЯЗВИМОСТЬ: Может произойти переполнение
        void* obj = buffer + currentSize;
        currentSize += totalSize;
        
        return obj;
    }
    
    char* getBuffer() {
        return buffer;
    }
    
    size_t getBufferSize() const {
        return bufferSize;
    }
    
    ~BufferOverflowAbstractFactory() {
        delete[] buffer;
        std::cout << "BufferOverflowAbstractFactory уничтожен" << std::endl;
    }
};

// ----------------------------------------------------------------------------
// УЯЗВИМОСТЬ 4: Use-After-Free при неправильном управлении объектами
// ----------------------------------------------------------------------------
class UseAfterFreeAbstractFactory {
private:
    std::vector<void*> objects;
    std::vector<bool> valid;
    
public:
    UseAfterFreeAbstractFactory() {
        std::cout << "UseAfterFreeAbstractFactory создан" << std::endl;
    }
    
    void* createObject(size_t size) {
        void* obj = new char[size];
        objects.push_back(obj);
        valid.push_back(true);
        
        return obj;
    }
    
    // УЯЗВИМОСТЬ: Use-after-free при удалении объекта
    void destroyObject(size_t index) {
        if (index < objects.size() && valid[index]) {
            delete[] static_cast<char*>(objects[index]);
            valid[index] = false;
            // УЯЗВИМОСТЬ: Не обнуляем указатель
        }
    }
    
    // УЯЗВИМОСТЬ: Возврат указателя на освобожденную память
    void* getObject(size_t index) {
        if (index < objects.size()) {
            return objects[index]; // Может быть dangling pointer
        }
        return nullptr;
    }
    
    bool isObjectValid(size_t index) const {
        if (index < valid.size()) {
            return valid[index];
        }
        return false;
    }
    
    ~UseAfterFreeAbstractFactory() {
        for (size_t i = 0; i < objects.size(); ++i) {
            if (valid[i]) {
                delete[] static_cast<char*>(objects[i]);
            }
        }
        std::cout << "UseAfterFreeAbstractFactory уничтожен" << std::endl;
    }
};

// ----------------------------------------------------------------------------
// ДЕМОНСТРАЦИЯ УЯЗВИМОСТЕЙ
// ----------------------------------------------------------------------------

void demonstrateTypeConfusion() {
    std::cout << "\n=== Демонстрация Type Confusion ===" << std::endl;
    
    VulnerableAbstractFactory factory;
    
    // Создаем объект одного типа
    int* intObj = factory.createObject<int>(sizeof(int));
    *intObj = 42;
    
    // УЯЗВИМОСТЬ: Получаем объект как другой тип
    char* charObj = static_cast<char*>(factory.getObject(0));
    
    std::cout << "Int значение: " << *intObj << std::endl;
    std::cout << "Char значение: " << *charObj << std::endl;
    std::cout << "Количество объектов: " << factory.getObjectCount() << std::endl;
}

void demonstrateMemoryLeak() {
    std::cout << "\n=== Демонстрация Memory Leak ===" << std::endl;
    
    LeakyAbstractFactory factory;
    
    try {
        // Нормальное использование
        void* obj1 = factory.createObject(100, "small");
        std::cout << "Создан объект размером 100 байт" << std::endl;
        
        // УЯЗВИМОСТЬ: Исключение приведет к утечке памяти
        void* obj2 = factory.createObject(2000000, "large");
    } catch (const std::exception& e) {
        std::cout << "Исключение: " << e.what() << std::endl;
        std::cout << "Память может быть утеряна!" << std::endl;
    }
}

void demonstrateBufferOverflow() {
    std::cout << "\n=== Демонстрация Buffer Overflow ===" << std::endl;
    
    BufferOverflowAbstractFactory factory;
    
    // Нормальное использование
    void* obj1 = factory.createObjectArray(10, 100);
    std::cout << "Создан массив: 10 * 100 = 1000 байт" << std::endl;
    
    // УЯЗВИМОСТЬ: Попытка создать очень большой массив
    void* obj2 = factory.createObjectArray(1000000, 1000);
    std::cout << "Попытка создать большой массив" << std::endl;
    
    std::cout << "Размер буфера: " << factory.getBufferSize() << std::endl;
}

void demonstrateUseAfterFree() {
    std::cout << "\n=== Демонстрация Use-After-Free ===" << std::endl;
    
    UseAfterFreeAbstractFactory factory;
    
    // Создаем объект
    void* obj = factory.createObject(1024);
    std::cout << "Объект создан" << std::endl;
    
    // Уничтожаем объект
    factory.destroyObject(0);
    std::cout << "Объект уничтожен" << std::endl;
    
    // УЯЗВИМОСТЬ: Попытка использовать после уничтожения
    void* danglingObj = factory.getObject(0);
    if (danglingObj) {
        std::cout << "Объект все еще доступен (dangling pointer)" << std::endl;
        std::cout << "Валидность: " << (factory.isObjectValid(0) ? "Да" : "Нет") << std::endl;
    }
}

// ----------------------------------------------------------------------------
// ИНСТРУМЕНТЫ АНАЛИЗА
// ----------------------------------------------------------------------------

void runStaticAnalysis() {
    std::cout << "\n=== Инструкции для статического анализа ===" << std::endl;
    std::cout << "1. Clang Static Analyzer:" << std::endl;
    std::cout << "   clang --analyze abstract_factory_vulnerabilities.cpp" << std::endl;
    std::cout << "2. Cppcheck:" << std::endl;
    std::cout << "   cppcheck --enable=all abstract_factory_vulnerabilities.cpp" << std::endl;
    std::cout << "3. PVS-Studio:" << std::endl;
    std::cout << "   pvs-studio-analyzer trace -- make" << std::endl;
}

void runDynamicAnalysis() {
    std::cout << "\n=== Инструкции для динамического анализа ===" << std::endl;
    std::cout << "1. AddressSanitizer:" << std::endl;
    std::cout << "   g++ -fsanitize=address -g abstract_factory_vulnerabilities.cpp -o abstract_factory_asan" << std::endl;
    std::cout << "   ./abstract_factory_asan" << std::endl;
    std::cout << "2. MemorySanitizer:" << std::endl;
    std::cout << "   g++ -fsanitize=memory -g abstract_factory_vulnerabilities.cpp -o abstract_factory_msan" << std::endl;
    std::cout << "   ./abstract_factory_msan" << std::endl;
    std::cout << "3. Valgrind:" << std::endl;
    std::cout << "   valgrind --tool=memcheck ./abstract_factory_vulnerabilities" << std::endl;
}

// ----------------------------------------------------------------------------
// ОСНОВНАЯ ФУНКЦИЯ
// ----------------------------------------------------------------------------

int main() {
    std::cout << "=== АНАЛИЗ УЯЗВИМОСТЕЙ В ПАТТЕРНЕ ABSTRACT FACTORY ===" << std::endl;
    
    // Демонстрация различных уязвимостей
    demonstrateTypeConfusion();
    demonstrateMemoryLeak();
    demonstrateBufferOverflow();
    demonstrateUseAfterFree();
    
    // Инструкции по анализу
    runStaticAnalysis();
    runDynamicAnalysis();
    
    std::cout << "\n=== ВНИМАНИЕ: Этот код содержит уязвимости! ===" << std::endl;
    std::cout << "Используйте только для обучения и анализа безопасности." << std::endl;
    
    return 0;
}