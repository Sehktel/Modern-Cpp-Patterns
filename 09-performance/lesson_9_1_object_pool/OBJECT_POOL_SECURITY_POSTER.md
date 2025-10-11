# 🛡️ Security Poster: Object Pool Pattern

## 🎯 Критические угрозы

```
🔴 КРИТИЧНЫЕ:
├─ Use-After-Return (CWE-416): Object used after returned to pool
├─ State Leakage: Pooled object retains sensitive data
├─ Double-Return: Same object returned twice → corruption
└─ Pool Exhaustion: All objects allocated, none available

🟡 ВЫСОКИЕ:
├─ Race Conditions: Concurrent pool access
├─ Resource Leak: Objects never returned to pool
└─ Initialization Bug: Pooled object not properly reset
```

## 💣 Ключевые уязвимости

### 1. Use-After-Return
```cpp
// ❌ УЯЗВИМО - Object used after return
class VulnerablePool {
public:
    Object* acquire() {
        return pool_.back();  // ⚠️ Raw pointer
        pool_.pop_back();
    }
    
    void release(Object* obj) {
        pool_.push_back(obj);
    }
};

Object* obj = pool.acquire();
pool.release(obj);
obj->use();  // 💥 UAR! obj back in pool, may be reused

// ✅ БЕЗОПАСНО - RAII guard
class PoolGuard {
    Pool& pool_;
    std::unique_ptr<Object> obj_;
    
public:
    PoolGuard(Pool& p) : pool_(p), obj_(pool_.acquire()) {}
    
    ~PoolGuard() {
        if (obj_) {
            pool_.release(std::move(obj_));  // ✅ Auto-return
        }
    }
    
    Object* operator->() { return obj_.get(); }
};

// Использование:
{
    PoolGuard guard(pool);
    guard->use();  // ✅ Safe
}  // ✅ Auto-returned
```

### 2. State Leakage между Uses
```cpp
// ❌ ОПАСНО - Sensitive data persists
class Connection {
    std::string auth_token_;  // ⚠️ Sensitive!
public:
    void authenticate(const std::string& token) {
        auth_token_ = token;
    }
    
    void query(const std::string& sql) {
        // Uses auth_token_
    }
};

// User A:
auto conn = pool.acquire();
conn->authenticate("admin_token");
pool.release(conn);

// User B:
conn = pool.acquire();  // 💥 Same object!
conn->query("SELECT *");  // 💥 Uses A's token!

// ✅ БЕЗОПАСНО - Reset on return
class SecurePool {
public:
    void release(Connection* conn) {
        conn->reset();  // ✅ Clear sensitive data
        pool_.push_back(conn);
    }
};

class Connection {
    void reset() {
        auth_token_.clear();  // ✅ Clear
        std::fill(auth_token_.begin(), auth_token_.end(), '\0');
    }
};
```

### 3. Double-Return Attack
```cpp
// ❌ УЯЗВИМО - No tracking
class UntrackedPool {
public:
    void release(Object* obj) {
        pool_.push_back(obj);  // ⚠️ No check!
    }
};

Object* obj = pool.acquire();
pool.release(obj);  // Return #1
pool.release(obj);  // 💥 Return #2 - now in pool twice!

auto obj1 = pool.acquire();  // Gets obj
auto obj2 = pool.acquire();  // Gets obj again!
obj1->modify();  // 💥 Affects obj2!

// ✅ БЕЗОПАСНО - Track allocated objects
class TrackedPool {
    std::set<Object*> allocated_;
    std::mutex mtx_;
    
public:
    Object* acquire() {
        std::lock_guard lock(mtx_);
        if (pool_.empty()) {
            auto* obj = new Object();
            allocated_.insert(obj);
            return obj;
        }
        
        auto* obj = pool_.back();
        pool_.pop_back();
        return obj;
    }
    
    void release(Object* obj) {
        std::lock_guard lock(mtx_);
        
        // ✅ Check if actually allocated
        if (allocated_.find(obj) == allocated_.end()) {
            throw std::invalid_argument("Object not from this pool");
        }
        
        // ✅ Check if already in pool
        if (std::find(pool_.begin(), pool_.end(), obj) != pool_.end()) {
            throw std::logic_error("Double-return detected");
        }
        
        obj->reset();
        pool_.push_back(obj);
    }
};
```

### 4. Pool Exhaustion DoS
```cpp
// ❌ DoS - Exhaust pool
for (int i = 0; i < 1000; ++i) {
    auto obj = pool.acquire();  // Never returned!
}
// → Pool exhausted, service unavailable

// ✅ БЕЗОПАСНО - Timeout + RAII
class TimeoutPool {
public:
    std::unique_ptr<Object> acquire(std::chrono::milliseconds timeout) {
        std::unique_lock lock(mtx_);
        
        if (!cv_.wait_for(lock, timeout, [this] {
            return !pool_.empty();
        })) {
            throw TimeoutException();  // ✅ Don't wait forever
        }
        
        auto obj = std::move(pool_.back());
        pool_.pop_back();
        
        return obj;  // ✅ unique_ptr - RAII guaranteed
    }
};
```

## 🛠️ Инструменты
```bash
# UAR detection
g++ -fsanitize=address -g pool.cpp

# Race detection
g++ -fsanitize=thread -g pool.cpp

# Pool utilization monitoring
./monitor_pool --report-every 10s
```

## 📋 Checklist
```
[ ] RAII guard для auto-return
[ ] Reset objects on return (clear sensitive data)
[ ] Double-return detection
[ ] Timeout на acquire
[ ] Thread-safe pool operations
[ ] Pool size limits
[ ] Monitoring (utilization, wait time)
[ ] Graceful degradation (create new if pool exhausted)
```

## 💡 Золотое правило
```
"Object Pool - это reuse для performance.
 RAII для lifetime, reset для security.
 Track allocations, detect double-return.
 Timeout acquisitions, monitor utilization!"
```

---
**Версия**: 1.0 | **Дата**: 2025-10-11

