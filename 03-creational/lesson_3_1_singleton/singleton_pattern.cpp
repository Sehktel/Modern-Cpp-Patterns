/**
 * @file singleton_pattern.cpp
 * @brief Демонстрация различных реализаций паттерна Singleton
 * 
 * Этот файл показывает эволюцию Singleton от классической (проблемной)
 * реализации до современных thread-safe подходов.
 * 
 * @author Sehktel
 * @license MIT License
 * @copyright Copyright (c) 2025 Sehktel
 * @version 1.0
 */

#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <chrono>

// ============================================================================
// КЛАССИЧЕСКИЙ SINGLETON (ПРОБЛЕМНЫЙ)
// ============================================================================

/**
 * @brief Классическая реализация Singleton (НЕ РЕКОМЕНДУЕТСЯ!)
 * 
 * Проблемы:
 * - Не thread-safe
 * - Утечка памяти (нет delete)
 * - Сложно тестировать
 * - Нарушает принципы SOLID
 */
class ClassicSingleton {
private:
    static ClassicSingleton* instance_;
    std::string data_;
    
    ClassicSingleton() : data_("Classic Singleton Data") {
        std::cout << "ClassicSingleton: Создан экземпляр" << std::endl;
    }
    
public:
    static ClassicSingleton* getInstance() {
        if (!instance_) {
            instance_ = new ClassicSingleton();  // Утечка памяти!
        }
        return instance_;
    }
    
    const std::string& getData() const { return data_; }
    void setData(const std::string& data) { data_ = data; }
    
    // ПРОБЛЕМА: Нет деструктора, который бы освободил память
    // ПРОБЛЕМА: Не thread-safe (race condition в getInstance)
};

// Статическая переменная
ClassicSingleton* ClassicSingleton::instance_ = nullptr;

// ============================================================================
// THREAD-SAFE SINGLETON С МЬЮТЕКСОМ
// ============================================================================

/**
 * @brief Thread-safe Singleton с использованием мьютекса
 * 
 * Улучшения:
 * - Thread-safe
 * - Автоматическое управление памятью
 * - Правильное уничтожение
 * 
 * Недостатки:
 * - Накладные расходы на блокировку при каждом вызове
 * - Двойная проверка блокировки
 */
class ThreadSafeSingleton {
private:
    static std::unique_ptr<ThreadSafeSingleton> instance_;
    static std::mutex mutex_;
    std::string data_;
    
    ThreadSafeSingleton() : data_("Thread-Safe Singleton Data") {
        std::cout << "ThreadSafeSingleton: Создан экземпляр" << std::endl;
    }
    
public:
    static ThreadSafeSingleton& getInstance() {
        // Двойная проверка блокировки (Double-Checked Locking)
        if (!instance_) {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!instance_) {
                instance_ = std::make_unique<ThreadSafeSingleton>();
            }
        }
        return *instance_;
    }
    
    const std::string& getData() const { return data_; }
    void setData(const std::string& data) { data_ = data; }
    
    // Запрещаем копирование и присваивание
    ThreadSafeSingleton(const ThreadSafeSingleton&) = delete;
    ThreadSafeSingleton& operator=(const ThreadSafeSingleton&) = delete;
};

// Статические переменные
std::unique_ptr<ThreadSafeSingleton> ThreadSafeSingleton::instance_ = nullptr;
std::mutex ThreadSafeSingleton::mutex_;

// ============================================================================
// MEYERS SINGLETON (РЕКОМЕНДУЕМЫЙ)
// ============================================================================

/**
 * @brief Singleton по Scott Meyers (РЕКОМЕНДУЕТСЯ!)
 * 
 * Преимущества:
 * - Thread-safe с C++11 (гарантируется стандартом)
 * - Ленивая инициализация
 * - Автоматическое управление памятью
 * - Нет накладных расходов на блокировки
 * - Простая реализация
 * 
 * Принцип: static переменная в функции инициализируется только один раз
 * и гарантированно thread-safe с C++11.
 */
class MeyersSingleton {
private:
    std::string data_;
    
    MeyersSingleton() : data_("Meyers Singleton Data") {
        std::cout << "MeyersSingleton: Создан экземпляр" << std::endl;
    }
    
public:
    static MeyersSingleton& getInstance() {
        static MeyersSingleton instance;  // Thread-safe с C++11!
        return instance;
    }
    
    const std::string& getData() const { return data_; }
    void setData(const std::string& data) { data_ = data; }
    
    // Запрещаем копирование и присваивание
    MeyersSingleton(const MeyersSingleton&) = delete;
    MeyersSingleton& operator=(const MeyersSingleton&) = delete;
};

// ============================================================================
// SINGLETON С std::call_once
// ============================================================================

/**
 * @brief Singleton с использованием std::call_once
 * 
 * Преимущества:
 * - Гарантированная однократная инициализация
 * - Thread-safe
 * - Ленивая инициализация
 * 
 * Используется когда нужен более явный контроль над инициализацией
 */
class CallOnceSingleton {
private:
    static std::unique_ptr<CallOnceSingleton> instance_;
    static std::once_flag initialized_;
    std::string data_;
    
    CallOnceSingleton() : data_("Call-Once Singleton Data") {
        std::cout << "CallOnceSingleton: Создан экземпляр" << std::endl;
    }
    
public:
    static CallOnceSingleton& getInstance() {
        std::call_once(initialized_, []() {
            instance_ = std::make_unique<CallOnceSingleton>();
        });
        return *instance_;
    }
    
    const std::string& getData() const { return data_; }
    void setData(const std::string& data) { data_ = data; }
    
    // Запрещаем копирование и присваивание
    CallOnceSingleton(const CallOnceSingleton&) = delete;
    CallOnceSingleton& operator=(const CallOnceSingleton&) = delete;
};

// Статические переменные
std::unique_ptr<CallOnceSingleton> CallOnceSingleton::instance_ = nullptr;
std::once_flag CallOnceSingleton::initialized_;

// ============================================================================
// ПРАКТИЧЕСКИЙ ПРИМЕР: ЛОГГЕР
// ============================================================================

/**
 * @brief Практический пример - Singleton логгер
 * 
 * Это один из случаев, где Singleton может быть оправдан:
 * - Глобальный доступ к логгеру
 * - Один экземпляр на все приложение
 * - Простота использования
 */
class Logger {
private:
    std::string logFile_;
    bool isInitialized_;
    
    Logger() : logFile_("application.log"), isInitialized_(false) {
        std::cout << "Logger: Инициализация логгера" << std::endl;
        isInitialized_ = true;
    }
    
public:
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }
    
    void log(const std::string& level, const std::string& message) {
        if (isInitialized_) {
            std::cout << "[" << level << "] " << message << std::endl;
            // В реальном приложении здесь была бы запись в файл
        }
    }
    
    void info(const std::string& message) {
        log("INFO", message);
    }
    
    void warning(const std::string& message) {
        log("WARN", message);
    }
    
    void error(const std::string& message) {
        log("ERROR", message);
    }
    
    // Запрещаем копирование и присваивание
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
};

// ============================================================================
// ДЕМОНСТРАЦИОННЫЕ ФУНКЦИИ
// ============================================================================

/**
 * @brief Демонстрация классического Singleton
 */
void demonstrateClassicSingleton() {
    std::cout << "\n=== Классический Singleton (проблемный) ===" << std::endl;
    
    auto* singleton1 = ClassicSingleton::getInstance();
    auto* singleton2 = ClassicSingleton::getInstance();
    
    std::cout << "singleton1 == singleton2: " << (singleton1 == singleton2) << std::endl;
    std::cout << "Данные: " << singleton1->getData() << std::endl;
    
    singleton1->setData("Измененные данные");
    std::cout << "Данные через singleton2: " << singleton2->getData() << std::endl;
    
    // ПРОБЛЕМА: Память никогда не освобождается!
    std::cout << "⚠️  ВНИМАНИЕ: Утечка памяти! delete не вызывается." << std::endl;
}

/**
 * @brief Демонстрация thread-safe Singleton
 */
void demonstrateThreadSafeSingleton() {
    std::cout << "\n=== Thread-Safe Singleton ===" << std::endl;
    
    auto& singleton1 = ThreadSafeSingleton::getInstance();
    auto& singleton2 = ThreadSafeSingleton::getInstance();
    
    std::cout << "singleton1 == singleton2: " << (&singleton1 == &singleton2) << std::endl;
    std::cout << "Данные: " << singleton1.getData() << std::endl;
    
    singleton1.setData("Thread-safe данные");
    std::cout << "Данные через singleton2: " << singleton2.getData() << std::endl;
}

/**
 * @brief Демонстрация Meyers Singleton
 */
void demonstrateMeyersSingleton() {
    std::cout << "\n=== Meyers Singleton (рекомендуемый) ===" << std::endl;
    
    auto& singleton1 = MeyersSingleton::getInstance();
    auto& singleton2 = MeyersSingleton::getInstance();
    
    std::cout << "singleton1 == singleton2: " << (&singleton1 == &singleton2) << std::endl;
    std::cout << "Данные: " << singleton1.getData() << std::endl;
    
    singleton1.setData("Meyers данные");
    std::cout << "Данные через singleton2: " << singleton2.getData() << std::endl;
}

/**
 * @brief Демонстрация Call-Once Singleton
 */
void demonstrateCallOnceSingleton() {
    std::cout << "\n=== Call-Once Singleton ===" << std::endl;
    
    auto& singleton1 = CallOnceSingleton::getInstance();
    auto& singleton2 = CallOnceSingleton::getInstance();
    
    std::cout << "singleton1 == singleton2: " << (&singleton1 == &singleton2) << std::endl;
    std::cout << "Данные: " << singleton1.getData() << std::endl;
    
    singleton1.setData("Call-once данные");
    std::cout << "Данные через singleton2: " << singleton2.getData() << std::endl;
}

/**
 * @brief Демонстрация практического использования логгера
 */
void demonstrateLogger() {
    std::cout << "\n=== Практический пример: Logger ===" << std::endl;
    
    auto& logger = Logger::getInstance();
    
    logger.info("Приложение запущено");
    logger.warning("Предупреждение: низкая память");
    logger.error("Ошибка: не удалось подключиться к базе данных");
    
    // Логгер доступен из любого места в программе
    auto& sameLogger = Logger::getInstance();
    sameLogger.info("Это тот же логгер");
}

/**
 * @brief Демонстрация многопоточности
 */
void demonstrateMultithreading() {
    std::cout << "\n=== Многопоточность и Singleton ===" << std::endl;
    
    const int numThreads = 5;
    std::vector<std::thread> threads;
    
    std::cout << "Запускаем " << numThreads << " потоков..." << std::endl;
    
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([i]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(10 * i));
            
            auto& singleton = MeyersSingleton::getInstance();
            singleton.setData("Данные из потока " + std::to_string(i));
            
            std::cout << "Поток " << i << ": " << singleton.getData() << std::endl;
        });
    }
    
    // Ждем завершения всех потоков
    for (auto& thread : threads) {
        thread.join();
    }
    
    std::cout << "Финальные данные: " << MeyersSingleton::getInstance().getData() << std::endl;
}

// ============================================================================
// ОСНОВНАЯ ФУНКЦИЯ
// ============================================================================

int main() {
    std::cout << "🏗️ Демонстрация паттерна Singleton" << std::endl;
    std::cout << std::string(50, '=') << std::endl;
    
    demonstrateClassicSingleton();
    demonstrateThreadSafeSingleton();
    demonstrateMeyersSingleton();
    demonstrateCallOnceSingleton();
    demonstrateLogger();
    demonstrateMultithreading();
    
    std::cout << "\n✅ Демонстрация Singleton завершена!" << std::endl;
    std::cout << "\n🎯 Ключевые выводы:" << std::endl;
    std::cout << "• Meyers Singleton - лучший выбор для большинства случаев" << std::endl;
    std::cout << "• Thread-safe с C++11 без накладных расходов" << std::endl;
    std::cout << "• Рассмотрите альтернативы: Dependency Injection" << std::endl;
    std::cout << "• Singleton нарушает принципы SOLID" << std::endl;
    std::cout << "• Используйте только когда действительно нужен единственный экземпляр" << std::endl;
    
    return 0;
}
