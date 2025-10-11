# 🛡️ Security Poster: Command Queue Pattern

## 🎯 Критические угрозы

```
🔴 КРИТИЧНЫЕ:
├─ Queue Overflow (CWE-770): Unbounded command queue growth
├─ Command Injection: Malicious commands в queue
├─ Race Conditions: Concurrent queue access
└─ Priority Inversion: High-priority commands starved

🟡 ВЫСОКИЕ:
├─ Command Replay: Duplicate command execution
├─ Stale Commands: Outdated commands execute too late
└─ Resource Leak: Commands hold resources indefinitely
```

## 💣 Ключевые уязвимости

### 1. Queue Overflow DoS
```cpp
// ❌ DoS - Unbounded queue
while (true) {
    queue.push(HeavyCommand());  // 💥 Unbounded!
}
// → Memory exhaustion

// ✅ БЕЗОПАСНО - Bounded queue + rejection
class BoundedCommandQueue {
    std::queue<Command*> queue_;
    const size_t MAX_SIZE = 10000;
    std::mutex mtx_;
    std::condition_variable cv_;
public:
    bool push(Command* cmd, std::chrono::milliseconds timeout) {
        std::unique_lock lock(mtx_);
        
        if (!cv_.wait_for(lock, timeout, [this] {
            return queue_.size() < MAX_SIZE;
        })) {
            return false;  // ✅ Reject if full
        }
        
        queue_.push(cmd);
        cv_.notify_one();
        return true;
    }
};
```

### 2. Command Injection
```cpp
// ❌ УЯЗВИМО - Unvalidated commands
Command* deserialize(const std::string& data) {
    return CommandFactory::create(data);  // ⚠️ No validation!
}

// Attacker sends: "ShellCommand(rm -rf /)"
auto cmd = deserialize(user_input);
queue.push(cmd);  // 💥 Malicious command queued!

// ✅ БЕЗОПАСНО - Whitelist validation
Command* deserialize(const std::string& data) {
    auto type = extract_type(data);
    
    // ✅ Whitelist check
    static const std::set<std::string> ALLOWED = {
        "UpdateUserCommand",
        "SendEmailCommand",
        "LogCommand"
    };
    
    if (ALLOWED.find(type) == ALLOWED.end()) {
        throw SecurityException("Command type not whitelisted");
    }
    
    return CommandFactory::create(data);
}
```

### 3. Race Condition на Queue
```cpp
// ❌ УЯЗВИМО - No synchronization
class RacyQueue {
    std::queue<Command*> queue_;
public:
    void push(Command* cmd) {
        queue_.push(cmd);  // 💥 Data race!
    }
    
    Command* pop() {
        if (queue_.empty()) return nullptr;
        auto cmd = queue_.front();  // 💥 Data race!
        queue_.pop();
        return cmd;
    }
};

// ✅ БЕЗОПАСНО - Thread-safe queue
class SafeQueue {
    std::queue<Command*> queue_;
    std::mutex mtx_;
    std::condition_variable cv_;
public:
    void push(Command* cmd) {
        {
            std::lock_guard lock(mtx_);
            queue_.push(cmd);
        }
        cv_.notify_one();
    }
    
    Command* pop() {
        std::unique_lock lock(mtx_);
        cv_.wait(lock, [this] { return !queue_.empty(); });
        
        auto cmd = queue_.front();
        queue_.pop();
        return cmd;
    }
};
```

### 4. Command Replay Attack
```cpp
// ❌ ОПАСНО - No duplicate detection
void execute_commands() {
    while (auto cmd = queue.pop()) {
        cmd->execute();  // ⚠️ May execute duplicates!
    }
}

// Attacker submits same command ID multiple times
// → Command executed multiple times

// ✅ БЕЗОПАСНО - Idempotency tracking
class IdempotentQueue {
    std::set<CommandId> executed_;
    std::mutex mtx_;
public:
    void execute_commands() {
        while (auto cmd = queue.pop()) {
            std::lock_guard lock(mtx_);
            
            // ✅ Check if already executed
            if (executed_.find(cmd->getId()) != executed_.end()) {
                delete cmd;
                continue;  // ✅ Skip duplicate
            }
            
            cmd->execute();
            executed_.insert(cmd->getId());
        }
    }
};
```

### 5. Priority Inversion
```cpp
// ❌ ОПАСНО - FIFO ignores priority
std::queue<Command*> fifo_queue;

// Low-priority command submitted first
fifo_queue.push(low_priority_cmd);  // Position 0

// High-priority command submitted later
fifo_queue.push(high_priority_cmd);  // Position 1

// High-priority waits for low-priority! 💥

// ✅ БЕЗОПАСНО - Priority queue
class PriorityCommandQueue {
    struct QueueEntry {
        int priority;
        Command* command;
        
        bool operator<(const QueueEntry& other) const {
            return priority < other.priority;  // Higher priority first
        }
    };
    
    std::priority_queue<QueueEntry> queue_;
    std::mutex mtx_;
    
public:
    void push(Command* cmd, int priority) {
        std::lock_guard lock(mtx_);
        queue_.push({priority, cmd});
    }
    
    Command* pop() {
        std::unique_lock lock(mtx_);
        if (queue_.empty()) return nullptr;
        
        auto entry = queue_.top();
        queue_.pop();
        return entry.command;
    }
};
```

### 6. Stale Command Execution
```cpp
// ❌ ОПАСНО - Old commands execute blindly
auto cmd = queue.pop();
cmd->execute();  // 💥 May be hours old!

// ✅ БЕЗОПАСНО - Timeout checking
class TimedCommand : public Command {
    std::chrono::system_clock::time_point created_;
    std::chrono::seconds timeout_;
public:
    bool isExpired() const {
        auto now = std::chrono::system_clock::now();
        return (now - created_) > timeout_;
    }
};

void execute_commands() {
    while (auto cmd = queue.pop()) {
        if (cmd->isExpired()) {
            delete cmd;
            continue;  // ✅ Skip stale command
        }
        
        cmd->execute();
    }
}
```

## 🛠️ Инструменты
```bash
# Race detection
g++ -fsanitize=thread -g command_queue.cpp

# Queue monitoring
./monitor_queue --size --latency --drop-rate

# Load testing
./stress_test --commands 100000 --rate 10000/s
```

## 📋 Checklist
```
[ ] Bounded queue (reject на overflow)
[ ] Command whitelist validation
[ ] Thread-safe queue operations
[ ] Idempotency tracking (command IDs)
[ ] Priority queue (если нужна prioritization)
[ ] Timeout для stale commands
[ ] Exception handling в command execution
[ ] Monitoring (queue depth, processing time)
[ ] Graceful degradation (reject vs block)
```

## 💡 Золотое правило
```
"Command Queue - это async execution buffer.
 Bound the queue, validate commands (whitelist).
 Track IDs для idempotency.
 Priority queue if priorities matter.
 Timeout stale commands!"
```

---
**Версия**: 1.0 | **Дата**: 2025-10-11

