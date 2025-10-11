# 🛡️ Security Poster: Smart Pointers

## 🎯 Критические угрозы

```
🔴 КРИТИЧНЫЕ:
├─ Double-Free (CWE-415): unique_ptr double release
├─ Use-After-Free (CWE-416): Dangling weak_ptr/raw pointers
├─ Reference Cycle (CWE-401): shared_ptr circular references → memory leak
└─ Nullptr Dereference (CWE-476): Unchecked smart pointer access

🟡 ВЫСОКИЕ:
├─ Aliasing Bug: shared_ptr aliasing constructor misuse
├─ Custom Deleter Injection: Malicious deleter in shared_ptr
└─ Thread-Safety: shared_ptr race conditions
```

## 💣 Ключевые уязвимости

### 1. Double-Free с unique_ptr
```cpp
// ❌ УЯЗВИМО
auto ptr = std::make_unique<Data>();
Data* raw = ptr.get();
ptr.reset();  // Delete #1
delete raw;   // 💥 Delete #2 - double-free!

// ✅ БЕЗОПАСНО
auto ptr = std::make_unique<Data>();
// Нет raw pointer access
```

### 2. shared_ptr Circular References
```cpp
// ❌ УТЕЧКА
struct Node {
    std::shared_ptr<Node> next;  // 💥 Cycle leak
};

// ✅ БЕЗОПАСНО
struct Node {
    std::weak_ptr<Node> next;  // ✅ Break cycle
};
```

### 3. weak_ptr Race Condition
```cpp
// ❌ УЯЗВИМО - TOCTOU
if (!weak.expired()) {  // Check
    // RACE WINDOW
    auto shared = weak.lock();  // Use
    shared->method();  // 💥 May be null!
}

// ✅ БЕЗОПАСНО
if (auto shared = weak.lock()) {  // Atomic check-and-lock
    shared->method();  // ✅ Guaranteed valid
}
```

## 🛠️ Инструменты
```bash
# Double-free detection
g++ -fsanitize=address

# Memory leak detection (cycles)
valgrind --leak-check=full --show-leak-kinds=all
```

## 📋 Checklist
```
[ ] Используйте make_unique/make_shared
[ ] weak_ptr для breaking cycles
[ ] Atomic lock() для weak_ptr
[ ] No raw pointer ownership
[ ] Custom deleters validated
[ ] Thread-safe shared_ptr usage
```

## 💡 Золотое правило
```
"Smart pointers manage lifetime - DON'T mix with raw ownership.
 Use weak_ptr для cycles, atomic lock() для races.
 Never release() без крайней необходимости!"
```

---
**Версия**: 1.0 | **Дата**: 2025-10-11

