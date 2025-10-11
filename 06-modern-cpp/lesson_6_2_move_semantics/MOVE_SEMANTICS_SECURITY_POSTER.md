# 🛡️ Security Poster: Move Semantics

## 🎯 Критические угрозы

```
🔴 КРИТИЧНЫЕ:
├─ Use-After-Move (CWE-825): Access to moved-from object
├─ Double-Move: Moving already-moved object
├─ Dangling Reference: Reference to moved-from object
└─ Self-Move Assignment: Moving object to itself

🟡 ВЫСОКИЕ:
├─ Exception Safety: Move operations throwing exceptions
├─ State Corruption: Invalid state after partial move
└─ Resource Leak: Move fails but resources allocated
```

## 💣 Ключевые уязвимости

### 1. Use-After-Move
```cpp
// ❌ УЯЗВИМО
std::string str = "data";
auto moved = std::move(str);
std::cout << str;  // 💥 UAM - undefined!

// ✅ БЕЗОПАСНО
std::string str = "data";
auto moved = std::move(str);
// Don't use str after move
```

### 2. Self-Move Assignment
```cpp
// ❌ ОПАСНО
class Resource {
    int* data_;
    Resource& operator=(Resource&& other) {
        delete[] data_;  // Free own data
        data_ = other.data_;  // 💥 Self-move: already freed!
        other.data_ = nullptr;
        return *this;
    }
};

// ✅ БЕЗОПАСНО
Resource& operator=(Resource&& other) noexcept {
    if (this != &other) {  // ✅ Check self-assignment
        delete[] data_;
        data_ = other.data_;
        other.data_ = nullptr;
    }
    return *this;
}
```

### 3. Exception-Unsafe Move
```cpp
// ❌ УЯЗВИМО
class Container {
    T* data_;
    Container(Container&& other) {
        data_ = other.data_;
        other.data_ = nullptr;
        validate();  // 💥 May throw - leave invalid state!
    }
};

// ✅ БЕЗОПАСНО
Container(Container&& other) noexcept {  // ✅ noexcept
    data_ = other.data_;
    other.data_ = nullptr;
}
```

## 🛠️ Инструменты
```bash
# Use-after-move detection
clang-tidy -checks='bugprone-use-after-move'

# Lifetime analysis
clang -Wlifetime
```

## 📋 Checklist
```
[ ] Move constructors/operators noexcept
[ ] Self-assignment check
[ ] Don't use after std::move()
[ ] Leave moved-from in valid state
[ ] No exceptions в move operations
[ ] Test with moved-from objects
```

## 💡 Золотое правило
```
"After std::move(), object is 'walking dead' - valid but unspecified.
 Don't use it (except destroy/assign).
 Move ops must be noexcept!
 Always check self-assignment!"
```

---
**Версия**: 1.0 | **Дата**: 2025-10-11

