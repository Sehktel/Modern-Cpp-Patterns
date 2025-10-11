# 🛡️ Security Poster: Type Erasure

## 🎯 Критические угрозы

```
🔴 КРИТИЧНЫЕ:
├─ Type Confusion (CWE-843): Incorrect type reconstruction
├─ Buffer Overflow (CWE-120): SBO (Small Buffer Optimization) overflow
├─ Use-After-Free (CWE-416): Dangling pointer в type-erased storage
└─ Allocation Failure: Heap exhaustion в type erasure

🟡 ВЫСОКИЕ:
├─ Slicing: Partial object copy
├─ Virtual Function Overhead: Performance degradation
└─ Exception Safety: Invalid state после failed copy
```

## 💣 Ключевые уязвимости

### 1. Type Confusion при Восстановлении Типа
```cpp
// ❌ УЯЗВИМО
class Any {
    void* data_;
public:
    template<typename T>
    T& get() {
        return *static_cast<T*>(data_);  // 💥 No type checking!
    }
};

Any a = 42;
auto s = a.get<std::string>();  // 💥 Type confusion!

// ✅ БЕЗОПАСНО
class Any {
    void* data_;
    std::type_info* type_;
public:
    template<typename T>
    T& get() {
        if (*type_ != typeid(T)) {  // ✅ Runtime check
            throw std::bad_cast();
        }
        return *static_cast<T*>(data_);
    }
};
```

### 2. SBO Buffer Overflow
```cpp
// ❌ УЯЗВИМО
class Function {
    alignas(std::max_align_t) char buffer_[16];  // ⚠️ Fixed size
    
public:
    template<typename F>
    Function(F f) {
        new (buffer_) F(f);  // 💥 May overflow if sizeof(F) > 16!
    }
};

// ✅ БЕЗОПАСНО
class Function {
    static constexpr size_t BUFFER_SIZE = 32;
    alignas(std::max_align_t) char buffer_[BUFFER_SIZE];
    void* heap_ptr_ = nullptr;
    
public:
    template<typename F>
    Function(F f) {
        if (sizeof(F) <= BUFFER_SIZE) {  // ✅ Check size
            new (buffer_) F(f);  // SBO
        } else {
            heap_ptr_ = new F(f);  // Heap allocation
        }
    }
};
```

### 3. Dangling Pointer в Type-Erased Storage
```cpp
// ❌ УЯЗВИМО
class Callback {
    void* func_;
public:
    template<typename F>
    void set(F&& f) {
        func_ = &f;  // 💥 Stores pointer to temporary!
    }
};

Callback cb;
cb.set([](){ return 42; });  // Lambda temporary
cb.call();  // 💥 UAF!

// ✅ БЕЗОПАСНО
class Callback {
    std::unique_ptr<void, void(*)(void*)> func_;
    
public:
    template<typename F>
    void set(F f) {
        auto* ptr = new F(std::move(f));  // ✅ Owns copy
        func_ = {ptr, [](void* p) { delete static_cast<F*>(p); }};
    }
};
```

### 4. std::function Allocation Attack
```cpp
// ❌ DoS через allocation
std::vector<std::function<void()>> callbacks;

for (int i = 0; i < 1000000; ++i) {
    std::string large_capture(1024*1024, 'A');  // 1MB
    callbacks.push_back([large_capture]() {});  // 💥 1TB total!
}
// Memory exhaustion → DoS

// ✅ БЕЗОПАСНО - Limit storage
template<size_t MaxSize = 256>
class BoundedFunction {
    alignas(std::max_align_t) char buffer_[MaxSize];
    
public:
    template<typename F>
    BoundedFunction(F f) {
        static_assert(sizeof(F) <= MaxSize, "Function too large");
        new (buffer_) F(std::move(f));
    }
};
```

## 🛠️ Инструменты
```bash
# Buffer overflow detection
g++ -fsanitize=address

# Type confusion detection
g++ -fsanitize=undefined -fno-sanitize-recover=undefined

# Allocation tracking
valgrind --tool=massif
```

## 📋 Checklist
```
[ ] Runtime type checking (typeid/type_index)
[ ] SBO buffer size validation
[ ] Ownership semantics clear (copy/move)
[ ] Exception safety в type erasure operations
[ ] Resource limits (max size)
[ ] No dangling pointers
[ ] Proper alignment
```

## 💡 Золотое правило
```
"Type erasure hides types - MUST track them.
 Validate size для SBO.
 Store copies, not pointers.
 Check types at runtime!
 Limit resource usage!"
```

---
**Версия**: 1.0 | **Дата**: 2025-10-11

