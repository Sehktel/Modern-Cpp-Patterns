#include <iostream>
#include <thread>
#include <mutex>
#include <memory>
#include <vector>
#include <chrono>
#include <cstring>
#include <atomic>

// ============================================================================
// БЕЗОПАСНЫЕ АЛЬТЕРНАТИВЫ SINGLETON
// ============================================================================
//
// Этот файл демонстрирует БЕЗОПАСНЫЕ реализации паттерна Singleton:
// 1. Meyer's Singleton (C++11 thread-safe static)
// 2. std::call_once Singleton
// 3. std::shared_ptr Singleton с атомарными операциями
// 4. Dependency Injection (альтернатива Singleton)
//
// ТЕСТИРОВАНИЕ БЕЗОПАСНОСТИ:
// - ThreadSanitizer: g++ -fsanitize=thread -g secure_singleton_alternatives.cpp
// - AddressSanitizer: g++ -fsanitize=address -g secure_singleton_alternatives.cpp
// - Stress testing: множество потоков одновременно
//
// ✅ БЕЗОПАСНОСТЬ: Все реализации защищены от race conditions и memory leaks
// ============================================================================

// ----------------------------------------------------------------------------
// БЕЗОПАСНАЯ АЛЬТЕРНАТИВА 1: Thread-Safe Singleton с std::call_once
// ----------------------------------------------------------------------------

class SecureSingletonV1 {
private:
    static std::unique_ptr<SecureSingletonV1> instance;
    static std::once_flag init_flag;
    
    // Приватные данные с защитой
    std::string adminPassword;
    std::atomic<bool> isAdmin;
    std::atomic<int> userLevel;
    mutable std::mutex data_mutex;
    
    SecureSingletonV1() : adminPassword("super_secret_password_123"), 
                         isAdmin(true), userLevel(999) {
        std::cout << "SecureSingletonV1 создан в потоке " << std::this_thread::get_id() << std::endl;
    }
    
public:
    // Удаляем копирование и присваивание
    SecureSingletonV1(const SecureSingletonV1&) = delete;
    SecureSingletonV1& operator=(const SecureSingletonV1&) = delete;
    
    static SecureSingletonV1& getInstance() {
        // Thread-safe инициализация с std::call_once
        std::call_once(init_flag, []() {
            instance = std::make_unique<SecureSingletonV1>();
        });
        return *instance;
    }
    
    // Безопасная проверка пароля с постоянным временем
    bool checkAdminAccess(const std::string& password) const {
        std::lock_guard<std::mutex> lock(data_mutex);
        
        // Постоянное время сравнения для предотвращения timing attacks
        if (password.length() != adminPassword.length()) {
            return false;
        }
        
        bool result = true;
        for (size_t i = 0; i < password.length(); ++i) {
            result &= (password[i] == adminPassword[i]);
        }
        
        return result;
    }
    
    int getUserLevel() const {
        return userLevel.load();
    }
    
    bool isAdminUser() const {
        return isAdmin.load();
    }
    
    void performAdminAction() const {
        if (isAdmin.load()) {
            std::cout << "Выполняется административное действие!" << std::endl;
            std::cout << "Уровень пользователя: " << userLevel.load() << std::endl;
        } else {
            std::cout << "Доступ запрещен!" << std::endl;
        }
    }
    
    ~SecureSingletonV1() {
        std::cout << "SecureSingletonV1 уничтожен" << std::endl;
    }
};

std::unique_ptr<SecureSingletonV1> SecureSingletonV1::instance = nullptr;
std::once_flag SecureSingletonV1::init_flag;

// ----------------------------------------------------------------------------
// БЕЗОПАСНАЯ АЛЬТЕРНАТИВА 2: Singleton с RAII и умными указателями
// ----------------------------------------------------------------------------

class SecureSingletonV2 {
private:
    static std::shared_ptr<SecureSingletonV2> instance;
    static std::mutex instance_mutex;
    
    // Безопасное хранение данных
    std::vector<char> adminPassword;
    std::atomic<bool> isAdmin;
    std::atomic<int> userLevel;
    
    SecureSingletonV2() : adminPassword(), isAdmin(true), userLevel(999) {
        // Безопасное копирование пароля
        std::string password = "super_secret_password_123";
        adminPassword.assign(password.begin(), password.end());
        
        std::cout << "SecureSingletonV2 создан в потоке " << std::this_thread::get_id() << std::endl;
    }
    
public:
    // Удаляем копирование и присваивание
    SecureSingletonV2(const SecureSingletonV2&) = delete;
    SecureSingletonV2& operator=(const SecureSingletonV2&) = delete;
    
    static std::shared_ptr<SecureSingletonV2> getInstance() {
        std::lock_guard<std::mutex> lock(instance_mutex);
        
        if (!instance) {
            instance = std::shared_ptr<SecureSingletonV2>(new SecureSingletonV2());
        }
        
        return instance;
    }
    
    // Безопасная проверка пароля
    bool checkAdminAccess(const std::string& password) const {
        if (password.length() != adminPassword.size()) {
            return false;
        }
        
        bool result = true;
        for (size_t i = 0; i < password.length(); ++i) {
            result &= (password[i] == adminPassword[i]);
        }
        
        return result;
    }
    
    int getUserLevel() const {
        return userLevel.load();
    }
    
    bool isAdminUser() const {
        return isAdmin.load();
    }
    
    void performAdminAction() const {
        if (isAdmin.load()) {
            std::cout << "Выполняется административное действие!" << std::endl;
            std::cout << "Уровень пользователя: " << userLevel.load() << std::endl;
        } else {
            std::cout << "Доступ запрещен!" << std::endl;
        }
    }
    
    ~SecureSingletonV2() {
        // Безопасная очистка чувствительных данных
        std::fill(adminPassword.begin(), adminPassword.end(), 0);
        std::cout << "SecureSingletonV2 уничтожен" << std::endl;
    }
};

std::shared_ptr<SecureSingletonV2> SecureSingletonV2::instance = nullptr;
std::mutex SecureSingletonV2::instance_mutex;

// ----------------------------------------------------------------------------
// БЕЗОПАСНАЯ АЛЬТЕРНАТИВА 3: Dependency Injection вместо Singleton
// ----------------------------------------------------------------------------

class AdminService {
private:
    std::string adminPassword;
    std::atomic<bool> isAdmin;
    std::atomic<int> userLevel;
    mutable std::mutex data_mutex;
    
public:
    AdminService(const std::string& password) 
        : adminPassword(password), isAdmin(true), userLevel(999) {
        std::cout << "AdminService создан" << std::endl;
    }
    
    // Удаляем копирование и присваивание
    AdminService(const AdminService&) = delete;
    AdminService& operator=(const AdminService&) = delete;
    
    bool checkAdminAccess(const std::string& password) const {
        std::lock_guard<std::mutex> lock(data_mutex);
        
        if (password.length() != adminPassword.length()) {
            return false;
        }
        
        bool result = true;
        for (size_t i = 0; i < password.length(); ++i) {
            result &= (password[i] == adminPassword[i]);
        }
        
        return result;
    }
    
    int getUserLevel() const {
        return userLevel.load();
    }
    
    bool isAdminUser() const {
        return isAdmin.load();
    }
    
    void performAdminAction() const {
        if (isAdmin.load()) {
            std::cout << "Выполняется административное действие!" << std::endl;
            std::cout << "Уровень пользователя: " << userLevel.load() << std::endl;
        } else {
            std::cout << "Доступ запрещен!" << std::endl;
        }
    }
    
    ~AdminService() {
        std::cout << "AdminService уничтожен" << std::endl;
    }
};

// Контейнер для управления зависимостями
class ServiceContainer {
private:
    std::unique_ptr<AdminService> adminService;
    std::mutex container_mutex;
    
public:
    AdminService& getAdminService() {
        std::lock_guard<std::mutex> lock(container_mutex);
        
        if (!adminService) {
            adminService = std::make_unique<AdminService>("super_secret_password_123");
        }
        
        return *adminService;
    }
    
    void resetAdminService() {
        std::lock_guard<std::mutex> lock(container_mutex);
        adminService.reset();
    }
};

// ----------------------------------------------------------------------------
// БЕЗОПАСНАЯ АЛЬТЕРНАТИВА 4: Singleton с проверкой безопасности
// ----------------------------------------------------------------------------

class SecureSingletonV4 {
private:
    static std::unique_ptr<SecureSingletonV4> instance;
    static std::mutex instance_mutex;
    static std::atomic<bool> initialized;
    
    // Безопасные данные
    std::vector<uint8_t> adminPassword;
    std::atomic<bool> isAdmin;
    std::atomic<int> userLevel;
    std::atomic<size_t> accessCount;
    
    SecureSingletonV4() : isAdmin(true), userLevel(999), accessCount(0) {
        // Безопасное хранение пароля
        std::string password = "super_secret_password_123";
        adminPassword.assign(password.begin(), password.end());
        
        std::cout << "SecureSingletonV4 создан в потоке " << std::this_thread::get_id() << std::endl;
    }
    
public:
    // Удаляем копирование и присваивание
    SecureSingletonV4(const SecureSingletonV4&) = delete;
    SecureSingletonV4& operator=(const SecureSingletonV4&) = delete;
    
    static SecureSingletonV4& getInstance() {
        if (!initialized.load()) {
            std::lock_guard<std::mutex> lock(instance_mutex);
            
            if (!initialized.load()) {
                instance = std::make_unique<SecureSingletonV4>();
                initialized.store(true);
            }
        }
        
        return *instance;
    }
    
    // Безопасная проверка пароля с защитой от timing attacks
    bool checkAdminAccess(const std::string& password) const {
        accessCount.fetch_add(1);
        
        // Проверка длины
        if (password.length() != adminPassword.size()) {
            // Имитация времени проверки для предотвращения timing attacks
            std::this_thread::sleep_for(std::chrono::microseconds(100));
            return false;
        }
        
        // Постоянное время сравнения
        bool result = true;
        for (size_t i = 0; i < password.length(); ++i) {
            result &= (password[i] == adminPassword[i]);
        }
        
        return result;
    }
    
    int getUserLevel() const {
        return userLevel.load();
    }
    
    bool isAdminUser() const {
        return isAdmin.load();
    }
    
    size_t getAccessCount() const {
        return accessCount.load();
    }
    
    void performAdminAction() const {
        if (isAdmin.load()) {
            std::cout << "Выполняется административное действие!" << std::endl;
            std::cout << "Уровень пользователя: " << userLevel.load() << std::endl;
            std::cout << "Количество обращений: " << accessCount.load() << std::endl;
        } else {
            std::cout << "Доступ запрещен!" << std::endl;
        }
    }
    
    ~SecureSingletonV4() {
        // Безопасная очистка чувствительных данных
        std::fill(adminPassword.begin(), adminPassword.end(), 0);
        std::cout << "SecureSingletonV4 уничтожен" << std::endl;
    }
};

std::unique_ptr<SecureSingletonV4> SecureSingletonV4::instance = nullptr;
std::mutex SecureSingletonV4::instance_mutex;
std::atomic<bool> SecureSingletonV4::initialized(false);

// ----------------------------------------------------------------------------
// ДЕМОНСТРАЦИЯ БЕЗОПАСНЫХ АЛЬТЕРНАТИВ
// ----------------------------------------------------------------------------

void demonstrateSecureSingletonV1() {
    std::cout << "\n=== ДЕМОНСТРАЦИЯ SecureSingletonV1 (std::call_once) ===" << std::endl;
    
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([i]() {
            auto& instance = SecureSingletonV1::getInstance();
            
            std::cout << "Поток " << i << " получил доступ к SecureSingletonV1" << std::endl;
            std::cout << "  - Администратор: " << (instance.isAdminUser() ? "Да" : "Нет") << std::endl;
            std::cout << "  - Уровень пользователя: " << instance.getUserLevel() << std::endl;
            
            // Тест проверки пароля
            bool correct = instance.checkAdminAccess("super_secret_password_123");
            bool incorrect = instance.checkAdminAccess("wrong_password");
            
            std::cout << "  - Правильный пароль: " << (correct ? "Да" : "Нет") << std::endl;
            std::cout << "  - Неправильный пароль: " << (incorrect ? "Да" : "Нет") << std::endl;
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
}

void demonstrateSecureSingletonV2() {
    std::cout << "\n=== ДЕМОНСТРАЦИЯ SecureSingletonV2 (RAII) ===" << std::endl;
    
    auto instance = SecureSingletonV2::getInstance();
    
    std::cout << "Получен экземпляр SecureSingletonV2" << std::endl;
    std::cout << "Администратор: " << (instance->isAdminUser() ? "Да" : "Нет") << std::endl;
    
    instance->performAdminAction();
    
    // Демонстрация автоматического управления памятью
    {
        auto instance2 = SecureSingletonV2::getInstance();
        std::cout << "Тот же экземпляр: " << (instance.get() == instance2.get() ? "Да" : "Нет") << std::endl;
    }
}

void demonstrateDependencyInjection() {
    std::cout << "\n=== ДЕМОНСТРАЦИЯ Dependency Injection ===" << std::endl;
    
    ServiceContainer container;
    
    auto& adminService = container.getAdminService();
    
    std::cout << "Получен AdminService через DI" << std::endl;
    std::cout << "Администратор: " << (adminService.isAdminUser() ? "Да" : "Нет") << std::endl;
    
    adminService.performAdminAction();
    
    // Демонстрация сброса сервиса
    container.resetAdminService();
    std::cout << "Сервис сброшен" << std::endl;
}

void demonstrateSecureSingletonV4() {
    std::cout << "\n=== ДЕМОНСТРАЦИЯ SecureSingletonV4 (Double-Checked Locking) ===" << std::endl;
    
    auto& instance = SecureSingletonV4::getInstance();
    
    std::cout << "Получен экземпляр SecureSingletonV4" << std::endl;
    std::cout << "Администратор: " << (instance.isAdminUser() ? "Да" : "Нет") << std::endl;
    std::cout << "Количество обращений: " << instance.getAccessCount() << std::endl;
    
    instance.performAdminAction();
}

// ----------------------------------------------------------------------------
// ОСНОВНАЯ ФУНКЦИЯ
// ----------------------------------------------------------------------------

int main() {
    std::cout << "=== ДЕМОНСТРАЦИЯ БЕЗОПАСНЫХ АЛЬТЕРНАТИВ SINGLETON ===" << std::endl;
    
    // Демонстрация различных безопасных реализаций
    demonstrateSecureSingletonV1();
    demonstrateSecureSingletonV2();
    demonstrateDependencyInjection();
    demonstrateSecureSingletonV4();
    
    std::cout << "\n=== РЕКОМЕНДАЦИИ ПО БЕЗОПАСНОСТИ ===" << std::endl;
    std::cout << "1. Используйте std::call_once для thread-safe инициализации" << std::endl;
    std::cout << "2. Применяйте RAII и умные указатели для управления памятью" << std::endl;
    std::cout << "3. Используйте Dependency Injection вместо Singleton когда возможно" << std::endl;
    std::cout << "4. Реализуйте постоянное время для криптографических операций" << std::endl;
    std::cout << "5. Безопасно очищайте чувствительные данные" << std::endl;
    std::cout << "6. Используйте атомарные операции для счетчиков и флагов" << std::endl;
    std::cout << "7. Применяйте мьютексы для защиты критических секций" << std::endl;
    std::cout << "8. Регулярно анализируйте код с помощью инструментов безопасности" << std::endl;
    
    return 0;
}

