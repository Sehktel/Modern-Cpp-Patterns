# 🛡️ Security Poster: Flyweight Pattern

## 🎯 Критические угрозы

```
🔴 КРИТИЧНЫЕ:
├─ Shared State Mutation: Concurrent modification of intrinsic state
├─ State Leakage: Sensitive data in shared flyweight
├─ Aliasing Bug: Multiple references to same flyweight → unintended coupling
└─ Race Conditions: Concurrent flyweight factory access

🟡 ВЫСОКИЕ:
├─ Memory Disclosure: Intrinsic state reveals secrets
├─ Cache Poisoning: Malicious flyweight in factory cache
└─ Resource Exhaustion: Unbounded flyweight creation
```

## 💣 Ключевые уязвимости

### 1. Shared State Mutation
```cpp
// ❌ ОПАСНО - Mutable shared state
class MutableFlyweight {
    std::string intrinsic_state_;  // ⚠️ Shared!
public:
    void setIntrinsic(const std::string& s) {
        intrinsic_state_ = s;  // 💥 Modifies shared state!
    }
};

auto fw1 = factory.getFlyweight("key");
auto fw2 = factory.getFlyweight("key");  // Same object

fw1->setIntrinsic("modified");
// → fw2 also affected!

// ✅ БЕЗОПАСНО - Immutable intrinsic state
class ImmutableFlyweight {
    const std::string intrinsic_state_;  // ✅ Const!
public:
    ImmutableFlyweight(std::string s) : intrinsic_state_(std::move(s)) {}
    
    std::string getIntrinsic() const { return intrinsic_state_; }
    
    // ✅ No setter!
};
```

### 2. State Leakage через Shared Flyweight
```cpp
// ❌ УЯЗВИМО - Sensitive data shared
class PasswordFormatFlyweight {
    std::regex pattern_;  // Shared
public:
    bool validate(const std::string& password) {
        return std::regex_match(password, pattern_);
    }
};

// Multiple users share same flyweight
auto validator = factory.get("password_validator");
validator->validate(user1_password);  // ⚠️ Password in shared context?
validator->validate(user2_password);

// ✅ БЕЗОПАСНО - No sensitive data in flyweight
class SafePasswordValidator {
    const std::regex pattern_;  // ✅ Only pattern (intrinsic)
public:
    bool validate(const std::string& password) const {
        // ✅ Password is extrinsic (passed in, not stored)
        return std::regex_match(password, pattern_);
    }
};
```

### 3. Aliasing Bug - Unintended Coupling
```cpp
// ❌ ОПАСНО - Shared reference causes coupling
class SharedGlyph : public Flyweight {
    mutable int render_count_ = 0;  // ⚠️ Shared mutable!
public:
    void render() const {
        render_count_++;  // 💥 Affects all users!
    }
};

auto g1 = factory.getGlyph('A');
auto g2 = factory.getGlyph('A');  // Same object

g1->render();  // Increments render_count_
g2->render();  // Increments again
// render_count_ = 2 (coupled)

// ✅ БЕЗОПАСНО - Extrinsic state for per-use data
class SafeGlyph : public Flyweight {
    // ✅ Intrinsic only (immutable, shared)
    const char character_;
    const Font font_;
public:
    void render(RenderContext& ctx) const {
        // ✅ Extrinsic state in context
        ctx.incrementRenderCount();
        drawCharacter(character_, font_, ctx.getPosition());
    }
};
```

### 4. Factory Race Condition
```cpp
// ❌ УЯЗВИМО - Non-thread-safe factory
class RacyFactory {
    std::map<std::string, Flyweight*> cache_;
public:
    Flyweight* get(const std::string& key) {
        if (cache_.find(key) == cache_.end()) {  // 💥 Race!
            cache_[key] = new Flyweight(key);
        }
        return cache_[key];
    }
};

// ✅ БЕЗОПАСНО - Thread-safe factory
class SafeFactory {
    std::map<std::string, std::shared_ptr<Flyweight>> cache_;
    std::shared_mutex mutex_;
public:
    std::shared_ptr<Flyweight> get(const std::string& key) {
        // ✅ Try shared lock first (read)
        {
            std::shared_lock lock(mutex_);
            if (auto it = cache_.find(key); it != cache_.end()) {
                return it->second;
            }
        }
        
        // ✅ Exclusive lock for write
        std::unique_lock lock(mutex_);
        
        // ✅ Double-check (another thread may have created)
        if (auto it = cache_.find(key); it != cache_.end()) {
            return it->second;
        }
        
        auto flyweight = std::make_shared<Flyweight>(key);
        cache_[key] = flyweight;
        return flyweight;
    }
};
```

### 5. Factory Cache Poisoning
```cpp
// ❌ УЯЗВИМО - Unvalidated factory keys
Flyweight* get(const std::string& user_provided_key) {
    return cache_[user_provided_key];  // 💥 Creates any flyweight!
}

// Attacker: get("../../etc/passwd")
// → May create malicious flyweight

// ✅ БЕЗОПАСНО - Whitelist validation
class ValidatedFactory {
    std::set<std::string> allowed_keys_ = {"A", "B", "C"};
public:
    std::shared_ptr<Flyweight> get(const std::string& key) {
        // ✅ Whitelist check
        if (allowed_keys_.find(key) == allowed_keys_.end()) {
            throw std::invalid_argument("Invalid flyweight key");
        }
        
        // Create/return flyweight
    }
};
```

## 🛠️ Инструменты
```bash
# Race detection
g++ -fsanitize=thread -g flyweight.cpp

# Memory analysis (shared state)
valgrind --tool=memcheck --show-leak-kinds=all ./flyweight_app

# Factory cache monitoring
./monitor_factory --cache-size --hit-rate
```

## 📋 Checklist
```
[ ] Immutable intrinsic state (const members)
[ ] Extrinsic state passed as parameters
[ ] Thread-safe factory (shared_mutex)
[ ] Whitelist validation для factory keys
[ ] No sensitive data в shared state
[ ] smart_ptr для lifetime management
[ ] Monitoring (cache size, hit rate)
[ ] Bounded cache (eviction policy)
```

## 💡 Золотое правило
```
"Flyweight shares intrinsic state - MUST be immutable.
 Extrinsic state via parameters, never stored.
 Thread-safe factory, validate keys.
 No secrets in shared state!"
```

---
**Версия**: 1.0 | **Дата**: 2025-10-11

