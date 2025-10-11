# 🛡️ Security Poster: Actor Model Pattern

## 🎯 Критические угрозы

```
🔴 КРИТИЧНЫЕ:
├─ Message Queue Overflow (CWE-770): Unbounded mailbox
├─ Message Corruption: Concurrent message access
├─ Actor Deadlock: Circular message dependencies
└─ Use-After-Free: Messages referencing dead actors

🟡 ВЫСОКИЕ:
├─ Message Ordering: Lost or reordered messages
├─ Actor Starvation: Some actors never process messages
└─ Resource Leak: Actors hold resources indefinitely
```

## 💣 Ключевые уязвимости

### 1. Mailbox Overflow
```cpp
// ❌ DoS через message flooding
while (true) {
    actor.send(Message{});  // 💥 Unbounded mailbox!
}

// ✅ БЕЗОПАСНО - Bounded mailbox + backpressure
class BoundedActor {
    std::queue<Message> mailbox_;
    const size_t MAX_MESSAGES = 1000;
    
public:
    bool send(Message msg, std::chrono::milliseconds timeout) {
        std::unique_lock lock(mutex_);
        
        if (!cv_.wait_for(lock, timeout, [this] {
            return mailbox_.size() < MAX_MESSAGES;
        })) {
            return false;  // ✅ Backpressure
        }
        
        mailbox_.push(std::move(msg));
        cv_.notify_one();
        return true;
    }
};
```

### 2. Actor Deadlock
```cpp
// ❌ DEADLOCK - Circular dependency
Actor A, B;

// A sends to B, waits for response
A.send_and_wait(B, msg1);

// B sends to A, waits for response  
B.send_and_wait(A, msg2);  // 💥 Deadlock!

// ✅ БЕЗОПАСНО - Async messaging only
class AsyncActor {
public:
    void send(Message msg) {  // ✅ Fire-and-forget
        mailbox_.push(std::move(msg));
    }
    
    // NO synchronous send_and_wait!
};
```

### 3. UAF - Message References Dead Actor
```cpp
// ❌ УЯЗВИМО
struct Message {
    Actor* sender;  // ⚠️ Raw pointer
};

void handler(const Message& msg) {
    msg.sender->send(response);  // 💥 UAF if sender dead!
}

// ✅ БЕЗОПАСНО - Weak references
struct Message {
    std::weak_ptr<Actor> sender;  // ✅ Weak ref
};

void handler(const Message& msg) {
    if (auto sender = msg.sender.lock()) {
        sender->send(response);  // ✅ Safe
    }
}
```

### 4. Message Corruption
```cpp
// ❌ УЯЗВИМО - Shared mutable message
struct Message {
    std::vector<int> data;  // ⚠️ Mutable
};

Message msg;
actor1.send(msg);  // Actor1 может modify
actor2.send(msg);  // Actor2 видит modified data!

// ✅ БЕЗОПАСНО - Immutable messages
struct ImmutableMessage {
    const std::vector<int> data;  // ✅ Const
    
    ImmutableMessage(std::vector<int> d) 
        : data(std::move(d)) {}
};

// ИЛИ move-only messages
actor1.send(std::move(msg));  // ✅ Ownership transfer
```

## 🛠️ Инструменты
```bash
# Race detection
g++ -fsanitize=thread -g actor_model.cpp

# Message flow tracing
./trace_messages --actor A --depth 10

# Deadlock detection
valgrind --tool=helgrind ./actor_system
```

## 📋 Checklist
```
[ ] Bounded mailboxes
[ ] Immutable or move-only messages
[ ] Weak references между actors
[ ] No synchronous messaging
[ ] Exception handling в message handlers
[ ] Actor lifecycle management
[ ] Message ordering guarantees
[ ] Monitoring (mailbox size, actor count)
```

## 💡 Золотое правило
```
"Actors communicate via messages - NEVER share state.
 Messages must be immutable or moved.
 Bound mailboxes, use weak_ptr для actor refs.
 NO synchronous messaging (deadlock risk)!"
```

---
**Версия**: 1.0 | **Дата**: 2025-10-11

