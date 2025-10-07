/**
 * @file event_loop.cpp
 * @brief Реализация Event Loop для Reactor Pattern
 * 
 * Реализован Event Loop с поддержкой:
 * - Основной цикл обработки событий
 * - Интеграция с epoll/select
 * - Обработка таймеров
 * - Управление жизненным циклом
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

// Типы событий
enum class EventType {
    READ,
    WRITE,
    TIMER,
    SIGNAL,
    CUSTOM
};

// Базовое событие
struct Event {
    int fd;
    EventType type;
    std::function<void()> callback;
    std::chrono::system_clock::time_point timestamp;
    
    Event(int file_descriptor, EventType event_type, std::function<void()> cb)
        : fd(file_descriptor), type(event_type), callback(std::move(cb)),
          timestamp(std::chrono::system_clock::now()) {}
    
    // Компаратор для приоритетной очереди (ранние события первыми)
    bool operator>(const Event& other) const {
        return timestamp > other.timestamp;
    }
};

// Timer событие
struct TimerEvent {
    std::chrono::system_clock::time_point when;
    std::function<void()> callback;
    bool repeat;
    std::chrono::milliseconds interval;
    
    TimerEvent(std::chrono::system_clock::time_point time, 
               std::function<void()> cb, 
               bool is_repeat = false,
               std::chrono::milliseconds repeat_interval = std::chrono::milliseconds(0))
        : when(time), callback(std::move(cb)), repeat(is_repeat), interval(repeat_interval) {}
    
    bool operator>(const TimerEvent& other) const {
        return when > other.when;
    }
};

// Event Loop
class EventLoop {
private:
    std::atomic<bool> running_{false};
    std::thread loop_thread_;
    
    // I/O события
    std::unordered_map<int, std::shared_ptr<Event>> io_events_;
    std::mutex io_events_mutex_;
    
    // Timer события
    std::priority_queue<TimerEvent, std::vector<TimerEvent>, std::greater<TimerEvent>> timer_queue_;
    std::mutex timer_mutex_;
    
    // Кастомные события
    std::queue<std::function<void()>> custom_events_;
    std::mutex custom_events_mutex_;
    std::condition_variable custom_events_condition_;
    
    // Статистика
    std::atomic<size_t> events_processed_{0};
    std::atomic<size_t> io_events_processed_{0};
    std::atomic<size_t> timer_events_processed_{0};
    std::atomic<size_t> custom_events_processed_{0};
    
public:
    EventLoop() {
        std::cout << "Event Loop создан" << std::endl;
    }
    
    ~EventLoop() {
        stop();
    }
    
    // Запуск event loop
    void start() {
        if (running_.load()) {
            std::cout << "Event Loop уже запущен" << std::endl;
            return;
        }
        
        running_.store(true);
        loop_thread_ = std::thread([this]() { runLoop(); });
        std::cout << "Event Loop запущен" << std::endl;
    }
    
    // Остановка event loop
    void stop() {
        if (!running_.load()) return;
        
        std::cout << "Останавливаем Event Loop..." << std::endl;
        running_.store(false);
        
        // Уведомляем о кастомных событиях
        custom_events_condition_.notify_all();
        
        if (loop_thread_.joinable()) {
            loop_thread_.join();
        }
        
        printStats();
        std::cout << "Event Loop остановлен" << std::endl;
    }
    
    // Регистрация I/O события
    void registerIOEvent(int fd, EventType type, std::function<void()> callback) {
        std::lock_guard<std::mutex> lock(io_events_mutex_);
        
        auto event = std::make_shared<Event>(fd, type, std::move(callback));
        io_events_[fd] = event;
        
        std::cout << "Зарегистрировано I/O событие для fd=" << fd 
                  << ", тип=" << static_cast<int>(type) << std::endl;
    }
    
    // Отмена I/O события
    void unregisterIOEvent(int fd) {
        std::lock_guard<std::mutex> lock(io_events_mutex_);
        
        auto it = io_events_.find(fd);
        if (it != io_events_.end()) {
            io_events_.erase(it);
            std::cout << "Отменено I/O событие для fd=" << fd << std::endl;
        }
    }
    
    // Добавление timer события
    void addTimerEvent(std::chrono::milliseconds delay, 
                      std::function<void()> callback,
                      bool repeat = false) {
        auto when = std::chrono::system_clock::now() + delay;
        
        std::lock_guard<std::mutex> lock(timer_mutex_);
        timer_queue_.emplace(when, std::move(callback), repeat, delay);
        
        std::cout << "Добавлено timer событие через " << delay.count() << " мс" << std::endl;
    }
    
    // Добавление кастомного события
    void postCustomEvent(std::function<void()> callback) {
        {
            std::lock_guard<std::mutex> lock(custom_events_mutex_);
            custom_events_.push(std::move(callback));
        }
        custom_events_condition_.notify_one();
        
        std::cout << "Добавлено кастомное событие" << std::endl;
    }
    
    // Получение статистики
    void printStats() const {
        std::cout << "\n=== Event Loop Statistics ===" << std::endl;
        std::cout << "Всего событий обработано: " << events_processed_.load() << std::endl;
        std::cout << "I/O событий: " << io_events_processed_.load() << std::endl;
        std::cout << "Timer событий: " << timer_events_processed_.load() << std::endl;
        std::cout << "Кастомных событий: " << custom_events_processed_.load() << std::endl;
        std::cout << "=============================" << std::endl;
    }
    
private:
    void runLoop() {
        std::cout << "Event Loop начал работу" << std::endl;
        
        while (running_.load()) {
            try {
                // 1. Обрабатываем timer события
                processTimerEvents();
                
                // 2. Обрабатываем I/O события
                processIOEvents();
                
                // 3. Обрабатываем кастомные события
                processCustomEvents();
                
                // Небольшая пауза для предотвращения busy waiting
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                
            } catch (const std::exception& e) {
                std::cerr << "Ошибка в Event Loop: " << e.what() << std::endl;
            }
        }
        
        std::cout << "Event Loop завершил работу" << std::endl;
    }
    
    void processTimerEvents() {
        std::lock_guard<std::mutex> lock(timer_mutex_);
        
        auto now = std::chrono::system_clock::now();
        
        while (!timer_queue_.empty() && timer_queue_.top().when <= now) {
            auto timer_event = timer_queue_.top();
            timer_queue_.pop();
            
            try {
                timer_event.callback();
                timer_events_processed_.fetch_add(1);
                events_processed_.fetch_add(1);
                
                // Если повторяющийся timer, добавляем обратно
                if (timer_event.repeat) {
                    auto next_time = now + timer_event.interval;
                    timer_queue_.emplace(next_time, timer_event.callback, 
                                       timer_event.repeat, timer_event.interval);
                }
                
            } catch (const std::exception& e) {
                std::cerr << "Ошибка в timer событии: " << e.what() << std::endl;
            }
        }
    }
    
    void processIOEvents() {
        std::lock_guard<std::mutex> lock(io_events_mutex_);
        
        if (io_events_.empty()) return;
        
        // Простая имитация select() для демонстрации
        for (auto& pair : io_events_) {
            auto& event = pair.second;
            
            // Имитируем готовность к чтению/записи
            static std::random_device rd;
            static std::mt19937 gen(rd());
            static std::uniform_int_distribution<> dis(1, 100);
            
            if (dis(gen) <= 5) { // 5% вероятность готовности
                try {
                    event->callback();
                    io_events_processed_.fetch_add(1);
                    events_processed_.fetch_add(1);
                } catch (const std::exception& e) {
                    std::cerr << "Ошибка в I/O событии: " << e.what() << std::endl;
                }
            }
        }
    }
    
    void processCustomEvents() {
        std::unique_lock<std::mutex> lock(custom_events_mutex_);
        
        // Ждем события с таймаутом
        if (custom_events_condition_.wait_for(lock, std::chrono::milliseconds(10),
            [this] { return !custom_events_.empty() || !running_.load(); })) {
            
            while (!custom_events_.empty()) {
                auto callback = custom_events_.front();
                custom_events_.pop();
                
                lock.unlock();
                
                try {
                    callback();
                    custom_events_processed_.fetch_add(1);
                    events_processed_.fetch_add(1);
                } catch (const std::exception& e) {
                    std::cerr << "Ошибка в кастомном событии: " << e.what() << std::endl;
                }
                
                lock.lock();
            }
        }
    }
};

// Простой TCP сервер для демонстрации
class TCPServer {
private:
    int server_fd_;
    int port_;
    EventLoop& event_loop_;
    std::atomic<bool> running_{false};
    
public:
    TCPServer(int port, EventLoop& loop) : port_(port), event_loop_(loop), server_fd_(-1) {}
    
    ~TCPServer() {
        stop();
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
        
        // Регистрируем событие чтения
        event_loop_.registerIOEvent(server_fd_, EventType::READ, 
            [this]() { handleNewConnection(); });
        
        running_.store(true);
        std::cout << "TCP сервер запущен на порту " << port_ << std::endl;
    }
    
    void stop() {
        if (!running_.load()) return;
        
        running_.store(false);
        
        if (server_fd_ >= 0) {
            event_loop_.unregisterIOEvent(server_fd_);
            close(server_fd_);
            server_fd_ = -1;
        }
        
        std::cout << "TCP сервер остановлен" << std::endl;
    }
    
private:
    void handleNewConnection() {
        struct sockaddr_in client_address;
        socklen_t client_len = sizeof(client_address);
        
        int client_fd = accept(server_fd_, (struct sockaddr*)&client_address, &client_len);
        
        if (client_fd >= 0) {
            std::cout << "Новое соединение принято, fd=" << client_fd << std::endl;
            
            // Делаем клиентский сокет неблокирующим
            int flags = fcntl(client_fd, F_GETFL, 0);
            fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);
            
            // Регистрируем событие чтения для клиента
            event_loop_.registerIOEvent(client_fd, EventType::READ,
                [this, client_fd]() { handleClientData(client_fd); });
        }
    }
    
    void handleClientData(int client_fd) {
        char buffer[1024];
        ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
        
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            std::cout << "Получены данные от клиента " << client_fd 
                      << ": " << buffer << std::endl;
            
            // Отправляем ответ
            std::string response = "HTTP/1.1 200 OK\r\n\r\nHello from Event Loop!";
            write(client_fd, response.c_str(), response.length());
            
        } else if (bytes_read == 0) {
            // Соединение закрыто клиентом
            std::cout << "Клиент " << client_fd << " отключился" << std::endl;
            event_loop_.unregisterIOEvent(client_fd);
            close(client_fd);
        }
    }
};

// Демонстрация базового Event Loop
void demonstrateBasicEventLoop() {
    std::cout << "\n=== Демонстрация базового Event Loop ===" << std::endl;
    
    EventLoop loop;
    loop.start();
    
    // Добавляем timer события
    loop.addTimerEvent(std::chrono::milliseconds(500), []() {
        std::cout << "Timer событие 1 выполнено!" << std::endl;
    });
    
    loop.addTimerEvent(std::chrono::milliseconds(1000), []() {
        std::cout << "Timer событие 2 выполнено!" << std::endl;
    });
    
    // Добавляем повторяющийся timer
    loop.addTimerEvent(std::chrono::milliseconds(200), []() {
        std::cout << "Повторяющийся timer!" << std::endl;
    }, true);
    
    // Добавляем кастомные события
    for (int i = 0; i < 5; ++i) {
        loop.postCustomEvent([i]() {
            std::cout << "Кастомное событие " << i << " выполнено!" << std::endl;
        });
    }
    
    // Работаем 3 секунды
    std::this_thread::sleep_for(std::chrono::seconds(3));
    
    loop.stop();
}

// Демонстрация TCP сервера
void demonstrateTCPServer() {
    std::cout << "\n=== Демонстрация TCP сервера ===" << std::endl;
    
    EventLoop loop;
    loop.start();
    
    try {
        TCPServer server(8080, loop);
        server.start();
        
        // Работаем 5 секунд
        std::this_thread::sleep_for(std::chrono::seconds(5));
        
        server.stop();
    } catch (const std::exception& e) {
        std::cerr << "Ошибка сервера: " << e.what() << std::endl;
    }
    
    loop.stop();
}

// Демонстрация комбинированных событий
void demonstrateCombinedEvents() {
    std::cout << "\n=== Демонстрация комбинированных событий ===" << std::endl;
    
    EventLoop loop;
    loop.start();
    
    int counter = 0;
    
    // Timer, который добавляет кастомные события
    loop.addTimerEvent(std::chrono::milliseconds(1000), [&loop, &counter]() {
        std::cout << "Timer добавляет кастомное событие" << std::endl;
        
        loop.postCustomEvent([&counter]() {
            counter++;
            std::cout << "Кастомное событие выполнено, счетчик: " << counter << std::endl;
        });
    }, true);
    
    // Работаем 3 секунды
    std::this_thread::sleep_for(std::chrono::seconds(3));
    
    std::cout << "Итоговый счетчик: " << counter << std::endl;
    loop.stop();
}

int main() {
    std::cout << "=== Event Loop для Reactor Pattern ===" << std::endl;
    
    try {
        demonstrateBasicEventLoop();
        demonstrateTCPServer();
        demonstrateCombinedEvents();
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\n=== Демонстрация завершена ===" << std::endl;
    return 0;
}
