#include <iostream>
#include <thread>
#include <mutex>
#include <memory>
#include <vector>
#include <chrono>
#include <cstring>

// ============================================================================
// УЯЗВИМЫЕ РЕАЛИЗАЦИИ FACADE ДЛЯ АНАЛИЗА БЕЗОПАСНОСТИ
// ============================================================================

// TODO: Добавить комментарии на русском языке
// TODO: Создать эксплоиты для каждой уязвимости
// TODO: Добавить инструменты анализа

// ----------------------------------------------------------------------------
// УЯЗВИМОСТЬ 1: Information Disclosure через Facade
// ----------------------------------------------------------------------------
class VulnerableFacade {
private:
    void* subsystem1;
    void* subsystem2;
    void* subsystem3;
    char* sensitiveData;
    size_t dataSize;
    
public:
    VulnerableFacade() : subsystem1(nullptr), subsystem2(nullptr), subsystem3(nullptr), 
                        sensitiveData(nullptr), dataSize(0) {
        std::cout << "VulnerableFacade создан" << std::endl;
    }
    
    // УЯЗВИМОСТЬ: Небезопасная инициализация подсистем
    void initializeSubsystems() {
        // УЯЗВИМОСТЬ: Небезопасное выделение памяти
        subsystem1 = new char[1024];
        subsystem2 = new char[2048];
        subsystem3 = new char[4096];
        
        // УЯЗВИМОСТЬ: Хранение чувствительных данных в открытом виде
        sensitiveData = new char[512];
        strcpy(sensitiveData, "Sensitive password: admin123");
        dataSize = strlen(sensitiveData);
    }
    
    // УЯЗВИМОСТЬ: Возврат указателя на чувствительные данные
    char* getSensitiveData() {
        return sensitiveData;
    }
    
    // УЯЗВИМОСТЬ: Небезопасная обработка подсистем
    void processRequest(const char* request) {
        if (subsystem1) {
            // УЯЗВИМОСТЬ: Небезопасное копирование
            memcpy(subsystem1, request, strlen(request));
        }
        
        if (subsystem2) {
            // УЯЗВИМОСТЬ: Может произойти переполнение
            strcpy(static_cast<char*>(subsystem2), request);
        }
        
        if (subsystem3) {
            // УЯЗВИМОСТЬ: Небезопасная обработка
            memcpy(subsystem3, sensitiveData, dataSize);
        }
    }
    
    // УЯЗВИМОСТЬ: Возврат указателей на внутренние данные
    void* getSubsystem1() { return subsystem1; }
    void* getSubsystem2() { return subsystem2; }
    void* getSubsystem3() { return subsystem3; }
    
    ~VulnerableFacade() {
        delete[] static_cast<char*>(subsystem1);
        delete[] static_cast<char*>(subsystem2);
        delete[] static_cast<char*>(subsystem3);
        delete[] sensitiveData;
        std::cout << "VulnerableFacade уничтожен" << std::endl;
    }
};

// ----------------------------------------------------------------------------
// УЯЗВИМОСТЬ 2: Buffer Overflow при обработке запросов
// ----------------------------------------------------------------------------
class BufferOverflowFacade {
private:
    char* requestBuffer;
    size_t bufferSize;
    size_t currentSize;
    
public:
    BufferOverflowFacade() : requestBuffer(nullptr), bufferSize(0), currentSize(0) {
        std::cout << "BufferOverflowFacade создан" << std::endl;
    }
    
    // УЯЗВИМОСТЬ: Небезопасная обработка запросов
    void processRequest(const char* request, size_t length) {
        // УЯЗВИМОСТЬ: Нет проверки переполнения буфера
        if (currentSize + length > bufferSize) {
            bufferSize = currentSize + length;
            char* newBuffer = new char[bufferSize];
            if (requestBuffer) {
                memcpy(newBuffer, requestBuffer, currentSize);
                delete[] requestBuffer;
            }
            requestBuffer = newBuffer;
        }
        
        // УЯЗВИМОСТЬ: Может произойти переполнение
        memcpy(requestBuffer + currentSize, request, length);
        currentSize += length;
    }
    
    // УЯЗВИМОСТЬ: Возврат указателя на внутренние данные
    char* getRequestBuffer() {
        return requestBuffer;
    }
    
    size_t getBufferSize() const {
        return bufferSize;
    }
    
    size_t getCurrentSize() const {
        return currentSize;
    }
    
    ~BufferOverflowFacade() {
        delete[] requestBuffer;
        std::cout << "BufferOverflowFacade уничтожен" << std::endl;
    }
};

// ----------------------------------------------------------------------------
// УЯЗВИМОСТЬ 3: Use-After-Free при неправильном управлении подсистемами
// ----------------------------------------------------------------------------
class UseAfterFreeFacade {
private:
    void* subsystem1;
    void* subsystem2;
    bool isValid;
    
public:
    UseAfterFreeFacade() : subsystem1(nullptr), subsystem2(nullptr), isValid(false) {
        std::cout << "UseAfterFreeFacade создан" << std::endl;
    }
    
    void initializeSubsystems() {
        subsystem1 = new char[1024];
        subsystem2 = new char[2048];
        isValid = true;
    }
    
    // УЯЗВИМОСТЬ: Use-after-free при освобождении подсистем
    void releaseSubsystem1() {
        delete[] static_cast<char*>(subsystem1);
        subsystem1 = nullptr;
        // УЯЗВИМОСТЬ: Не обнуляем указатель
    }
    
    // УЯЗВИМОСТЬ: Возврат указателя на освобожденную память
    void* getSubsystem1() {
        return subsystem1; // Может быть dangling pointer
    }
    
    bool isSubsystem1Valid() const {
        return isValid && subsystem1 != nullptr;
    }
    
    void processRequest(const char* request) {
        if (isValid && subsystem1) {
            // УЯЗВИМОСТЬ: Может произойти use-after-free
            strcpy(static_cast<char*>(subsystem1), request);
        }
        
        if (subsystem2) {
            strcpy(static_cast<char*>(subsystem2), request);
        }
    }
    
    ~UseAfterFreeFacade() {
        delete[] static_cast<char*>(subsystem1);
        delete[] static_cast<char*>(subsystem2);
        std::cout << "UseAfterFreeFacade уничтожен" << std::endl;
    }
};

// ----------------------------------------------------------------------------
// УЯЗВИМОСТЬ 4: Integer Overflow при подсчете запросов
// ----------------------------------------------------------------------------
class IntegerOverflowFacade {
private:
    size_t requestCount;
    size_t maxRequests;
    
public:
    IntegerOverflowFacade() : requestCount(0), maxRequests(1000) {
        std::cout << "IntegerOverflowFacade создан" << std::endl;
    }
    
    // УЯЗВИМОСТЬ: Integer overflow при обработке запросов
    bool processRequest(const char* request) {
        // УЯЗВИМОСТЬ: Может произойти переполнение
        requestCount++;
        
        // УЯЗВИМОСТЬ: Проверка может быть обойдена при overflow
        if (requestCount < maxRequests) {
            std::cout << "Обработка запроса " << requestCount << std::endl;
            return true;
        }
        
        std::cout << "Превышено максимальное количество запросов" << std::endl;
        return false;
    }
    
    // УЯЗВИМОСТЬ: Небезопасное создание буфера
    void* createBuffer() {
        // УЯЗВИМОСТЬ: При overflow может быть выделена неправильная память
        return new char[requestCount];
    }
    
    size_t getRequestCount() const {
        return requestCount;
    }
    
    size_t getMaxRequests() const {
        return maxRequests;
    }
    
    ~IntegerOverflowFacade() {
        std::cout << "IntegerOverflowFacade уничтожен" << std::endl;
    }
};

// ----------------------------------------------------------------------------
// ДЕМОНСТРАЦИЯ УЯЗВИМОСТЕЙ
// ----------------------------------------------------------------------------

void demonstrateInformationDisclosure() {
    std::cout << "\n=== Демонстрация Information Disclosure ===" << std::endl;
    
    VulnerableFacade facade;
    facade.initializeSubsystems();
    
    // УЯЗВИМОСТЬ: Получение доступа к чувствительным данным
    char* sensitiveData = facade.getSensitiveData();
    if (sensitiveData) {
        std::cout << "Получены чувствительные данные: " << sensitiveData << std::endl;
    }
    
    // УЯЗВИМОСТЬ: Получение доступа к подсистемам
    void* subsystem1 = facade.getSubsystem1();
    void* subsystem2 = facade.getSubsystem2();
    void* subsystem3 = facade.getSubsystem3();
    
    std::cout << "Получен доступ к подсистемам:" << std::endl;
    std::cout << "Subsystem1: " << subsystem1 << std::endl;
    std::cout << "Subsystem2: " << subsystem2 << std::endl;
    std::cout << "Subsystem3: " << subsystem3 << std::endl;
}

void demonstrateBufferOverflow() {
    std::cout << "\n=== Демонстрация Buffer Overflow ===" << std::endl;
    
    BufferOverflowFacade facade;
    
    // Нормальное использование
    facade.processRequest("Hello", 5);
    std::cout << "Обработан запрос размером 5 байт" << std::endl;
    
    // УЯЗВИМОСТЬ: Попытка обработки очень больших данных
    std::string largeRequest(1000000, 'A');
    facade.processRequest(largeRequest.c_str(), largeRequest.length());
    
    std::cout << "Обработан запрос размером: " << facade.getCurrentSize() << std::endl;
    std::cout << "Размер буфера: " << facade.getBufferSize() << std::endl;
}

void demonstrateUseAfterFree() {
    std::cout << "\n=== Демонстрация Use-After-Free ===" << std::endl;
    
    UseAfterFreeFacade facade;
    facade.initializeSubsystems();
    
    // Освобождаем подсистему
    facade.releaseSubsystem1();
    std::cout << "Подсистема 1 освобождена" << std::endl;
    
    // УЯЗВИМОСТЬ: Попытка использования после освобождения
    void* subsystem1 = facade.getSubsystem1();
    if (subsystem1) {
        std::cout << "Подсистема 1 все еще доступна (dangling pointer)" << std::endl;
        std::cout << "Валидность: " << (facade.isSubsystem1Valid() ? "Да" : "Нет") << std::endl;
    }
    
    // Попытка обработки запроса
    facade.processRequest("Test request");
}

void demonstrateIntegerOverflow() {
    std::cout << "\n=== Демонстрация Integer Overflow ===" << std::endl;
    
    IntegerOverflowFacade facade;
    
    // Нормальное использование
    for (int i = 0; i < 100; ++i) {
        facade.processRequest("Normal request");
    }
    
    std::cout << "Количество запросов: " << facade.getRequestCount() << std::endl;
    
    // УЯЗВИМОСТЬ: Integer overflow
    size_t largeCount = SIZE_MAX - 100;
    for (size_t i = 0; i < largeCount; ++i) {
        facade.processRequest("Overflow request");
    }
    
    std::cout << "Количество запросов после overflow: " << facade.getRequestCount() << std::endl;
    
    // Попытка создания буфера
    void* buffer = facade.createBuffer();
    if (buffer) {
        std::cout << "Буфер создан (возможно из-за overflow)" << std::endl;
        delete[] static_cast<char*>(buffer);
    }
}

// ----------------------------------------------------------------------------
// ИНСТРУМЕНТЫ АНАЛИЗА
// ----------------------------------------------------------------------------

void runStaticAnalysis() {
    std::cout << "\n=== Инструкции для статического анализа ===" << std::endl;
    std::cout << "1. Clang Static Analyzer:" << std::endl;
    std::cout << "   clang --analyze facade_vulnerabilities.cpp" << std::endl;
    std::cout << "2. Cppcheck:" << std::endl;
    std::cout << "   cppcheck --enable=all facade_vulnerabilities.cpp" << std::endl;
    std::cout << "3. PVS-Studio:" << std::endl;
    std::cout << "   pvs-studio-analyzer trace -- make" << std::endl;
}

void runDynamicAnalysis() {
    std::cout << "\n=== Инструкции для динамического анализа ===" << std::endl;
    std::cout << "1. AddressSanitizer:" << std::endl;
    std::cout << "   g++ -fsanitize=address -g facade_vulnerabilities.cpp -o facade_asan" << std::endl;
    std::cout << "   ./facade_asan" << std::endl;
    std::cout << "2. MemorySanitizer:" << std::endl;
    std::cout << "   g++ -fsanitize=memory -g facade_vulnerabilities.cpp -o facade_msan" << std::endl;
    std::cout << "   ./facade_msan" << std::endl;
    std::cout << "3. Valgrind:" << std::endl;
    std::cout << "   valgrind --tool=memcheck ./facade_vulnerabilities" << std::endl;
}

// ----------------------------------------------------------------------------
// ОСНОВНАЯ ФУНКЦИЯ
// ----------------------------------------------------------------------------

int main() {
    std::cout << "=== АНАЛИЗ УЯЗВИМОСТЕЙ В ПАТТЕРНЕ FACADE ===" << std::endl;
    
    // Демонстрация различных уязвимостей
    demonstrateInformationDisclosure();
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