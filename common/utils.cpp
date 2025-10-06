/**
 * @file utils.cpp
 * @brief Реализация общих утилит для курса по паттернам программирования
 * 
 * Этот файл содержит реализацию Logger и других вспомогательных классов,
 * используемых в примерах курса.
 * 
 * @author Sehktel
 * @license MIT License
 * @copyright Copyright (c) 2025 Sehktel
 * @version 1.0
 */

#include "utils.h"
#include <iomanip>
#include <ctime>

namespace cpp_patterns {

// Реализация Logger
Logger::Logger(const std::string& prefix) 
    : prefix_(prefix), currentLevel_(Level::INFO) {
}

void Logger::debug(const std::string& message) {
    log(Level::DEBUG, message);
}

void Logger::info(const std::string& message) {
    log(Level::INFO, message);
}

void Logger::warning(const std::string& message) {
    log(Level::WARNING, message);
}

void Logger::error(const std::string& message) {
    log(Level::ERROR, message);
}

void Logger::setLevel(Level level) {
    currentLevel_ = level;
}

void Logger::log(Level level, const std::string& message) {
    // Логируем только если уровень сообщения >= текущего уровня
    if (level >= currentLevel_) {
        auto now = std::time(nullptr);
        auto tm = *std::localtime(&now);
        
        std::cout << std::put_time(&tm, "[%H:%M:%S]")
                  << " [" << levelToString(level) << "]"
                  << " [" << prefix_ << "] "
                  << message << std::endl;
    }
}

std::string Logger::levelToString(Level level) const {
    switch (level) {
        case Level::DEBUG:   return "DEBUG";
        case Level::INFO:    return "INFO ";
        case Level::WARNING: return "WARN ";
        case Level::ERROR:   return "ERROR";
        default:             return "UNKNOWN";
    }
}

// Реализация Utils
std::unique_ptr<int> Utils::createInt(int value) {
    return std::make_unique<int>(value);
}

std::string Utils::processString(std::string input) {
    // Демонстрация move semantics - input передается по значению
    // и может быть перемещен без копирования
    input += "_processed";
    return input; // Возвращаем по значению, но с move semantics
}

} // namespace cpp_patterns
