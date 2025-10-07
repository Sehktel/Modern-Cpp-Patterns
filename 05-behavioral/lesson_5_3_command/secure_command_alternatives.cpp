#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <functional>
#include <chrono>
#include <random>
#include <sstream>
#include <regex>
#include <set>

/**
 * @file secure_command_alternatives.cpp
 * @brief Безопасные реализации паттерна Command
 */

// ============================================================================
// БЕЗОПАСНАЯ РЕАЛИЗАЦИЯ 1: COMMAND С ВАЛИДАЦИЕЙ И WHITELIST
// Решает: Command Injection
// ============================================================================

class ICommand {
public:
    virtual ~ICommand() = default;
    virtual void execute() = 0;
    virtual std::string getDescription() const = 0;
    virtual bool validate() const = 0;
};

// Безопасная команда с валидацией
class SafeShellCommand : public ICommand {
private:
    std::string command_;
    std::vector<std::string> args_;
    
    // Whitelist разрешенных команд
    static const std::set<std::string>& getAllowedCommands() {
        static std::set<std::string> allowed = {
            "ls", "pwd", "date", "whoami", "echo"
        };
        return allowed;
    }
    
    bool isValidArgument(const std::string& arg) const {
        // Проверка на опасные символы
        std::regex dangerous("[;&|`$()<>\\\\]");
        return !std::regex_search(arg, dangerous);
    }
    
public:
    SafeShellCommand(const std::string& cmd, const std::vector<std::string>& args)
        : command_(cmd), args_(args) {}
    
    bool validate() const override {
        // Проверка 1: Команда в whitelist
        if (getAllowedCommands().find(command_) == getAllowedCommands().end()) {
            std::cout << "❌ Команда '" << command_ << "' не разрешена\n";
            return false;
        }
        
        // Проверка 2: Аргументы не содержат injection
        for (const auto& arg : args_) {
            if (!isValidArgument(arg)) {
                std::cout << "❌ Опасный аргумент: " << arg << "\n";
                return false;
            }
        }
        
        return true;
    }
    
    void execute() override {
        if (!validate()) {
            throw std::runtime_error("Команда не прошла валидацию");
        }
        
        std::cout << "✅ Безопасное выполнение: " << command_;
        for (const auto& arg : args_) {
            std::cout << " " << arg;
        }
        std::cout << "\n";
        
        // Безопасное выполнение через execv или аналог
        // (для демонстрации просто выводим)
    }
    
    std::string getDescription() const override {
        return "SafeShell: " + command_;
    }
};

void demonstrateSafeShellCommand() {
    std::cout << "\n=== БЕЗОПАСНАЯ РЕАЛИЗАЦИЯ 1: Command с валидацией ===\n";
    
    // Легитимная команда
    SafeShellCommand cmd1("ls", {"-la"});
    cmd1.execute();
    
    // Попытка injection
    try {
        SafeShellCommand cmd2("ls", {"; rm -rf /"});
        cmd2.execute();
    } catch (const std::exception& e) {
        std::cout << "Блокировано: " << e.what() << "\n";
    }
    
    // Попытка неразрешенной команды
    try {
        SafeShellCommand cmd3("rm", {"-rf", "/tmp"});
        cmd3.execute();
    } catch (const std::exception& e) {
        std::cout << "Блокировано: " << e.what() << "\n";
    }
    
    std::cout << "✅ Все injection атаки заблокированы\n";
}

// ============================================================================
// БЕЗОПАСНАЯ РЕАЛИЗАЦИЯ 2: COMMAND С ПРОВЕРКОЙ ПРАВ
// Решает: Privilege Escalation
// ============================================================================

enum class Permission {
    USER,
    ADMIN,
    ROOT
};

class SecureContext {
private:
    std::string username_;
    Permission permission_;
    
public:
    SecureContext(const std::string& user, Permission perm)
        : username_(user), permission_(perm) {}
    
    std::string getUsername() const { return username_; }
    Permission getPermission() const { return permission_; }
};

// Команда с встроенной проверкой прав
class PrivilegedCommand : public ICommand {
private:
    std::string action_;
    Permission required_permission_;
    std::shared_ptr<SecureContext> context_;  // Контекст безопасности
    
    bool checkPermission() const {
        return context_->getPermission() >= required_permission_;
    }
    
public:
    PrivilegedCommand(const std::string& action, 
                     Permission required,
                     std::shared_ptr<SecureContext> ctx)
        : action_(action), required_permission_(required), context_(ctx) {}
    
    bool validate() const override {
        return checkPermission();
    }
    
    void execute() override {
        // Проверка прав ВНУТРИ execute - атомарно!
        if (!checkPermission()) {
            throw std::runtime_error(
                "Недостаточно прав для " + context_->getUsername());
        }
        
        std::cout << "✅ Выполнение привилегированной команды: " << action_
                  << " (пользователь: " << context_->getUsername() << ")\n";
        
        // Выполнение действия...
    }
    
    std::string getDescription() const override {
        return "Privileged: " + action_;
    }
};

void demonstrateSecurePrivilegedCommand() {
    std::cout << "\n=== БЕЗОПАСНАЯ РЕАЛИЗАЦИЯ 2: Команда с проверкой прав ===\n";
    
    auto admin_ctx = std::make_shared<SecureContext>("admin", Permission::ADMIN);
    auto user_ctx = std::make_shared<SecureContext>("user", Permission::USER);
    
    // Admin выполняет админ команду - OK
    PrivilegedCommand admin_cmd("delete_user", Permission::ADMIN, admin_ctx);
    admin_cmd.execute();
    
    // User пытается выполнить админ команду - FAIL
    try {
        PrivilegedCommand user_cmd("delete_user", Permission::ADMIN, user_ctx);
        user_cmd.execute();
    } catch (const std::exception& e) {
        std::cout << "❌ Блокировано: " << e.what() << "\n";
    }
    
    std::cout << "✅ Проверка прав атомарна - нет TOCTOU\n";
}

// ============================================================================
// БЕЗОПАСНАЯ РЕАЛИЗАЦИЯ 3: COMMAND С SANITIZED HISTORY
// Решает: Information Leakage
// ============================================================================

class SensitiveCommand : public ICommand {
private:
    std::string action_;
    std::string sensitive_data_;  // Не должно попасть в логи
    
public:
    SensitiveCommand(const std::string& action, const std::string& data)
        : action_(action), sensitive_data_(data) {}
    
    void execute() override {
        std::cout << "Выполнение: " << action_ << "\n";
        // Используем sensitive_data_ внутри
    }
    
    bool validate() const override { return true; }
    
    std::string getDescription() const override {
        // НЕ включаем sensitive_data в описание!
        return action_ + ": [REDACTED]";
    }
    
    // Для аудита - хеш вместо данных
    std::string getAuditLog() const {
        size_t hash = std::hash<std::string>{}(sensitive_data_);
        return action_ + ": hash=" + std::to_string(hash);
    }
};

class SecureCommandHistory {
private:
    std::vector<std::shared_ptr<ICommand>> history_;
    
public:
    void add(std::shared_ptr<ICommand> cmd) {
        history_.push_back(cmd);
    }
    
    void showHistory() const {
        std::cout << "\n=== Безопасная история команд ===\n";
        for (size_t i = 0; i < history_.size(); ++i) {
            std::cout << i << ": " << history_[i]->getDescription() << "\n";
        }
    }
    
    // Безопасное сохранение с редактированием
    void saveToFile(const std::string& filename) const {
        std::ofstream file(filename);
        for (auto& cmd : history_) {
            // Сохраняем только безопасное описание
            file << cmd->getDescription() << "\n";
        }
        file.close();
        std::cout << "✅ История сохранена без конфиденциальных данных\n";
    }
};

void demonstrateSecureHistory() {
    std::cout << "\n=== БЕЗОПАСНАЯ РЕАЛИЗАЦИЯ 3: Sanitized History ===\n";
    
    SecureCommandHistory history;
    
    // Команда с чувствительными данными
    auto pwd_cmd = std::make_shared<SensitiveCommand>(
        "PasswordChange", "SuperSecret123!");
    pwd_cmd->execute();
    history.add(pwd_cmd);
    
    auto api_cmd = std::make_shared<SensitiveCommand>(
        "APICall", "api_key=sk-1234567890abcdef");
    api_cmd->execute();
    history.add(api_cmd);
    
    // История НЕ содержит конфиденциальных данных
    history.showHistory();
    history.saveToFile("/tmp/secure_history.log");
}

// ============================================================================
// БЕЗОПАСНАЯ РЕАЛИЗАЦИЯ 4: SIGNED COMMANDS С NONCE
// Решает: Replay Attacks
// ============================================================================

class SignedCommand : public ICommand {
private:
    std::string action_;
    std::string nonce_;  // Уникальный ID
    std::chrono::system_clock::time_point timestamp_;
    std::string signature_;
    
    static std::set<std::string>& getUsedNonces() {
        static std::set<std::string> used;
        return used;
    }
    
    std::string generateNonce() const {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> dis(0, 999999);
        
        return std::to_string(dis(gen)) + "-" + 
               std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
    }
    
    std::string computeSignature() const {
        // Упрощенная подпись (в реальности - HMAC-SHA256)
        size_t hash = std::hash<std::string>{}(action_ + nonce_);
        return std::to_string(hash);
    }
    
public:
    SignedCommand(const std::string& action)
        : action_(action),
          nonce_(generateNonce()),
          timestamp_(std::chrono::system_clock::now()),
          signature_(computeSignature()) {}
    
    bool validate() const override {
        // Проверка 1: Nonce не использован
        if (getUsedNonces().find(nonce_) != getUsedNonces().end()) {
            std::cout << "❌ Nonce уже использован - replay attack!\n";
            return false;
        }
        
        // Проверка 2: Timestamp не слишком старый (5 минут)
        auto now = std::chrono::system_clock::now();
        auto age = std::chrono::duration_cast<std::chrono::minutes>(now - timestamp_);
        if (age.count() > 5) {
            std::cout << "❌ Команда слишком старая\n";
            return false;
        }
        
        // Проверка 3: Подпись корректна
        if (signature_ != computeSignature()) {
            std::cout << "❌ Неверная подпись\n";
            return false;
        }
        
        return true;
    }
    
    void execute() override {
        if (!validate()) {
            throw std::runtime_error("Команда не прошла валидацию");
        }
        
        // Отмечаем nonce как использованный
        getUsedNonces().insert(nonce_);
        
        std::cout << "✅ Выполнение подписанной команды: " << action_ 
                  << " (nonce: " << nonce_ << ")\n";
    }
    
    std::string getDescription() const override {
        return "Signed: " + action_ + " [" + nonce_ + "]";
    }
    
    // Нельзя просто скопировать - nonce будет тот же!
    SignedCommand copy() const = delete;
};

void demonstrateReplayProtection() {
    std::cout << "\n=== БЕЗОПАСНАЯ РЕАЛИЗАЦИЯ 4: Replay Protection ===\n";
    
    SignedCommand cmd("Transfer:$100");
    
    std::cout << "Первое выполнение:\n";
    cmd.execute();
    
    std::cout << "\nПопытка replay:\n";
    try {
        cmd.execute();  // Nonce уже использован!
    } catch (const std::exception& e) {
        std::cout << "Блокировано: " << e.what() << "\n";
    }
    
    std::cout << "✅ Replay attack предотвращен\n";
}

// ============================================================================
// БЕЗОПАСНАЯ РЕАЛИЗАЦИЯ 5: BOUNDED MACRO COMMANDS
// Решает: DoS через Command Amplification
// ============================================================================

class BoundedMacroCommand : public ICommand {
private:
    std::vector<std::shared_ptr<ICommand>> commands_;
    std::string name_;
    static constexpr size_t MAX_COMMANDS = 100;
    static constexpr size_t MAX_DEPTH = 3;
    size_t depth_ = 0;
    
    size_t countTotalCommands() const {
        size_t total = commands_.size();
        for (auto& cmd : commands_) {
            // Если это тоже макро - считаем рекурсивно
            auto macro = std::dynamic_pointer_cast<BoundedMacroCommand>(cmd);
            if (macro) {
                total += macro->countTotalCommands();
            }
        }
        return total;
    }
    
public:
    explicit BoundedMacroCommand(const std::string& name, size_t depth = 0)
        : name_(name), depth_(depth) {}
    
    bool add(std::shared_ptr<ICommand> cmd) {
        // Проверка глубины вложенности
        auto macro = std::dynamic_pointer_cast<BoundedMacroCommand>(cmd);
        if (macro && macro->depth_ >= MAX_DEPTH) {
            std::cout << "❌ Превышена максимальная глубина вложенности\n";
            return false;
        }
        
        // Проверка общего количества команд
        if (countTotalCommands() >= MAX_COMMANDS) {
            std::cout << "❌ Превышен лимит команд\n";
            return false;
        }
        
        if (macro) {
            macro->depth_ = depth_ + 1;
        }
        
        commands_.push_back(cmd);
        return true;
    }
    
    bool validate() const override {
        return countTotalCommands() < MAX_COMMANDS;
    }
    
    void execute() override {
        if (!validate()) {
            throw std::runtime_error("Макро превышает лимиты");
        }
        
        std::cout << "[Bounded Macro: " << name_ << "] Выполнение " 
                  << commands_.size() << " команд (depth=" << depth_ << ")\n";
        
        for (auto& cmd : commands_) {
            cmd->execute();
        }
    }
    
    std::string getDescription() const override {
        return "BoundedMacro: " + name_ + " (" + 
               std::to_string(countTotalCommands()) + " total cmds)";
    }
};

void demonstrateBoundedMacro() {
    std::cout << "\n=== БЕЗОПАСНАЯ РЕАЛИЗАЦИЯ 5: Bounded Macro Commands ===\n";
    
    auto macro1 = std::make_shared<BoundedMacroCommand>("Level1");
    for (int i = 0; i < 10; ++i) {
        auto cmd = std::make_shared<SafeShellCommand>("echo", 
            std::vector<std::string>{"L1-" + std::to_string(i)});
        macro1->add(cmd);
    }
    
    auto macro2 = std::make_shared<BoundedMacroCommand>("Level2");
    for (int i = 0; i < 5; ++i) {
        macro2->add(macro1);  // 5 * 10 = 50 команд - OK
    }
    
    auto macro3 = std::make_shared<BoundedMacroCommand>("Level3");
    for (int i = 0; i < 10; ++i) {
        if (!macro3->add(macro2)) {  // Попытка: 10 * 50 = 500 - FAIL
            std::cout << "Лимит достигнут на итерации " << i << "\n";
            break;
        }
    }
    
    std::cout << "Финальное количество команд: " 
              << macro3->getDescription() << "\n";
    std::cout << "✅ DoS предотвращен через ограничения\n";
}

// ============================================================================
// MAIN
// ============================================================================

int main() {
    std::cout << "=== БЕЗОПАСНЫЕ РЕАЛИЗАЦИИ COMMAND PATTERN ===\n";
    
    demonstrateSafeShellCommand();
    demonstrateSecurePrivilegedCommand();
    demonstrateSecureHistory();
    demonstrateReplayProtection();
    demonstrateBoundedMacro();
    
    std::cout << "\n=== РЕКОМЕНДАЦИИ ===\n";
    std::cout << "✅ Используйте whitelist для команд\n";
    std::cout << "✅ Валидируйте все аргументы (regex, escaping)\n";
    std::cout << "✅ Проверяйте права атомарно внутри execute()\n";
    std::cout << "✅ Не храните sensitive data в истории\n";
    std::cout << "✅ Используйте nonce/timestamp для replay protection\n";
    std::cout << "✅ Ограничивайте вложенность макро-команд\n";
    std::cout << "✅ Подписывайте критические команды (HMAC)\n";
    
    return 0;
}
