#include <iostream>
#include <thread>
#include <mutex>
#include <memory>
#include <vector>
#include <chrono>
#include <cstring>

// ============================================================================
// УЯЗВИМЫЕ РЕАЛИЗАЦИИ DECORATOR ДЛЯ АНАЛИЗА БЕЗОПАСНОСТИ
// ============================================================================

// TODO: Добавить комментарии на русском языке
// TODO: Создать эксплоиты для каждой уязвимости
// TODO: Добавить инструменты анализа

// ----------------------------------------------------------------------------
// УЯЗВИМОСТЬ 1: Stack Overflow при глубокой цепочке декораторов
// ----------------------------------------------------------------------------
class VulnerableDecorator {
private:
    VulnerableDecorator* next;
    char* data;
    size_t dataSize;
    
public:
    VulnerableDecorator() : next(nullptr), data(nullptr), dataSize(0) {
        std::cout << "VulnerableDecorator создан" << std::endl;
    }
    
    // УЯЗВИМОСТЬ: Небезопасное добавление декоратора
    void addDecorator(VulnerableDecorator* decorator) {
        if (next) {
            // УЯЗВИМОСТЬ: Рекурсивный вызов может привести к stack overflow
            next->addDecorator(decorator);
        } else {
            next = decorator;
        }
    }
    
    // УЯЗВИМОСТЬ: Небезопасное добавление данных
    void setData(const char* str, size_t length) {
        dataSize = length;
        data = new char[dataSize];
        // УЯЗВИМОСТЬ: Нет проверки размера
        memcpy(data, str, length);
    }
    
    // УЯЗВИМОСТЬ: Рекурсивный вызов может привести к stack overflow
    void process() {
        std::cout << "Обработка декоратора" << std::endl;
        if (next) {
            next->process();
        }
    }
    
    // УЯЗВИМОСТЬ: Возврат указателя на внутренние данные
    char* getData() {
        return data;
    }
    
    ~VulnerableDecorator() {
        delete[] data;
        delete next;
        std::cout << "VulnerableDecorator уничтожен" << std::endl;
    }
};

// ----------------------------------------------------------------------------
// УЯЗВИМОСТЬ 2: Memory Leak при исключениях в цепочке
// ----------------------------------------------------------------------------
class LeakyDecorator {
private:
    LeakyDecorator* next;
    std::vector<void*> allocatedMemory;
    
public:
    LeakyDecorator() : next(nullptr) {
        std::cout << "LeakyDecorator создан" << std::endl;
    }
    
    // УЯЗВИМОСТЬ: Memory leak при исключении
    void addDecorator(LeakyDecorator* decorator) {
        // УЯЗВИМОСТЬ: Если здесь произойдет исключение,
        // память не будет освобождена
        next = decorator;
        
        // УЯЗВИМОСТЬ: Исключение может произойти здесь
        void* memory = new char[1024];
        allocatedMemory.push_back(memory);
        
        if (allocatedMemory.size() > 1000) {
            throw std::runtime_error("Слишком много декораторов");
        }
    }
    
    // УЯЗВИМОСТЬ: Небезопасная обработка
    void process() {
        if (next) {
            next->process();
        }
        
        // Обработка текущего декоратора
        for (void* mem : allocatedMemory) {
            // УЯЗВИМОСТЬ: Может произойти исключение
            memset(mem, 'A', 1024);
        }
    }
    
    ~LeakyDecorator() {
        // УЯЗВИМОСТЬ: Может быть вызван дважды
        for (void* mem : allocatedMemory) {
            delete[] static_cast<char*>(mem);
        }
        delete next;
        std::cout << "LeakyDecorator уничтожен" << std::endl;
    }
};

// ----------------------------------------------------------------------------
// УЯЗВИМОСТЬ 3: Use-After-Free при неправильном управлении цепочкой
// ----------------------------------------------------------------------------
class UseAfterFreeDecorator {
private:
    UseAfterFreeDecorator* next;
    char* data;
    bool isValid;
    
public:
    UseAfterFreeDecorator() : next(nullptr), data(nullptr), isValid(false) {
        std::cout << "UseAfterFreeDecorator создан" << std::endl;
    }
    
    void setData(const char* str) {
        data = new char[strlen(str) + 1];
        strcpy(data, str);
        isValid = true;
    }
    
    void addDecorator(UseAfterFreeDecorator* decorator) {
        next = decorator;
    }
    
    // УЯЗВИМОСТЬ: Use-after-free при освобождении данных
    void releaseData() {
        delete[] data;
        data = nullptr;
        isValid = false;
        // УЯЗВИМОСТЬ: Не обнуляем указатель
    }
    
    // УЯЗВИМОСТЬ: Возврат указателя на освобожденную память
    char* getData() {
        return data; // Может быть dangling pointer
    }
    
    bool isDataValid() const {
        return isValid;
    }
    
    void process() {
        if (next) {
            next->process();
        }
        
        if (isValid && data) {
            std::cout << "Данные: " << data << std::endl;
        }
    }
    
    ~UseAfterFreeDecorator() {
        delete[] data;
        delete next;
        std::cout << "UseAfterFreeDecorator уничтожен" << std::endl;
    }
};

// ----------------------------------------------------------------------------
// УЯЗВИМОСТЬ 4: Integer Overflow при подсчете декораторов
// ----------------------------------------------------------------------------
class IntegerOverflowDecorator {
private:
    IntegerOverflowDecorator* next;
    size_t decoratorCount;
    
public:
    IntegerOverflowDecorator() : next(nullptr), decoratorCount(1) {
        std::cout << "IntegerOverflowDecorator создан" << std::endl;
    }
    
    // УЯЗВИМОСТЬ: Integer overflow при добавлении декоратора
    void addDecorator(IntegerOverflowDecorator* decorator) {
        if (next) {
            next->addDecorator(decorator);
        } else {
            next = decorator;
        }
        
        // УЯЗВИМОСТЬ: Может произойти переполнение
        decoratorCount++;
        
        // УЯЗВИМОСТЬ: Проверка может быть обойдена при overflow
        if (decoratorCount < SIZE_MAX) {
            std::cout << "Количество декораторов: " << decoratorCount << std::endl;
        }
    }
    
    // УЯЗВИМОСТЬ: Небезопасная обработка с переполнением
    void process() {
        // УЯЗВИМОСТЬ: При overflow может произойти бесконечный цикл
        for (size_t i = 0; i < decoratorCount; ++i) {
            std::cout << "Обработка декоратора " << i << std::endl;
        }
        
        if (next) {
            next->process();
        }
    }
    
    size_t getDecoratorCount() const {
        return decoratorCount;
    }
    
    ~IntegerOverflowDecorator() {
        delete next;
        std::cout << "IntegerOverflowDecorator уничтожен" << std::endl;
    }
};

// ----------------------------------------------------------------------------
// ДЕМОНСТРАЦИЯ УЯЗВИМОСТЕЙ
// ----------------------------------------------------------------------------

void demonstrateStackOverflow() {
    std::cout << "\n=== Демонстрация Stack Overflow ===" << std::endl;
    
    VulnerableDecorator* root = new VulnerableDecorator();
    
    // Создаем глубокую цепочку декораторов
    for (int i = 0; i < 1000; ++i) {
        VulnerableDecorator* decorator = new VulnerableDecorator();
        decorator->setData("Test data", 9);
        root->addDecorator(decorator);
    }
    
    std::cout << "Создана цепочка из 1000 декораторов" << std::endl;
    
    // УЯЗВИМОСТЬ: Попытка обработки может привести к stack overflow
    try {
        root->process();
    } catch (...) {
        std::cout << "Произошло исключение (возможно stack overflow)" << std::endl;
    }
    
    delete root;
}

void demonstrateMemoryLeak() {
    std::cout << "\n=== Демонстрация Memory Leak ===" << std::endl;
    
    LeakyDecorator* root = new LeakyDecorator();
    
    try {
        // Создаем цепочку декораторов
        for (int i = 0; i < 100; ++i) {
            LeakyDecorator* decorator = new LeakyDecorator();
            root->addDecorator(decorator);
        }
        
        std::cout << "Создана цепочка декораторов" << std::endl;
        
        // УЯЗВИМОСТЬ: Исключение приведет к утечке памяти
        LeakyDecorator* leakyDecorator = new LeakyDecorator();
        root->addDecorator(leakyDecorator);
        
    } catch (const std::exception& e) {
        std::cout << "Исключение: " << e.what() << std::endl;
        std::cout << "Память может быть утеряна!" << std::endl;
    }
    
    delete root;
}

void demonstrateUseAfterFree() {
    std::cout << "\n=== Демонстрация Use-After-Free ===" << std::endl;
    
    UseAfterFreeDecorator* root = new UseAfterFreeDecorator();
    root->setData("Root data");
    
    UseAfterFreeDecorator* decorator = new UseAfterFreeDecorator();
    decorator->setData("Decorator data");
    
    root->addDecorator(decorator);
    
    // Освобождаем данные
    root->releaseData();
    std::cout << "Данные освобождены" << std::endl;
    
    // УЯЗВИМОСТЬ: Попытка использования после освобождения
    char* data = root->getData();
    if (data) {
        std::cout << "Данные все еще доступны (dangling pointer)" << std::endl;
        std::cout << "Валидность: " << (root->isDataValid() ? "Да" : "Нет") << std::endl;
    }
    
    delete root;
}

void demonstrateIntegerOverflow() {
    std::cout << "\n=== Демонстрация Integer Overflow ===" << std::endl;
    
    IntegerOverflowDecorator* root = new IntegerOverflowDecorator();
    
    // Создаем много декораторов
    for (int i = 0; i < 100; ++i) {
        IntegerOverflowDecorator* decorator = new IntegerOverflowDecorator();
        root->addDecorator(decorator);
    }
    
    std::cout << "Количество декораторов: " << root->getDecoratorCount() << std::endl;
    
    // УЯЗВИМОСТЬ: Integer overflow
    size_t largeCount = SIZE_MAX - 100;
    for (size_t i = 0; i < largeCount; ++i) {
        IntegerOverflowDecorator* decorator = new IntegerOverflowDecorator();
        root->addDecorator(decorator);
    }
    
    std::cout << "Количество декораторов после overflow: " << root->getDecoratorCount() << std::endl;
    
    delete root;
}

// ----------------------------------------------------------------------------
// ИНСТРУМЕНТЫ АНАЛИЗА
// ----------------------------------------------------------------------------

void runStaticAnalysis() {
    std::cout << "\n=== Инструкции для статического анализа ===" << std::endl;
    std::cout << "1. Clang Static Analyzer:" << std::endl;
    std::cout << "   clang --analyze decorator_vulnerabilities.cpp" << std::endl;
    std::cout << "2. Cppcheck:" << std::endl;
    std::cout << "   cppcheck --enable=all decorator_vulnerabilities.cpp" << std::endl;
    std::cout << "3. PVS-Studio:" << std::endl;
    std::cout << "   pvs-studio-analyzer trace -- make" << std::endl;
}

void runDynamicAnalysis() {
    std::cout << "\n=== Инструкции для динамического анализа ===" << std::endl;
    std::cout << "1. AddressSanitizer:" << std::endl;
    std::cout << "   g++ -fsanitize=address -g decorator_vulnerabilities.cpp -o decorator_asan" << std::endl;
    std::cout << "   ./decorator_asan" << std::endl;
    std::cout << "2. MemorySanitizer:" << std::endl;
    std::cout << "   g++ -fsanitize=memory -g decorator_vulnerabilities.cpp -o decorator_msan" << std::endl;
    std::cout << "   ./decorator_msan" << std::endl;
    std::cout << "3. Valgrind:" << std::endl;
    std::cout << "   valgrind --tool=memcheck ./decorator_vulnerabilities" << std::endl;
}

// ----------------------------------------------------------------------------
// ОСНОВНАЯ ФУНКЦИЯ
// ----------------------------------------------------------------------------

int main() {
    std::cout << "=== АНАЛИЗ УЯЗВИМОСТЕЙ В ПАТТЕРНЕ DECORATOR ===" << std::endl;
    
    // Демонстрация различных уязвимостей
    demonstrateStackOverflow();
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