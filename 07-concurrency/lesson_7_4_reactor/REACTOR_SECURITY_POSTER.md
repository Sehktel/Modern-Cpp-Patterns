# 🛡️ Security Poster: Reactor Pattern

## 🎯 Критические угрозы

```
🔴 КРИТИЧНЫЕ:
├─ Event Loop DoS (CWE-400): Slow handler blocks all events
├─ Resource Exhaustion: Too many event handlers registered
├─ Race Conditions: Concurrent handler registration/deregistration
└─ Use-After-Free: Handler references deleted resources

🟡 ВЫСОКИЕ:
├─ Event Ordering: Lost or reordered events
├─ Handler Injection: Malicious event handler registration
└─ Timeout Starvation: Some events never timeout
```

## 💣 Ключевые уязвимости

### 1. Event Loop DoS - Slow Handler
```cpp
// ❌ DoS - One slow handler blocks all
void event_loop() {
    while (true) {
        auto events = demultiplexer.select();
        for (auto& event : events) {
            handlers[event.fd]->handle(event);  // 💥 Blocking!
        }
    }
}

void slow_handler(Event event) {
    std::this_thread::sleep_for(std::chrono::seconds(10));  // 💥 Blocks loop!
}

// ✅ БЕЗОПАСНО - Timeout + async dispatch
void event_loop() {
    while (true) {
        auto events = demultiplexer.select(timeout=1s);  // ✅ Timeout
        
        for (auto& event : events) {
            thread_pool.submit([event, handler=handlers[event.fd]] {
                handler->handle(event);  // ✅ Async
            });
        }
    }
}
```

### 2. Resource Exhaustion - Too Many Handlers
```cpp
// ❌ DoS через handler registration
while (true) {
    reactor.register_handler(new Handler());  // 💥 Unbounded!
}

// ✅ БЕЗОПАСНО - Limit handlers
class BoundedReactor {
    std::map<int, Handler*> handlers_;
    const size_t MAX_HANDLERS = 10000;
    
public:
    void register_handler(int fd, Handler* handler) {
        if (handlers_.size() >= MAX_HANDLERS) {
            throw std::runtime_error("Too many handlers");
        }
        handlers_[fd] = handler;
    }
};
```

### 3. Race Condition - Handler Registration
```cpp
// ❌ УЯЗВИМО - Concurrent registration
void register_handler(int fd, Handler* h) {
    handlers_[fd] = h;  // 💥 Data race!
}

void deregister_handler(int fd) {
    delete handlers_[fd];  // 💥 Data race!
    handlers_.erase(fd);
}

// ✅ БЕЗОПАСНО - Synchronized
void register_handler(int fd, std::unique_ptr<Handler> h) {
    std::lock_guard lock(mutex_);
    handlers_[fd] = std::move(h);
}

void deregister_handler(int fd) {
    std::lock_guard lock(mutex_);
    handlers_.erase(fd);  // ✅ unique_ptr auto-deletes
}
```

### 4. UAF - Handler References Deleted Resource
```cpp
// ❌ УЯЗВИМО
class Connection {
    Buffer* buffer_;  // ⚠️ Raw pointer
public:
    void handle_read(Event e) {
        buffer_->append(e.data);  // 💥 UAF if buffer deleted!
    }
};

// ✅ БЕЗОПАСНО - Shared ownership
class Connection {
    std::shared_ptr<Buffer> buffer_;  // ✅ Shared ownership
public:
    void handle_read(Event e) {
        buffer_->append(e.data);  // ✅ Safe
    }
};
```

### 5. Handler Injection Attack
```cpp
// ❌ ОПАСНО - Unvalidated handler registration
void register_from_config(const Config& cfg) {
    for (auto& [fd, handler_name] : cfg.handlers) {
        auto* handler = HandlerFactory::create(handler_name);
        register_handler(fd, handler);  // ⚠️ No validation!
    }
}

// ✅ БЕЗОПАСНО - Whitelist
void register_from_config(const Config& cfg) {
    static const std::set<std::string> ALLOWED = {
        "HttpHandler", "TcpHandler", "UdpHandler"
    };
    
    for (auto& [fd, handler_name] : cfg.handlers) {
        if (ALLOWED.find(handler_name) == ALLOWED.end()) {
            throw SecurityException("Handler not whitelisted");
        }
        
        auto handler = HandlerFactory::create(handler_name);
        register_handler(fd, std::move(handler));
    }
}
```

## 🛠️ Инструменты
```bash
# Race detection
g++ -fsanitize=thread -g reactor.cpp

# Event loop profiling
perf record -e cycles ./reactor_server
perf report

# Handler timeout monitoring
./monitor_handlers --timeout 5s
```

## 📋 Checklist
```
[ ] Handler execution timeout
[ ] Bounded handler count
[ ] Thread-safe handler registration
[ ] Async handler dispatch (thread pool)
[ ] Smart pointers для resource management
[ ] Handler whitelist validation
[ ] Event queue size limit
[ ] Graceful shutdown
[ ] Monitoring (event rate, handler latency)
```

## 💡 Золотое правило
```
"Event loop - это single point of failure.
 NEVER block the loop (timeout + async).
 Limit handlers, validate registration.
 Use shared_ptr для resources!
 Monitor handler performance!"
```

---
**Версия**: 1.0 | **Дата**: 2025-10-11

