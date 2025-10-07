/**
 * @file memory_optimization.cpp
 * @brief Оптимизация памяти для Flyweight Pattern
 * 
 * Реализована оптимизация памяти с поддержкой:
 * - Анализ использования памяти
 * - Оптимизация структур данных
 * - Кэширование объектов
 * - Мониторинг производительности
 */

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <mutex>
#include <chrono>
#include <atomic>
#include <cstring>
#include <sstream>

// Утилита для измерения памяти
class MemoryTracker {
private:
    std::atomic<size_t> allocations_{0};
    std::atomic<size_t> deallocations_{0};
    std::atomic<size_t> total_allocated_bytes_{0};
    std::atomic<size_t> current_allocated_bytes_{0};
    std::atomic<size_t> peak_allocated_bytes_{0};
    
public:
    void recordAllocation(size_t bytes) {
        allocations_.fetch_add(1);
        total_allocated_bytes_.fetch_add(bytes);
        
        size_t current = current_allocated_bytes_.fetch_add(bytes) + bytes;
        
        // Обновляем пик
        size_t peak = peak_allocated_bytes_.load();
        while (current > peak && 
               !peak_allocated_bytes_.compare_exchange_weak(peak, current)) {}
    }
    
    void recordDeallocation(size_t bytes) {
        deallocations_.fetch_add(1);
        current_allocated_bytes_.fetch_sub(bytes);
    }
    
    void printStats() const {
        std::cout << "\n=== Memory Tracker Statistics ===" << std::endl;
        std::cout << "Allocations: " << allocations_.load() << std::endl;
        std::cout << "Deallocations: " << deallocations_.load() << std::endl;
        std::cout << "Total allocated: " << (total_allocated_bytes_.load() / 1024) << " KB" << std::endl;
        std::cout << "Current allocated: " << (current_allocated_bytes_.load() / 1024) << " KB" << std::endl;
        std::cout << "Peak allocated: " << (peak_allocated_bytes_.load() / 1024) << " KB" << std::endl;
        std::cout << "=================================" << std::endl;
    }
    
    size_t getCurrentBytes() const {
        return current_allocated_bytes_.load();
    }
    
    size_t getPeakBytes() const {
        return peak_allocated_bytes_.load();
    }
    
    void reset() {
        allocations_.store(0);
        deallocations_.store(0);
        total_allocated_bytes_.store(0);
        current_allocated_bytes_.store(0);
        peak_allocated_bytes_.store(0);
    }
};

// Глобальный трекер памяти
static MemoryTracker g_memory_tracker;

// Пример БЕЗ Flyweight: каждый символ хранит полную информацию
class CharacterWithoutFlyweight {
private:
    char character_;
    std::string font_family_;
    int font_size_;
    std::string color_;
    bool bold_;
    bool italic_;
    int position_x_;
    int position_y_;
    
public:
    CharacterWithoutFlyweight(char ch, const std::string& font, int size,
                             const std::string& col, bool b, bool i, int x, int y)
        : character_(ch), font_family_(font), font_size_(size), 
          color_(col), bold_(b), italic_(i), position_x_(x), position_y_(y) {
        
        // Регистрируем аллокацию
        size_t bytes = sizeof(*this) + font_family_.capacity() + color_.capacity();
        g_memory_tracker.recordAllocation(bytes);
    }
    
    ~CharacterWithoutFlyweight() {
        size_t bytes = sizeof(*this) + font_family_.capacity() + color_.capacity();
        g_memory_tracker.recordDeallocation(bytes);
    }
    
    void render() const {
        std::cout << character_;
    }
    
    size_t getMemorySize() const {
        return sizeof(*this) + font_family_.capacity() + color_.capacity();
    }
};

// Пример С Flyweight: разделяемое (intrinsic) и уникальное (extrinsic) состояние

// Разделяемое состояние (Flyweight)
class CharacterStyle {
private:
    std::string font_family_;
    int font_size_;
    std::string color_;
    bool bold_;
    bool italic_;
    
public:
    CharacterStyle(const std::string& font, int size, const std::string& col, 
                  bool b, bool i)
        : font_family_(font), font_size_(size), color_(col), bold_(b), italic_(i) {
        
        size_t bytes = sizeof(*this) + font_family_.capacity() + color_.capacity();
        g_memory_tracker.recordAllocation(bytes);
    }
    
    ~CharacterStyle() {
        size_t bytes = sizeof(*this) + font_family_.capacity() + color_.capacity();
        g_memory_tracker.recordDeallocation(bytes);
    }
    
    // Получение уникального ключа
    std::string getKey() const {
        std::ostringstream oss;
        oss << font_family_ << "_" << font_size_ << "_" << color_ 
            << "_" << bold_ << "_" << italic_;
        return oss.str();
    }
    
    void applyStyle() const {
        // Применение стиля к символу
    }
    
    size_t getMemorySize() const {
        return sizeof(*this) + font_family_.capacity() + color_.capacity();
    }
};

// Фабрика Flyweight для управления разделяемыми стилями
class CharacterStyleFactory {
private:
    std::unordered_map<std::string, std::shared_ptr<CharacterStyle>> styles_;
    mutable std::mutex mutex_;
    
public:
    std::shared_ptr<CharacterStyle> getStyle(const std::string& font, int size,
                                             const std::string& color,
                                             bool bold, bool italic) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // Создаем временный объект для получения ключа
        CharacterStyle temp_style(font, size, color, bold, italic);
        std::string key = temp_style.getKey();
        
        // Ищем существующий стиль
        auto it = styles_.find(key);
        if (it != styles_.end()) {
            return it->second;
        }
        
        // Создаем новый стиль
        auto style = std::make_shared<CharacterStyle>(font, size, color, bold, italic);
        styles_[key] = style;
        
        std::cout << "Создан новый стиль: " << key 
                  << " (всего стилей: " << styles_.size() << ")" << std::endl;
        
        return style;
    }
    
    size_t getStyleCount() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return styles_.size();
    }
    
    size_t getTotalMemory() const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        size_t total = 0;
        for (const auto& pair : styles_) {
            total += pair.second->getMemorySize();
        }
        
        return total;
    }
    
    void printStats() const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        std::cout << "\n=== Character Style Factory ===" << std::endl;
        std::cout << "Уникальных стилей: " << styles_.size() << std::endl;
        std::cout << "Общая память стилей: " << (getTotalMemory() / 1024) << " KB" << std::endl;
        std::cout << "===============================" << std::endl;
    }
};

// Символ с Flyweight (только уникальное состояние)
class CharacterWithFlyweight {
private:
    char character_;
    std::shared_ptr<CharacterStyle> style_;  // Разделяемое состояние
    int position_x_;  // Уникальное состояние
    int position_y_;  // Уникальное состояние
    
public:
    CharacterWithFlyweight(char ch, std::shared_ptr<CharacterStyle> style, 
                          int x, int y)
        : character_(ch), style_(std::move(style)), 
          position_x_(x), position_y_(y) {
        
        // Регистрируем только уникальное состояние
        g_memory_tracker.recordAllocation(sizeof(*this));
    }
    
    ~CharacterWithFlyweight() {
        g_memory_tracker.recordDeallocation(sizeof(*this));
    }
    
    void render() const {
        style_->applyStyle();
        std::cout << character_;
    }
    
    size_t getMemorySize() const {
        return sizeof(*this);
    }
};

// Текстовый документ БЕЗ Flyweight
class DocumentWithoutFlyweight {
private:
    std::vector<std::unique_ptr<CharacterWithoutFlyweight>> characters_;
    
public:
    void addCharacter(char ch, const std::string& font, int size,
                     const std::string& color, bool bold, bool italic,
                     int x, int y) {
        characters_.push_back(
            std::make_unique<CharacterWithoutFlyweight>(ch, font, size, color, bold, italic, x, y)
        );
    }
    
    void render() const {
        for (const auto& ch : characters_) {
            ch->render();
        }
        std::cout << std::endl;
    }
    
    size_t getCharacterCount() const {
        return characters_.size();
    }
    
    size_t getTotalMemory() const {
        size_t total = sizeof(*this);
        for (const auto& ch : characters_) {
            total += ch->getMemorySize();
        }
        return total;
    }
    
    void printStats() const {
        std::cout << "\n=== Document WITHOUT Flyweight ===" << std::endl;
        std::cout << "Количество символов: " << characters_.size() << std::endl;
        std::cout << "Общая память: " << (getTotalMemory() / 1024) << " KB" << std::endl;
        std::cout << "Память на символ: " << (getTotalMemory() / characters_.size()) << " bytes" << std::endl;
        std::cout << "===================================" << std::endl;
    }
};

// Текстовый документ С Flyweight
class DocumentWithFlyweight {
private:
    std::vector<std::unique_ptr<CharacterWithFlyweight>> characters_;
    std::shared_ptr<CharacterStyleFactory> style_factory_;
    
public:
    DocumentWithFlyweight()
        : style_factory_(std::make_shared<CharacterStyleFactory>()) {}
    
    void addCharacter(char ch, const std::string& font, int size,
                     const std::string& color, bool bold, bool italic,
                     int x, int y) {
        auto style = style_factory_->getStyle(font, size, color, bold, italic);
        characters_.push_back(
            std::make_unique<CharacterWithFlyweight>(ch, style, x, y)
        );
    }
    
    void render() const {
        for (const auto& ch : characters_) {
            ch->render();
        }
        std::cout << std::endl;
    }
    
    size_t getCharacterCount() const {
        return characters_.size();
    }
    
    size_t getTotalMemory() const {
        size_t total = sizeof(*this);
        
        // Память символов (только уникальное состояние)
        for (const auto& ch : characters_) {
            total += ch->getMemorySize();
        }
        
        // Память разделяемых стилей
        total += style_factory_->getTotalMemory();
        
        return total;
    }
    
    void printStats() const {
        std::cout << "\n=== Document WITH Flyweight ===" << std::endl;
        std::cout << "Количество символов: " << characters_.size() << std::endl;
        std::cout << "Уникальных стилей: " << style_factory_->getStyleCount() << std::endl;
        std::cout << "Общая память: " << (getTotalMemory() / 1024) << " KB" << std::endl;
        std::cout << "Память на символ: " << (getTotalMemory() / characters_.size()) << " bytes" << std::endl;
        std::cout << "====================================" << std::endl;
        
        style_factory_->printStats();
    }
};

// Генератор тестового текста
std::string generateTestText(size_t length) {
    std::string text;
    text.reserve(length);
    
    const std::string sample = "The quick brown fox jumps over the lazy dog. "
                               "Lorem ipsum dolor sit amet, consectetur adipiscing elit. ";
    
    for (size_t i = 0; i < length; ++i) {
        text += sample[i % sample.length()];
    }
    
    return text;
}

// Сравнение подходов
void compareMemoryUsage() {
    std::cout << "\n=== Сравнение использования памяти ===" << std::endl;
    
    const size_t TEXT_LENGTH = 10000;
    std::string test_text = generateTestText(TEXT_LENGTH);
    
    std::cout << "\nТестовый текст: " << TEXT_LENGTH << " символов" << std::endl;
    
    // БЕЗ Flyweight
    std::cout << "\n--- Создание документа БЕЗ Flyweight ---" << std::endl;
    g_memory_tracker.reset();
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    {
        DocumentWithoutFlyweight doc;
        
        int x = 0, y = 0;
        for (char ch : test_text) {
            doc.addCharacter(ch, "Arial", 12, "Black", false, false, x++, y);
            if (x >= 80) { x = 0; y++; }
        }
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        doc.printStats();
        std::cout << "Время создания: " << duration.count() << " ms" << std::endl;
    }
    
    g_memory_tracker.printStats();
    size_t memory_without_flyweight = g_memory_tracker.getPeakBytes();
    
    // С Flyweight
    std::cout << "\n--- Создание документа С Flyweight ---" << std::endl;
    g_memory_tracker.reset();
    
    start_time = std::chrono::high_resolution_clock::now();
    
    {
        DocumentWithFlyweight doc;
        
        int x = 0, y = 0;
        for (size_t i = 0; i < test_text.length(); ++i) {
            char ch = test_text[i];
            
            // Варьируем стили для реализма
            std::string font = (i / 100 % 2 == 0) ? "Arial" : "Times New Roman";
            int size = 12;
            std::string color = (i / 50 % 2 == 0) ? "Black" : "Blue";
            bool bold = (i / 200 % 2 == 0);
            bool italic = (i / 150 % 2 == 0);
            
            doc.addCharacter(ch, font, size, color, bold, italic, x++, y);
            if (x >= 80) { x = 0; y++; }
        }
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        doc.printStats();
        std::cout << "Время создания: " << duration.count() << " ms" << std::endl;
    }
    
    g_memory_tracker.printStats();
    size_t memory_with_flyweight = g_memory_tracker.getPeakBytes();
    
    // Итоговое сравнение
    std::cout << "\n=== Итоговое сравнение ===" << std::endl;
    std::cout << "Память БЕЗ Flyweight: " << (memory_without_flyweight / 1024) << " KB" << std::endl;
    std::cout << "Память С Flyweight: " << (memory_with_flyweight / 1024) << " KB" << std::endl;
    
    double saving = 100.0 * (memory_without_flyweight - memory_with_flyweight) / memory_without_flyweight;
    std::cout << "Экономия памяти: " << saving << "%" << std::endl;
    std::cout << "=========================" << std::endl;
}

// Демонстрация масштабируемости
void demonstrateScalability() {
    std::cout << "\n=== Демонстрация масштабируемости ===" << std::endl;
    
    std::vector<size_t> sizes = {1000, 5000, 10000, 50000};
    
    for (size_t size : sizes) {
        std::cout << "\n--- Размер текста: " << size << " символов ---" << std::endl;
        
        std::string test_text = generateTestText(size);
        
        g_memory_tracker.reset();
        
        {
            DocumentWithFlyweight doc;
            
            int x = 0, y = 0;
            for (size_t i = 0; i < test_text.length(); ++i) {
                char ch = test_text[i];
                
                std::string font = (i / 100 % 3 == 0) ? "Arial" : 
                                  (i / 100 % 3 == 1) ? "Times New Roman" : "Courier";
                int size_val = 10 + (i / 500 % 3) * 2;  // 10, 12, 14
                std::string color = (i / 50 % 4 == 0) ? "Black" : 
                                   (i / 50 % 4 == 1) ? "Blue" : 
                                   (i / 50 % 4 == 2) ? "Red" : "Green";
                bool bold = (i / 200 % 2 == 0);
                bool italic = (i / 150 % 2 == 0);
                
                doc.addCharacter(ch, font, size_val, color, bold, italic, x++, y);
                if (x >= 80) { x = 0; y++; }
            }
            
            std::cout << "Память: " << (doc.getTotalMemory() / 1024) << " KB" << std::endl;
            std::cout << "Память на символ: " << (doc.getTotalMemory() / size) << " bytes" << std::endl;
        }
        
        std::cout << "Peak память: " << (g_memory_tracker.getPeakBytes() / 1024) << " KB" << std::endl;
    }
}

int main() {
    std::cout << "=== Flyweight Pattern: Memory Optimization ===" << std::endl;
    
    try {
        compareMemoryUsage();
        demonstrateScalability();
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\n=== Демонстрация завершена ===" << std::endl;
    return 0;
}
