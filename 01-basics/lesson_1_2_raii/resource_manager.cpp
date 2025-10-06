#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <chrono>
#include <thread>

/**
 * @file resource_manager.cpp
 * @brief Продвинутые примеры управления ресурсами с RAII
 * 
 * Этот файл демонстрирует более сложные сценарии использования RAII
 * в реальных приложениях.
 */

// ============================================================================
// СИМУЛЯЦИЯ СИСТЕМНОГО РЕСУРСА
// ============================================================================

/**
 * @brief Симуляция системного ресурса (например, handle в Windows API)
 */
class SystemResource {
private:
    int handle_;
    std::string resourceName_;
    static int nextHandle_;
    
public:
    explicit SystemResource(const std::string& name) 
        : handle_(++nextHandle_), resourceName_(name) {
        std::cout << "SystemResource: Создан ресурс '" << name 
                  << "' с handle " << handle_ << std::endl;
        
        // Симуляция возможной ошибки при создании ресурса
        if (name.empty()) {
            throw std::invalid_argument("Имя ресурса не может быть пустым");
        }
    }
    
    ~SystemResource() {
        std::cout << "SystemResource: Освобожден ресурс '" << resourceName_ 
                  << "' с handle " << handle_ << std::endl;
    }
    
    // Запрещаем копирование
    SystemResource(const SystemResource&) = delete;
    SystemResource& operator=(const SystemResource&) = delete;
    
    // Разрешаем перемещение
    SystemResource(SystemResource&& other) noexcept 
        : handle_(other.handle_), resourceName_(std::move(other.resourceName_)) {
        other.handle_ = -1;
        std::cout << "SystemResource: Перемещение ресурса '" << resourceName_ << "'" << std::endl;
    }
    
    SystemResource& operator=(SystemResource&& other) noexcept {
        if (this != &other) {
            handle_ = other.handle_;
            resourceName_ = std::move(other.resourceName_);
            other.handle_ = -1;
            std::cout << "SystemResource: Перемещение ресурса с присваиванием" << std::endl;
        }
        return *this;
    }
    
    int getHandle() const { return handle_; }
    const std::string& getName() const { return resourceName_; }
    bool isValid() const { return handle_ != -1; }
};

// Статическая переменная для генерации уникальных handle
int SystemResource::nextHandle_ = 0;

// ============================================================================
// МЕНЕДЖЕР РЕСУРСОВ
// ============================================================================

/**
 * @brief Менеджер ресурсов с RAII
 * 
 * Демонстрирует:
 * - Управление коллекцией ресурсов
 * - Exception safety при работе с коллекциями
 * - Автоматическое освобождение всех ресурсов
 */
class ResourceManager {
private:
    std::vector<std::unique_ptr<SystemResource>> resources_;
    std::string managerName_;
    
public:
    explicit ResourceManager(const std::string& name) : managerName_(name) {
        std::cout << "ResourceManager: Создан менеджер '" << name << "'" << std::endl;
    }
    
    ~ResourceManager() {
        std::cout << "ResourceManager: Уничтожение менеджера '" << managerName_ 
                  << "' с " << resources_.size() << " ресурсами" << std::endl;
        // Все ресурсы автоматически освободятся при уничтожении unique_ptr
    }
    
    // Запрещаем копирование
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    
    // Разрешаем перемещение
    ResourceManager(ResourceManager&& other) noexcept 
        : resources_(std::move(other.resources_)), managerName_(std::move(other.managerName_)) {
        std::cout << "ResourceManager: Перемещение менеджера" << std::endl;
    }
    
    ResourceManager& operator=(ResourceManager&& other) noexcept {
        if (this != &other) {
            resources_ = std::move(other.resources_);
            managerName_ = std::move(other.managerName_);
            std::cout << "ResourceManager: Перемещение менеджера с присваиванием" << std::endl;
        }
        return *this;
    }
    
    /**
     * @brief Добавляет новый ресурс в менеджер
     * @param name Имя ресурса
     * @return Ссылка на созданный ресурс
     */
    SystemResource& addResource(const std::string& name) {
        auto resource = std::make_unique<SystemResource>(name);
        SystemResource& ref = *resource;
        resources_.push_back(std::move(resource));
        
        std::cout << "ResourceManager: Добавлен ресурс '" << name << "'" << std::endl;
        return ref;
    }
    
    /**
     * @brief Удаляет ресурс по имени
     * @param name Имя ресурса для удаления
     * @return true если ресурс найден и удален
     */
    bool removeResource(const std::string& name) {
        auto it = std::find_if(resources_.begin(), resources_.end(),
            [&name](const std::unique_ptr<SystemResource>& resource) {
                return resource->getName() == name;
            });
        
        if (it != resources_.end()) {
            std::cout << "ResourceManager: Удаляем ресурс '" << name << "'" << std::endl;
            resources_.erase(it);
            return true;
        }
        
        return false;
    }
    
    /**
     * @brief Находит ресурс по имени
     * @param name Имя ресурса
     * @return Указатель на ресурс или nullptr
     */
    SystemResource* findResource(const std::string& name) {
        auto it = std::find_if(resources_.begin(), resources_.end(),
            [&name](const std::unique_ptr<SystemResource>& resource) {
                return resource->getName() == name;
            });
        
        return (it != resources_.end()) ? it->get() : nullptr;
    }
    
    /**
     * @brief Возвращает количество ресурсов
     */
    size_t getResourceCount() const {
        return resources_.size();
    }
    
    /**
     * @brief Выводит информацию о всех ресурсах
     */
    void printResources() const {
        std::cout << "ResourceManager '" << managerName_ << "' содержит " 
                  << resources_.size() << " ресурсов:" << std::endl;
        
        for (const auto& resource : resources_) {
            std::cout << "  - " << resource->getName() 
                      << " (handle: " << resource->getHandle() << ")" << std::endl;
        }
    }
    
    /**
     * @brief Очищает все ресурсы
     */
    void clear() {
        std::cout << "ResourceManager: Очистка всех ресурсов" << std::endl;
        resources_.clear();
    }
};

// ============================================================================
// RAII ДЛЯ ТАЙМЕРОВ
// ============================================================================

/**
 * @brief RAII wrapper для измерения времени выполнения
 */
class Timer {
private:
    std::chrono::high_resolution_clock::time_point startTime_;
    std::string operationName_;
    
public:
    explicit Timer(const std::string& operation) 
        : startTime_(std::chrono::high_resolution_clock::now()), operationName_(operation) {
        std::cout << "Timer: Начинаем измерение '" << operation << "'" << std::endl;
    }
    
    ~Timer() {
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime_);
        
        std::cout << "Timer: Операция '" << operationName_ 
                  << "' заняла " << duration.count() << " мс" << std::endl;
    }
    
    // Запрещаем копирование и перемещение
    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;
    Timer(Timer&&) = delete;
    Timer& operator=(Timer&&) = delete;
};

// ============================================================================
// RAII ДЛЯ БЛОКИРОВКИ С АВТОМАТИЧЕСКИМ ОСВОБОЖДЕНИЕМ
// ============================================================================

/**
 * @brief RAII wrapper для блокировки с таймаутом
 */
class LockWithTimeout {
private:
    bool locked_;
    std::string lockName_;
    
public:
    explicit LockWithTimeout(const std::string& name) : locked_(false), lockName_(name) {
        std::cout << "LockWithTimeout: Пытаемся заблокировать '" << name << "'" << std::endl;
        
        // Симуляция блокировки с таймаутом
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        locked_ = true;
        
        std::cout << "LockWithTimeout: Заблокировано '" << name << "'" << std::endl;
    }
    
    ~LockWithTimeout() {
        if (locked_) {
            std::cout << "LockWithTimeout: Разблокировано '" << lockName_ << "'" << std::endl;
        }
    }
    
    // Запрещаем копирование и перемещение
    LockWithTimeout(const LockWithTimeout&) = delete;
    LockWithTimeout& operator=(const LockWithTimeout&) = delete;
    LockWithTimeout(LockWithTimeout&&) = delete;
    LockWithTimeout& operator=(LockWithTimeout&&) = delete;
    
    bool isLocked() const { return locked_; }
};

// ============================================================================
// ДЕМОНСТРАЦИОННЫЕ ФУНКЦИИ
// ============================================================================

/**
 * @brief Демонстрация работы с менеджером ресурсов
 */
void demonstrateResourceManager() {
    std::cout << "\n=== Демонстрация менеджера ресурсов ===" << std::endl;
    
    try {
        ResourceManager manager("Database Connections");
        
        // Добавляем ресурсы
        auto& db1 = manager.addResource("Primary Database");
        auto& db2 = manager.addResource("Secondary Database");
        auto& cache = manager.addResource("Redis Cache");
        
        manager.printResources();
        
        // Работаем с ресурсами
        std::cout << "Работаем с ресурсом: " << db1.getName() 
                  << " (handle: " << db1.getHandle() << ")" << std::endl;
        
        // Удаляем один ресурс
        manager.removeResource("Secondary Database");
        manager.printResources();
        
        // При выходе из области видимости все ресурсы автоматически освободятся
        
    } catch (const std::exception& e) {
        std::cout << "Ошибка в менеджере ресурсов: " << e.what() << std::endl;
        // Все ресурсы все равно освободятся благодаря RAII
    }
}

/**
 * @brief Демонстрация автоматического измерения времени
 */
void demonstrateTimer() {
    std::cout << "\n=== Демонстрация автоматического таймера ===" << std::endl;
    
    {
        Timer timer("Создание большого массива");
        
        // Симуляция длительной операции
        std::vector<int> largeArray(1000000);
        std::iota(largeArray.begin(), largeArray.end(), 0);
        
        // Таймер автоматически выведет время выполнения при выходе из области видимости
    }
    
    {
        Timer timer("Сортировка массива");
        
        std::vector<int> numbers = {5, 2, 8, 1, 9, 3, 7, 4, 6};
        std::sort(numbers.begin(), numbers.end());
        
        // Время сортировки будет автоматически выведено
    }
}

/**
 * @brief Демонстрация блокировок с таймаутом
 */
void demonstrateLockWithTimeout() {
    std::cout << "\n=== Демонстрация блокировок с таймаутом ===" << std::endl;
    
    try {
        LockWithTimeout lock1("Critical Section 1");
        
        std::cout << "Выполняем критическую операцию 1..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        
        {
            LockWithTimeout lock2("Critical Section 2");
            
            std::cout << "Выполняем критическую операцию 2..." << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
            
            // lock2 автоматически освободится здесь
        }
        
        std::cout << "Продолжаем работу с lock1..." << std::endl;
        
        // lock1 автоматически освободится при выходе из области видимости
        
    } catch (const std::exception& e) {
        std::cout << "Ошибка в блокировке: " << e.what() << std::endl;
        // Блокировки все равно освободятся
    }
}

/**
 * @brief Демонстрация исключений в RAII
 */
void demonstrateExceptionsInRAII() {
    std::cout << "\n=== Демонстрация исключений в RAII ===" << std::endl;
    
    try {
        ResourceManager manager("Exception Test");
        
        // Добавляем несколько ресурсов
        manager.addResource("Resource 1");
        manager.addResource("Resource 2");
        
        std::cout << "Ресурсов до исключения: " << manager.getResourceCount() << std::endl;
        
        // Пытаемся создать ресурс с пустым именем (вызовет исключение)
        try {
            manager.addResource("");  // Это вызовет исключение
        } catch (const std::exception& e) {
            std::cout << "Поймали исключение: " << e.what() << std::endl;
        }
        
        std::cout << "Ресурсов после исключения: " << manager.getResourceCount() << std::endl;
        
        // При выходе из области видимости все ресурсы автоматически освободятся
        
    } catch (const std::exception& e) {
        std::cout << "Внешнее исключение: " << e.what() << std::endl;
    }
}

/**
 * @brief Демонстрация move semantics в менеджере ресурсов
 */
void demonstrateMoveSemanticsInManager() {
    std::cout << "\n=== Демонстрация Move Semantics в менеджере ===" << std::endl;
    
    // Создаем первый менеджер
    ResourceManager manager1("Original Manager");
    manager1.addResource("Resource A");
    manager1.addResource("Resource B");
    
    std::cout << "manager1 содержит " << manager1.getResourceCount() << " ресурсов" << std::endl;
    
    // Перемещаем ресурсы в новый менеджер
    ResourceManager manager2 = std::move(manager1);
    
    std::cout << "manager2 содержит " << manager2.getResourceCount() << " ресурсов" << std::endl;
    
    // manager1 теперь пустой, но безопасно уничтожается
    // manager2 владеет всеми ресурсами
}

// ============================================================================
// ОСНОВНАЯ ФУНКЦИЯ
// ============================================================================

int main() {
    std::cout << "🔧 Продвинутые примеры управления ресурсами с RAII" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    
    demonstrateResourceManager();
    demonstrateTimer();
    demonstrateLockWithTimeout();
    demonstrateExceptionsInRAII();
    demonstrateMoveSemanticsInManager();
    
    std::cout << "\n✅ Все демонстрации завершены!" << std::endl;
    std::cout << "Обратите внимание:" << std::endl;
    std::cout << "• Все ресурсы автоматически освобождаются" << std::endl;
    std::cout << "• Исключения не нарушают освобождение ресурсов" << std::endl;
    std::cout << "• Move semantics эффективно передает владение" << std::endl;
    std::cout << "• RAII обеспечивает предсказуемое управление ресурсами" << std::endl;
    
    return 0;
}
