#include <iostream>
#include <thread>
#include <mutex>
#include <memory>
#include <vector>
#include <chrono>
#include <cstring>

// ============================================================================
// УЯЗВИМЫЕ РЕАЛИЗАЦИИ BUILDER ДЛЯ АНАЛИЗА БЕЗОПАСНОСТИ
// ============================================================================

// TODO: Добавить комментарии на русском языке
// TODO: Создать эксплоиты для каждой уязвимости
// TODO: Добавить инструменты анализа

// ----------------------------------------------------------------------------
// УЯЗВИМОСТЬ 1: Buffer Overflow при построении объектов
// ----------------------------------------------------------------------------
class VulnerableBuilder {
private:
    char* buffer;
    size_t bufferSize;
    size_t currentSize;
    
public:
    VulnerableBuilder() : buffer(nullptr), bufferSize(0), currentSize(0) {
        std::cout << "VulnerableBuilder создан" << std::endl;
    }
    
    // УЯЗВИМОСТЬ: Buffer overflow при добавлении данных
    void addData(const char* data, size_t length) {
        // УЯЗВИМОСТЬ: Нет проверки переполнения
        if (currentSize + length > bufferSize) {
            // УЯЗВИМОСТЬ: Небезопасное перевыделение памяти
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
    
    ~VulnerableBuilder() {
        delete[] buffer;
        std::cout << "VulnerableBuilder уничтожен" << std::endl;
    }
};

// ----------------------------------------------------------------------------
// УЯЗВИМОСТЬ 2: Memory Leak при исключениях
// ----------------------------------------------------------------------------
class LeakyBuilder {
private:
    std::vector<void*> allocatedBlocks;
    char* currentBuffer;
    
public:
    LeakyBuilder() : currentBuffer(nullptr) {
        std::cout << "LeakyBuilder создан" << std::endl;
    }
    
    // УЯЗВИМОСТЬ: Memory leak при исключении
    void buildStep(const char* stepName, size_t dataSize) {
        // УЯЗВИМОСТЬ: Если здесь произойдет исключение,
        // память не будет освобождена
        currentBuffer = new char[dataSize];
        allocatedBlocks.push_back(currentBuffer);
        
        // УЯЗВИМОСТЬ: Исключение может произойти здесь
        if (dataSize > 1000000) {
            throw std::runtime_error("Слишком большой шаг построения");
        }
        
        // Заполняем буфер данными
        memset(currentBuffer, 'A', dataSize);
    }
    
    ~LeakyBuilder() {
        // УЯЗВИМОСТЬ: Может быть вызван дважды
        for (void* block : allocatedBlocks) {
            delete[] static_cast<char*>(block);
        }
        std::cout << "LeakyBuilder уничтожен" << std::endl;
    }
};

// ----------------------------------------------------------------------------
// УЯЗВИМОСТЬ 3: Use-After-Free при неправильном управлении состоянием
// ----------------------------------------------------------------------------
class UseAfterFreeBuilder {
private:
    char* data;
    bool isBuilt;
    
public:
    UseAfterFreeBuilder() : data(nullptr), isBuilt(false) {
        std::cout << "UseAfterFreeBuilder создан" << std::endl;
    }
    
    void build() {
        if (!isBuilt) {
            data = new char[1024];
            strcpy(data, "Built object data");
            isBuilt = true;
        }
    }
    
    // УЯЗВИМОСТЬ: Использование после освобождения
    void reset() {
        delete[] data;
        data = nullptr;
        isBuilt = false;
        // УЯЗВИМОСТЬ: Не обнуляем указатель
    }
    
    // УЯЗВИМОСТЬ: Возврат указателя на освобожденную память
    char* getData() {
        return data; // Может быть nullptr или dangling pointer
    }
    
    ~UseAfterFreeBuilder() {
        delete[] data;
        std::cout << "UseAfterFreeBuilder уничтожен" << std::endl;
    }
};

// ----------------------------------------------------------------------------
// УЯЗВИМОСТЬ 4: Integer Overflow при вычислении размеров
// ----------------------------------------------------------------------------
class IntegerOverflowBuilder {
private:
    size_t totalSize;
    std::vector<size_t> componentSizes;
    
public:
    IntegerOverflowBuilder() : totalSize(0) {
        std::cout << "IntegerOverflowBuilder создан" << std::endl;
    }
    
    // УЯЗВИМОСТЬ: Integer overflow при добавлении компонентов
    void addComponent(size_t size) {
        componentSizes.push_back(size);
        
        // УЯЗВИМОСТЬ: Может произойти переполнение
        totalSize += size;
        
        // УЯЗВИМОСТЬ: Проверка может быть обойдена при overflow
        if (totalSize < SIZE_MAX) {
            std::cout << "Добавлен компонент размером " << size << std::endl;
        }
    }
    
    // УЯЗВИМОСТЬ: Небезопасное создание объекта
    void* buildObject() {
        // УЯЗВИМОСТЬ: При overflow может быть выделена неправильная память
        return new char[totalSize];
    }
    
    size_t getTotalSize() const {
        return totalSize;
    }
    
    ~IntegerOverflowBuilder() {
        std::cout << "IntegerOverflowBuilder уничтожен" << std::endl;
    }
};

// ----------------------------------------------------------------------------
// ДЕМОНСТРАЦИЯ УЯЗВИМОСТЕЙ
// ----------------------------------------------------------------------------

void demonstrateBufferOverflow() {
    std::cout << "\n=== Демонстрация Buffer Overflow ===" << std::endl;
    
    VulnerableBuilder builder;
    
    // Нормальное использование
    builder.addData("Hello", 5);
    std::cout << "Добавлены данные, размер: " << builder.getSize() << std::endl;
    
    // УЯЗВИМОСТЬ: Попытка добавить очень большие данные
    std::string largeData(1000000, 'A');
    builder.addData(largeData.c_str(), largeData.length());
    std::cout << "Добавлены большие данные, размер: " << builder.getSize() << std::endl;
}

void demonstrateMemoryLeak() {
    std::cout << "\n=== Демонстрация Memory Leak ===" << std::endl;
    
    LeakyBuilder builder;
    
    try {
        // Нормальное использование
        builder.buildStep("step1", 100);
        std::cout << "Выполнен шаг 1" << std::endl;
        
        // УЯЗВИМОСТЬ: Исключение приведет к утечке памяти
        builder.buildStep("step2", 2000000);
    } catch (const std::exception& e) {
        std::cout << "Исключение: " << e.what() << std::endl;
        std::cout << "Память может быть утеряна!" << std::endl;
    }
}

void demonstrateUseAfterFree() {
    std::cout << "\n=== Демонстрация Use-After-Free ===" << std::endl;
    
    UseAfterFreeBuilder builder;
    
    // Строим объект
    builder.build();
    std::cout << "Объект построен" << std::endl;
    
    // Сбрасываем состояние
    builder.reset();
    std::cout << "Состояние сброшено" << std::endl;
    
    // УЯЗВИМОСТЬ: Попытка использовать после сброса
    char* data = builder.getData();
    if (data) {
        std::cout << "Данные: " << data << std::endl;
    } else {
        std::cout << "Данные недоступны (правильно)" << std::endl;
    }
}

void demonstrateIntegerOverflow() {
    std::cout << "\n=== Демонстрация Integer Overflow ===" << std::endl;
    
    IntegerOverflowBuilder builder;
    
    // Нормальное использование
    builder.addComponent(100);
    builder.addComponent(200);
    std::cout << "Общий размер: " << builder.getTotalSize() << std::endl;
    
    // УЯЗВИМОСТЬ: Integer overflow
    size_t largeSize = SIZE_MAX - 100;
    builder.addComponent(largeSize);
    std::cout << "Общий размер после overflow: " << builder.getTotalSize() << std::endl;
}

// ----------------------------------------------------------------------------
// ИНСТРУМЕНТЫ АНАЛИЗА
// ----------------------------------------------------------------------------

void runStaticAnalysis() {
    std::cout << "\n=== Инструкции для статического анализа ===" << std::endl;
    std::cout << "1. Clang Static Analyzer:" << std::endl;
    std::cout << "   clang --analyze builder_vulnerabilities.cpp" << std::endl;
    std::cout << "2. Cppcheck:" << std::endl;
    std::cout << "   cppcheck --enable=all builder_vulnerabilities.cpp" << std::endl;
    std::cout << "3. PVS-Studio:" << std::endl;
    std::cout << "   pvs-studio-analyzer trace -- make" << std::endl;
}

void runDynamicAnalysis() {
    std::cout << "\n=== Инструкции для динамического анализа ===" << std::endl;
    std::cout << "1. AddressSanitizer:" << std::endl;
    std::cout << "   g++ -fsanitize=address -g builder_vulnerabilities.cpp -o builder_asan" << std::endl;
    std::cout << "   ./builder_asan" << std::endl;
    std::cout << "2. MemorySanitizer:" << std::endl;
    std::cout << "   g++ -fsanitize=memory -g builder_vulnerabilities.cpp -o builder_msan" << std::endl;
    std::cout << "   ./builder_msan" << std::endl;
    std::cout << "3. Valgrind:" << std::endl;
    std::cout << "   valgrind --tool=memcheck ./builder_vulnerabilities" << std::endl;
}

// ----------------------------------------------------------------------------
// ОСНОВНАЯ ФУНКЦИЯ
// ----------------------------------------------------------------------------

int main() {
    std::cout << "=== АНАЛИЗ УЯЗВИМОСТЕЙ В ПАТТЕРНЕ BUILDER ===" << std::endl;
    
    // Демонстрация различных уязвимостей
    demonstrateBufferOverflow();
    demonstrateMemoryLeak();
    demonstrateUseAfterFree();
    demonstrateIntegerOverflow();
    
    // Инструкции по анализу
    runStaticAnalysis();
    runDynamicAnalysis();
    
    std::cout << "\n=== ВНИМАНИЕ: Этот код содержит уязвимости! ===" << std::endl;
    std::cout << "Используйте только для обучения и анализа безопасности." << std::endl;
    
    return 0;
}