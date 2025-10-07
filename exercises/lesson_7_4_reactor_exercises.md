# 🏋️ Упражнения: Reactor Pattern

## 📋 Задание 1: Базовый Reactor с select()

### Описание
Реализуйте базовый Reactor используя POSIX `select()` для мультиплексирования I/O событий.

### Требования
1. ✅ Event loop на основе `select()`
2. ✅ EventHandler интерфейс
3. ✅ Регистрация/отмена регистрации handlers
4. ✅ Обработка READ, WRITE, ERROR событий
5. ✅ Graceful shutdown

### Шаблон для реализации
```cpp
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <unordered_map>
#include <memory>
#include <atomic>

enum class EventType {
    READ,
    WRITE,
    ERROR
};

class EventHandler {
public:
    virtual ~EventHandler() = default;
    
    // TODO: Объявите виртуальные методы
    virtual void handleRead() = 0;
    virtual void handleWrite() = 0;
    virtual void handleError() = 0;
    virtual int getFileDescriptor() const = 0;
};

class Reactor {
private:
    std::unordered_map<int, std::shared_ptr<EventHandler>> handlers_;
    std::mutex handlers_mutex_;
    std::atomic<bool> running_{false};
    
public:
    // TODO: Реализуйте регистрацию handler
    void registerHandler(std::shared_ptr<EventHandler> handler) {
        // Ваш код здесь
    }
    
    // TODO: Реализуйте отмену регистрации
    void unregisterHandler(int fd) {
        // Ваш код здесь
    }
    
    // TODO: Реализуйте event loop с select()
    void run() {
        running_.store(true);
        
        while (running_.load()) {
            fd_set read_fds, write_fds, error_fds;
            FD_ZERO(&read_fds);
            FD_ZERO(&write_fds);
            FD_ZERO(&error_fds);
            
            int max_fd = 0;
            
            // TODO: Заполните fd_sets из handlers_
            
            struct timeval timeout;
            timeout.tv_sec = 1;
            timeout.tv_usec = 0;
            
            // TODO: Вызовите select()
            
            // TODO: Обработайте готовые события
        }
    }
    
    void stop() {
        running_.store(false);
    }
};
```

### Тесты
```cpp
class TestHandler : public EventHandler {
private:
    int fd_;
    int read_count_ = 0;
    
public:
    TestHandler(int fd) : fd_(fd) {}
    
    void handleRead() override {
        char buffer[256];
        ssize_t n = read(fd_, buffer, sizeof(buffer));
        if (n > 0) {
            read_count_++;
            std::cout << "Read " << n << " bytes" << std::endl;
        }
    }
    
    void handleWrite() override {
        std::cout << "Write ready" << std::endl;
    }
    
    void handleError() override {
        std::cerr << "Error on fd " << fd_ << std::endl;
    }
    
    int getFileDescriptor() const override {
        return fd_;
    }
    
    int getReadCount() const { return read_count_; }
};

void testReactor() {
    Reactor reactor;
    
    // Создаем pipe для тестирования
    int pipefd[2];
    pipe(pipefd);
    
    auto handler = std::make_shared<TestHandler>(pipefd[0]);
    reactor.registerHandler(handler);
    
    // Запускаем reactor в отдельном потоке
    std::thread reactor_thread([&reactor]() {
        reactor.run();
    });
    
    // Пишем данные в pipe
    const char* msg = "Hello, Reactor!";
    write(pipefd[1], msg, strlen(msg));
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    reactor.stop();
    reactor_thread.join();
    
    assert(handler->getReadCount() > 0);
    std::cout << "Test PASSED ✅" << std::endl;
    
    close(pipefd[0]);
    close(pipefd[1]);
}
```

---

## 📋 Задание 2: TCP Echo Server на Reactor

### Описание
Реализуйте полнофункциональный TCP echo server используя Reactor Pattern.

### Требования
1. ✅ AcceptHandler для принятия соединений
2. ✅ ConnectionHandler для обработки клиентов
3. ✅ Неблокирующий I/O (O_NONBLOCK)
4. ✅ Поддержка множественных соединений
5. ✅ Статистика: количество соединений, bytes transferred

### Шаблон для реализации
```cpp
class AcceptHandler : public EventHandler {
private:
    int server_fd_;
    Reactor& reactor_;
    std::atomic<size_t> connection_count_{0};
    
public:
    AcceptHandler(int server_fd, Reactor& reactor) 
        : server_fd_(server_fd), reactor_(reactor) {}
    
    void handleRead() override {
        // TODO: Принять новое соединение
        // TODO: Создать ConnectionHandler для нового клиента
        // TODO: Зарегистрировать handler в reactor
    }
    
    int getFileDescriptor() const override {
        return server_fd_;
    }
};

class ConnectionHandler : public EventHandler {
private:
    int client_fd_;
    Reactor& reactor_;
    std::string buffer_;
    
public:
    ConnectionHandler(int client_fd, Reactor& reactor) 
        : client_fd_(client_fd), reactor_(reactor) {}
    
    ~ConnectionHandler() {
        if (client_fd_ >= 0) {
            close(client_fd_);
        }
    }
    
    void handleRead() override {
        // TODO: Прочитать данные от клиента
        // TODO: Echo обратно (сохранить в buffer для write)
    }
    
    void handleWrite() override {
        // TODO: Записать данные клиенту из buffer_
    }
    
    void handleError() override {
        // TODO: Закрыть соединение
        // TODO: Отменить регистрацию в reactor
    }
    
    int getFileDescriptor() const override {
        return client_fd_;
    }
};
```

### Тесты
```cpp
void testEchoServer() {
    // Создаем server socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);
    
    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 5);
    
    // Делаем неблокирующим
    int flags = fcntl(server_fd, F_GETFL, 0);
    fcntl(server_fd, F_SETFL, flags | O_NONBLOCK);
    
    // Создаем reactor
    Reactor reactor;
    auto accept_handler = std::make_shared<AcceptHandler>(server_fd, reactor);
    reactor.registerHandler(accept_handler);
    
    // Запускаем reactor
    std::thread reactor_thread([&reactor]() {
        reactor.run();
    });
    
    // Даем время на тестирование
    std::this_thread::sleep_for(std::chrono::seconds(10));
    
    reactor.stop();
    reactor_thread.join();
    
    close(server_fd);
}
```

---

## 📋 Задание 3: Timer Support в Reactor

### Описание
Добавьте поддержку таймеров в Reactor используя `timerfd_create()` (Linux) или priority queue.

### Требования
1. ✅ TimerHandler для обработки таймеров
2. ✅ One-shot и repeating таймеры
3. ✅ Приоритетная очередь таймеров
4. ✅ Интеграция с select() timeout
5. ✅ Callback при срабатывании таймера

### Шаблон
```cpp
class TimerHandler : public EventHandler {
private:
    int timer_fd_;
    std::chrono::milliseconds interval_;
    std::function<void()> callback_;
    bool repeating_;
    
public:
    // TODO: Реализуйте timer handler
    
    void handleRead() override {
        uint64_t expirations;
        read(timer_fd_, &expirations, sizeof(expirations));
        
        if (callback_) {
            callback_();
        }
    }
};

class TimerQueue {
private:
    struct Timer {
        std::chrono::system_clock::time_point when;
        std::function<void()> callback;
        bool repeating;
        std::chrono::milliseconds interval;
    };
    
    std::priority_queue<Timer> timers_;
    
public:
    // TODO: Реализуйте timer queue
};
```

---

## 📋 Задание 4: Reactor с epoll (Linux)

### Описание
Реализуйте Reactor используя `epoll` для лучшей производительности с большим количеством соединений.

### Требования
1. ✅ Использование `epoll_create()`, `epoll_ctl()`, `epoll_wait()`
2. ✅ Edge-triggered режим
3. ✅ Поддержка 10,000+ соединений (C10K)
4. ✅ O(1) сложность обработки событий
5. ✅ Метрики производительности

### Шаблон
```cpp
#include <sys/epoll.h>

class EpollReactor {
private:
    int epoll_fd_;
    std::unordered_map<int, std::shared_ptr<EventHandler>> handlers_;
    std::atomic<bool> running_{false};
    
    static constexpr int MAX_EVENTS = 128;
    
public:
    EpollReactor() {
        // TODO: Создайте epoll instance
        epoll_fd_ = epoll_create1(0);
    }
    
    ~EpollReactor() {
        if (epoll_fd_ >= 0) {
            close(epoll_fd_);
        }
    }
    
    void registerHandler(std::shared_ptr<EventHandler> handler, uint32_t events) {
        // TODO: Добавьте handler через epoll_ctl
        struct epoll_event ev;
        ev.events = events;  // EPOLLIN | EPOLLOUT | EPOLLET
        ev.data.fd = handler->getFileDescriptor();
        
        epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, handler->getFileDescriptor(), &ev);
        
        handlers_[handler->getFileDescriptor()] = handler;
    }
    
    void run() {
        running_.store(true);
        
        struct epoll_event events[MAX_EVENTS];
        
        while (running_.load()) {
            // TODO: Вызовите epoll_wait
            int nfds = epoll_wait(epoll_fd_, events, MAX_EVENTS, 1000);
            
            // TODO: Обработайте готовые события
        }
    }
};
```

---

## 📋 Задание 5: HTTP Server на Reactor

### Описание
Реализуйте простой HTTP server используя Reactor Pattern.

### Требования
1. ✅ Парсинг HTTP requests
2. ✅ Генерация HTTP responses
3. ✅ Поддержка Keep-Alive
4. ✅ Routing (GET /path)
5. ✅ Статические файлы

### Пример HTTP Handler
```cpp
class HTTPHandler : public EventHandler {
private:
    int client_fd_;
    std::string request_buffer_;
    std::string response_buffer_;
    
    enum class State {
        READING_REQUEST,
        PROCESSING,
        WRITING_RESPONSE
    } state_;
    
public:
    // TODO: Реализуйте HTTP handler
    
    void handleRead() override {
        // TODO: Читайте HTTP request
        // TODO: Парсите request
        // TODO: Генерируйте response
    }
    
    void handleWrite() override {
        // TODO: Отправьте HTTP response
    }
};

// Пример HTTP response
std::string createHTTPResponse(int status_code, const std::string& body) {
    return "HTTP/1.1 " + std::to_string(status_code) + " OK\r\n"
           "Content-Length: " + std::to_string(body.length()) + "\r\n"
           "Content-Type: text/html\r\n"
           "\r\n" + body;
}
```

---

**Автор**: Senior C++ Developer  
**Дата**: 2025-10-07  
**Сложность**: ⭐⭐⭐⭐⭐ (Экспертный уровень)
