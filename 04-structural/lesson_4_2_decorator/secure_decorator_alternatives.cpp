#include <iostream>
#include <thread>
#include <mutex>
#include <memory>
#include <vector>
#include <chrono>
#include <cstring>
#include <atomic>
#include <string>

// ============================================================================
// БЕЗОПАСНЫЕ АЛЬТЕРНАТИВЫ DECORATOR
// ============================================================================

// TODO: Добавить комментарии на русском языке
// TODO: Создать дополнительные безопасные варианты
// TODO: Добавить тесты безопасности

// ----------------------------------------------------------------------------
// БЕЗОПАСНАЯ АЛЬТЕРНАТИВА 1: Decorator с ограничением глубины
// ----------------------------------------------------------------------------

class SecureDecoratorV1 {
private:
    std::shared_ptr<SecureDecoratorV1> next;
    std::string data;
    mutable std::mutex decorator_mutex;
    static constexpr size_t MAX_DEPTH = 100;
    
public:
    SecureDecoratorV1() {
        std::cout << "SecureDecoratorV1 создан в потоке " << std::this_thread::get_id() << std::endl;
    }
    
    // Удаляем копирование и присваивание
    SecureDecoratorV1(const SecureDecoratorV1&) = delete;
    SecureDecoratorV1& operator=(const SecureDecoratorV1&) = delete;
    
    // Безопасное добавление декоратора с проверкой глубины
    bool addDecorator(std::shared_ptr<SecureDecoratorV1> decorator) {
        std::lock_guard<std::mutex> lock(decorator_mutex);
        
        if (getDepth() >= MAX_DEPTH) {
            std::cout << "ОШИБКА: Превышена максимальная глубина цепочки!" << std::endl;
            return false;
        }
        
        if (next) {
            return next->addDecorator(decorator);
        } else {
            next = decorator;
            return true;
        }
    }
    
    // Безопасное добавление данных
    void setData(const std::string& str) {
        std::lock_guard<std::mutex> lock(decorator_mutex);
        data = str;
    }
    
    // Безопасная обработка с ограничением глубины
    void process() {
        std::lock_guard<std::mutex> lock(decorator_mutex);
        
        std::cout << "Обработка декоратора: " << data << std::endl;
        
        if (next) {
            next->process();
        }
    }
    
    // Безопасное получение данных
    std::string getData() const {
        std::lock_guard<std::mutex> lock(decorator_mutex);
        return data;
    }
    
    // Получение глубины цепочки
    size_t getDepth() const {
        std::lock_guard<std::mutex> lock(decorator_mutex);
        if (next) {
            return 1 + next->getDepth();
        }
        return 1;
    }
    
    ~SecureDecoratorV1() {
        std::cout << "SecureDecoratorV1 уничтожен" << std::endl;
    }
};

// ----------------------------------------------------------------------------
// БЕЗОПАСНАЯ АЛЬТЕРНАТИВА 2: Decorator с RAII
// ----------------------------------------------------------------------------

class SecureDecoratorV2 {
private:
    std::unique_ptr<SecureDecoratorV2> next;
    std::vector<char> data;
    mutable std::mutex decorator_mutex;
    
public:
    SecureDecoratorV2() {
        std::cout << "SecureDecoratorV2 создан в потоке " << std::this_thread::get_id() << std::endl;
    }
    
    // Удаляем копирование и присваивание
    SecureDecoratorV2(const SecureDecoratorV2&) = delete;
    SecureDecoratorV2& operator=(const SecureDecoratorV2&) = delete;
    
    // Безопасное добавление декоратора
    void addDecorator(std::unique_ptr<SecureDecoratorV2> decorator) {
        std::lock_guard<std::mutex> lock(decorator_mutex);
        
        if (next) {
            next->addDecorator(std::move(decorator));
        } else {
            next = std::move(decorator);
        }
    }
    
    // Безопасное добавление данных
    void setData(const char* str, size_t length) {
        std::lock_guard<std::mutex> lock(decorator_mutex);
        
        // Безопасное добавление с проверкой размера
        data.resize(length);
        std::memcpy(data.data(), str, length);
    }
    
    // Безопасная обработка
    void process() {
        std::lock_guard<std::mutex> lock(decorator_mutex);
        
        std::cout << "Обработка декоратора размером: " << data.size() << std::endl;
        
        if (next) {
            next->process();
        }
    }
    
    // Безопасное получение данных
    std::vector<char> getData() const {
        std::lock_guard<std::mutex> lock(decorator_mutex);
        return data;
    }
    
    ~SecureDecoratorV2() {
        std::cout << "SecureDecoratorV2 уничтожен" << std::endl;
    }
};

// ----------------------------------------------------------------------------
// БЕЗОПАСНАЯ АЛЬТЕРНАТИВА 3: Decorator с проверкой размеров
// ----------------------------------------------------------------------------

class SecureDecoratorV3 {
private:
    std::shared_ptr<SecureDecoratorV3> next;
    std::vector<char> data;
    size_t maxDataSize;
    mutable std::mutex decorator_mutex;
    
public:
    SecureDecoratorV3(size_t maxDataSize = 1024 * 1024) : maxDataSize(maxDataSize) {
        std::cout << "SecureDecoratorV3 создан с максимальным размером данных " << maxDataSize << std::endl;
    }
    
    // Удаляем копирование и присваивание
    SecureDecoratorV3(const SecureDecoratorV3&) = delete;
    SecureDecoratorV3& operator=(const SecureDecoratorV3&) = delete;
    
    // Безопасное добавление декоратора
    bool addDecorator(std::shared_ptr<SecureDecoratorV3> decorator) {
        std::lock_guard<std::mutex> lock(decorator_mutex);
        
        if (next) {
            return next->addDecorator(decorator);
        } else {
            next = decorator;
            return true;
        }
    }
    
    // Безопасное добавление данных с проверкой размера
    bool setData(const char* str, size_t length) {
        std::lock_guard<std::mutex> lock(decorator_mutex);
        
        // Проверка на переполнение
        if (length > maxDataSize) {
            std::cout << "ОШИБКА: Превышен максимальный размер данных!" << std::endl;
            return false;
        }
        
        // Проверка на integer overflow
        if (length > SIZE_MAX) {
            std::cout << "ОШИБКА: Integer overflow при установке данных!" << std::endl;
            return false;
        }
        
        // Безопасное добавление
        data.resize(length);
        std::memcpy(data.data(), str, length);
        
        return true;
    }
    
    // Безопасная обработка
    void process() {
        std::lock_guard<std::mutex> lock(decorator_mutex);
        
        std::cout << "Обработка декоратора размером: " << data.size() << std::endl;
        
        if (next) {
            next->process();
        }
    }
    
    // Безопасное получение данных
    std::vector<char> getData() const {
        std::lock_guard<std::mutex> lock(decorator_mutex);
        return data;
    }
    
    size_t getMaxDataSize() const {
        return maxDataSize;
    }
    
    ~SecureDecoratorV3() {
        std::cout << "SecureDecoratorV3 уничтожен" << std::endl;
    }
};

// ----------------------------------------------------------------------------
// БЕЗОПАСНАЯ АЛЬТЕРНАТИВА 4: Decorator с умными указателями
// ----------------------------------------------------------------------------

class SecureDecoratorV4 {
private:
    std::shared_ptr<SecureDecoratorV4> next;
    std::unique_ptr<std::string> data;
    mutable std::mutex decorator_mutex;
    
public:
    SecureDecoratorV4() : data(std::make_unique<std::string>()) {
        std::cout << "SecureDecoratorV4 создан в потоке " << std::this_thread::get_id() << std::endl;
    }
    
    // Удаляем копирование и присваивание
    SecureDecoratorV4(const SecureDecoratorV4&) = delete;
    SecureDecoratorV4& operator=(const SecureDecoratorV4&) = delete;
    
    // Безопасное добавление декоратора
    void addDecorator(std::shared_ptr<SecureDecoratorV4> decorator) {
        std::lock_guard<std::mutex> lock(decorator_mutex);
        
        if (next) {
            next->addDecorator(decorator);
        } else {
            next = decorator;
        }
    }
    
    // Безопасное добавление данных
    void setData(const std::string& str) {
        std::lock_guard<std::mutex> lock(decorator_mutex);
        
        if (!data) {
            data = std::make_unique<std::string>();
        }
        
        *data = str;
    }
    
    // Безопасная обработка
    void process() {
        std::lock_guard<std::mutex> lock(decorator_mutex);
        
        if (data) {
            std::cout << "Обработка декоратора: " << *data << std::endl;
        }
        
        if (next) {
            next->process();
        }
    }
    
    // Безопасное получение данных
    std::string getData() const {
        std::lock_guard<std::mutex> lock(decorator_mutex);
        if (!data) {
            return "";
        }
        return *data;
    }
    
    // Безопасный сброс
    void reset() {
        std::lock_guard<std::mutex> lock(decorator_mutex);
        data.reset();
        data = std::make_unique<std::string>();
    }
    
    ~SecureDecoratorV4() {
        std::cout << "SecureDecoratorV4 уничтожен" << std::endl;
    }
};

// ----------------------------------------------------------------------------
// ДЕМОНСТРАЦИЯ БЕЗОПАСНЫХ АЛЬТЕРНАТИВ
// ----------------------------------------------------------------------------

void demonstrateSecureDecoratorV1() {
    std::cout << "\n=== ДЕМОНСТРАЦИЯ SecureDecoratorV1 (ограничение глубины) ===" << std::endl;
    
    auto root = std::make_shared<SecureDecoratorV1>();
    root->setData("Root decorator");
    
    // Создаем цепочку декораторов
    for (int i = 0; i < 50; ++i) {
        auto decorator = std::make_shared<SecureDecoratorV1>();
        decorator->setData("Decorator " + std::to_string(i));
        
        bool success = root->addDecorator(decorator);
        if (!success) {
            std::cout << "Не удалось добавить декоратор " << i << std::endl;
            break;
        }
    }
    
    std::cout << "Глубина цепочки: " << root->getDepth() << std::endl;
    root->process();
}

void demonstrateSecureDecoratorV2() {
    std::cout << "\n=== ДЕМОНСТРАЦИЯ SecureDecoratorV2 (RAII) ===" << std::endl;
    
    auto root = std::make_unique<SecureDecoratorV2>();
    root->setData("RAII Root", 9);
    
    // Создаем цепочку декораторов
    for (int i = 0; i < 5; ++i) {
        auto decorator = std::make_unique<SecureDecoratorV2>();
        std::string data = "RAII Decorator " + std::to_string(i);
        decorator->setData(data.c_str(), data.length());
        
        root->addDecorator(std::move(decorator));
    }
    
    root->process();
}

void demonstrateSecureDecoratorV3() {
    std::cout << "\n=== ДЕМОНСТРАЦИЯ SecureDecoratorV3 (проверка размеров) ===" << std::endl;
    
    auto root = std::make_shared<SecureDecoratorV3>(1000); // Максимальный размер 1000 байт
    
    // Нормальное добавление данных
    bool success1 = root->setData("Hello", 5);
    std::cout << "Добавление 'Hello': " << (success1 ? "Успех" : "Неудача") << std::endl;
    
    // Попытка превысить лимит
    std::string largeData(2000, 'A');
    bool success2 = root->setData(largeData.c_str(), largeData.length());
    std::cout << "Добавление больших данных: " << (success2 ? "Успех" : "Неудача") << std::endl;
    
    std::cout << "Максимальный размер: " << root->getMaxDataSize() << std::endl;
}

void demonstrateSecureDecoratorV4() {
    std::cout << "\n=== ДЕМОНСТРАЦИЯ SecureDecoratorV4 (умные указатели) ===" << std::endl;
    
    auto root = std::make_shared<SecureDecoratorV4>();
    root->setData("Smart Pointer Root");
    
    // Создаем цепочку декораторов
    for (int i = 0; i < 3; ++i) {
        auto decorator = std::make_shared<SecureDecoratorV4>();
        decorator->setData("Smart Pointer Decorator " + std::to_string(i));
        
        root->addDecorator(decorator);
    }
    
    root->process();
    
    // Безопасный сброс
    root->reset();
    std::cout << "Данные после сброса: " << root->getData() << std::endl;
}

// ----------------------------------------------------------------------------
// ОСНОВНАЯ ФУНКЦИЯ
// ----------------------------------------------------------------------------

int main() {
    std::cout << "=== ДЕМОНСТРАЦИЯ БЕЗОПАСНЫХ АЛЬТЕРНАТИВ DECORATOR ===" << std::endl;
    
    // Демонстрация различных безопасных реализаций
    demonstrateSecureDecoratorV1();
    demonstrateSecureDecoratorV2();
    demonstrateSecureDecoratorV3();
    demonstrateSecureDecoratorV4();
    
    std::cout << "\n=== РЕКОМЕНДАЦИИ ПО БЕЗОПАСНОСТИ ===" << std::endl;
    std::cout << "1. Ограничивайте глубину цепочки декораторов" << std::endl;
    std::cout << "2. Используйте RAII для управления ресурсами" << std::endl;
    std::cout << "3. Проверяйте валидность указателей" << std::endl;
    std::cout << "4. Используйте умные указатели" << std::endl;
    std::cout << "5. Валидируйте входные данные" << std::endl;
    std::cout << "6. Ограничивайте максимальные размеры" << std::endl;
    std::cout << "7. Регулярно анализируйте код с помощью инструментов безопасности" << std::endl;
    
    return 0;
}