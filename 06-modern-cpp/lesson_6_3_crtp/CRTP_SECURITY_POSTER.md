# 🛡️ Security Poster: CRTP (Curiously Recurring Template Pattern)

## 🎯 Критические угрозы

```
🔴 КРИТИЧНЫЕ:
├─ Undefined Behavior (CWE-758): Base access before derived construction
├─ ODR Violation (One Definition Rule): Multiple template instantiations
├─ Type Confusion: Wrong derived type in template
└─ Slicing: Using base pointer for CRTP class

🟡 ВЫСОКИЕ:
├─ Compilation Bloat: Template code explosion
├─ Name Hiding: Derived hides base methods unintentionally
└─ Incomplete Type Issues: Forward declaration problems
```

## 💣 Ключевые уязвимости

### 1. Undefined Behavior - Access Before Construction
```cpp
// ❌ УЯЗВИМО
template<typename Derived>
class Base {
public:
    Base() {
        static_cast<Derived*>(this)->init();  // 💥 UB! Derived not constructed yet
    }
};

// ✅ БЕЗОПАСНО
template<typename Derived>
class Base {
public:
    void initialize() {  // ✅ Call AFTER construction
        static_cast<Derived*>(this)->init();
    }
};

class Derived : public Base<Derived> {
public:
    Derived() {
        initialize();  // ✅ Safe - after construction
    }
};
```

### 2. Type Confusion в CRTP
```cpp
// ❌ ОПАСНО
template<typename Derived>
class Base {
    void method() {
        auto* d = static_cast<Derived*>(this);  // ⚠️ Unchecked cast
        d->derivedMethod();
    }
};

class WrongDerived : public Base<AnotherClass> {};  // 💥 Wrong type!

// ✅ БЕЗОПАСНО - Enforce правильный derived type
template<typename Derived>
class Base {
private:
    friend Derived;  // ✅ Only Derived can inherit
    Base() = default;
    
    Derived& derived() {
        return static_cast<Derived&>(*this);  // ✅ Type-safe
    }
};
```

### 3. Slicing Attack
```cpp
// ❌ ОПАСНО
template<typename Derived>
class Counter {
    static inline int count_ = 0;
public:
    Counter() { ++count_; }
    static int getCount() { return count_; }
};

class A : public Counter<A> {};
class B : public Counter<B> {};

Base<A>* ptr = new A();  // 💥 Slicing - loses CRTP!
delete ptr;  // Wrong destructor called

// ✅ БЕЗОПАСНО
template<typename Derived>
class Counter {
protected:
    ~Counter() = default;  // ✅ Prevent polymorphic deletion
};
```

## 🛠️ Инструменты
```bash
# UB detection
g++ -fsanitize=undefined -g crtp_code.cpp

# Static analysis
clang-tidy -checks='misc-unconventional-assign-operator,cppcoreguidelines-*'
```

## 📋 Checklist
```
[ ] No base method calls в base constructor
[ ] Protected/private base destructor
[ ] Friend declaration для type safety
[ ] No polymorphic usage (no virtual)
[ ] Derived correctly specifies self as template arg
[ ] No slicing (use references/specific types)
```

## 💡 Золотое правило
```
"CRTP - это compile-time polymorphism.
 DON'T call derived methods from base constructor.
 DON'T use polymorphically (base pointers).
 DON'T forget to specify correct Derived type!"
```

---
**Версия**: 1.0 | **Дата**: 2025-10-11

