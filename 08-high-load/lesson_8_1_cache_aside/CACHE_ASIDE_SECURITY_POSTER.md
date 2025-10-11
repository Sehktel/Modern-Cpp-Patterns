# 🛡️ Security Poster: Cache-Aside Pattern

## 🎯 Критические угрозы

```
🔴 КРИТИЧНЫЕ:
├─ Cache Poisoning (CWE-641): Malicious data cached
├─ Stale Data Reads: Cache doesn't reflect DB updates
├─ Cache Stampede: Thundering herd на cache miss
└─ Race Conditions: Concurrent cache updates

🟡 ВЫСОКИЕ:
├─ Timing Attacks: Cache hit/miss timing reveals secrets
├─ Memory Exhaustion: Unbounded cache growth
└─ Inconsistency: Cache-DB divergence
```

## 💣 Ключевые уязвимости

### 1. Cache Poisoning
```cpp
// ❌ УЯЗВИМО - Unvalidated cache
class VulnerableCache {
public:
    std::string get(const std::string& key) {
        if (cache_.count(key)) {
            return cache_[key];  // 💥 May return poisoned data
        }
        
        auto value = db_.query(key);
        cache_[key] = value;  // ⚠️ No validation
        return value;
    }
};

// ✅ БЕЗОПАСНО - Validated + TTL
class SecureCache {
    struct Entry {
        std::string value;
        std::chrono::system_clock::time_point timestamp;
        std::string hmac;  // ✅ Integrity check
    };
    
public:
    std::string get(const std::string& key) {
        if (auto it = cache_.find(key); it != cache_.end()) {
            if (validate(it->second)) {
                return it->second.value;  // ✅ Validated
            }
            cache_.erase(it);  // ✅ Remove invalid
        }
        
        auto value = db_.query(key);
        cache_entry(key, value);
        return value;
    }
    
private:
    bool validate(const Entry& entry) {
        // ✅ Check TTL
        auto now = std::chrono::system_clock::now();
        if (now - entry.timestamp > ttl_) {
            return false;
        }
        
        // ✅ Verify HMAC
        auto expected = compute_hmac(entry.value, secret_);
        return expected == entry.hmac;
    }
};
```

### 2. Cache Stampede
```cpp
// ❌ DoS - All threads hit DB simultaneously
std::string get(const std::string& key) {
    if (!cache_.count(key)) {
        // 💥 1000 threads all execute this
        auto value = db_.query(key);  // Database overload!
        cache_[key] = value;
        return value;
    }
    return cache_[key];
}

// ✅ БЕЗОПАСНО - Single-flight pattern
class StampedeProofCache {
    std::map<std::string, std::future<std::string>> loading_;
    std::mutex mtx_;
    
public:
    std::string get(const std::string& key) {
        if (auto cached = try_cache(key)) {
            return *cached;
        }
        
        std::unique_lock lock(mtx_);
        
        // ✅ Check if already loading
        if (auto it = loading_.find(key); it != loading_.end()) {
            auto future = it->second;
            lock.unlock();
            return future.get();  // ✅ Wait for first load
        }
        
        // ✅ Start loading (only first thread)
        std::promise<std::string> promise;
        loading_[key] = promise.get_future();
        lock.unlock();
        
        auto value = db_.query(key);  // ✅ Only one DB query
        cache_[key] = value;
        
        promise.set_value(value);
        loading_.erase(key);
        
        return value;
    }
};
```

### 3. Timing Attack - Cache Hit/Miss
```cpp
// ❌ УЯЗВИМО - Timing reveals cache state
auto start = std::chrono::high_resolution_clock::now();
auto value = cache.get(secret_key);
auto end = std::chrono::high_resolution_clock::now();

auto duration = end - start;
if (duration < threshold) {
    // Cache hit → key exists!
} else {
    // Cache miss → key doesn't exist or expired
}

// ✅ ЗАЩИТА - Constant-time lookup
class ConstantTimeCache {
public:
    std::string get(const std::string& key) {
        // ✅ Always query both cache AND DB
        auto cached = cache_.get(key);
        auto db_value = db_.query(key);
        
        if (cached.has_value() && validate(cached.value())) {
            // Use cached, but queried DB too
            return cached.value();
        }
        
        return db_value;
    }
};

// ИЛИ add random delay
auto value = cache.get(key);
std::this_thread::sleep_for(random_delay(0ms, 10ms));  // ✅ Noise
return value;
```

## 🛠️ Инструменты
```bash
# Race detection
g++ -fsanitize=thread

# Cache hit/miss ratio monitoring
./monitor_cache --report-every 1m

# Memory usage tracking
valgrind --tool=massif ./cache_app
```

## 📋 Checklist
```
[ ] Cache entry validation (TTL + HMAC)
[ ] Bounded cache size (LRU eviction)
[ ] Single-flight для cache misses
[ ] Thread-safe cache operations
[ ] Constant-time lookup (if security-critical)
[ ] Cache invalidation strategy
[ ] Monitoring (hit rate, size, staleness)
```

## 💡 Золотое правило
```
"Cache - это untrusted data до validation.
 TTL + HMAC для integrity.
 Single-flight для stampede prevention.
 Constant-time если timing attacks matter!"
```

---
**Версия**: 1.0 | **Дата**: 2025-10-11

