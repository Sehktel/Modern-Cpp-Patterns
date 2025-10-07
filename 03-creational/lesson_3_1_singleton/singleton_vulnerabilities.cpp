#include <iostream>
#include <thread>
#include <mutex>
#include <memory>
#include <vector>

// ============================================================================
// УЯЗВИМЫЕ РЕАЛИЗАЦИИ SINGLETON ДЛЯ АНАЛИЗА БЕЗОПАСНОСТИ
// ============================================================================

// TODO: Добавить комментарии на русском языке
// TODO: Создать эксплоиты для каждой уязвимости
// TODO: Добавить инструменты анализа

// ----------------------------------------------------------------------------
// УЯЗВИМОСТЬ 1: Race Condition при инициализации
// ----------------------------------------------------------------------------
class UnsafeSingleton {
private:
    static UnsafeSingleton* instance;
    int data;
    
    UnsafeSingleton() : data(42) {
        std::cout << "UnsafeSingleton создан" << std::endl;
    }
    
public:
    static UnsafeSingleton* getInstance() {
        // УЯЗВИМОСТЬ: Race condition - два потока могут одновременно
        // проверить instance == nullptr и создать два объекта
        if (instance == nullptr) {
            instance = new UnsafeSingleton();
        }
        return instance;
    }
    
    int getData() const { return data; }
    void setData(int value) { data = value; }
    
    ~UnsafeSingleton() {
        std::cout << "UnsafeSingleton уничтожен" << std::endl;
    }
};

UnsafeSingleton* UnsafeSingleton::instance = nullptr;

// ----------------------------------------------------------------------------
// УЯЗВИМОСТЬ 2: Memory Leak при исключениях
// ----------------------------------------------------------------------------
class LeakySingleton {
private:
    static LeakySingleton* instance;
    std::vector<int>* data;
    
    LeakySingleton() : data(new std::vector<int>(1000000)) {
        // УЯЗВИМОСТЬ: Если здесь произойдет исключение,
        // память не будет освобождена
        std::cout << "LeakySingleton создан" << std::endl;
    }
    
public:
    static LeakySingleton* getInstance() {
        if (instance == nullptr) {
            instance = new LeakySingleton();
        }
        return instance;
    }
    
    ~LeakySingleton() {
        delete data; // УЯЗВИМОСТЬ: Может быть вызван дважды
        std::cout << "LeakySingleton уничтожен" << std::endl;
    }
};

LeakySingleton* LeakySingleton::instance = nullptr;

// ----------------------------------------------------------------------------
// УЯЗВИМОСТЬ 3: Use-After-Free
// ----------------------------------------------------------------------------
class UseAfterFreeSingleton {
private:
    static UseAfterFreeSingleton* instance;
    char* buffer;
    
    UseAfterFreeSingleton() : buffer(new char[1024]) {
        std::cout << "UseAfterFreeSingleton создан" << std::endl;
    }
    
public:
    static UseAfterFreeSingleton* getInstance() {
        if (instance == nullptr) {
            instance = new UseAfterFreeSingleton();
        }
        return instance;
    }
    
    void writeToBuffer(const char* data) {
        // УЯЗВИМОСТЬ: Нет проверки размера
        strcpy(buffer, data);
    }
    
    char* getBuffer() {
        return buffer; // УЯЗВИМОСТЬ: Возвращаем указатель на внутренние данные
    }
    
    static void destroy() {
        delete instance;
        instance = nullptr; // УЯЗВИМОСТЬ: Не обнуляем указатель
    }
    
    ~UseAfterFreeSingleton() {
        delete[] buffer;
        std::cout << "UseAfterFreeSingleton уничтожен" << std::endl;
    }
};

UseAfterFreeSingleton* UseAfterFreeSingleton::instance = nullptr;

// ----------------------------------------------------------------------------
// УЯЗВИМОСТЬ 4: Integer Overflow в счетчике ссылок
// ----------------------------------------------------------------------------
class ReferenceCountedSingleton {
private:
    static ReferenceCountedSingleton* instance;
    int* refCount;
    int data;
    
    ReferenceCountedSingleton() : refCount(new int(1)), data(100) {
        std::cout << "ReferenceCountedSingleton создан" << std::endl;
    }
    
public:
    static ReferenceCountedSingleton* getInstance() {
        if (instance == nullptr) {
            instance = new ReferenceCountedSingleton();
        } else {
            // УЯЗВИМОСТЬ: Integer overflow при большом количестве ссылок
            (*refCount)++;
        }
        return instance;
    }
    
    void release() {
        (*refCount)--;
        if (*refCount <= 0) {
            delete this;
            instance = nullptr;
        }
    }
    
    int getRefCount() const { return *refCount; }
    int getData() const { return data; }
    
    ~ReferenceCountedSingleton() {
        delete refCount;
        std::cout << "ReferenceCountedSingleton уничтожен" << std::endl;
    }
};

ReferenceCountedSingleton* ReferenceCountedSingleton::instance = nullptr;

// ----------------------------------------------------------------------------
// ДЕМОНСТРАЦИЯ УЯЗВИМОСТЕЙ
// ----------------------------------------------------------------------------

void demonstrateRaceCondition() {
    std::cout << "\n=== Демонстрация Race Condition ===" << std::endl;
    
    std::vector<std::thread> threads;
    
    // Создаем несколько потоков, которые одновременно обращаются к Singleton
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([i]() {
            auto instance = UnsafeSingleton::getInstance();
            instance->setData(i);
            std::cout << "Поток " << i << ": data = " << instance->getData() << std::endl;
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
}

void demonstrateUseAfterFree() {
    std::cout << "\n=== Демонстрация Use-After-Free ===" << std::endl;
    
    auto instance = UseAfterFreeSingleton::getInstance();
    instance->writeToBuffer("Hello, World!");
    
    std::cout << "Данные в буфере: " << instance->getBuffer() << std::endl;
    
    // Уничтожаем объект
    UseAfterFreeSingleton::destroy();
    
    // УЯЗВИМОСТЬ: Попытка использовать после уничтожения
    // Это может привести к crash или неопределенному поведению
    std::cout << "Попытка использовать после уничтожения..." << std::endl;
    // instance->writeToBuffer("This will crash!"); // Раскомментировать для демонстрации
}

void demonstrateIntegerOverflow() {
    std::cout << "\n=== Демонстрация Integer Overflow ===" << std::endl;
    
    auto instance = ReferenceCountedSingleton::getInstance();
    
    // Симулируем большое количество ссылок
    for (int i = 0; i < 1000; ++i) {
        ReferenceCountedSingleton::getInstance();
    }
    
    std::cout << "Количество ссылок: " << instance->getRefCount() << std::endl;
    
    // УЯЗВИМОСТЬ: При переполнении счетчик может стать отрицательным
    // что приведет к преждевременному уничтожению объекта
}

// ----------------------------------------------------------------------------
// ИНСТРУМЕНТЫ АНАЛИЗА
// ----------------------------------------------------------------------------

void runStaticAnalysis() {
    std::cout << "\n=== Инструкции для статического анализа ===" << std::endl;
    std::cout << "1. Clang Static Analyzer:" << std::endl;
    std::cout << "   clang --analyze singleton_vulnerabilities.cpp" << std::endl;
    std::cout << "2. Cppcheck:" << std::endl;
    std::cout << "   cppcheck --enable=all singleton_vulnerabilities.cpp" << std::endl;
    std::cout << "3. PVS-Studio:" << std::endl;
    std::cout << "   pvs-studio-analyzer trace -- make" << std::endl;
}

void runDynamicAnalysis() {
    std::cout << "\n=== Инструкции для динамического анализа ===" << std::endl;
    std::cout << "1. ThreadSanitizer (для race conditions):" << std::endl;
    std::cout << "   g++ -fsanitize=thread -g singleton_vulnerabilities.cpp -o singleton_tsan" << std::endl;
    std::cout << "   ./singleton_tsan" << std::endl;
    std::cout << "2. AddressSanitizer (для use-after-free):" << std::endl;
    std::cout << "   g++ -fsanitize=address -g singleton_vulnerabilities.cpp -o singleton_asan" << std::endl;
    std::cout << "   ./singleton_asan" << std::endl;
    std::cout << "3. Valgrind:" << std::endl;
    std::cout << "   valgrind --tool=helgrind ./singleton_vulnerabilities" << std::endl;
}

// ----------------------------------------------------------------------------
// ОСНОВНАЯ ФУНКЦИЯ
// ----------------------------------------------------------------------------

int main() {
    std::cout << "=== АНАЛИЗ УЯЗВИМОСТЕЙ В ПАТТЕРНЕ SINGLETON ===" << std::endl;
    
    // Демонстрация различных уязвимостей
    demonstrateRaceCondition();
    demonstrateUseAfterFree();
    demonstrateIntegerOverflow();
    
    // Инструкции по анализу
    runStaticAnalysis();
    runDynamicAnalysis();
    
    std::cout << "\n=== ВНИМАНИЕ: Этот код содержит уязвимости! ===" << std::endl;
    std::cout << "Используйте только для обучения и анализа безопасности." << std::endl;
    
    return 0;
}

