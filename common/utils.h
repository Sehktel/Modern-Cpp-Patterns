/**
 * @file utils.h
 * @brief Заголовочный файл с общими утилитами для курса по паттернам программирования
 * 
 * Этот файл содержит объявления Logger и других вспомогательных классов,
 * используемых в примерах курса.
 * 
 * @author Sehktel
 * @license MIT License
 * @copyright Copyright (c) 2025 Sehktel
 * @version 1.0
 */

#pragma once

#include <iostream>
#include <string>
#include <memory>
#include <functional>

namespace cpp_patterns {

/**
 * @brief Простой логгер для демонстрации паттернов
 * 
 * Этот класс используется в различных уроках для показа
 * принципов проектирования и паттернов.
 */
class Logger {
public:
    enum class Level {
        DEBUG,
        INFO,
        WARNING,
        ERROR
    };

    // Конструктор принимает префикс для идентификации логгера
    explicit Logger(const std::string& prefix);
    
    // Методы логирования различных уровней
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    
    // Установка уровня логирования
    void setLevel(Level level);
    
private:
    std::string prefix_;
    Level currentLevel_;
    
    void log(Level level, const std::string& message);
    std::string levelToString(Level level) const;
};

/**
 * @brief Утилиты для демонстрации различных концепций
 */
class Utils {
public:
    // Простая функция для демонстрации RAII
    static std::unique_ptr<int> createInt(int value);
    
    // Функция для демонстрации move semantics
    static std::string processString(std::string input);
    
    // Шаблонная функция для демонстрации type erasure
    template<typename T>
    static void printTypeInfo(const T& value) {
        std::cout << "Type: " << typeid(T).name() 
                  << ", Value: " << value << std::endl;
    }
};

/**
 * @brief Базовый интерфейс для демонстрации полиморфизма
 */
class Drawable {
public:
    virtual ~Drawable() = default;
    virtual void draw() const = 0;
    virtual std::string getDescription() const = 0;
};

/**
 * @brief Простая реализация Drawable
 */
class Rectangle : public Drawable {
private:
    int width_, height_;
    
public:
    Rectangle(int width, int height) : width_(width), height_(height) {}
    
    void draw() const override {
        std::cout << "Рисуем прямоугольник " << width_ << "x" << height_ << std::endl;
    }
    
    std::string getDescription() const override {
        return "Прямоугольник " + std::to_string(width_) + "x" + std::to_string(height_);
    }
};

/**
 * @brief Еще одна реализация Drawable
 */
class Circle : public Drawable {
private:
    int radius_;
    
public:
    explicit Circle(int radius) : radius_(radius) {}
    
    void draw() const override {
        std::cout << "Рисуем круг радиусом " << radius_ << std::endl;
    }
    
    std::string getDescription() const override {
        return "Круг радиусом " + std::to_string(radius_);
    }
};

} // namespace cpp_patterns
