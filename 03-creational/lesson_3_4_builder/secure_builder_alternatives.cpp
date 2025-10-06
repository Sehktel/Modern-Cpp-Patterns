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
// БЕЗОПАСНЫЕ АЛЬТЕРНАТИВЫ BUILDER
// ============================================================================

// TODO: Добавить комментарии на русском языке
// TODO: Создать дополнительные безопасные варианты
// TODO: Добавить тесты безопасности

// ----------------------------------------------------------------------------
// БЕЗОПАСНАЯ АЛЬТЕРНАТИВА 1: Builder с std::vector
// ----------------------------------------------------------------------------

class SecureBuilderV1 {
private:
    std::vector<char> buffer;
    mutable std::mutex buffer_mutex;
    
public:
    SecureBuilderV1() {
        std::cout << "SecureBuilderV1 создан в потоке " << std::this_thread::get_id() << std::endl;
    }
    
    // Удаляем копирование и присваивание
    SecureBuilderV1(const SecureBuilderV1&) = delete;
    SecureBuilderV1& operator=(const SecureBuilderV1&) = delete;
    
    // Безопасное добавление данных
    void addData(const char* data, size_t length) {
        std::lock_guard<std::mutex> lock(buffer_mutex);
        
        // Безопасное добавление с проверкой размера
        size_t oldSize = buffer.size();
        buffer.resize(oldSize + length);
        
        // Безопасное копирование
        std::memcpy(buffer.data() + oldSize, data, length);
    }
    
    // Безопасное добавление строки
    void addString(const std::string& str) {
        addData(str.c_str(), str.length());
    }
    
    // Безопасное получение данных
    std::vector<char> getData() const {
        std::lock_guard<std::mutex> lock(buffer_mutex);
        return buffer;
    }
    
    size_t getSize() const {
        std::lock_guard<std::mutex> lock(buffer_mutex);
        return buffer.size();
    }
    
    ~SecureBuilderV1() {
        std::cout << "SecureBuilderV1 уничтожен" << std::endl;
    }
};

// ----------------------------------------------------------------------------
// БЕЗОПАСНАЯ АЛЬТЕРНАТИВА 2: Builder с std::string
// ----------------------------------------------------------------------------

class SecureBuilderV2 {
private:
    std::string data;
    mutable std::mutex data_mutex;
    
public:
    SecureBuilderV2() {
        std::cout << "SecureBuilderV2 создан в потоке " << std::this_thread::get_id() << std::endl;
    }
    
    // Удаляем копирование и присваивание
    SecureBuilderV2(const SecureBuilderV2&) = delete;
    SecureBuilderV2& operator=(const SecureBuilderV2&) = delete;
    
    // Безопасное добавление данных
    void addData(const char* str) {
        std::lock_guard<std::mutex> lock(data_mutex);
        data += str;
    }
    
    void addData(const std::string& str) {
        std::lock_guard<std::mutex> lock(data_mutex);
        data += str;
    }
    
    // Безопасное получение данных
    std::string getData() const {
        std::lock_guard<std::mutex> lock(data_mutex);
        return data;
    }
    
    size_t getSize() const {
        std::lock_guard<std::mutex> lock(data_mutex);
        return data.size();
    }
    
    ~SecureBuilderV2() {
        std::cout << "SecureBuilderV2 уничтожен" << std::endl;
    }
};

// ----------------------------------------------------------------------------
// БЕЗОПАСНАЯ АЛЬТЕРНАТИВА 3: Builder с проверкой размеров
// ----------------------------------------------------------------------------

class SecureBuilderV3 {
private:
    std::vector<char> buffer;
    size_t maxSize;
    mutable std::mutex buffer_mutex;
    
public:
    SecureBuilderV3(size_t maxSize = 1024 * 1024) : maxSize(maxSize) {
        std::cout << "SecureBuilderV3 создан с максимальным размером " << maxSize << std::endl;
    }
    
    // Удаляем копирование и присваивание
    SecureBuilderV3(const SecureBuilderV3&) = delete;
    SecureBuilderV3& operator=(const SecureBuilderV3&) = delete;
    
    // Безопасное добавление с проверкой размера
    bool addData(const char* data, size_t length) {
        std::lock_guard<std::mutex> lock(buffer_mutex);
        
        // Проверка на переполнение
        if (buffer.size() + length > maxSize) {
            std::cout << "ОШИБКА: Превышен максимальный размер буфера!" << std::endl;
            return false;
        }
        
        // Проверка на integer overflow
        if (buffer.size() > SIZE_MAX - length) {
            std::cout << "ОШИБКА: Integer overflow при добавлении данных!" << std::endl;
            return false;
        }
        
        // Безопасное добавление
        size_t oldSize = buffer.size();
        buffer.resize(oldSize + length);
        std::memcpy(buffer.data() + oldSize, data, length);
        
        return true;
    }
    
    // Безопасное получение данных
    std::vector<char> getData() const {
        std::lock_guard<std::mutex> lock(buffer_mutex);
        return buffer;
    }
    
    size_t getSize() const {
        std::lock_guard<std::mutex> lock(buffer_mutex);
        return buffer.size();
    }
    
    size_t getMaxSize() const {
        return maxSize;
    }
    
    ~SecureBuilderV3() {
        std::cout << "SecureBuilderV3 уничтожен" << std::endl;
    }
};

// ----------------------------------------------------------------------------
// БЕЗОПАСНАЯ АЛЬТЕРНАТИВА 4: Builder с RAII и умными указателями
// ----------------------------------------------------------------------------

class SecureBuilderV4 {
private:
    std::unique_ptr<std::vector<char>> buffer;
    mutable std::mutex buffer_mutex;
    
public:
    SecureBuilderV4() : buffer(std::make_unique<std::vector<char>>()) {
        std::cout << "SecureBuilderV4 создан в потоке " << std::this_thread::get_id() << std::endl;
    }
    
    // Удаляем копирование и присваивание
    SecureBuilderV4(const SecureBuilderV4&) = delete;
    SecureBuilderV4& operator=(const SecureBuilderV4&) = delete;
    
    // Безопасное добавление данных
    void addData(const char* data, size_t length) {
        std::lock_guard<std::mutex> lock(buffer_mutex);
        
        if (!buffer) {
            buffer = std::make_unique<std::vector<char>>();
        }
        
        // Безопасное добавление
        size_t oldSize = buffer->size();
        buffer->resize(oldSize + length);
        std::memcpy(buffer->data() + oldSize, data, length);
    }
    
    // Безопасное получение данных
    std::vector<char> getData() const {
        std::lock_guard<std::mutex> lock(buffer_mutex);
        if (!buffer) {
            return std::vector<char>();
        }
        return *buffer;
    }
    
    size_t getSize() const {
        std::lock_guard<std::mutex> lock(buffer_mutex);
        if (!buffer) {
            return 0;
        }
        return buffer->size();
    }
    
    // Безопасный сброс
    void reset() {
        std::lock_guard<std::mutex> lock(buffer_mutex);
        buffer.reset();
        buffer = std::make_unique<std::vector<char>>();
    }
    
    ~SecureBuilderV4() {
        std::cout << "SecureBuilderV4 уничтожен" << std::endl;
    }
};

// ----------------------------------------------------------------------------
// ДЕМОНСТРАЦИЯ БЕЗОПАСНЫХ АЛЬТЕРНАТИВ
// ----------------------------------------------------------------------------

void demonstrateSecureBuilderV1() {
    std::cout << "\n=== ДЕМОНСТРАЦИЯ SecureBuilderV1 (std::vector) ===" << std::endl;
    
    SecureBuilderV1 builder;
    
    // Безопасное добавление данных
    builder.addData("Hello, ", 7);
    builder.addString("World!");
    
    std::cout << "Размер данных: " << builder.getSize() << std::endl;
    
    auto data = builder.getData();
    std::cout << "Данные: ";
    for (char c : data) {
        std::cout << c;
    }
    std::cout << std::endl;
}

void demonstrateSecureBuilderV2() {
    std::cout << "\n=== ДЕМОНСТРАЦИЯ SecureBuilderV2 (std::string) ===" << std::endl;
    
    SecureBuilderV2 builder;
    
    // Безопасное добавление данных
    builder.addData("Secure ");
    builder.addData("Builder ");
    builder.addData("with std::string");
    
    std::cout << "Размер данных: " << builder.getSize() << std::endl;
    std::cout << "Данные: " << builder.getData() << std::endl;
}

void demonstrateSecureBuilderV3() {
    std::cout << "\n=== ДЕМОНСТРАЦИЯ SecureBuilderV3 (с проверкой размеров) ===" << std::endl;
    
    SecureBuilderV3 builder(1000); // Максимальный размер 1000 байт
    
    // Нормальное добавление
    bool success1 = builder.addData("Hello", 5);
    std::cout << "Добавление 'Hello': " << (success1 ? "Успех" : "Неудача") << std::endl;
    
    // Попытка превысить лимит
    std::string largeData(2000, 'A');
    bool success2 = builder.addData(largeData.c_str(), largeData.length());
    std::cout << "Добавление больших данных: " << (success2 ? "Успех" : "Неудача") << std::endl;
    
    std::cout << "Текущий размер: " << builder.getSize() << std::endl;
    std::cout << "Максимальный размер: " << builder.getMaxSize() << std::endl;
}

void demonstrateSecureBuilderV4() {
    std::cout << "\n=== ДЕМОНСТРАЦИЯ SecureBuilderV4 (RAII) ===" << std::endl;
    
    SecureBuilderV4 builder;
    
    // Безопасное добавление данных
    builder.addData("RAII ", 5);
    builder.addData("Builder", 7);
    
    std::cout << "Размер данных: " << builder.getSize() << std::endl;
    
    // Безопасный сброс
    builder.reset();
    std::cout << "Размер после сброса: " << builder.getSize() << std::endl;
}

// ----------------------------------------------------------------------------
// ОСНОВНАЯ ФУНКЦИЯ
// ----------------------------------------------------------------------------

int main() {
    std::cout << "=== ДЕМОНСТРАЦИЯ БЕЗОПАСНЫХ АЛЬТЕРНАТИВ BUILDER ===" << std::endl;
    
    // Демонстрация различных безопасных реализаций
    demonstrateSecureBuilderV1();
    demonstrateSecureBuilderV2();
    demonstrateSecureBuilderV3();
    demonstrateSecureBuilderV4();
    
    std::cout << "\n=== РЕКОМЕНДАЦИИ ПО БЕЗОПАСНОСТИ ===" << std::endl;
    std::cout << "1. Используйте std::vector и std::string вместо сырых указателей" << std::endl;
    std::cout << "2. Проверяйте размеры перед операциями с памятью" << std::endl;
    std::cout << "3. Применяйте RAII и умные указатели" << std::endl;
    std::cout << "4. Используйте мьютексы для многопоточности" << std::endl;
    std::cout << "5. Валидируйте входные данные" << std::endl;
    std::cout << "6. Ограничивайте максимальные размеры" << std::endl;
    std::cout << "7. Регулярно анализируйте код с помощью инструментов безопасности" << std::endl;
    
    return 0;
}