#include <iostream>
#include <thread>
#include <mutex>
#include <memory>
#include <vector>
#include <chrono>
#include <cstring>

// ============================================================================
// УЯЗВИМЫЕ РЕАЛИЗАЦИИ PROXY ДЛЯ АНАЛИЗА БЕЗОПАСНОСТИ
// ============================================================================

// TODO: Добавить комментарии на русском языке
// TODO: Создать эксплоиты для каждой уязвимости
// TODO: Добавить инструменты анализа

// ----------------------------------------------------------------------------
// УЯЗВИМОСТЬ 1: Authentication Bypass через Proxy
// ----------------------------------------------------------------------------
class VulnerableProxy {
private:
    void* realObject;
    char* username;
    char* password;
    bool isAuthenticated;
    
public:
    VulnerableProxy() : realObject(nullptr), username(nullptr), password(nullptr), isAuthenticated(false) {
        std::cout << "VulnerableProxy создан" << std::endl;
    }
    
    // УЯЗВИМОСТЬ: Небезопасная аутентификация
    bool authenticate(const char* user, const char* pass) {
        // УЯЗВИМОСТЬ: Небезопасное сравнение паролей
        if (strcmp(user, "admin") == 0 && strcmp(pass, "password") == 0) {
            isAuthenticated = true;
            username = new char[strlen(user) + 1];
            password = new char[strlen(pass) + 1];
            strcpy(username, user);
            strcpy(password, pass);
            return true;
        }
        return false;
    }
    
    // УЯЗВИМОСТЬ: Небезопасный доступ к реальному объекту
    void* getRealObject() {
        if (isAuthenticated) {
            return realObject;
        }
        return nullptr;
    }
    
    // УЯЗВИМОСТЬ: Небезопасная установка реального объекта
    void setRealObject(void* obj) {
        realObject = obj;
    }
    
    // УЯЗВИМОСТЬ: Возврат учетных данных
    char* getUsername() {
        return username;
    }
    
    char* getPassword() {
        return password;
    }
    
    bool isAuth() const {
        return isAuthenticated;
    }
    
    ~VulnerableProxy() {
        delete[] username;
        delete[] password;
        std::cout << "VulnerableProxy уничтожен" << std::endl;
    }
};

// ----------------------------------------------------------------------------
// УЯЗВИМОСТЬ 2: Buffer Overflow при обработке запросов
// ----------------------------------------------------------------------------
class BufferOverflowProxy {
private:
    char* requestBuffer;
    size_t bufferSize;
    size_t currentSize;
    
public:
    BufferOverflowProxy() : requestBuffer(nullptr), bufferSize(0), currentSize(0) {
        std::cout << "BufferOverflowProxy создан" << std::endl;
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
    
    ~BufferOverflowProxy() {
        delete[] requestBuffer;
        std::cout << "BufferOverflowProxy уничтожен" << std::endl;
    }
};

// ----------------------------------------------------------------------------
// УЯЗВИМОСТЬ 3: Use-After-Free при неправильном управлении объектами
// ----------------------------------------------------------------------------
class UseAfterFreeProxy {
private:
    void* realObject;
    bool isValid;
    
public:
    UseAfterFreeProxy() : realObject(nullptr), isValid(false) {
        std::cout << "UseAfterFreeProxy создан" << std::endl;
    }
    
    void setRealObject(void* obj) {
        realObject = obj;
        isValid = true;
    }
    
    // УЯЗВИМОСТЬ: Use-after-free при освобождении объекта
    void releaseRealObject() {
        delete[] static_cast<char*>(realObject);
        realObject = nullptr;
        isValid = false;
        // УЯЗВИМОСТЬ: Не обнуляем указатель
    }
    
    // УЯЗВИМОСТЬ: Возврат указателя на освобожденную память
    void* getRealObject() {
        return realObject; // Может быть dangling pointer
    }
    
    bool isObjectValid() const {
        return isValid && realObject != nullptr;
    }
    
    void processRequest(const char* request) {
        if (isValid && realObject) {
            // УЯЗВИМОСТЬ: Может произойти use-after-free
            strcpy(static_cast<char*>(realObject), request);
        }
    }
    
    ~UseAfterFreeProxy() {
        delete[] static_cast<char*>(realObject);
        std::cout << "UseAfterFreeProxy уничтожен" << std::endl;
    }
};

// ----------------------------------------------------------------------------
// УЯЗВИМОСТЬ 4: Integer Overflow при подсчете запросов
// ----------------------------------------------------------------------------
class IntegerOverflowProxy {
private:
    size_t requestCount;
    size_t maxRequests;
    
public:
    IntegerOverflowProxy() : requestCount(0), maxRequests(1000) {
        std::cout << "IntegerOverflowProxy создан" << std::endl;
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
    
    ~IntegerOverflowProxy() {
        std::cout << "IntegerOverflowProxy уничтожен" << std::endl;
    }
};

// ----------------------------------------------------------------------------
// ДЕМОНСТРАЦИЯ УЯЗВИМОСТЕЙ
// ----------------------------------------------------------------------------

void demonstrateAuthenticationBypass() {
    std::cout << "\n=== Демонстрация Authentication Bypass ===" << std::endl;
    
    VulnerableProxy proxy;
    
    // Нормальная аутентификация
    bool auth1 = proxy.authenticate("admin", "password");
    std::cout << "Аутентификация 'admin': " << (auth1 ? "Успех" : "Неудача") << std::endl;
    
    // УЯЗВИМОСТЬ: Попытка обхода аутентификации
    bool auth2 = proxy.authenticate("user", "wrong");
    std::cout << "Аутентификация 'user': " << (auth2 ? "Успех" : "Неудача") << std::endl;
    
    // УЯЗВИМОСТЬ: Получение учетных данных
    char* username = proxy.getUsername();
    char* password = proxy.getPassword();
    
    if (username && password) {
        std::cout << "Получены учетные данные: " << username << " / " << password << std::endl;
    }
    
    // УЯЗВИМОСТЬ: Установка реального объекта без проверки
    char* obj = new char[1024];
    proxy.setRealObject(obj);
    
    void* realObj = proxy.getRealObject();
    if (realObj) {
        std::cout << "Получен доступ к реальному объекту" << std::endl;
    }
    
    delete[] obj;
}

void demonstrateBufferOverflow() {
    std::cout << "\n=== Демонстрация Buffer Overflow ===" << std::endl;
    
    BufferOverflowProxy proxy;
    
    // Нормальное использование
    proxy.processRequest("Hello", 5);
    std::cout << "Обработан запрос размером 5 байт" << std::endl;
    
    // УЯЗВИМОСТЬ: Попытка обработки очень больших данных
    std::string largeRequest(1000000, 'A');
    proxy.processRequest(largeRequest.c_str(), largeRequest.length());
    
    std::cout << "Обработан запрос размером: " << proxy.getCurrentSize() << std::endl;
    std::cout << "Размер буфера: " << proxy.getBufferSize() << std::endl;
}

void demonstrateUseAfterFree() {
    std::cout << "\n=== Демонстрация Use-After-Free ===" << std::endl;
    
    UseAfterFreeProxy proxy;
    
    // Создаем объект
    char* obj = new char[1024];
    strcpy(obj, "Real object data");
    
    proxy.setRealObject(obj);
    std::cout << "Реальный объект установлен" << std::endl;
    
    // Освобождаем объект
    proxy.releaseRealObject();
    std::cout << "Реальный объект освобожден" << std::endl;
    
    // УЯЗВИМОСТЬ: Попытка использования после освобождения
    void* realObj = proxy.getRealObject();
    if (realObj) {
        std::cout << "Реальный объект все еще доступен (dangling pointer)" << std::endl;
        std::cout << "Валидность: " << (proxy.isObjectValid() ? "Да" : "Нет") << std::endl;
    }
    
    // Попытка обработки запроса
    proxy.processRequest("Test request");
}

void demonstrateIntegerOverflow() {
    std::cout << "\n=== Демонстрация Integer Overflow ===" << std::endl;
    
    IntegerOverflowProxy proxy;
    
    // Нормальное использование
    for (int i = 0; i < 100; ++i) {
        proxy.processRequest("Normal request");
    }
    
    std::cout << "Количество запросов: " << proxy.getRequestCount() << std::endl;
    
    // УЯЗВИМОСТЬ: Integer overflow
    size_t largeCount = SIZE_MAX - 100;
    for (size_t i = 0; i < largeCount; ++i) {
        proxy.processRequest("Overflow request");
    }
    
    std::cout << "Количество запросов после overflow: " << proxy.getRequestCount() << std::endl;
    
    // Попытка создания буфера
    void* buffer = proxy.createBuffer();
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
    std::cout << "   clang --analyze proxy_vulnerabilities.cpp" << std::endl;
    std::cout << "2. Cppcheck:" << std::endl;
    std::cout << "   cppcheck --enable=all proxy_vulnerabilities.cpp" << std::endl;
    std::cout << "3. PVS-Studio:" << std::endl;
    std::cout << "   pvs-studio-analyzer trace -- make" << std::endl;
}

void runDynamicAnalysis() {
    std::cout << "\n=== Инструкции для динамического анализа ===" << std::endl;
    std::cout << "1. AddressSanitizer:" << std::endl;
    std::cout << "   g++ -fsanitize=address -g proxy_vulnerabilities.cpp -o proxy_asan" << std::endl;
    std::cout << "   ./proxy_asan" << std::endl;
    std::cout << "2. MemorySanitizer:" << std::endl;
    std::cout << "   g++ -fsanitize=memory -g proxy_vulnerabilities.cpp -o proxy_msan" << std::endl;
    std::cout << "   ./proxy_msan" << std::endl;
    std::cout << "3. Valgrind:" << std::endl;
    std::cout << "   valgrind --tool=memcheck ./proxy_vulnerabilities" << std::endl;
}

// ----------------------------------------------------------------------------
// ОСНОВНАЯ ФУНКЦИЯ
// ----------------------------------------------------------------------------

int main() {
    std::cout << "=== АНАЛИЗ УЯЗВИМОСТЕЙ В ПАТТЕРНЕ PROXY ===" << std::endl;
    
    // Демонстрация различных уязвимостей
    demonstrateAuthenticationBypass();
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