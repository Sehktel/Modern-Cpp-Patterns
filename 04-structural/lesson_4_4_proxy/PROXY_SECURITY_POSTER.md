# 🛡️ Security Poster: Proxy Pattern

## 🎯 Обзор угроз Proxy Pattern

```
┌────────────────────────────────────────────────────────────────────┐
│                 КРИТИЧЕСКИЕ УЯЗВИМОСТИ PROXY PATTERN               │
└────────────────────────────────────────────────────────────────────┘

🔴 КРИТИЧНЫЕ УГРОЗЫ:
├─ Authentication Bypass (CWE-287): Direct access to real subject
├─ TOCTOU (CWE-367): Check-then-use race в proxy
├─ Proxy Injection: Malicious proxy в chain
└─ Resource Leak: Proxy doesn't clean up real subject

🟡 ВЫСОКИЕ УГРОЗЫ:
├─ Cache Poisoning: Malicious data cached by proxy
├─ Authorization Logic Error: Incorrect permission checks
└─ Side-Channel Attacks: Timing differences reveal information
```

## 💣 Ключевые уязвимости

### 1. Authentication Bypass via Direct Access

```cpp
// ❌ УЯЗВИМЫЙ КОД - Real subject accessible
class SecureDocument {
public:
    std::string read() { return "Confidential data"; }
};

class ProtectionProxy {
private:
    SecureDocument* document_;  // ⚠️ Raw pointer
public:
    std::string read() {
        if (checkAuthorization()) {
            return document_->read();  // ✅ Authorized
        }
        throw UnauthorizedException();
    }
    
    SecureDocument* getDocument() { 
        return document_;  // 💥 Exposes real subject!
    }
};

// АТАКА: Bypass proxy
ProtectionProxy proxy;
auto* doc = proxy.getDocument();
auto data = doc->read();  // 💥 No authorization check!

// ✅ БЕЗОПАСНЫЙ КОД - Hide real subject
class SecureProxy {
private:
    std::unique_ptr<SecureDocument> document_;  // ✅ Exclusive ownership
    
public:
    SecureProxy() : document_(std::make_unique<SecureDocument>()) {}
    
    std::string read() {
        if (!checkAuthorization()) {
            throw UnauthorizedException();
        }
        return document_->read();
    }
    
    // ✅ No getter для document_
};
```

### 2. TOCTOU в Authorization Check

```cpp
// ❌ УЯЗВИМЫЙ КОД - Race condition
class TOCTOUProxy {
private:
    User* user_;
    
public:
    void performAction() {
        // Time of Check
        if (user_->hasPermission(Action::WRITE)) {
            // ⚠️ RACE WINDOW: permissions могут измениться
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            // Time of Use
            realSubject_->write();  // 💥 May execute without permission!
        }
    }
};

// ЭКСПЛОЙТ: Change permissions в race window
std::thread victim([&]() {
    proxy.performAction();  // Checks permission
});

std::thread attacker([&]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    user->revokePermission(Action::WRITE);  // Remove permission
});

// ✅ БЕЗОПАСНЫЙ КОД - Atomic check-and-use
class SafeProxy {
private:
    std::mutex mutex_;
    
public:
    void performAction() {
        std::lock_guard lock(mutex_);
        
        // ✅ Check и use под одной блокировкой
        if (!user_->hasPermission(Action::WRITE)) {
            throw UnauthorizedException();
        }
        
        realSubject_->write();  // Выполняется БЕЗ разблокировки
    }
};

// Альтернатива: Capability-based access
class CapabilityProxy {
public:
    void performAction(const Capability& cap) {
        // ✅ Capability immutable - no TOCTOU
        if (!cap.allows(Action::WRITE)) {
            throw UnauthorizedException();
        }
        
        realSubject_->write();
    }
};
```

### 3. Cache Poisoning в Caching Proxy

```cpp
// ❌ УЯЗВИМЫЙ КОД - Unvalidated cache
class CachingProxy {
private:
    std::map<std::string, std::string> cache_;
    
public:
    std::string get(const std::string& key) {
        // ⚠️ Cache lookup без validation
        if (cache_.find(key) != cache_.end()) {
            return cache_[key];  // 💥 May return poisoned data!
        }
        
        auto value = realSubject_->get(key);
        cache_[key] = value;  // ⚠️ Caches без validation
        return value;
    }
};

// АТАКА: Cache poisoning
// 1. Attacker gains temporary write access
proxy.set("admin_password", "attacker_password");  // Poisoned!

// 2. Later, legitimate access uses cached value
auto password = proxy.get("admin_password");  // 💥 Returns attacker's value!

// ✅ БЕЗОПАСНЫЙ КОД - Validated caching
class SecureCachingProxy {
private:
    struct CacheEntry {
        std::string value;
        std::chrono::system_clock::time_point timestamp;
        std::string integrity_hash;  // ✅ HMAC signature
    };
    
    std::map<std::string, CacheEntry> cache_;
    const std::chrono::seconds TTL{300};  // 5 min TTL
    
    bool validateEntry(const CacheEntry& entry) {
        // ✅ Check TTL
        auto now = std::chrono::system_clock::now();
        if (now - entry.timestamp > TTL) {
            return false;  // Expired
        }
        
        // ✅ Verify integrity
        auto expected_hash = computeHMAC(entry.value, secret_key_);
        return expected_hash == entry.integrity_hash;
    }
    
public:
    std::string get(const std::string& key) {
        auto it = cache_.find(key);
        
        if (it != cache_.end() && validateEntry(it->second)) {
            return it->second.value;  // ✅ Validated cache hit
        }
        
        // Cache miss или invalid entry
        auto value = realSubject_->get(key);
        
        CacheEntry entry;
        entry.value = value;
        entry.timestamp = std::chrono::system_clock::now();
        entry.integrity_hash = computeHMAC(value, secret_key_);
        
        cache_[key] = entry;
        return value;
    }
};
```

### 4. Proxy Chain Injection

```cpp
// ❌ УЯЗВИМЫЙ КОД - Unvalidated proxy chain
Subject* buildProxyChain(const std::vector<std::string>& proxy_types) {
    Subject* subject = new RealSubject();
    
    for (const auto& type : proxy_types) {
        subject = ProxyFactory::create(type, subject);  // ⚠️ No validation!
    }
    
    return subject;
}

// АТАКА: Inject malicious proxy
std::vector<std::string> malicious_chain = {
    "LoggingProxy",
    "MaliciousProxy",  // 💥 Injected!
    "CachingProxy"
};

// ✅ БЕЗОПАСНЫЙ КОД - Whitelist validation
Subject* buildSecureProxyChain(const std::vector<std::string>& proxy_types) {
    static const std::set<std::string> ALLOWED_PROXIES = {
        "LoggingProxy",
        "CachingProxy",
        "ProtectionProxy"
    };
    
    auto subject = std::make_unique<RealSubject>();
    
    for (const auto& type : proxy_types) {
        // ✅ Whitelist check
        if (ALLOWED_PROXIES.find(type) == ALLOWED_PROXIES.end()) {
            throw SecurityException("Proxy type not whitelisted: " + type);
        }
        
        subject = ProxyFactory::create(type, std::move(subject));
    }
    
    return subject.release();
}
```

## 🛠️ Инструменты

```bash
# TOCTOU detection
g++ -fsanitize=thread -g proxy_vulnerabilities.cpp

# Authorization testing
./security_tests --test-auth-bypass --test-toctou

# Cache integrity validation
./cache_validator --verify-hmac --check-ttl
```

## 📋 Security Checklist

```
[ ] Real subject encapsulated (private)
[ ] Atomic authorization checks (no TOCTOU)
[ ] Cache validation (TTL + integrity)
[ ] Proxy chain whitelist
[ ] No direct subject access
[ ] Secure credential storage
[ ] Audit logging для access attempts
[ ] Rate limiting для expensive operations
```

## 🎓 Best Practices

### Multi-Layer Protection Proxy
```cpp
class SecureProtectionProxy : public Subject {
private:
    std::unique_ptr<Subject> realSubject_;
    std::mutex accessMutex_;
    
    // Audit logging
    void logAccess(const std::string& operation, bool granted) {
        logger_.log(
            getCurrentUser().getId(),
            operation,
            granted ? "GRANTED" : "DENIED",
            std::chrono::system_clock::now()
        );
    }
    
    // Rate limiting
    bool checkRateLimit() {
        static std::map<UserId, RateLimiter> limiters;
        auto userId = getCurrentUser().getId();
        return limiters[userId].allowRequest();
    }
    
public:
    void operation() override {
        std::lock_guard lock(accessMutex_);  // ✅ Atomic
        
        // 1. ✅ Authentication
        if (!isAuthenticated()) {
            logAccess("operation", false);
            throw UnauthenticatedException();
        }
        
        // 2. ✅ Authorization  
        if (!hasPermission(Permission::EXECUTE)) {
            logAccess("operation", false);
            throw UnauthorizedException();
        }
        
        // 3. ✅ Rate limiting
        if (!checkRateLimit()) {
            logAccess("operation", false);
            throw RateLimitExceededException();
        }
        
        // 4. ✅ Execute
        realSubject_->operation();
        logAccess("operation", true);
    }
};
```

### Capability-Based Proxy (избегает TOCTOU)
```cpp
class Capability {
    Permission permission_;
    std::chrono::system_clock::time_point expiry_;
    std::string signature_;  // HMAC
    
public:
    bool isValid() const {
        if (std::chrono::system_clock::now() > expiry_) {
            return false;  // Expired
        }
        
        // Verify signature
        auto expected = computeHMAC(permission_, secret_key_);
        return expected == signature_;
    }
    
    Permission getPermission() const { return permission_; }
};

class CapabilityProxy : public Subject {
public:
    void operation(const Capability& cap) {
        // ✅ Capability immutable - no TOCTOU
        if (!cap.isValid()) {
            throw InvalidCapabilityException();
        }
        
        if (cap.getPermission() != Permission::EXECUTE) {
            throw UnauthorizedException();
        }
        
        realSubject_->operation();
    }
};
```

## 💡 Золотое правило

```
"Proxy контролирует доступ - это security boundary.
 Hide real subject, prevent bypass.
 Atomic check-and-use (no TOCTOU).
 Validate ALL cached data.
 Log ALL access attempts!"
```

---

**Версия**: 1.0 | **Дата**: 2025-10-11 | **Лицензия**: MIT

