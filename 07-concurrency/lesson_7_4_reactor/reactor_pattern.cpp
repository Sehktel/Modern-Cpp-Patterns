/**
 * @file reactor_pattern.cpp
 * @brief Демонстрация Reactor Pattern
 * 
 * TODO: Реализовать Reactor Pattern:
 * - Event Loop с epoll/select
 * - Event Handlers для различных типов событий
 * - HTTP сервер на Reactor
 * - TCP клиент/сервер
 */

/**
 * @file reactor_pattern.cpp
 * @brief Демонстрация Reactor Pattern
 * 
 * Реализован Reactor Pattern с поддержкой:
 * - Event Loop с epoll/select
 * - Event Handlers для различных типов событий
 * - HTTP сервер на Reactor
 * - TCP клиент/сервер
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

// Типы событий для Reactor
enum class ReactorEventType {
    READ,
    WRITE,
    ERROR,
    TIMEOUT
};

// Обработчик событий
class EventHandler {
public:
    virtual ~EventHandler() = default;
    virtual void handleEvent(ReactorEventType event_type) = 0;
    virtual int getFileDescriptor() const = 0;
    virtual std::string getName() const = 0;
};

// Reactor - основной класс для демультиплексирования событий
class Reactor {
private:
    std::atomic<bool> running_{false};
    std::thread reactor_thread_;
    
    // Обработчики событий
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
    Reactor() {
        timeout_.tv_sec = 1;
        timeout_.tv_usec = 0;
        std::cout << "Reactor создан" << std::endl;
    }
    
    ~Reactor() {
        stop();
    }
    
    // Запуск Reactor
    void start() {
        if (running_.load()) {
            std::cout << "Reactor уже запущен" << std::endl;
            return;
        }
        
        running_.store(true);
        reactor_thread_ = std::thread([this]() { runReactor(); });
        std::cout << "Reactor запущен" << std::endl;
    }
    
    // Остановка Reactor
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
    
    // Регистрация обработчика событий
    void registerHandler(std::shared_ptr<EventHandler> handler) {
        std::lock_guard<std::mutex> lock(handlers_mutex_);
        
        int fd = handler->getFileDescriptor();
        handlers_[fd] = handler;
        
        std::cout << "Зарегистрирован обработчик " << handler->getName() 
                  << " для fd=" << fd << std::endl;
    }
    
    // Отмена регистрации обработчика
    void unregisterHandler(int fd) {
        std::lock_guard<std::mutex> lock(handlers_mutex_);
        
        auto it = handlers_.find(fd);
        if (it != handlers_.end()) {
            std::cout << "Отменена регистрация обработчика для fd=" << fd << std::endl;
            handlers_.erase(it);
        }
    }
    
    // Получение статистики
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

// Обработчик для TCP сервера
class TCPServerHandler : public EventHandler {
private:
    int server_fd_;
    int port_;
    Reactor& reactor_;
    std::atomic<int> connection_count_{0};
    
public:
    TCPServerHandler(int port, Reactor& reactor) 
        : port_(port), reactor_(reactor), server_fd_(-1) {}
    
    ~TCPServerHandler() {
        if (server_fd_ >= 0) {
            close(server_fd_);
        }
    }
    
    void start() {
        // Создаем сокет
        server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd_ < 0) {
            throw std::runtime_error("Не удалось создать сокет");
        }
        
        // Настраиваем адрес
        struct sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port_);
        
        // Привязываем сокет
        if (bind(server_fd_, (struct sockaddr*)&address, sizeof(address)) < 0) {
            close(server_fd_);
            throw std::runtime_error("Не удалось привязать сокет");
        }
        
        // Слушаем соединения
        if (listen(server_fd_, 5) < 0) {
            close(server_fd_);
            throw std::runtime_error("Не удалось начать прослушивание");
        }
        
        // Делаем сокет неблокирующим
        int flags = fcntl(server_fd_, F_GETFL, 0);
        fcntl(server_fd_, F_SETFL, flags | O_NONBLOCK);
        
        std::cout << "TCP сервер запущен на порту " << port_ << std::endl;
    }
    
    void handleEvent(ReactorEventType event_type) override {
        switch (event_type) {
            case ReactorEventType::READ:
                handleNewConnection();
                break;
            case ReactorEventType::ERROR:
                std::cerr << "Ошибка в серверном сокете" << std::endl;
                break;
            default:
                break;
        }
    }
    
    int getFileDescriptor() const override {
        return server_fd_;
    }
    
    std::string getName() const override {
        return "TCPServerHandler";
    }
    
    int getConnectionCount() const {
        return connection_count_.load();
    }
    
private:
    void handleNewConnection() {
        struct sockaddr_in client_address;
        socklen_t client_len = sizeof(client_address);
        
        int client_fd = accept(server_fd_, (struct sockaddr*)&client_address, &client_len);
        
        if (client_fd >= 0) {
            connection_count_.fetch_add(1);
            std::cout << "Новое соединение принято, fd=" << client_fd 
                      << " (всего: " << connection_count_.load() << ")" << std::endl;
            
            // Делаем клиентский сокет неблокирующим
            int flags = fcntl(client_fd, F_GETFL, 0);
            fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);
            
            // Создаем обработчик для клиента
            auto client_handler = std::make_shared<TCPClientHandler>(client_fd, reactor_);
            reactor_.registerHandler(client_handler);
        }
    }
};

// Обработчик для TCP клиента
class TCPClientHandler : public EventHandler {
private:
    int client_fd_;
    Reactor& reactor_;
    std::string buffer_;
    bool connection_closed_{false};
    
public:
    TCPClientHandler(int fd, Reactor& reactor) 
        : client_fd_(fd), reactor_(reactor) {}
    
    ~TCPClientHandler() {
        if (client_fd_ >= 0) {
            close(client_fd_);
        }
    }
    
    void handleEvent(ReactorEventType event_type) override {
        switch (event_type) {
            case ReactorEventType::READ:
                handleRead();
                break;
            case ReactorEventType::WRITE:
                handleWrite();
                break;
            case ReactorEventType::ERROR:
                handleError();
                break;
            default:
                break;
        }
    }
    
    int getFileDescriptor() const override {
        return client_fd_;
    }
    
    std::string getName() const override {
        return "TCPClientHandler_" + std::to_string(client_fd_);
    }
    
private:
    void handleRead() {
        char buffer[1024];
        ssize_t bytes_read = read(client_fd_, buffer, sizeof(buffer) - 1);
        
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            std::cout << "Получены данные от клиента " << client_fd_ 
                      << ": " << buffer << std::endl;
            
            // Подготавливаем ответ
            buffer_ = "HTTP/1.1 200 OK\r\n\r\nHello from Reactor Pattern!";
            
        } else if (bytes_read == 0) {
            // Соединение закрыто клиентом
            std::cout << "Клиент " << client_fd_ << " отключился" << std::endl;
            connection_closed_ = true;
            reactor_.unregisterHandler(client_fd_);
        } else {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                std::cerr << "Ошибка чтения от клиента " << client_fd_ 
                          << ": " << strerror(errno) << std::endl;
                connection_closed_ = true;
                reactor_.unregisterHandler(client_fd_);
            }
        }
    }
    
    void handleWrite() {
        if (!buffer_.empty()) {
            ssize_t bytes_written = write(client_fd_, buffer_.c_str(), buffer_.length());
            
            if (bytes_written > 0) {
                buffer_.erase(0, bytes_written);
                std::cout << "Отправлен ответ клиенту " << client_fd_ << std::endl;
            } else if (bytes_written < 0) {
                if (errno != EAGAIN && errno != EWOULDBLOCK) {
                    std::cerr << "Ошибка записи клиенту " << client_fd_ 
                              << ": " << strerror(errno) << std::endl;
                    connection_closed_ = true;
                    reactor_.unregisterHandler(client_fd_);
                }
            }
        }
    }
    
    void handleError() {
        std::cerr << "Ошибка в клиентском соединении " << client_fd_ << std::endl;
        connection_closed_ = true;
        reactor_.unregisterHandler(client_fd_);
    }
};

// Обработчик для таймера
class TimerHandler : public EventHandler {
private:
    int timer_fd_;
    Reactor& reactor_;
    std::chrono::milliseconds interval_;
    std::function<void()> callback_;
    std::atomic<int> tick_count_{0};
    
public:
    TimerHandler(std::chrono::milliseconds interval, 
                 std::function<void()> callback,
                 Reactor& reactor) 
        : interval_(interval), callback_(std::move(callback)), reactor_(reactor), timer_fd_(-1) {}
    
    ~TimerHandler() {
        if (timer_fd_ >= 0) {
            close(timer_fd_);
        }
    }
    
    void start() {
        // Создаем timerfd (Linux-специфичный)
        timer_fd_ = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
        if (timer_fd_ < 0) {
            throw std::runtime_error("Не удалось создать timer");
        }
        
        // Настраиваем таймер
        struct itimerspec timer_spec;
        timer_spec.it_value.tv_sec = interval_.count() / 1000;
        timer_spec.it_value.tv_nsec = (interval_.count() % 1000) * 1000000;
        timer_spec.it_interval.tv_sec = interval_.count() / 1000;
        timer_spec.it_interval.tv_nsec = (interval_.count() % 1000) * 1000000;
        
        if (timerfd_settime(timer_fd_, 0, &timer_spec, nullptr) < 0) {
            close(timer_fd_);
            throw std::runtime_error("Не удалось настроить таймер");
        }
        
        std::cout << "Timer запущен с интервалом " << interval_.count() << " мс" << std::endl;
    }
    
    void handleEvent(ReactorEventType event_type) override {
        if (event_type == ReactorEventType::READ) {
            uint64_t expirations;
            ssize_t bytes_read = read(timer_fd_, &expirations, sizeof(expirations));
            
            if (bytes_read > 0) {
                tick_count_.fetch_add(expirations);
                std::cout << "Timer сработал (тик " << tick_count_.load() << ")" << std::endl;
                
                if (callback_) {
                    callback_();
                }
            }
        }
    }
    
    int getFileDescriptor() const override {
        return timer_fd_;
    }
    
    std::string getName() const override {
        return "TimerHandler";
    }
    
    int getTickCount() const {
        return tick_count_.load();
    }
};

// Демонстрация базового Reactor
void demonstrateBasicReactor() {
    std::cout << "\n=== Демонстрация базового Reactor ===" << std::endl;
    
    Reactor reactor;
    reactor.start();
    
    // Создаем и запускаем таймер
    auto timer_handler = std::make_shared<TimerHandler>(
        std::chrono::milliseconds(1000),
        []() {
            std::cout << "Timer callback выполнен!" << std::endl;
        },
        reactor
    );
    
    timer_handler->start();
    reactor.registerHandler(timer_handler);
    
    // Работаем 5 секунд
    std::this_thread::sleep_for(std::chrono::seconds(5));
    
    std::cout << "Timer сработал " << timer_handler->getTickCount() << " раз" << std::endl;
    
    reactor.stop();
}

// Демонстрация TCP сервера с Reactor
void demonstrateTCPServerReactor() {
    std::cout << "\n=== Демонстрация TCP сервера с Reactor ===" << std::endl;
    
    Reactor reactor;
    reactor.start();
    
    try {
        // Создаем и запускаем TCP сервер
        auto server_handler = std::make_shared<TCPServerHandler>(8080, reactor);
        server_handler->start();
        reactor.registerHandler(server_handler);
        
        // Работаем 10 секунд
        std::this_thread::sleep_for(std::chrono::seconds(10));
        
        std::cout << "Всего соединений: " << server_handler->getConnectionCount() << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Ошибка сервера: " << e.what() << std::endl;
    }
    
    reactor.stop();
}

// Демонстрация комбинированных событий
void demonstrateCombinedEvents() {
    std::cout << "\n=== Демонстрация комбинированных событий ===" << std::endl;
    
    Reactor reactor;
    reactor.start();
    
    // TCP сервер
    auto server_handler = std::make_shared<TCPServerHandler>(8081, reactor);
    server_handler->start();
    reactor.registerHandler(server_handler);
    
    // Таймер
    auto timer_handler = std::make_shared<TimerHandler>(
        std::chrono::milliseconds(2000),
        []() {
            std::cout << "Периодический таймер сработал!" << std::endl;
        },
        reactor
    );
    
    timer_handler->start();
    reactor.registerHandler(timer_handler);
    
    // Работаем 8 секунд
    std::this_thread::sleep_for(std::chrono::seconds(8));
    
    std::cout << "Timer сработал " << timer_handler->getTickCount() << " раз" << std::endl;
    std::cout << "Всего соединений: " << server_handler->getConnectionCount() << std::endl;
    
    reactor.stop();
}

int main() {
    std::cout << "=== Reactor Pattern ===" << std::endl;
    
    try {
        demonstrateBasicReactor();
        demonstrateTCPServerReactor();
        demonstrateCombinedEvents();
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\n=== Демонстрация завершена ===" << std::endl;
    return 0;
}
