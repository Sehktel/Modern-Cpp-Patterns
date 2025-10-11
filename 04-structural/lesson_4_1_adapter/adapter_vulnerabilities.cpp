#include <iostream>
#include <thread>
#include <mutex>
#include <memory>
#include <vector>
#include <chrono>
#include <cstring>

// ============================================================================
// УЯЗВИМЫЕ РЕАЛИЗАЦИИ ADAPTER ДЛЯ АНАЛИЗА БЕЗОПАСНОСТИ
// ============================================================================

// TODO: Добавить комментарии на русском языке
// TODO: Создать эксплоиты для каждой уязвимости
// TODO: Добавить инструменты анализа

// ----------------------------------------------------------------------------
// УЯЗВИМОСТЬ 1: Type Confusion при адаптации интерфейсов
// ----------------------------------------------------------------------------
class VulnerableAdapter {
private:
    void* adaptedObject;
    size_t objectSize;
    bool isValid;
    
public:
    VulnerableAdapter() : adaptedObject(nullptr), objectSize(0), isValid(false) {
        std::cout << "VulnerableAdapter создан" << std::endl;
    }
    
    // УЯЗВИМОСТЬ: Небезопасная адаптация объекта
    template<typename T>
    void adapt(T* obj) {
        // УЯЗВИМОСТЬ: Небезопасное приведение типа
        adaptedObject = static_cast<void*>(obj);
        objectSize = sizeof(T);
        isValid = true;
    }
    
    // УЯЗВИМОСТЬ: Небезопасное получение адаптированного объекта
    template<typename T>
    T* getAdapted() {
        if (!isValid) {
            return nullptr;
        }
        
        // УЯЗВИМОСТЬ: Небезопасное приведение к типу T
        return static_cast<T*>(adaptedObject);
    }
    
    // УЯЗВИМОСТЬ: Возврат объекта неправильного типа
    void* getRawObject() {
        return adaptedObject;
    }
    
    bool isValidObject() const {
        return isValid;
    }
    
    ~VulnerableAdapter() {
        std::cout << "VulnerableAdapter уничтожен" << std::endl;
    }
};

// ----------------------------------------------------------------------------
// УЯЗВИМОСТЬ 2: Buffer Overflow при копировании данных
// ----------------------------------------------------------------------------
class BufferOverflowAdapter {
private:
    char* buffer;
    size_t bufferSize;
    size_t currentSize;
    
public:
    BufferOverflowAdapter() : buffer(nullptr), bufferSize(0), currentSize(0) {
        std::cout << "BufferOverflowAdapter создан" << std::endl;
    }
    
    // УЯЗВИМОСТЬ: Buffer overflow при адаптации данных
    void adaptData(const char* data, size_t length) {
        // УЯЗВИМОСТЬ: Нет проверки переполнения буфера
        if (currentSize + length > bufferSize) {
            bufferSize = currentSize + length;
            char* newBuffer = new char[bufferSize];
            if (buffer) {
                memcpy(newBuffer, buffer, currentSize);
                delete[] buffer;
            }
            buffer = newBuffer;
        }
        
        // УЯЗВИМОСТЬ: Может произойти переполнение
        memcpy(buffer + currentSize, data, length);
        currentSize += length;
    }
    
    // УЯЗВИМОСТЬ: Возврат указателя на внутренние данные
    char* getBuffer() {
        return buffer;
    }
    
    size_t getSize() const {
        return currentSize;
    }
    
    ~BufferOverflowAdapter() {
        delete[] buffer;
        std::cout << "BufferOverflowAdapter уничтожен" << std::endl;
    }
};

// ----------------------------------------------------------------------------
// УЯЗВИМОСТЬ 3: Use-After-Free при неправильном управлении адаптированными объектами
// ----------------------------------------------------------------------------
class UseAfterFreeAdapter {
private:
    void* adaptedObject;
    bool isOwned;
    bool isValid;
    
public:
    UseAfterFreeAdapter() : adaptedObject(nullptr), isOwned(false), isValid(false) {
        std::cout << "UseAfterFreeAdapter создан" << std::endl;
    }
    
    // УЯЗВИМОСТЬ: Небезопасная адаптация с передачей владения
    void adaptWithOwnership(void* obj) {
        adaptedObject = obj;
        isOwned = true;
        isValid = true;
    }
    
    // УЯЗВИМОСТЬ: Небезопасная адаптация без передачи владения
    void adaptWithoutOwnership(void* obj) {
        adaptedObject = obj;
        isOwned = false;
        isValid = true;
    }
    
    // УЯЗВИМОСТЬ: Use-after-free при освобождении объекта
    void release() {
        if (isOwned && adaptedObject) {
            delete[] static_cast<char*>(adaptedObject);
            adaptedObject = nullptr;
            isOwned = false;
            isValid = false;
        }
    }
    
    // УЯЗВИМОСТЬ: Возврат указателя на освобожденную память
    void* getObject() {
        return adaptedObject; // Может быть dangling pointer
    }
    
    bool isObjectValid() const {
        return isValid;
    }
    
    ~UseAfterFreeAdapter() {
        if (isOwned && adaptedObject) {
            delete[] static_cast<char*>(adaptedObject);
        }
        std::cout << "UseAfterFreeAdapter уничтожен" << std::endl;
    }
};

// ----------------------------------------------------------------------------
// УЯЗВИМОСТЬ 4: Integer Overflow при вычислении размеров адаптации
// ----------------------------------------------------------------------------
class IntegerOverflowAdapter {
private:
    std::vector<size_t> sizes;
    size_t totalSize;
    
public:
    IntegerOverflowAdapter() : totalSize(0) {
        std::cout << "IntegerOverflowAdapter создан" << std::endl;
    }
    
    // УЯЗВИМОСТЬ: Integer overflow при добавлении размеров
    void addSize(size_t size) {
        sizes.push_back(size);
        
        // УЯЗВИМОСТЬ: Может произойти переполнение
        totalSize += size;
        
        // УЯЗВИМОСТЬ: Проверка может быть обойдена при overflow
        if (totalSize < SIZE_MAX) {
            std::cout << "Добавлен размер: " << size << std::endl;
        }
    }
    
    // УЯЗВИМОСТЬ: Небезопасное создание адаптированного объекта
    void* createAdaptedObject() {
        // УЯЗВИМОСТЬ: При overflow может быть выделена неправильная память
        return new char[totalSize];
    }
    
    size_t getTotalSize() const {
        return totalSize;
    }
    
    size_t getSizeCount() const {
        return sizes.size();
    }
    
    ~IntegerOverflowAdapter() {
        std::cout << "IntegerOverflowAdapter уничтожен" << std::endl;
    }
};

// ----------------------------------------------------------------------------
// ДЕМОНСТРАЦИЯ УЯЗВИМОСТЕЙ
// ----------------------------------------------------------------------------

void demonstrateTypeConfusion() {
    std::cout << "\n=== Демонстрация Type Confusion ===" << std::endl;
    
    VulnerableAdapter adapter;
    
    // Создаем объект одного типа
    int* intObj = new int(42);
    adapter.adapt(intObj);
    
    std::cout << "Адаптирован int объект" << std::endl;
    
    // УЯЗВИМОСТЬ: Получаем объект как другой тип
    char* charObj = adapter.getAdapted<char>();
    if (charObj) {
        std::cout << "Получен как char: " << *charObj << std::endl;
    }
    
    // УЯЗВИМОСТЬ: Получаем как указатель (type confusion)
    void** ptrObj = static_cast<void**>(adapter.getAdapted<void*>());
    if (ptrObj) {
        std::cout << "Получен как указатель: " << ptrObj << std::endl;
    }
    
    delete intObj;
}

void demonstrateBufferOverflow() {
    std::cout << "\n=== Демонстрация Buffer Overflow ===" << std::endl;
    
    BufferOverflowAdapter adapter;
    
    // Нормальное использование
    adapter.adaptData("Hello", 5);
    std::cout << "Адаптированы данные размером 5 байт" << std::endl;
    
    // УЯЗВИМОСТЬ: Попытка адаптировать очень большие данные
    std::string largeData(1000000, 'A');
    adapter.adaptData(largeData.c_str(), largeData.length());
    std::cout << "Адаптированы большие данные размером: " << adapter.getSize() << std::endl;
}

void demonstrateUseAfterFree() {
    std::cout << "\n=== Демонстрация Use-After-Free ===" << std::endl;
    
    UseAfterFreeAdapter adapter;
    
    // Создаем объект
    char* obj = new char[1024];
    strcpy(obj, "Adapted object data");
    
    // Адаптируем с передачей владения
    adapter.adaptWithOwnership(obj);
    std::cout << "Объект адаптирован с передачей владения" << std::endl;
    
    // Освобождаем объект
    adapter.release();
    std::cout << "Объект освобожден" << std::endl;
    
    // УЯЗВИМОСТЬ: Попытка использовать после освобождения
    void* danglingObj = adapter.getObject();
    if (danglingObj) {
        std::cout << "Объект все еще доступен (dangling pointer)" << std::endl;
        std::cout << "Валидность: " << (adapter.isObjectValid() ? "Да" : "Нет") << std::endl;
    }
}

void demonstrateIntegerOverflow() {
    std::cout << "\n=== Демонстрация Integer Overflow ===" << std::endl;
    
    IntegerOverflowAdapter adapter;
    
    // Нормальное использование
    adapter.addSize(100);
    adapter.addSize(200);
    std::cout << "Общий размер: " << adapter.getTotalSize() << std::endl;
    
    // УЯЗВИМОСТЬ: Integer overflow
    size_t largeSize = SIZE_MAX - 100;
    adapter.addSize(largeSize);
    std::cout << "Общий размер после overflow: " << adapter.getTotalSize() << std::endl;
    
    // Попытка создать объект
    void* obj = adapter.createAdaptedObject();
    if (obj) {
        std::cout << "Объект создан (возможно из-за overflow)" << std::endl;
        delete[] static_cast<char*>(obj);
    }
}

// ----------------------------------------------------------------------------
// ИНСТРУМЕНТЫ АНАЛИЗА
// ----------------------------------------------------------------------------

void runStaticAnalysis() {
    std::cout << "\n=== Инструкции для статического анализа ===" << std::endl;
    std::cout << "1. Clang Static Analyzer:" << std::endl;
    std::cout << "   clang --analyze adapter_vulnerabilities.cpp" << std::endl;
    std::cout << "2. Cppcheck:" << std::endl;
    std::cout << "   cppcheck --enable=all adapter_vulnerabilities.cpp" << std::endl;
    std::cout << "3. PVS-Studio:" << std::endl;
    std::cout << "   pvs-studio-analyzer trace -- make" << std::endl;
}

void runDynamicAnalysis() {
    std::cout << "\n=== Инструкции для динамического анализа ===" << std::endl;
    std::cout << "1. AddressSanitizer:" << std::endl;
    std::cout << "   g++ -fsanitize=address -g adapter_vulnerabilities.cpp -o adapter_asan" << std::endl;
    std::cout << "   ./adapter_asan" << std::endl;
    std::cout << "2. MemorySanitizer:" << std::endl;
    std::cout << "   g++ -fsanitize=memory -g adapter_vulnerabilities.cpp -o adapter_msan" << std::endl;
    std::cout << "   ./adapter_msan" << std::endl;
    std::cout << "3. Valgrind:" << std::endl;
    std::cout << "   valgrind --tool=memcheck ./adapter_vulnerabilities" << std::endl;
}

// ----------------------------------------------------------------------------
// ОСНОВНАЯ ФУНКЦИЯ
// ----------------------------------------------------------------------------

int main() {
    std::cout << "=== АНАЛИЗ УЯЗВИМОСТЕЙ В ПАТТЕРНЕ ADAPTER ===" << std::endl;
    
    // Демонстрация различных уязвимостей
    demonstrateTypeConfusion();
    demonstrateBufferOverflow();
    demonstrateUseAfterFree();
    demonstrateIntegerOverflow();
    
    // Инструкции по анализу
    runStaticAnalysis();
    runDynamicAnalysis();
    
    std::cout << "\n=== ВНИМАНИЕ: Этот код содержит уязвимости! ===" << std::endl;
    std::cout << "Используйте только для обучения и анализа безопасности." << std::endl;
    
    return 0;
}