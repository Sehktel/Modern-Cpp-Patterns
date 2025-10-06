// Дополнительные реализации логгера для демонстрации различных паттернов

#include "utils.h"
#include <fstream>
#include <mutex>

namespace cpp_patterns {

/**
 * @brief Файловый логгер - демонстрирует расширение функциональности
 * без изменения базового интерфейса (принцип открытости/закрытости)
 */
class FileLogger : public Logger {
private:
    std::ofstream file_;
    mutable std::mutex fileMutex_;
    
public:
    explicit FileLogger(const std::string& prefix, const std::string& filename)
        : Logger(prefix), file_(filename, std::ios::app) {
        if (!file_.is_open()) {
            throw std::runtime_error("Не удалось открыть файл для логирования: " + filename);
        }
    }
    
    ~FileLogger() {
        if (file_.is_open()) {
            file_.close();
        }
    }
    
    // Переопределяем метод логирования для записи в файл
    void log(Level level, const std::string& message) {
        // Сначала вызываем базовую версию для вывода в консоль
        Logger::log(level, message);
        
        // Затем записываем в файл
        std::lock_guard<std::mutex> lock(fileMutex_);
        if (file_.is_open()) {
            auto now = std::time(nullptr);
            auto tm = *std::localtime(&now);
            
            file_ << std::put_time(&tm, "[%Y-%m-%d %H:%M:%S]")
                  << " [" << levelToString(level) << "]"
                  << " [" << getPrefix() << "] "
                  << message << std::endl;
            file_.flush();
        }
    }
    
private:
    std::string getPrefix() const {
        // Возвращаем префикс логгера (упрощенная реализация)
        return "FileLogger";
    }
};

/**
 * @brief Комбинированный логгер - демонстрирует композицию
 * Логирует одновременно в консоль и файл
 */
class CompositeLogger {
private:
    std::unique_ptr<Logger> consoleLogger_;
    std::unique_ptr<FileLogger> fileLogger_;
    
public:
    CompositeLogger(const std::string& prefix, const std::string& filename)
        : consoleLogger_(std::make_unique<Logger>(prefix))
        , fileLogger_(std::make_unique<FileLogger>(prefix + "_file", filename)) {
    }
    
    void debug(const std::string& message) {
        consoleLogger_->debug(message);
        fileLogger_->debug(message);
    }
    
    void info(const std::string& message) {
        consoleLogger_->info(message);
        fileLogger_->info(message);
    }
    
    void warning(const std::string& message) {
        consoleLogger_->warning(message);
        fileLogger_->warning(message);
    }
    
    void error(const std::string& message) {
        consoleLogger_->error(message);
        fileLogger_->error(message);
    }
    
    void setLevel(Logger::Level level) {
        consoleLogger_->setLevel(level);
        fileLogger_->setLevel(level);
    }
};

} // namespace cpp_patterns
