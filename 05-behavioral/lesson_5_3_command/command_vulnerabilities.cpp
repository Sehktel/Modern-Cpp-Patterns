#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <cstdlib>
#include <sstream>
#include <fstream>
#include <map>
#include <thread>
#include <chrono>

/**
 * @file command_vulnerabilities.cpp
 * @brief Уязвимые реализации паттерна Command
 * 
 * ⚠️  ТОЛЬКО ДЛЯ ОБРАЗОВАТЕЛЬНЫХ ЦЕЛЕЙ!
 * Демонстрирует опасные практики в Command паттерне.
 */

// ============================================================================
// УЯЗВИМОСТЬ 1: COMMAND INJECTION
// Проблема: Прямое выполнение пользовательского ввода как системной команды
// ============================================================================

class ICommand {
public:
    virtual ~ICommand() = default;
    virtual void execute() = 0;
    virtual std::string getDescription() const = 0;
};

// УЯЗВИМАЯ РЕАЛИЗАЦИЯ: Выполнение shell команд без валидации
class ShellCommand : public ICommand {
private:
    std::string command_;
    
public:
    explicit ShellCommand(const std::string& cmd) : command_(cmd) {}
    
    void execute() override {
        std::cout << "[ShellCommand] Выполнение: " << command_ << "\n";
        // ОПАСНО: Прямое выполнение пользовательского ввода!
        int result = system(command_.c_str());  // COMMAND INJECTION!
        std::cout << "Результат: " << result << "\n";
    }
    
    std::string getDescription() const override {
        return "Shell: " + command_;
    }
};

void demonstrateCommandInjection() {
    std::cout << "\n=== УЯЗВИМОСТЬ 1: Command Injection ===\n";
    
    // Легитимное использование
    ShellCommand cmd1("echo Hello");
    cmd1.execute();
    
    // АТАКА: Injection через разделитель команд
    ShellCommand cmd2("echo Hello; rm -rf /tmp/test");  // Выполнит ОБЕ команды!
    cmd2.execute();
    
    // АТАКА: Injection через pipe
    ShellCommand cmd3("cat /etc/passwd | grep root");  // Чтение системных файлов
    cmd3.execute();
    
    // АТАКА: Injection через backticks
    ShellCommand cmd4("echo `whoami`");  // Выполнение вложенной команды
    cmd4.execute();
    
    std::cout << "⚠️  Атакующий может выполнить ЛЮБЫЕ системные команды!\n";
}

// ============================================================================
// УЯЗВИМОСТЬ 2: PRIVILEGE ESCALATION
// Проблема: Нет проверки прав перед выполнением привилегированных команд
// ============================================================================

enum class Permission {
    USER,
    ADMIN,
    ROOT
};

class User {
public:
    std::string name;
    Permission permission;
    
    User(const std::string& n, Permission p) : name(n), permission(p) {}
};

// УЯЗВИМАЯ РЕАЛИЗАЦИЯ: Проверка прав отделена от выполнения
class PrivilegedCommand : public ICommand {
private:
    std::string action_;
    Permission required_permission_;
    
public:
    PrivilegedCommand(const std::string& action, Permission perm)
        : action_(action), required_permission_(perm) {}
    
    Permission getRequiredPermission() const {
        return required_permission_;
    }
    
    void execute() override {
        // ОПАСНО: Нет проверки прав внутри execute!
        std::cout << "🔓 Выполнение привилегированной команды: " << action_ << "\n";
        
        if (action_ == "delete_user") {
            std::cout << "  → Пользователь удален\n";
        } else if (action_ == "change_password") {
            std::cout << "  → Пароль изменен\n";
        } else if (action_ == "grant_admin") {
            std::cout << "  → Права администратора выданы\n";
        }
    }
    
    std::string getDescription() const override {
        return "Privileged: " + action_;
    }
};

class VulnerableCommandExecutor {
private:
    User* current_user_;
    
public:
    VulnerableCommandExecutor(User* user) : current_user_(user) {}
    
    void setUser(User* user) {
        current_user_ = user;
    }
    
    void executeCommand(PrivilegedCommand* cmd) {
        // УЯЗВИМОСТЬ: Проверка и выполнение не атомарны
        if (current_user_->permission >= cmd->getRequiredPermission()) {
            std::cout << "[Executor] Проверка прав пройдена для " 
                      << current_user_->name << "\n";
            
            // Задержка для демонстрации TOCTOU
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            // Time-of-Use: current_user_ мог измениться!
            cmd->execute();
        } else {
            std::cout << "[Executor] Отказано: недостаточно прав для " 
                      << current_user_->name << "\n";
        }
    }
};

void demonstratePrivilegeEscalation() {
    std::cout << "\n=== УЯЗВИМОСТЬ 2: Privilege Escalation ===\n";
    
    User admin("admin", Permission::ADMIN);
    User regular_user("user", Permission::USER);
    
    VulnerableCommandExecutor executor(&regular_user);
    
    PrivilegedCommand admin_cmd("grant_admin", Permission::ADMIN);
    
    // Поток 1: Пытается выполнить админ команду
    std::thread attacker([&executor, &admin_cmd, &regular_user]() {
        std::cout << "\n[Атакующий поток] Попытка выполнить admin команду как USER\n";
        executor.executeCommand(&admin_cmd);
    });
    
    // Поток 2: Меняет пользователя на админа во время проверки
    std::thread privilege_changer([&executor, &admin]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        std::cout << "[Другой поток] Меняем пользователя на ADMIN!\n";
        executor.setUser(&admin);  // TOCTOU!
    });
    
    attacker.join();
    privilege_changer.join();
    
    std::cout << "\n⚠️  Атакующий обошел проверку прав через TOCTOU!\n";
}

// ============================================================================
// УЯЗВИМОСТЬ 3: COMMAND HISTORY LEAKAGE
// Проблема: История команд хранит конфиденциальные данные
// ============================================================================

class PasswordChangeCommand : public ICommand {
private:
    std::string username_;
    std::string new_password_;  // ОПАСНО: Пароль в plain text!
    
public:
    PasswordChangeCommand(const std::string& user, const std::string& pass)
        : username_(user), new_password_(pass) {}
    
    void execute() override {
        std::cout << "Изменение пароля для " << username_ << "\n";
        // Меняем пароль...
    }
    
    std::string getDescription() const override {
        // ОПАСНО: Пароль виден в описании!
        return "PasswordChange: " + username_ + " -> " + new_password_;
    }
};

class CommandHistory {
private:
    std::vector<std::shared_ptr<ICommand>> history_;
    
public:
    void add(std::shared_ptr<ICommand> cmd) {
        history_.push_back(cmd);
    }
    
    void showHistory() {
        std::cout << "\n=== История команд ===\n";
        for (size_t i = 0; i < history_.size(); ++i) {
            std::cout << i << ": " << history_[i]->getDescription() << "\n";
        }
    }
    
    void saveToFile(const std::string& filename) {
        std::ofstream file(filename);
        for (auto& cmd : history_) {
            file << cmd->getDescription() << "\n";  // УТЕЧКА!
        }
        file.close();
    }
};

void demonstrateHistoryLeakage() {
    std::cout << "\n=== УЯЗВИМОСТЬ 3: Command History Leakage ===\n";
    
    CommandHistory history;
    
    // Пользователь меняет пароль
    auto pwd_cmd = std::make_shared<PasswordChangeCommand>("alice", "SuperSecret123!");
    pwd_cmd->execute();
    history.add(pwd_cmd);
    
    // Другие команды
    auto shell_cmd = std::make_shared<ShellCommand>("ls -la /home/alice/.ssh");
    shell_cmd->execute();
    history.add(shell_cmd);
    
    // ОПАСНО: Пароль виден в истории!
    history.showHistory();
    
    // ОПАСНО: Пароль сохранен в файл!
    history.saveToFile("/tmp/command_history.log");
    std::cout << "\n⚠️  Пароли и команды сохранены в /tmp/command_history.log!\n";
    std::cout << "⚠️  Любой может прочитать конфиденциальные данные!\n";
}

// ============================================================================
// УЯЗВИМОСТЬ 4: DESERIALIZATION ATTACK
// Проблема: Небезопасная десериализация команд
// ============================================================================

class SerializableCommand : public ICommand {
private:
    std::string type_;
    std::string data_;
    
public:
    SerializableCommand(const std::string& type, const std::string& data)
        : type_(type), data_(data) {}
    
    // ОПАСНО: Десериализация без валидации
    static std::shared_ptr<SerializableCommand> deserialize(const std::string& serialized) {
        std::istringstream iss(serialized);
        std::string type, data;
        
        std::getline(iss, type, '|');
        std::getline(iss, data);
        
        // ОПАСНО: Создание команды без проверки типа!
        return std::make_shared<SerializableCommand>(type, data);
    }
    
    void execute() override {
        std::cout << "[" << type_ << "] Выполнение с данными: " << data_ << "\n";
        
        if (type_ == "shell") {
            // ОПАСНО: Выполнение произвольной команды!
            system(data_.c_str());
        } else if (type_ == "eval") {
            // ОПАСНО: Динамическое выполнение кода
            std::cout << "Выполнение кода: " << data_ << "\n";
        }
    }
    
    std::string getDescription() const override {
        return type_ + "|" + data_;
    }
    
    std::string serialize() const {
        return type_ + "|" + data_;
    }
};

void demonstrateDeserializationAttack() {
    std::cout << "\n=== УЯЗВИМОСТЬ 4: Deserialization Attack ===\n";
    
    // Легитимная команда
    auto legit_cmd = std::make_shared<SerializableCommand>("print", "Hello");
    std::string serialized = legit_cmd->serialize();
    std::cout << "Легитимная команда: " << serialized << "\n";
    
    // АТАКА: Вредоносная десериализация
    std::string malicious = "shell|cat /etc/passwd";
    auto evil_cmd = SerializableCommand::deserialize(malicious);
    
    std::cout << "\nАтакующий отправил: " << malicious << "\n";
    std::cout << "Выполнение десериализованной команды:\n";
    evil_cmd->execute();
    
    std::cout << "\n⚠️  Произвольный код выполнен через десериализацию!\n";
}

// ============================================================================
// УЯЗВИМОСТЬ 5: REPLAY ATTACK
// Проблема: Критические команды могут быть воспроизведены повторно
// ============================================================================

class Transaction {
public:
    std::string from;
    std::string to;
    double amount;
    
    Transaction(const std::string& f, const std::string& t, double a)
        : from(f), to(t), amount(a) {}
};

class TransferCommand : public ICommand {
private:
    Transaction transaction_;
    
public:
    explicit TransferCommand(const Transaction& tx) : transaction_(tx) {}
    
    void execute() override {
        std::cout << "💸 Перевод $" << transaction_.amount 
                  << " от " << transaction_.from 
                  << " к " << transaction_.to << "\n";
        // Выполняем перевод...
    }
    
    std::string getDescription() const override {
        return "Transfer: $" + std::to_string(transaction_.amount);
    }
    
    // ОПАСНО: Команду можно скопировать и выполнить снова!
    TransferCommand copy() const {
        return TransferCommand(transaction_);
    }
};

void demonstrateReplayAttack() {
    std::cout << "\n=== УЯЗВИМОСТЬ 5: Replay Attack ===\n";
    
    // Легитимная транзакция
    Transaction tx("Alice", "Bob", 100.0);
    TransferCommand transfer(tx);
    
    std::cout << "Легитимный перевод:\n";
    transfer.execute();
    
    // АТАКА: Злоумышленник перехватывает и повторяет команду
    std::cout << "\nАтакующий перехватил команду и повторил 3 раза:\n";
    for (int i = 0; i < 3; ++i) {
        auto replayed = transfer.copy();
        replayed.execute();
    }
    
    std::cout << "\n⚠️  Деньги списаны 4 раза вместо 1!\n";
    std::cout << "⚠️  Нет защиты от повторного выполнения (nonce, timestamp)\n";
}

// ============================================================================
// УЯЗВИМОСТЬ 6: MACRO COMMAND AMPLIFICATION
// Проблема: Вложенные макро-команды могут вызвать DoS
// ============================================================================

class MacroCommand : public ICommand {
private:
    std::vector<std::shared_ptr<ICommand>> commands_;
    std::string name_;
    
public:
    explicit MacroCommand(const std::string& name) : name_(name) {}
    
    void add(std::shared_ptr<ICommand> cmd) {
        commands_.push_back(cmd);
    }
    
    void execute() override {
        std::cout << "[Macro: " << name_ << "] Выполнение " 
                  << commands_.size() << " команд\n";
        
        for (auto& cmd : commands_) {
            cmd->execute();
        }
    }
    
    std::string getDescription() const override {
        return "Macro: " + name_ + " (" + std::to_string(commands_.size()) + " cmds)";
    }
};

void demonstrateMacroAmplification() {
    std::cout << "\n=== УЯЗВИМОСТЬ 6: Macro Command Amplification (DoS) ===\n";
    
    // Создаем вложенные макро-команды
    auto macro1 = std::make_shared<MacroCommand>("Level1");
    for (int i = 0; i < 10; ++i) {
        macro1->add(std::make_shared<ShellCommand>("echo Level1-" + std::to_string(i)));
    }
    
    auto macro2 = std::make_shared<MacroCommand>("Level2");
    for (int i = 0; i < 10; ++i) {
        macro2->add(macro1);  // 10 * 10 = 100 команд
    }
    
    auto macro3 = std::make_shared<MacroCommand>("Level3");
    for (int i = 0; i < 10; ++i) {
        macro3->add(macro2);  // 10 * 100 = 1000 команд
    }
    
    std::cout << "Выполнение макро-команды уровня 3:\n";
    std::cout << "⚠️  Это выполнит 1000+ команд!\n";
    
    // macro3->execute();  // Не запускаем, чтобы не спамить
    
    std::cout << "\n⚠️  DoS через экспоненциальное увеличение команд!\n";
    std::cout << "⚠️  Нет ограничения на глубину вложенности\n";
}

// ============================================================================
// MAIN
// ============================================================================

int main() {
    std::cout << "=== УЯЗВИМОСТИ COMMAND PATTERN ===\n";
    std::cout << "⚠️  ВНИМАНИЕ: Этот код содержит уязвимости для обучения!\n";
    
    try {
        demonstrateCommandInjection();
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << "\n";
    }
    
    try {
        demonstratePrivilegeEscalation();
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << "\n";
    }
    
    try {
        demonstrateHistoryLeakage();
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << "\n";
    }
    
    try {
        demonstrateDeserializationAttack();
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << "\n";
    }
    
    try {
        demonstrateReplayAttack();
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << "\n";
    }
    
    try {
        demonstrateMacroAmplification();
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << "\n";
    }
    
    std::cout << "\n=== ИНСТРУМЕНТЫ АНАЛИЗА ===\n";
    std::cout << "• AddressSanitizer: g++ -fsanitize=address command_vulnerabilities.cpp\n";
    std::cout << "• Valgrind: valgrind --leak-check=full ./command_vulnerabilities\n";
    std::cout << "• Static Analysis: clang --analyze command_vulnerabilities.cpp\n";
    
    return 0;
}
