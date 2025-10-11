# 🛡️ Security Poster: Facade Pattern

## 🎯 Обзор угроз Facade Pattern

```
┌────────────────────────────────────────────────────────────────────┐
│                КРИТИЧЕСКИЕ УЯЗВИМОСТИ FACADE PATTERN               │
└────────────────────────────────────────────────────────────────────┘

🔴 КРИТИЧНЫЕ УГРОЗЫ:
├─ Information Disclosure (CWE-200): Facade exposes internal details
├─ Authorization Bypass (CWE-863): Direct subsystem access
├─ API Abuse: Facade doesn't validate subsystem interactions
└─ State Leakage: Shared state между facade methods

🟡 ВЫСОКИЕ УГРОЗЫ:
├─ Insufficient Input Validation: Facade trusts caller
├─ Error Information Leak: Internal errors exposed через facade
└─ Resource Exhaustion: Uncontrolled subsystem usage
```

## 💣 Ключевые уязвимости

### 1. Authorization Bypass через Direct Access

```cpp
// ❌ УЯЗВИМЫЙ КОД - Subsystems publicly accessible
class SystemFacade {
public:
    SubsystemA* subsystemA_;  // ⚠️ Public!
    SubsystemB* subsystemB_;  // ⚠️ Public!
    
    void performSecureOperation() {
        if (checkAuthorization()) {
            subsystemA_->sensitiveOperation();  // ✅ Authorized
        }
    }
};

// АТАКА: Bypass authorization
SystemFacade facade;
facade.subsystemA_->sensitiveOperation();  // 💥 No auth check!

// ✅ БЕЗОПАСНЫЙ КОД - Hide subsystems
class SecureFacade {
private:
    std::unique_ptr<SubsystemA> subsystemA_;  // ✅ Private
    std::unique_ptr<SubsystemB> subsystemB_;  // ✅ Private
    
public:
    void performSecureOperation() {
        if (!checkAuthorization()) {
            throw UnauthorizedException();
        }
        subsystemA_->sensitiveOperation();  // ✅ Always authorized
    }
    
    // ✅ No direct subsystem access possible
};
```

### 2. Information Disclosure через Error Messages

```cpp
// ❌ УЯЗВИМЫЙ КОД - Leaks internal details
class LeakyFacade {
public:
    void processData(const std::string& data) {
        try {
            subsystemA_->parse(data);
            subsystemB_->validate(data);
        } catch (const std::exception& e) {
            // 💥 Exposes internal error
            throw std::runtime_error(
                "Error in " + std::string(typeid(*subsystemA_).name()) + 
                ": " + e.what() + 
                " at " + __FILE__ + ":" + std::to_string(__LINE__)
            );
        }
    }
};

// ✅ БЕЗОПАСНЫЙ КОД - Sanitized errors
class SecureFacade {
public:
    void processData(const std::string& data) {
        try {
            subsystemA_->parse(data);
            subsystemB_->validate(data);
        } catch (const std::exception& e) {
            // ✅ Log detailed error internally
            logger_.error("Internal error: " + std::string(e.what()));
            
            // ✅ Return generic error to caller
            throw std::runtime_error("Data processing failed");
        }
    }
};
```

### 3. Insufficient Input Validation

```cpp
// ❌ УЯЗВИМЫЙ КОД - Trusts caller, no validation
class UnsafeFacade {
public:
    void processUserInput(const std::string& input) {
        // ⚠️ No validation!
        subsystemA_->handleData(input);
        subsystemB_->storeData(input);
        // Subsystems may assume data is validated
    }
};

// АТАКА: Malformed input
facade.processUserInput("<script>alert('XSS')</script>");  // 💥 XSS
facade.processUserInput("'; DROP TABLE users; --");        // 💥 SQL injection

// ✅ БЕЗОПАСНЫЙ КОД - Validation layer
class SecureFacade {
private:
    bool validateInput(const std::string& input) {
        // ✅ Size check
        if (input.length() > MAX_INPUT_SIZE) {
            return false;
        }
        
        // ✅ Content validation
        static const std::regex dangerous_chars(R"([<>;'\"&|$`])");
        if (std::regex_search(input, dangerous_chars)) {
            return false;
        }
        
        return true;
    }
    
    std::string sanitizeInput(const std::string& input) {
        // ✅ Escape dangerous characters
        std::string sanitized;
        for (char c : input) {
            switch (c) {
                case '<': sanitized += "&lt;"; break;
                case '>': sanitized += "&gt;"; break;
                case '&': sanitized += "&amp;"; break;
                case '"': sanitized += "&quot;"; break;
                case '\'': sanitized += "&#x27;"; break;
                default: sanitized += c;
            }
        }
        return sanitized;
    }
    
public:
    void processUserInput(const std::string& input) {
        // ✅ Validate first
        if (!validateInput(input)) {
            throw std::invalid_argument("Invalid input");
        }
        
        // ✅ Sanitize before passing to subsystems
        auto sanitized = sanitizeInput(input);
        
        subsystemA_->handleData(sanitized);
        subsystemB_->storeData(sanitized);
    }
};
```

### 4. State Leakage между Operations

```cpp
// ❌ УЯЗВИМЫЙ КОД - Shared mutable state
class StatefulFacade {
private:
    User* current_user_ = nullptr;  // ⚠️ Shared state
    
public:
    void login(const std::string& username, const std::string& password) {
        current_user_ = authenticateUser(username, password);
    }
    
    void performOperation() {
        // ⚠️ Uses shared state
        if (current_user_ && current_user_->hasPermission()) {
            doOperation();
        }
    }
};

// АТАКА: Race condition / TOCTOU
// Thread 1:
facade.login("admin", "password");
facade.performOperation();  // Authorized

// Thread 2 (simultaneously):
facade.login("attacker", "wrong");  // Sets current_user_ = attacker
// Thread 1's performOperation() now uses attacker's permissions!

// ✅ БЕЗОПАСНЫЙ КОД - Stateless или thread-local state
class StatelessFacade {
public:
    // ✅ Stateless - auth token passed explicitly
    void performOperation(const AuthToken& token) {
        if (!validateToken(token)) {
            throw UnauthorizedException();
        }
        
        // ✅ Используем token, не shared state
        doOperationAsUser(token.getUserId());
    }
};

// Альтернатива: Thread-local state
class ThreadSafeFacade {
private:
    thread_local static User* current_user_;  // ✅ Thread-local
    
public:
    void performOperation() {
        if (!current_user_ || !current_user_->hasPermission()) {
            throw UnauthorizedException();
        }
        doOperation();
    }
};
```

## 🛠️ Инструменты

```bash
# Information leak detection
grep -r "e.what()" facade_implementation.cpp

# Authorization testing
./security_tests --test-authorization-bypass

# Input validation testing
./fuzzer --target facade_interface --iterations 10000
```

## 📋 Security Checklist

```
[ ] Subsystems private (encapsulated)
[ ] Authorization checks в facade methods
[ ] Input validation ПЕРЕД subsystem calls
[ ] Sanitized error messages
[ ] No internal state exposed
[ ] No direct subsystem access
[ ] Stateless design или thread-safe state
[ ] Rate limiting для expensive operations
```

## 🎓 Best Practices

### Security-First Facade
```cpp
class SecureAPIFacade {
private:
    AuthService auth_;
    ValidationService validator_;
    LogService logger_;
    
    // Private subsystems
    DatabaseSubsystem db_;
    CacheSubsystem cache_;
    
public:
    void executeOperation(const Request& req) {
        // 1. ✅ Authentication
        auto token = auth_.authenticate(req.credentials);
        
        // 2. ✅ Authorization
        if (!auth_.authorize(token, Operation::EXECUTE)) {
            logger_.logSecurityEvent("Unauthorized access attempt", req);
            throw UnauthorizedException();
        }
        
        // 3. ✅ Input validation
        if (!validator_.validate(req.data)) {
            logger_.logSecurityEvent("Invalid input", req);
            throw ValidationException();
        }
        
        // 4. ✅ Rate limiting
        if (!checkRateLimit(token.getUserId())) {
            throw RateLimitException();
        }
        
        try {
            // 5. ✅ Execute with subsystems
            auto result = db_.query(req.data);
            cache_.store(result);
            
            // 6. ✅ Audit log
            logger_.logOperation(token.getUserId(), Operation::EXECUTE);
            
        } catch (const std::exception& e) {
            // 7. ✅ Error handling
            logger_.error("Operation failed: " + std::string(e.what()));
            throw std::runtime_error("Operation failed");  // Generic error
        }
    }
};
```

## 💡 Золотое правило

```
"Facade - это security boundary.
 Validate ALL inputs, sanitize ALL outputs.
 Hide subsystems, enforce authorization.
 Never leak internal details in errors!"
```

---

**Версия**: 1.0 | **Дата**: 2025-10-11 | **Лицензия**: MIT

