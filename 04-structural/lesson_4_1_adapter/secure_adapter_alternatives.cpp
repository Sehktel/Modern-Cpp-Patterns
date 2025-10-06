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
// БЕЗОПАСНЫЕ АЛЬТЕРНАТИВЫ ADAPTER
// ============================================================================

// TODO: Добавить комментарии на русском языке
// TODO: Создать дополнительные безопасные варианты
// TODO: Добавить тесты безопасности

// ----------------------------------------------------------------------------
// БЕЗОПАСНАЯ АЛЬТЕРНАТИВА 1: Adapter с безопасными типами
// ----------------------------------------------------------------------------

template<typename T>
class SecureAdapterV1 {
private:
    std::shared_ptr<T> adaptedObject;
    mutable std::mutex adapter_mutex;
    
public:
    SecureAdapterV1() {
        std::cout << "SecureAdapterV1 создан в потоке " << std::this_thread::get_id() << std::endl;
    }
    
    // Удаляем копирование и присваивание
    SecureAdapterV1(const SecureAdapterV1&) = delete;
    SecureAdapterV1& operator=(const SecureAdapterV1&) = delete;
    
    // Безопасная адаптация объекта
    void adapt(std::shared_ptr<T> obj) {
        std::lock_guard<std::mutex> lock(adapter_mutex);
        adaptedObject = obj;
    }
    
    // Безопасное получение адаптированного объекта
    std::shared_ptr<T> getAdapted() const {
        std::lock_guard<std::mutex> lock(adapter_mutex);
        return adaptedObject;
    }
    
    bool isValid() const {
        std::lock_guard<std::mutex> lock(adapter_mutex);
        return adaptedObject != nullptr;
    }
    
    ~SecureAdapterV1() {
        std::cout << "SecureAdapterV1 уничтожен" << std::endl;
    }
};

// ----------------------------------------------------------------------------
// БЕЗОПАСНАЯ АЛЬТЕРНАТИВА 2: Adapter с std::vector
// ----------------------------------------------------------------------------

class SecureAdapterV2 {
private:
    std::vector<char> buffer;
    mutable std::mutex buffer_mutex;
    
public:
    SecureAdapterV2() {
        std::cout << "SecureAdapterV2 создан в потоке " << std::this_thread::get_id() << std::endl;
    }
    
    // Удаляем копирование и присваивание
    SecureAdapterV2(const SecureAdapterV2&) = delete;
    SecureAdapterV2& operator=(const SecureAdapterV2&) = delete;
    
    // Безопасная адаптация данных
    void adaptData(const char* data, size_t length) {
        std::lock_guard<std::mutex> lock(buffer_mutex);
        
        // Безопасное добавление с проверкой размера
        size_t oldSize = buffer.size();
        buffer.resize(oldSize + length);
        
        // Безопасное копирование
        std::memcpy(buffer.data() + oldSize, data, length);
    }
    
    // Безопасная адаптация строки
    void adaptString(const std::string& str) {
        adaptData(str.c_str(), str.length());
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
    
    ~SecureAdapterV2() {
        std::cout << "SecureAdapterV2 уничтожен" << std::endl;
    }
};

// ----------------------------------------------------------------------------
// БЕЗОПАСНАЯ АЛЬТЕРНАТИВА 3: Adapter с проверкой размеров
// ----------------------------------------------------------------------------

class SecureAdapterV3 {
private:
    std::vector<char> buffer;
    size_t maxSize;
    mutable std::mutex buffer_mutex;
    
public:
    SecureAdapterV3(size_t maxSize = 1024 * 1024) : maxSize(maxSize) {
        std::cout << "SecureAdapterV3 создан с максимальным размером " << maxSize << std::endl;
    }
    
    // Удаляем копирование и присваивание
    SecureAdapterV3(const SecureAdapterV3&) = delete;
    SecureAdapterV3& operator=(const SecureAdapterV3&) = delete;
    
    // Безопасная адаптация с проверкой размера
    bool adaptData(const char* data, size_t length) {
        std::lock_guard<std::mutex> lock(buffer_mutex);
        
        // Проверка на переполнение
        if (buffer.size() + length > maxSize) {
            std::cout << "ОШИБКА: Превышен максимальный размер буфера!" << std::endl;
            return false;
        }
        
        // Проверка на integer overflow
        if (buffer.size() > SIZE_MAX - length) {
            std::cout << "ОШИБКА: Integer overflow при адаптации данных!" << std::endl;
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
    
    ~SecureAdapterV3() {
        std::cout << "SecureAdapterV3 уничтожен" << std::endl;
    }
};

// ----------------------------------------------------------------------------
// БЕЗОПАСНАЯ АЛЬТЕРНАТИВА 4: Adapter с RAII и умными указателями
// ----------------------------------------------------------------------------

class SecureAdapterV4 {
private:
    std::unique_ptr<std::vector<char>> buffer;
    mutable std::mutex buffer_mutex;
    
public:
    SecureAdapterV4() : buffer(std::make_unique<std::vector<char>>()) {
        std::cout << "SecureAdapterV4 создан в потоке " << std::this_thread::get_id() << std::endl;
    }
    
    // Удаляем копирование и присваивание
    SecureAdapterV4(const SecureAdapterV4&) = delete;
    SecureAdapterV4& operator=(const SecureAdapterV4&) = delete;
    
    // Безопасная адаптация данных
    void adaptData(const char* data, size_t length) {
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
    
    ~SecureAdapterV4() {
        std::cout << "SecureAdapterV4 уничтожен" << std::endl;
    }
};

// ----------------------------------------------------------------------------
// ДЕМОНСТРАЦИЯ БЕЗОПАСНЫХ АЛЬТЕРНАТИВ
// ----------------------------------------------------------------------------

void demonstrateSecureAdapterV1() {
    std::cout << "\n=== ДЕМОНСТРАЦИЯ SecureAdapterV1 (безопасные типы) ===" << std::endl;
    
    SecureAdapterV1<int> adapter;
    
    // Безопасная адаптация объекта
    auto intObj = std::make_shared<int>(42);
    adapter.adapt(intObj);
    
    std::cout << "Объект адаптирован" << std::endl;
    std::cout << "Валидность: " << (adapter.isValid() ? "Да" : "Нет") << std::endl;
    
    auto adaptedObj = adapter.getAdapted();
    if (adaptedObj) {
        std::cout << "Адаптированный объект: " << *adaptedObj << std::endl;
    }
}

void demonstrateSecureAdapterV2() {
    std::cout << "\n=== ДЕМОНСТРАЦИЯ SecureAdapterV2 (std::vector) ===" << std::endl;
    
    SecureAdapterV2 adapter;
    
    // Безопасная адаптация данных
    adapter.adaptData("Hello, ", 7);
    adapter.adaptString("World!");
    
    std::cout << "Размер данных: " << adapter.getSize() << std::endl;
    
    auto data = adapter.getData();
    std::cout << "Данные: ";
    for (char c : data) {
        std::cout << c;
    }
    std::cout << std::endl;
}

void demonstrateSecureAdapterV3() {
    std::cout << "\n=== ДЕМОНСТРАЦИЯ SecureAdapterV3 (с проверкой размеров) ===" << std::endl;
    
    SecureAdapterV3 adapter(1000); // Максимальный размер 1000 байт
    
    // Нормальная адаптация
    bool success1 = adapter.adaptData("Hello", 5);
    std::cout << "Адаптация 'Hello': " << (success1 ? "Успех" : "Неудача") << std::endl;
    
    // Попытка превысить лимит
    std::string largeData(2000, 'A');
    bool success2 = adapter.adaptData(largeData.c_str(), largeData.length());
    std::cout << "Адаптация больших данных: " << (success2 ? "Успех" : "Неудача") << std::endl;
    
    std::cout << "Текущий размер: " << adapter.getSize() << std::endl;
    std::cout << "Максимальный размер: " << adapter.getMaxSize() << std::endl;
}

void demonstrateSecureAdapterV4() {
    std::cout << "\n=== ДЕМОНСТРАЦИЯ SecureAdapterV4 (RAII) ===" << std::endl;
    
    SecureAdapterV4 adapter;
    
    // Безопасная адаптация данных
    adapter.adaptData("RAII ", 5);
    adapter.adaptData("Adapter", 7);
    
    std::cout << "Размер данных: " << adapter.getSize() << std::endl;
    
    // Безопасный сброс
    adapter.reset();
    std::cout << "Размер после сброса: " << adapter.getSize() << std::endl;
}

// ----------------------------------------------------------------------------
// ОСНОВНАЯ ФУНКЦИЯ
// ----------------------------------------------------------------------------

int main() {
    std::cout << "=== ДЕМОНСТРАЦИЯ БЕЗОПАСНЫХ АЛЬТЕРНАТИВ ADAPTER ===" << std::endl;
    
    // Демонстрация различных безопасных реализаций
    demonstrateSecureAdapterV1();
    demonstrateSecureAdapterV2();
    demonstrateSecureAdapterV3();
    demonstrateSecureAdapterV4();
    
    std::cout << "\n=== РЕКОМЕНДАЦИИ ПО БЕЗОПАСНОСТИ ===" << std::endl;
    std::cout << "1. Используйте безопасные типы и шаблоны" << std::endl;
    std::cout << "2. Проверяйте типы перед приведением" << std::endl;
    std::cout << "3. Применяйте RAII и умные указатели" << std::endl;
    std::cout << "4. Используйте мьютексы для многопоточности" << std::endl;
    std::cout << "5. Валидируйте входные данные" << std::endl;
    std::cout << "6. Ограничивайте максимальные размеры" << std::endl;
    std::cout << "7. Регулярно анализируйте код с помощью инструментов безопасности" << std::endl;
    
    return 0;
}