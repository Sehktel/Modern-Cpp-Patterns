/**
 * @file flyweight_pattern.cpp
 * @brief Демонстрация Flyweight Pattern
 * 
 * Реализован Flyweight Pattern с поддержкой:
 * - Разделяемое и неразделяемое состояние
 * - Фабрика flyweight объектов
 * - Оптимизация памяти
 * - Применение в играх и GUI
 */

#include <iostream>
#include <unordered_map>
#include <memory>
#include <string>
#include <vector>
#include <chrono>
#include <random>
#include <map>

// Базовый интерфейс для Flyweight
class Flyweight {
public:
    virtual ~Flyweight() = default;
    virtual void render(int x, int y, const std::string& extrinsic_data) = 0;
    virtual std::string getIntrinsicState() const = 0;
};

// Конкретный Flyweight для символов
class CharacterFlyweight : public Flyweight {
private:
    char character_;
    std::string font_;
    int size_;
    std::string color_;
    
public:
    CharacterFlyweight(char c, const std::string& font, int size, const std::string& color)
        : character_(c), font_(font), size_(size), color_(color) {
        std::cout << "Создан CharacterFlyweight для символа '" << character_ << "'" << std::endl;
    }
    
    void render(int x, int y, const std::string& extrinsic_data) override {
        std::cout << "Рендерим символ '" << character_ 
                  << "' в позиции (" << x << ", " << y << ")"
                  << " с данными: " << extrinsic_data << std::endl;
    }
    
    std::string getIntrinsicState() const override {
        return std::string(1, character_) + "_" + font_ + "_" + std::to_string(size_) + "_" + color_;
    }
    
    char getCharacter() const { return character_; }
    const std::string& getFont() const { return font_; }
    int getSize() const { return size_; }
    const std::string& getColor() const { return color_; }
};

// Конкретный Flyweight для деревьев в игре
class TreeFlyweight : public Flyweight {
private:
    std::string tree_type_;
    std::string texture_;
    int height_;
    std::string season_;
    
public:
    TreeFlyweight(const std::string& type, const std::string& texture, int height, const std::string& season)
        : tree_type_(type), texture_(texture), height_(height), season_(season) {
        std::cout << "Создан TreeFlyweight для типа '" << tree_type_ << "'" << std::endl;
    }
    
    void render(int x, int y, const std::string& extrinsic_data) override {
        std::cout << "Рендерим дерево типа '" << tree_type_ 
                  << "' в позиции (" << x << ", " << y << ")"
                  << " с данными: " << extrinsic_data << std::endl;
    }
    
    std::string getIntrinsicState() const override {
        return tree_type_ + "_" + texture_ + "_" + std::to_string(height_) + "_" + season_;
    }
    
    const std::string& getTreeType() const { return tree_type_; }
    const std::string& getTexture() const { return texture_; }
    int getHeight() const { return height_; }
    const std::string& getSeason() const { return season_; }
};

// Конкретный Flyweight для кнопок GUI
class ButtonFlyweight : public Flyweight {
private:
    std::string button_type_;
    std::string style_;
    int width_;
    int height_;
    std::string color_scheme_;
    
public:
    ButtonFlyweight(const std::string& type, const std::string& style, int width, int height, const std::string& color)
        : button_type_(type), style_(style), width_(width), height_(height), color_scheme_(color) {
        std::cout << "Создан ButtonFlyweight для типа '" << button_type_ << "'" << std::endl;
    }
    
    void render(int x, int y, const std::string& extrinsic_data) override {
        std::cout << "Рендерим кнопку типа '" << button_type_ 
                  << "' в позиции (" << x << ", " << y << ")"
                  << " с данными: " << extrinsic_data << std::endl;
    }
    
    std::string getIntrinsicState() const override {
        return button_type_ + "_" + style_ + "_" + std::to_string(width_) + "x" + std::to_string(height_) + "_" + color_scheme_;
    }
    
    const std::string& getButtonType() const { return button_type_; }
    const std::string& getStyle() const { return style_; }
    int getWidth() const { return width_; }
    int getHeight() const { return height_; }
    const std::string& getColorScheme() const { return color_scheme_; }
};

// Фабрика Flyweight объектов
class FlyweightFactory {
private:
    std::unordered_map<std::string, std::shared_ptr<Flyweight>> flyweights_;
    
public:
    // Получение или создание CharacterFlyweight
    std::shared_ptr<CharacterFlyweight> getCharacter(char c, const std::string& font, int size, const std::string& color) {
        std::string key = std::string(1, c) + "_" + font + "_" + std::to_string(size) + "_" + color;
        
        auto it = flyweights_.find(key);
        if (it != flyweights_.end()) {
            return std::dynamic_pointer_cast<CharacterFlyweight>(it->second);
        }
        
        auto flyweight = std::make_shared<CharacterFlyweight>(c, font, size, color);
        flyweights_[key] = flyweight;
        return flyweight;
    }
    
    // Получение или создание TreeFlyweight
    std::shared_ptr<TreeFlyweight> getTree(const std::string& type, const std::string& texture, int height, const std::string& season) {
        std::string key = type + "_" + texture + "_" + std::to_string(height) + "_" + season;
        
        auto it = flyweights_.find(key);
        if (it != flyweights_.end()) {
            return std::dynamic_pointer_cast<TreeFlyweight>(it->second);
        }
        
        auto flyweight = std::make_shared<TreeFlyweight>(type, texture, height, season);
        flyweights_[key] = flyweight;
        return flyweight;
    }
    
    // Получение или создание ButtonFlyweight
    std::shared_ptr<ButtonFlyweight> getButton(const std::string& type, const std::string& style, int width, int height, const std::string& color) {
        std::string key = type + "_" + style + "_" + std::to_string(width) + "x" + std::to_string(height) + "_" + color;
        
        auto it = flyweights_.find(key);
        if (it != flyweights_.end()) {
            return std::dynamic_pointer_cast<ButtonFlyweight>(it->second);
        }
        
        auto flyweight = std::make_shared<ButtonFlyweight>(type, style, width, height, color);
        flyweights_[key] = flyweight;
        return flyweight;
    }
    
    size_t getFlyweightCount() const {
        return flyweights_.size();
    }
    
    void printStats() const {
        std::cout << "FlyweightFactory: создано " << flyweights_.size() << " уникальных flyweight объектов" << std::endl;
    }
};

// Контекст для использования Flyweight
class TextContext {
private:
    std::shared_ptr<CharacterFlyweight> character_;
    int x_, y_;
    std::string additional_data_;
    
public:
    TextContext(std::shared_ptr<CharacterFlyweight> ch, int x, int y, const std::string& data)
        : character_(ch), x_(x), y_(y), additional_data_(data) {}
    
    void render() {
        character_->render(x_, y_, additional_data_);
    }
    
    int getX() const { return x_; }
    int getY() const { return y_; }
    const std::string& getAdditionalData() const { return additional_data_; }
};

// Контекст для деревьев в игре
class TreeContext {
private:
    std::shared_ptr<TreeFlyweight> tree_;
    int x_, y_;
    std::string additional_data_;
    
public:
    TreeContext(std::shared_ptr<TreeFlyweight> t, int x, int y, const std::string& data)
        : tree_(t), x_(x), y_(y), additional_data_(data) {}
    
    void render() {
        tree_->render(x_, y_, additional_data_);
    }
    
    int getX() const { return x_; }
    int getY() const { return y_; }
    const std::string& getAdditionalData() const { return additional_data_; }
};

// Контекст для кнопок GUI
class ButtonContext {
private:
    std::shared_ptr<ButtonFlyweight> button_;
    int x_, y_;
    std::string additional_data_;
    
public:
    ButtonContext(std::shared_ptr<ButtonFlyweight> b, int x, int y, const std::string& data)
        : button_(b), x_(x), y_(y), additional_data_(data) {}
    
    void render() {
        button_->render(x_, y_, additional_data_);
    }
    
    int getX() const { return x_; }
    int getY() const { return y_; }
    const std::string& getAdditionalData() const { return additional_data_; }
};

// Демонстрация текстового редактора
void demonstrateTextEditor() {
    std::cout << "\n=== Демонстрация текстового редактора ===" << std::endl;
    
    FlyweightFactory factory;
    std::vector<TextContext> text_contexts;
    
    std::string text = "Hello World!";
    std::vector<std::string> fonts = {"Arial", "Times", "Courier"};
    std::vector<int> sizes = {12, 14, 16, 18};
    std::vector<std::string> colors = {"black", "red", "blue", "green"};
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> font_dis(0, fonts.size() - 1);
    std::uniform_int_distribution<> size_dis(0, sizes.size() - 1);
    std::uniform_int_distribution<> color_dis(0, colors.size() - 1);
    
    // Создаем контексты для каждого символа
    for (size_t i = 0; i < text.length(); ++i) {
        char c = text[i];
        std::string font = fonts[font_dis(gen)];
        int size = sizes[size_dis(gen)];
        std::string color = colors[color_dis(gen)];
        
        auto character = factory.getCharacter(c, font, size, color);
        text_contexts.emplace_back(character, i * 10, 0, "text_editor");
    }
    
    std::cout << "Создано " << text_contexts.size() << " контекстов текста" << std::endl;
    factory.printStats();
    
    // Рендерим текст
    std::cout << "Рендерим текст:" << std::endl;
    for (const auto& context : text_contexts) {
        context.render();
    }
}

// Демонстрация игрового мира
void demonstrateGameWorld() {
    std::cout << "\n=== Демонстрация игрового мира ===" << std::endl;
    
    FlyweightFactory factory;
    std::vector<TreeContext> tree_contexts;
    
    std::vector<std::string> tree_types = {"Oak", "Pine", "Birch", "Maple"};
    std::vector<std::string> textures = {"bark_1", "bark_2", "bark_3"};
    std::vector<int> heights = {100, 150, 200, 250};
    std::vector<std::string> seasons = {"spring", "summer", "autumn", "winter"};
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> type_dis(0, tree_types.size() - 1);
    std::uniform_int_distribution<> texture_dis(0, textures.size() - 1);
    std::uniform_int_distribution<> height_dis(0, heights.size() - 1);
    std::uniform_int_distribution<> season_dis(0, seasons.size() - 1);
    std::uniform_int_distribution<> pos_dis(0, 1000);
    
    // Создаем 100 деревьев
    for (int i = 0; i < 100; ++i) {
        std::string type = tree_types[type_dis(gen)];
        std::string texture = textures[texture_dis(gen)];
        int height = heights[height_dis(gen)];
        std::string season = seasons[season_dis(gen)];
        
        auto tree = factory.getTree(type, texture, height, season);
        tree_contexts.emplace_back(tree, pos_dis(gen), pos_dis(gen), "game_world");
    }
    
    std::cout << "Создано " << tree_contexts.size() << " деревьев" << std::endl;
    factory.printStats();
    
    // Рендерим первые 10 деревьев
    std::cout << "Рендерим первые 10 деревьев:" << std::endl;
    for (int i = 0; i < 10; ++i) {
        tree_contexts[i].render();
    }
}

// Демонстрация GUI интерфейса
void demonstrateGUI() {
    std::cout << "\n=== Демонстрация GUI интерфейса ===" << std::endl;
    
    FlyweightFactory factory;
    std::vector<ButtonContext> button_contexts;
    
    std::vector<std::string> button_types = {"OK", "Cancel", "Save", "Delete", "Edit"};
    std::vector<std::string> styles = {"flat", "raised", "sunken"};
    std::vector<std::pair<int, int>> sizes = {{80, 30}, {100, 40}, {120, 35}};
    std::vector<std::string> colors = {"blue", "green", "red", "gray"};
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> type_dis(0, button_types.size() - 1);
    std::uniform_int_distribution<> style_dis(0, styles.size() - 1);
    std::uniform_int_distribution<> size_dis(0, sizes.size() - 1);
    std::uniform_int_distribution<> color_dis(0, colors.size() - 1);
    std::uniform_int_distribution<> pos_dis(0, 500);
    
    // Создаем 50 кнопок
    for (int i = 0; i < 50; ++i) {
        std::string type = button_types[type_dis(gen)];
        std::string style = styles[style_dis(gen)];
        auto size = sizes[size_dis(gen)];
        std::string color = colors[color_dis(gen)];
        
        auto button = factory.getButton(type, style, size.first, size.second, color);
        button_contexts.emplace_back(button, pos_dis(gen), pos_dis(gen), "gui_window");
    }
    
    std::cout << "Создано " << button_contexts.size() << " кнопок" << std::endl;
    factory.printStats();
    
    // Рендерим первые 10 кнопок
    std::cout << "Рендерим первые 10 кнопок:" << std::endl;
    for (int i = 0; i < 10; ++i) {
        button_contexts[i].render();
    }
}

// Демонстрация производительности
void demonstratePerformance() {
    std::cout << "\n=== Демонстрация производительности ===" << std::endl;
    
    FlyweightFactory factory;
    
    // Тест без Flyweight (создание множества объектов)
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::unique_ptr<CharacterFlyweight>> without_flyweight;
    for (int i = 0; i < 10000; ++i) {
        without_flyweight.push_back(std::make_unique<CharacterFlyweight>('A', "Arial", 12, "black"));
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto without_flyweight_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Без Flyweight: создано 10000 объектов за " << without_flyweight_time.count() << " микросекунд" << std::endl;
    
    // Тест с Flyweight (переиспользование объектов)
    start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::shared_ptr<CharacterFlyweight>> with_flyweight;
    for (int i = 0; i < 10000; ++i) {
        with_flyweight.push_back(factory.getCharacter('A', "Arial", 12, "black"));
    }
    
    end = std::chrono::high_resolution_clock::now();
    auto with_flyweight_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "С Flyweight: создано 10000 ссылок за " << with_flyweight_time.count() << " микросекунд" << std::endl;
    std::cout << "Уникальных объектов создано: " << factory.getFlyweightCount() << std::endl;
    
    double improvement = (double)without_flyweight_time.count() / with_flyweight_time.count();
    std::cout << "Улучшение производительности: " << improvement << "x" << std::endl;
}

// Демонстрация экономии памяти
void demonstrateMemorySavings() {
    std::cout << "\n=== Демонстрация экономии памяти ===" << std::endl;
    
    FlyweightFactory factory;
    
    // Создаем множество контекстов с повторяющимися flyweight объектами
    std::vector<TextContext> contexts;
    
    for (int i = 0; i < 1000; ++i) {
        char c = 'A' + (i % 26); // Циклически используем буквы A-Z
        auto character = factory.getCharacter(c, "Arial", 12, "black");
        contexts.emplace_back(character, i * 10, 0, "memory_test");
    }
    
    std::cout << "Создано 1000 контекстов текста" << std::endl;
    std::cout << "Уникальных flyweight объектов: " << factory.getFlyweightCount() << std::endl;
    
    // Без Flyweight потребовалось бы 1000 объектов
    // С Flyweight требуется только 26 объектов (A-Z)
    double memory_savings = (1000.0 - factory.getFlyweightCount()) / 1000.0 * 100;
    std::cout << "Экономия памяти: " << memory_savings << "%" << std::endl;
}

int main() {
    std::cout << "=== Flyweight Pattern ===" << std::endl;
    
    try {
        demonstrateTextEditor();
        demonstrateGameWorld();
        demonstrateGUI();
        demonstratePerformance();
        demonstrateMemorySavings();
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\n=== Демонстрация завершена ===" << std::endl;
    return 0;
}