/**
 * @file reactor_cpp23_comparison.cpp
 * @brief Сравнение Reactor Pattern: C++17/20 vs C++23
 * 
 * Этот файл демонстрирует различия между текущей реализацией
 * и улучшенной версией с использованием возможностей C++23.
 */

#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <unordered_map>
#include <functional>
#include <atomic>
#include <chrono>
#include <set>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

// ============================================================================
// C++17/20 РЕАЛИЗАЦИЯ (ТЕКУЩАЯ)
// ============================================================================

/**
 * @brief Текущая реализация Reactor (C++17/20)
 */
enum class ReactorEventType {
    READ,
    WRITE,
    ERROR,
    TIMEOUT
};

class EventHandler {
public:
    virtual ~EventHandler() = default;
    virtual void handleEvent(ReactorEventType event_type) = 0;
    virtual int getFileDescriptor() const = 0;
    virtual std::string getName() const = 0;
};

class ReactorCpp20 {
private:
    std::atomic<bool> running_{false};
    std::thread reactor_thread_;
    
    // ❌ C++17/20: std::unordered_map для обработчиков
    std::unordered_map<int, std::shared_ptr<EventHandler>> handlers_;
    std::mutex handlers_mutex_;
    
    // Статистика
    std::atomic<size_t> events_processed_{0};
    std::atomic<size_t> read_events_{0};
    std::atomic<size_t> write_events_{0};
    std::atomic<size_t> error_events_{0};
    
    // Таймаут для select
    struct timeval timeout_;
    
public:
    ReactorCpp20() {
        timeout_.tv_sec = 1;
        timeout_.tv_usec = 0;
        std::cout << "Reactor создан" << std::endl;
    }
    
    ~ReactorCpp20() {
        stop();
    }
    
    void start() {
        if (running_.load()) {
            std::cout << "Reactor уже запущен" << std::endl;
            return;
        }
        
        running_.store(true);
        reactor_thread_ = std::thread([this]() { runReactor(); });
        std::cout << "Reactor запущен" << std::endl;
    }
    
    void stop() {
        if (!running_.load()) return;
        
        std::cout << "Останавливаем Reactor..." << std::endl;
        running_.store(false);
        
        if (reactor_thread_.joinable()) {
            reactor_thread_.join();
        }
        
        printStats();
        std::cout << "Reactor остановлен" << std::endl;
    }
    
    // ❌ C++17/20: Простая регистрация без проверки ошибок
    void registerHandler(std::shared_ptr<EventHandler> handler) {
        std::lock_guard<std::mutex> lock(handlers_mutex_);
        
        int fd = handler->getFileDescriptor();
        handlers_[fd] = handler;
        
        std::cout << "Зарегистрирован обработчик " << handler->getName() 
                  << " для fd=" << fd << std::endl;
    }
    
    void unregisterHandler(int fd) {
        std::lock_guard<std::mutex> lock(handlers_mutex_);
        
        auto it = handlers_.find(fd);
        if (it != handlers_.end()) {
            std::cout << "Отменена регистрация обработчика для fd=" << fd << std::endl;
            handlers_.erase(it);
        }
    }
    
    // ❌ C++17/20: Простая статистика
    void printStats() const {
        std::cout << "\n=== Reactor Statistics ===" << std::endl;
        std::cout << "Всего событий обработано: " << events_processed_.load() << std::endl;
        std::cout << "Read событий: " << read_events_.load() << std::endl;
        std::cout << "Write событий: " << write_events_.load() << std::endl;
        std::cout << "Error событий: " << error_events_.load() << std::endl;
        std::cout << "=========================" << std::endl;
    }
    
private:
    void runReactor() {
        std::cout << "Reactor начал работу" << std::endl;
        
        while (running_.load()) {
            try {
                // Подготавливаем наборы файловых дескрипторов
                fd_set read_fds, write_fds, error_fds;
                FD_ZERO(&read_fds);
                FD_ZERO(&write_fds);
                FD_ZERO(&error_fds);
                
                int max_fd = 0;
                
                // Добавляем файловые дескрипторы в наборы
                {
                    std::lock_guard<std::mutex> lock(handlers_mutex_);
                    
                    for (const auto& pair : handlers_) {
                        int fd = pair.first;
                        FD_SET(fd, &read_fds);
                        FD_SET(fd, &write_fds);
                        FD_SET(fd, &error_fds);
                        
                        if (fd > max_fd) {
                            max_fd = fd;
                        }
                    }
                }
                
                // Если нет обработчиков, ждем немного
                if (max_fd == 0) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    continue;
                }
                
                // Вызываем select для демультиплексирования событий
                int result = select(max_fd + 1, &read_fds, &write_fds, &error_fds, &timeout_);
                
                if (result < 0) {
                    if (errno == EINTR) {
                        continue; // Перехвачен сигнал, продолжаем
                    }
                    std::cerr << "Ошибка select: " << strerror(errno) << std::endl;
                    break;
                }
                
                if (result == 0) {
                    // Таймаут - продолжаем работу
                    continue;
                }
                
                // Обрабатываем готовые события
                processEvents(read_fds, write_fds, error_fds);
                
            } catch (const std::exception& e) {
                std::cerr << "Ошибка в Reactor: " << e.what() << std::endl;
            }
        }
        
        std::cout << "Reactor завершил работу" << std::endl;
    }
    
    void processEvents(const fd_set& read_fds, const fd_set& write_fds, const fd_set& error_fds) {
        std::lock_guard<std::mutex> lock(handlers_mutex_);
        
        for (const auto& pair : handlers_) {
            int fd = pair.first;
            auto handler = pair.second;
            
            try {
                if (FD_ISSET(fd, &error_fds)) {
                    handler->handleEvent(ReactorEventType::ERROR);
                    error_events_.fetch_add(1);
                    events_processed_.fetch_add(1);
                } else if (FD_ISSET(fd, &read_fds)) {
                    handler->handleEvent(ReactorEventType::READ);
                    read_events_.fetch_add(1);
                    events_processed_.fetch_add(1);
                } else if (FD_ISSET(fd, &write_fds)) {
                    handler->handleEvent(ReactorEventType::WRITE);
                    write_events_.fetch_add(1);
                    events_processed_.fetch_add(1);
                }
            } catch (const std::exception& e) {
                std::cerr << "Ошибка в обработчике " << handler->getName() 
                          << ": " << e.what() << std::endl;
            }
        }
    }
};

// ============================================================================
// C++23 РЕАЛИЗАЦИЯ (УЛУЧШЕННАЯ)
// ============================================================================

// ✅ C++23: Новые заголовки
#ifdef __cpp_lib_expected
#include <expected>
#endif

#ifdef __cpp_lib_print
#include <print>
#endif

#ifdef __cpp_lib_flat_map
#include <flat_map>
#endif

#ifdef __cpp_lib_generator
#include <generator>
#endif

#ifdef __cpp_lib_stacktrace
#include <stacktrace>
#endif

/**
 * @brief Улучшенная реализация Reactor (C++23)
 */
struct ReactorEvent {
    int fd;
    ReactorEventType type;
    std::shared_ptr<EventHandler> handler;
};

class ReactorCpp23 {
private:
    std::atomic<bool> running_{false};
    std::thread reactor_thread_;
    
    // ✅ C++23: std::flat_map для более эффективного хранения
#ifdef __cpp_lib_flat_map
    std::flat_map<int, std::shared_ptr<EventHandler>> handlers_;
#else
    std::unordered_map<int, std::shared_ptr<EventHandler>> handlers_;
#endif
    
    std::mutex handlers_mutex_;
    
    // Статистика
    std::atomic<size_t> events_processed_{0};
    std::atomic<size_t> read_events_{0};
    std::atomic<size_t> write_events_{0};
    std::atomic<size_t> error_events_{0};
    
    // Таймаут для select
    struct timeval timeout_;
    
public:
    ReactorCpp23() {
        timeout_.tv_sec = 1;
        timeout_.tv_usec = 0;
        
#ifdef __cpp_lib_print
        std::print("Reactor создан\n");
#else
        std::cout << "Reactor создан" << std::endl;
#endif
    }
    
    ~ReactorCpp23() {
        stop();
    }
    
    void start() {
        if (running_.load()) {
#ifdef __cpp_lib_print
            std::print("Reactor уже запущен\n");
#else
            std::cout << "Reactor уже запущен" << std::endl;
#endif
            return;
        }
        
        running_.store(true);
        reactor_thread_ = std::thread([this]() { runReactor(); });
        
#ifdef __cpp_lib_print
        std::print("Reactor запущен\n");
#else
        std::cout << "Reactor запущен" << std::endl;
#endif
    }
    
    void stop() {
        if (!running_.load()) return;
        
#ifdef __cpp_lib_print
        std::print("Останавливаем Reactor...\n");
#else
        std::cout << "Останавливаем Reactor..." << std::endl;
#endif
        
        running_.store(false);
        
        if (reactor_thread_.joinable()) {
            reactor_thread_.join();
        }
        
        printStats();
        
#ifdef __cpp_lib_print
        std::print("Reactor остановлен\n");
#else
        std::cout << "Reactor остановлен" << std::endl;
#endif
    }
    
    // ✅ C++23: Регистрация с проверкой ошибок
    std::expected<void, std::string> registerHandler(std::shared_ptr<EventHandler> handler) {
        if (!handler) {
            return std::unexpected("Handler is null");
        }
        
        int fd = handler->getFileDescriptor();
        if (fd < 0) {
            return std::unexpected("Invalid file descriptor");
        }
        
        std::lock_guard<std::mutex> lock(handlers_mutex_);
        
        if (handlers_.contains(fd)) {
            return std::unexpected("Handler already registered for fd " + std::to_string(fd));
        }
        
        handlers_[fd] = handler;
        
#ifdef __cpp_lib_print
        std::print("✅ Зарегистрирован обработчик {} для fd={}\n", handler->getName(), fd);
#else
        std::cout << "✅ Зарегистрирован обработчик " << handler->getName() 
                  << " для fd=" << fd << std::endl;
#endif
        
        return {};
    }
    
    // ✅ C++23: Отмена регистрации с проверкой
    std::expected<void, std::string> unregisterHandler(int fd) {
        std::lock_guard<std::mutex> lock(handlers_mutex_);
        
        auto it = handlers_.find(fd);
        if (it == handlers_.end()) {
            return std::unexpected("Handler not found for fd " + std::to_string(fd));
        }
        
        handlers_.erase(it);
        
#ifdef __cpp_lib_print
        std::print("✅ Отменена регистрация обработчика для fd={}\n", fd);
#else
        std::cout << "✅ Отменена регистрация обработчика для fd=" << fd << std::endl;
#endif
        
        return {};
    }
    
    // ✅ C++23: Улучшенная статистика с форматированием
    void printStats() const {
#ifdef __cpp_lib_print
        std::print("\n=== Reactor Statistics ===\n");
        std::print("Всего событий обработано: {}\n", events_processed_.load());
        std::print("Read событий: {}\n", read_events_.load());
        std::print("Write событий: {}\n", write_events_.load());
        std::print("Error событий: {}\n", error_events_.load());
        std::print("Активных обработчиков: {}\n", handlers_.size());
        std::print("=========================\n");
#else
        std::cout << "\n=== Reactor Statistics ===" << std::endl;
        std::cout << "Всего событий обработано: " << events_processed_.load() << std::endl;
        std::cout << "Read событий: " << read_events_.load() << std::endl;
        std::cout << "Write событий: " << write_events_.load() << std::endl;
        std::cout << "Error событий: " << error_events_.load() << std::endl;
        std::cout << "Активных обработчиков: " << handlers_.size() << std::endl;
        std::cout << "=========================" << std::endl;
#endif
    }
    
    // ✅ C++23: Генератор событий
#ifdef __cpp_lib_generator
    std::generator<ReactorEvent> processEventsGenerator() {
        while (running_.load()) {
            fd_set read_fds, write_fds, error_fds;
            FD_ZERO(&read_fds);
            FD_ZERO(&write_fds);
            FD_ZERO(&error_fds);
            
            int max_fd = 0;
            
            {
                std::lock_guard<std::mutex> lock(handlers_mutex_);
                
                for (const auto& [fd, handler] : handlers_) {
                    FD_SET(fd, &read_fds);
                    FD_SET(fd, &write_fds);
                    FD_SET(fd, &error_fds);
                    
                    if (fd > max_fd) {
                        max_fd = fd;
                    }
                }
            }
            
            if (max_fd == 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }
            
            int result = select(max_fd + 1, &read_fds, &write_fds, &error_fds, &timeout_);
            
            if (result < 0) {
                if (errno == EINTR) continue;
                break;
            }
            
            if (result == 0) continue;
            
            // Генерируем события
            std::lock_guard<std::mutex> lock(handlers_mutex_);
            for (const auto& [fd, handler] : handlers_) {
                if (FD_ISSET(fd, &error_fds)) {
                    co_yield ReactorEvent{fd, ReactorEventType::ERROR, handler};
                } else if (FD_ISSET(fd, &read_fds)) {
                    co_yield ReactorEvent{fd, ReactorEventType::READ, handler};
                } else if (FD_ISSET(fd, &write_fds)) {
                    co_yield ReactorEvent{fd, ReactorEventType::WRITE, handler};
                }
            }
        }
    }
#endif
    
    // ✅ C++23: Получение детальной статистики
    struct DetailedStatistics {
        size_t totalEvents;
        size_t readEvents;
        size_t writeEvents;
        size_t errorEvents;
        size_t activeHandlers;
        double eventsPerSecond;
        std::chrono::system_clock::time_point startTime;
    };
    
    DetailedStatistics getDetailedStatistics() const {
        DetailedStatistics stats;
        stats.totalEvents = events_processed_.load();
        stats.readEvents = read_events_.load();
        stats.writeEvents = write_events_.load();
        stats.errorEvents = error_events_.load();
        stats.activeHandlers = handlers_.size();
        
        // В реальной реализации здесь бы вычислялись события в секунду
        stats.eventsPerSecond = 0.0;
        stats.startTime = std::chrono::system_clock::now();
        
        return stats;
    }
    
private:
    void runReactor() {
#ifdef __cpp_lib_print
        std::print("Reactor начал работу\n");
#else
        std::cout << "Reactor начал работу" << std::endl;
#endif
        
#ifdef __cpp_lib_generator
        // ✅ C++23: Использование генератора событий
        for (const auto& event : processEventsGenerator()) {
            try {
                event.handler->handleEvent(event.type);
                
                switch (event.type) {
                    case ReactorEventType::READ:
                        read_events_.fetch_add(1);
                        break;
                    case ReactorEventType::WRITE:
                        write_events_.fetch_add(1);
                        break;
                    case ReactorEventType::ERROR:
                        error_events_.fetch_add(1);
                        break;
                    default:
                        break;
                }
                
                events_processed_.fetch_add(1);
                
            } catch (const std::exception& e) {
#ifdef __cpp_lib_stacktrace
                auto trace = std::stacktrace::current();
                std::print("❌ Ошибка в обработчике {}: {}\n", event.handler->getName(), e.what());
                std::print("Stack trace:\n");
                for (const auto& frame : trace) {
                    std::print("  {}\n", frame.description());
                }
#else
                std::cerr << "❌ Ошибка в обработчике " << event.handler->getName() 
                          << ": " << e.what() << std::endl;
#endif
            }
        }
#else
        // Fallback для компиляторов без поддержки std::generator
        while (running_.load()) {
            // Стандартная реализация...
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
#endif
        
#ifdef __cpp_lib_print
        std::print("Reactor завершил работу\n");
#else
        std::cout << "Reactor завершил работу" << std::endl;
#endif
    }
};

// ============================================================================
// ПРОСТОЙ ОБРАБОТЧИК ДЛЯ ДЕМОНСТРАЦИИ
// ============================================================================

class SimpleEventHandler : public EventHandler {
private:
    int fd_;
    std::string name_;
    std::atomic<int> eventCount_{0};
    
public:
    SimpleEventHandler(int fd, const std::string& name) 
        : fd_(fd), name_(name) {}
    
    void handleEvent(ReactorEventType event_type) override {
        eventCount_.fetch_add(1);
        
#ifdef __cpp_lib_print
        std::print("[{}] Обработано событие {} (всего: {})\n", 
                   name_, static_cast<int>(event_type), eventCount_.load());
#else
        std::cout << "[" << name_ << "] Обработано событие " 
                  << static_cast<int>(event_type) << " (всего: " << eventCount_.load() << ")" << std::endl;
#endif
    }
    
    int getFileDescriptor() const override {
        return fd_;
    }
    
    std::string getName() const override {
        return name_;
    }
    
    int getEventCount() const {
        return eventCount_.load();
    }
};

// ============================================================================
// ДЕМОНСТРАЦИЯ РАЗЛИЧИЙ
// ============================================================================

void demonstrateDifferences() {
    std::cout << "\n=== СРАВНЕНИЕ REACTOR PATTERN: C++17/20 vs C++23 ===" << std::endl;
    
    // ============================================================================
    // C++17/20 ДЕМОНСТРАЦИЯ
    // ============================================================================
    std::cout << "\n--- C++17/20 Reactor ---" << std::endl;
    
    ReactorCpp20 reactor20;
    reactor20.start();
    
    // Регистрируем обработчики
    auto handler1 = std::make_shared<SimpleEventHandler>(1, "Handler1");
    auto handler2 = std::make_shared<SimpleEventHandler>(2, "Handler2");
    
    reactor20.registerHandler(handler1);
    reactor20.registerHandler(handler2);
    
    // Работаем некоторое время
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    reactor20.stop();
    
    // ============================================================================
    // C++23 ДЕМОНСТРАЦИЯ
    // ============================================================================
    std::cout << "\n--- C++23 Reactor ---" << std::endl;
    
    ReactorCpp23 reactor23;
    reactor23.start();
    
    // ✅ C++23: Регистрация с проверкой ошибок
    auto handler3 = std::make_shared<SimpleEventHandler>(3, "Handler3");
    auto handler4 = std::make_shared<SimpleEventHandler>(4, "Handler4");
    
    auto result1 = reactor23.registerHandler(handler3);
    if (!result1) {
        std::cout << "❌ Ошибка регистрации: " << result1.error() << std::endl;
    }
    
    auto result2 = reactor23.registerHandler(handler4);
    if (!result2) {
        std::cout << "❌ Ошибка регистрации: " << result2.error() << std::endl;
    }
    
    // Работаем некоторое время
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // ✅ C++23: Получение детальной статистики
    auto detailedStats = reactor23.getDetailedStatistics();
    
#ifdef __cpp_lib_print
    std::print("\n=== ДЕТАЛЬНАЯ СТАТИСТИКА ===\n");
    std::print("Всего событий: {}\n", detailedStats.totalEvents);
    std::print("Read событий: {}\n", detailedStats.readEvents);
    std::print("Write событий: {}\n", detailedStats.writeEvents);
    std::print("Error событий: {}\n", detailedStats.errorEvents);
    std::print("Активных обработчиков: {}\n", detailedStats.activeHandlers);
    std::print("===========================\n");
#else
    std::cout << "\n=== ДЕТАЛЬНАЯ СТАТИСТИКА ===" << std::endl;
    std::cout << "Всего событий: " << detailedStats.totalEvents << std::endl;
    std::cout << "Read событий: " << detailedStats.readEvents << std::endl;
    std::cout << "Write событий: " << detailedStats.writeEvents << std::endl;
    std::cout << "Error событий: " << detailedStats.errorEvents << std::endl;
    std::cout << "Активных обработчиков: " << detailedStats.activeHandlers << std::endl;
    std::cout << "===========================" << std::endl;
#endif
    
    reactor23.stop();
}

// ============================================================================
// ОСНОВНАЯ ФУНКЦИЯ
// ============================================================================

int main() {
    std::cout << "⚡ Сравнение Reactor Pattern: C++17/20 vs C++23" << std::endl;
    std::cout << std::string(70, '=') << std::endl;
    
    try {
        demonstrateDifferences();
        
        std::cout << "\n✅ Сравнение завершено!" << std::endl;
        
        std::cout << "\n🎯 Ключевые улучшения C++23:" << std::endl;
        std::cout << "1. std::flat_map - более эффективное хранение обработчиков" << std::endl;
        std::cout << "2. std::expected - четкая обработка ошибок регистрации" << std::endl;
        std::cout << "3. std::generator - ленивая генерация событий" << std::endl;
        std::cout << "4. std::print - более читаемый вывод" << std::endl;
        std::cout << "5. std::stacktrace - детальная отладка ошибок" << std::endl;
        std::cout << "6. Детальная статистика и мониторинг" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Ошибка: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

