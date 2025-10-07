# Урок 7.4: Reactor Pattern (Реактор)

## 🎯 Цель урока
Изучить паттерн Reactor - событийно-ориентированную архитектуру для эффективной обработки множественных I/O операций в одном потоке. Понять, как демультиплексировать события и масштабировать network-приложения.

## 📚 Что изучаем

### 1. Паттерн Reactor
- **Определение**: Синхронное демультиплексирование событий для обработки запросов от множественных источников
- **Назначение**: Эффективная обработка I/O событий без блокировки потоков
- **Применение**: Web-серверы (nginx, Node.js), game servers, network proxies, IoT gateways

### 2. Ключевые компоненты
- **Event Loop**: Основной цикл обработки событий
- **Event Demultiplexer**: Механизм мультиплексирования (select/poll/epoll/kqueue)
- **Event Handlers**: Обработчики для каждого типа события
- **Reactor**: Диспетчер событий к обработчикам

### 3. Типы событий
- **READ**: Данные доступны для чтения
- **WRITE**: Можно записывать данные без блокировки
- **ERROR**: Ошибка на файловом дескрипторе
- **TIMEOUT**: Таймаут операции

## 🔍 Ключевые концепции

### Архитектура Reactor

```
                    ┌──────────────────┐
                    │  Event Sources   │
                    │  (File Descs)    │
                    └──────────────────┘
                      │    │    │    │
               Socket │ File│Timer│Signal
                      ↓    ↓    ↓    ↓
            ┌─────────────────────────────┐
            │  Synchronous Event Demux    │
            │  (select/epoll/kqueue)      │
            └─────────────────────────────┘
                          ↓
            ┌─────────────────────────────┐
            │    Reactor (Dispatcher)     │
            └─────────────────────────────┘
              ↓        ↓        ↓        ↓
         Handler1  Handler2  Handler3  Handler4
```

### Базовая реализация

```cpp
#include <sys/select.h>
#include <unordered_map>
#include <memory>

enum class ReactorEventType {
    READ,
    WRITE,
    ERROR,
    TIMEOUT
};

class EventHandler {
public:
    virtual ~EventHandler() = default;
    virtual void handleEvent(ReactorEventType type) = 0;
    virtual int getFileDescriptor() const = 0;
    virtual std::string getName() const = 0;
};

class Reactor {
private:
    std::unordered_map<int, std::shared_ptr<EventHandler>> handlers_;
    std::mutex mutex_;
    std::atomic<bool> running_{false};
    
public:
    void registerHandler(std::shared_ptr<EventHandler> handler) {
        std::lock_guard<std::mutex> lock(mutex_);
        int fd = handler->getFileDescriptor();
        handlers_[fd] = handler;
    }
    
    void unregisterHandler(int fd) {
        std::lock_guard<std::mutex> lock(mutex_);
        handlers_.erase(fd);
    }
    
    void run() {
        running_.store(true);
        
        while (running_.load()) {
            fd_set read_fds, write_fds, error_fds;
            FD_ZERO(&read_fds);
            FD_ZERO(&write_fds);
            FD_ZERO(&error_fds);
            
            int max_fd = 0;
            
            // Регистрируем file descriptors
            {
                std::lock_guard<std::mutex> lock(mutex_);
                for (const auto& [fd, handler] : handlers_) {
                    FD_SET(fd, &read_fds);
                    FD_SET(fd, &write_fds);
                    FD_SET(fd, &error_fds);
                    max_fd = std::max(max_fd, fd);
                }
            }
            
            // Ждем события
            struct timeval timeout = {1, 0};
            int result = select(max_fd + 1, &read_fds, &write_fds, 
                              &error_fds, &timeout);
            
            if (result <= 0) continue;
            
            // Обрабатываем готовые события
            processEvents(read_fds, write_fds, error_fds);
        }
    }
    
    void stop() {
        running_.store(false);
    }
    
private:
    void processEvents(const fd_set& read_fds, 
                      const fd_set& write_fds,
                      const fd_set& error_fds) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        for (const auto& [fd, handler] : handlers_) {
            if (FD_ISSET(fd, &error_fds)) {
                handler->handleEvent(ReactorEventType::ERROR);
            } else if (FD_ISSET(fd, &read_fds)) {
                handler->handleEvent(ReactorEventType::READ);
            } else if (FD_ISSET(fd, &write_fds)) {
                handler->handleEvent(ReactorEventType::WRITE);
            }
        }
    }
};
```

### TCP Server Handler

```cpp
class TCPServerHandler : public EventHandler {
private:
    int server_fd_;
    int port_;
    Reactor& reactor_;
    
public:
    TCPServerHandler(int port, Reactor& reactor) 
        : port_(port), reactor_(reactor) {
        
        server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
        
        struct sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port_);
        
        bind(server_fd_, (struct sockaddr*)&address, sizeof(address));
        listen(server_fd_, 5);
        
        // Неблокирующий режим
        int flags = fcntl(server_fd_, F_GETFL, 0);
        fcntl(server_fd_, F_SETFL, flags | O_NONBLOCK);
    }
    
    void handleEvent(ReactorEventType type) override {
        if (type == ReactorEventType::READ) {
            acceptConnection();
        }
    }
    
    int getFileDescriptor() const override {
        return server_fd_;
    }
    
    std::string getName() const override {
        return "TCPServerHandler";
    }
    
private:
    void acceptConnection() {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        int client_fd = accept(server_fd_, 
                              (struct sockaddr*)&client_addr, 
                              &client_len);
        
        if (client_fd >= 0) {
            // Создаем handler для клиента
            auto client_handler = std::make_shared<ConnectionHandler>(
                client_fd, reactor_);
            reactor_.registerHandler(client_handler);
        }
    }
};
```

## 🎓 Best Practices

### ✅ DO (Рекомендуется)

1. **Используйте неблокирующий I/O**
   - Устанавливайте `O_NONBLOCK` для всех file descriptors
   - Избегайте блокирующих операций в handlers

2. **Правильно выбирайте демультиплексор**
   - Linux: `epoll` (O(1), масштабируется)
   - BSD/macOS: `kqueue` (O(1))
   - Windows: `IOCP` (асинхронный)
   - Переносимость: `select`/`poll`

3. **Offload CPU-intensive операции**
   - Используйте Thread Pool для тяжелых вычислений
   - Handler должен быть быстрым (< 1ms)
   - Асинхронная обработка через queue

4. **Мониторьте event loop**
   - Latency обработки событий
   - Queue depth для каждого handler
   - CPU utilization event loop

### ❌ DON'T (Не рекомендуется)

1. **НЕ блокируйте event loop**
   - Никаких `sleep()`, долгих вычислений
   - Никаких синхронных DB/API вызовов

2. **НЕ забывайте про backpressure**
   - Ограничивайте количество соединений
   - Используйте bounded buffers

3. **НЕ игнорируйте ошибки**
   - Обрабатывайте `ERROR` события
   - Graceful connection closure

## 📊 Сравнение подходов

| Характеристика | Thread-per-connection | Reactor |
|----------------|----------------------|---------|
| Масштабируемость | Плохая (< 1000 conn) | Отличная (10k+ conn) |
| Память | ~1-8 MB на поток | Минимальная |
| Context switching | Высокий overhead | Минимальный |
| Сложность | Простая | Средняя |
| Latency | Предсказуемая | Зависит от event loop |
| CPU utilization | Низкая | Высокая |

## 📁 Файлы урока

- `reactor_pattern.cpp` - Базовая реализация Reactor с select()
- `event_loop.cpp` - Event Loop с I/O, timer и custom событиями
- `reactor_vulnerabilities.cpp` - Уязвимости и атаки
- `secure_reactor_alternatives.cpp` - Безопасные альтернативы
- `SECURITY_ANALYSIS.md` - Анализ безопасности

## 🔗 Связанные паттерны

- **Proactor**: Асинхронная альтернатива (completion-based)
- **Half-Sync/Half-Async**: Reactor + Thread Pool
- **Leader/Followers**: Multi-threaded reactor
- **Thread Pool**: Для CPU-intensive обработки

## 📖 Дополнительные материалы

### Литература
- Douglas C. Schmidt - "Reactor: An Object Behavioral Pattern for Demultiplexing and Dispatching Handles for Synchronous Events"
- Stevens, Fenner, Rudoff - "Unix Network Programming"

### Реализации
- libevent - кросс-платформенная библиотека
- libuv - используется в Node.js
- Boost.Asio - C++ async I/O

## 🚀 Практическое применение

### Используется в
- **nginx** - web server (epoll на Linux)
- **Node.js** - JavaScript runtime (libuv)
- **Redis** - in-memory database (event loop)
- **lighttpd** - web server
- **memcached** - caching system

### Когда использовать
✅ Множество I/O соединений (web/game servers)
✅ Низкая latency требования
✅ Предсказуемые короткие handlers
✅ Network-bound приложения

### Когда НЕ использовать
❌ CPU-intensive обработка
❌ Блокирующие операции (file I/O, DB)
❌ Простые single-client приложения

---

**Автор**: Senior C++ Developer  
**Дата**: 2025-10-07  
**Версия**: 1.0
