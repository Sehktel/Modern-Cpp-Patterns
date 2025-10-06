#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <algorithm>
#include <stdexcept>

/**
 * @file product_factory.cpp
 * @brief Продвинутые примеры Factory Method паттерна
 * 
 * Этот файл демонстрирует более сложные сценарии использования
 * Factory Method в реальных приложениях.
 */

// ============================================================================
// СИСТЕМА СОЗДАНИЯ ИГРОВЫХ ОБЪЕКТОВ
// ============================================================================

/**
 * @brief Базовый класс для игровых объектов
 */
class GameObject {
protected:
    std::string name_;
    int health_;
    int damage_;
    
public:
    GameObject(const std::string& name, int health, int damage)
        : name_(name), health_(health), damage_(damage) {}
    
    virtual ~GameObject() = default;
    
    virtual void update() = 0;
    virtual void render() = 0;
    virtual void attack() = 0;
    virtual std::string getType() const = 0;
    
    const std::string& getName() const { return name_; }
    int getHealth() const { return health_; }
    int getDamage() const { return damage_; }
    
    void takeDamage(int damage) {
        health_ -= damage;
        if (health_ < 0) health_ = 0;
    }
    
    bool isAlive() const { return health_ > 0; }
};

/**
 * @brief Игрок
 */
class Player : public GameObject {
private:
    int experience_;
    int level_;
    
public:
    Player(const std::string& name, int health = 100, int damage = 20)
        : GameObject(name, health, damage), experience_(0), level_(1) {}
    
    void update() override {
        std::cout << "Игрок " << name_ << " обновляется (уровень " << level_ << ")" << std::endl;
    }
    
    void render() override {
        std::cout << "🛡️ Игрок: " << name_ << " [HP: " << health_ << ", DMG: " << damage_ 
                  << ", LVL: " << level_ << "]" << std::endl;
    }
    
    void attack() override {
        std::cout << "⚔️ Игрок " << name_ << " атакует с уроном " << damage_ << std::endl;
    }
    
    std::string getType() const override {
        return "Player";
    }
    
    void gainExperience(int exp) {
        experience_ += exp;
        if (experience_ >= 100) {
            levelUp();
        }
    }
    
private:
    void levelUp() {
        level_++;
        health_ += 20;
        damage_ += 5;
        experience_ = 0;
        std::cout << "🎉 Игрок " << name_ << " повысил уровень до " << level_ << "!" << std::endl;
    }
};

/**
 * @brief Враг
 */
class Enemy : public GameObject {
private:
    std::string enemyType_;
    
public:
    Enemy(const std::string& name, const std::string& type, int health = 50, int damage = 15)
        : GameObject(name, health, damage), enemyType_(type) {}
    
    void update() override {
        std::cout << "Враг " << name_ << " (" << enemyType_ << ") обновляется" << std::endl;
    }
    
    void render() override {
        std::cout << "👹 Враг: " << name_ << " (" << enemyType_ << ") [HP: " << health_ 
                  << ", DMG: " << damage_ << "]" << std::endl;
    }
    
    void attack() override {
        std::cout << "🗡️ Враг " << name_ << " атакует с уроном " << damage_ << std::endl;
    }
    
    std::string getType() const override {
        return "Enemy";
    }
    
    const std::string& getEnemyType() const { return enemyType_; }
};

/**
 * @brief NPC (Non-Player Character)
 */
class NPC : public GameObject {
private:
    std::string dialogue_;
    
public:
    NPC(const std::string& name, const std::string& dialogue, int health = 25)
        : GameObject(name, health, 0), dialogue_(dialogue) {}
    
    void update() override {
        std::cout << "NPC " << name_ << " обновляется" << std::endl;
    }
    
    void render() override {
        std::cout << "👤 NPC: " << name_ << " [HP: " << health_ << "]" << std::endl;
    }
    
    void attack() override {
        std::cout << "NPC " << name_ << " не атакует (дружелюбный)" << std::endl;
    }
    
    std::string getType() const override {
        return "NPC";
    }
    
    void talk() {
        std::cout << "💬 " << name_ << " говорит: \"" << dialogue_ << "\"" << std::endl;
    }
};

/**
 * @brief Фабрика игровых объектов
 */
class GameObjectFactory {
public:
    virtual ~GameObjectFactory() = default;
    virtual std::unique_ptr<GameObject> createGameObject(const std::string& type, const std::string& name) = 0;
    virtual std::vector<std::string> getSupportedTypes() const = 0;
};

/**
 * @brief Стандартная фабрика игровых объектов
 */
class StandardGameObjectFactory : public GameObjectFactory {
public:
    std::unique_ptr<GameObject> createGameObject(const std::string& type, const std::string& name) override {
        if (type == "player") {
            return std::make_unique<Player>(name);
        } else if (type == "enemy") {
            return std::make_unique<Enemy>(name, "Standard");
        } else if (type == "npc") {
            return std::make_unique<NPC>(name, "Привет, путник!");
        }
        
        throw std::invalid_argument("Неподдерживаемый тип игрового объекта: " + type);
    }
    
    std::vector<std::string> getSupportedTypes() const override {
        return {"player", "enemy", "npc"};
    }
};

/**
 * @brief Расширенная фабрика с настройками
 */
class AdvancedGameObjectFactory : public GameObjectFactory {
private:
    std::map<std::string, std::map<std::string, int>> stats_;
    
public:
    AdvancedGameObjectFactory() {
        // Настройки для разных типов врагов
        stats_["goblin"] = {{"health", 30}, {"damage", 10}};
        stats_["orc"] = {{"health", 80}, {"damage", 25}};
        stats_["dragon"] = {{"health", 200}, {"damage", 50}};
        
        // Настройки для NPC
        stats_["merchant"] = {{"health", 50}};
        stats_["guard"] = {{"health", 100}, {"damage", 20}};
    }
    
    std::unique_ptr<GameObject> createGameObject(const std::string& type, const std::string& name) override {
        if (type == "player") {
            return std::make_unique<Player>(name);
        } else if (type == "goblin") {
            auto stats = stats_["goblin"];
            return std::make_unique<Enemy>(name, "Goblin", stats["health"], stats["damage"]);
        } else if (type == "orc") {
            auto stats = stats_["orc"];
            return std::make_unique<Enemy>(name, "Orc", stats["health"], stats["damage"]);
        } else if (type == "dragon") {
            auto stats = stats_["dragon"];
            return std::make_unique<Enemy>(name, "Dragon", stats["health"], stats["damage"]);
        } else if (type == "merchant") {
            auto stats = stats_["merchant"];
            return std::make_unique<NPC>(name, "Хочешь купить что-нибудь?", stats["health"]);
        } else if (type == "guard") {
            auto stats = stats_["guard"];
            return std::make_unique<NPC>(name, "Стой! Кто идет?", stats["health"]);
        }
        
        throw std::invalid_argument("Неподдерживаемый тип игрового объекта: " + type);
    }
    
    std::vector<std::string> getSupportedTypes() const override {
        return {"player", "goblin", "orc", "dragon", "merchant", "guard"};
    }
};

// ============================================================================
// СИСТЕМА СОЗДАНИЯ UI ЭЛЕМЕНТОВ
// ============================================================================

/**
 * @brief Базовый класс для UI элементов
 */
class UIElement {
protected:
    std::string id_;
    int x_, y_;
    int width_, height_;
    
public:
    UIElement(const std::string& id, int x, int y, int width, int height)
        : id_(id), x_(x), y_(y), width_(width), height_(height) {}
    
    virtual ~UIElement() = default;
    
    virtual void render() = 0;
    virtual void handleClick() = 0;
    virtual void setText(const std::string& text) {}
    virtual std::string getType() const = 0;
    
    const std::string& getId() const { return id_; }
    int getX() const { return x_; }
    int getY() const { return y_; }
    int getWidth() const { return width_; }
    int getHeight() const { return height_; }
};

/**
 * @brief Кнопка
 */
class Button : public UIElement {
private:
    std::string text_;
    std::function<void()> onClickHandler_;
    
public:
    Button(const std::string& id, int x, int y, int width, int height, const std::string& text = "")
        : UIElement(id, x, y, width, height), text_(text) {}
    
    void render() override {
        std::cout << "🔘 Кнопка [" << id_ << "] '" << text_ << "' в (" << x_ << "," << y_ 
                  << ") размером " << width_ << "x" << height_ << std::endl;
    }
    
    void handleClick() override {
        std::cout << "🖱️ Кнопка '" << text_ << "' нажата!" << std::endl;
        if (onClickHandler_) {
            onClickHandler_();
        }
    }
    
    void setText(const std::string& text) override {
        text_ = text;
    }
    
    std::string getType() const override {
        return "Button";
    }
    
    void setOnClickHandler(std::function<void()> handler) {
        onClickHandler_ = handler;
    }
};

/**
 * @brief Текстовое поле
 */
class TextBox : public UIElement {
private:
    std::string text_;
    std::string placeholder_;
    
public:
    TextBox(const std::string& id, int x, int y, int width, int height, const std::string& placeholder = "")
        : UIElement(id, x, y, width, height), placeholder_(placeholder) {}
    
    void render() override {
        std::string displayText = text_.empty() ? placeholder_ : text_;
        std::cout << "📝 Текстовое поле [" << id_ << "] '" << displayText_ << "' в (" << x_ << "," << y_ 
                  << ") размером " << width_ << "x" << height_ << std::endl;
    }
    
    void handleClick() override {
        std::cout << "🖱️ Текстовое поле '" << id_ << "' активировано для ввода" << std::endl;
    }
    
    void setText(const std::string& text) override {
        text_ = text;
    }
    
    std::string getType() const override {
        return "TextBox";
    }
    
    const std::string& getText() const { return text_; }
};

/**
 * @brief Метка
 */
class Label : public UIElement {
private:
    std::string text_;
    
public:
    Label(const std::string& id, int x, int y, int width, int height, const std::string& text = "")
        : UIElement(id, x, y, width, height), text_(text) {}
    
    void render() override {
        std::cout << "🏷️ Метка [" << id_ << "] '" << text_ << "' в (" << x_ << "," << y_ 
                  << ") размером " << width_ << "x" << height_ << std::endl;
    }
    
    void handleClick() override {
        std::cout << "🖱️ Метка '" << text_ << "' нажата (неактивна)" << std::endl;
    }
    
    void setText(const std::string& text) override {
        text_ = text;
    }
    
    std::string getType() const override {
        return "Label";
    }
};

/**
 * @brief Фабрика UI элементов
 */
class UIElementFactory {
public:
    virtual ~UIElementFactory() = default;
    virtual std::unique_ptr<UIElement> createElement(const std::string& type, const std::string& id,
                                                    int x, int y, int width, int height) = 0;
};

/**
 * @brief Стандартная фабрика UI элементов
 */
class StandardUIElementFactory : public UIElementFactory {
public:
    std::unique_ptr<UIElement> createElement(const std::string& type, const std::string& id,
                                           int x, int y, int width, int height) override {
        if (type == "button") {
            return std::make_unique<Button>(id, x, y, width, height);
        } else if (type == "textbox") {
            return std::make_unique<TextBox>(id, x, y, width, height);
        } else if (type == "label") {
            return std::make_unique<Label>(id, x, y, width, height);
        }
        
        throw std::invalid_argument("Неподдерживаемый тип UI элемента: " + type);
    }
};

// ============================================================================
// ДЕМОНСТРАЦИОННЫЕ ФУНКЦИИ
// ============================================================================

/**
 * @brief Демонстрация создания игровых объектов
 */
void demonstrateGameObjectFactory() {
    std::cout << "\n=== Фабрика игровых объектов ===" << std::endl;
    
    auto factory = std::make_unique<AdvancedGameObjectFactory>();
    
    // Создаем игровые объекты
    std::vector<std::unique_ptr<GameObject>> gameObjects;
    
    gameObjects.push_back(factory->createGameObject("player", "Hero"));
    gameObjects.push_back(factory->createGameObject("goblin", "Goblin_1"));
    gameObjects.push_back(factory->createGameObject("orc", "Orc_Warrior"));
    gameObjects.push_back(factory->createGameObject("merchant", "Shop_Keeper"));
    gameObjects.push_back(factory->createGameObject("guard", "City_Guard"));
    
    // Отображаем все объекты
    std::cout << "\nИгровые объекты:" << std::endl;
    for (auto& obj : gameObjects) {
        obj->render();
    }
    
    // Симуляция боя
    std::cout << "\nСимуляция боя:" << std::endl;
    auto player = std::find_if(gameObjects.begin(), gameObjects.end(),
        [](const std::unique_ptr<GameObject>& obj) {
            return obj->getType() == "Player";
        });
    
    auto enemy = std::find_if(gameObjects.begin(), gameObjects.end(),
        [](const std::unique_ptr<GameObject>& obj) {
            return obj->getType() == "Enemy";
        });
    
    if (player != gameObjects.end() && enemy != gameObjects.end()) {
        (*player)->attack();
        (*enemy)->takeDamage((*player)->getDamage());
        
        (*enemy)->attack();
        (*player)->takeDamage((*enemy)->getDamage());
        
        std::cout << "\nПосле боя:" << std::endl;
        (*player)->render();
        (*enemy)->render();
    }
}

/**
 * @brief Демонстрация создания UI элементов
 */
void demonstrateUIElementFactory() {
    std::cout << "\n=== Фабрика UI элементов ===" << std::endl;
    
    auto factory = std::make_unique<StandardUIElementFactory>();
    
    // Создаем UI элементы
    std::vector<std::unique_ptr<UIElement>> elements;
    
    elements.push_back(factory->createElement("label", "title", 10, 10, 200, 30));
    elements.push_back(factory->createElement("textbox", "username", 10, 50, 200, 25));
    elements.push_back(factory->createElement("button", "login", 10, 85, 100, 30));
    elements.push_back(factory->createElement("button", "cancel", 120, 85, 100, 30));
    
    // Устанавливаем текст
    elements[0]->setText("Вход в систему");
    elements[1]->setText(""); // Пустое поле
    elements[2]->setText("Войти");
    elements[3]->setText("Отмена");
    
    // Отображаем элементы
    std::cout << "\nUI элементы:" << std::endl;
    for (auto& element : elements) {
        element->render();
    }
    
    // Симуляция кликов
    std::cout << "\nСимуляция кликов:" << std::endl;
    for (auto& element : elements) {
        element->handleClick();
    }
}

/**
 * @brief Демонстрация динамического создания объектов
 */
void demonstrateDynamicCreation() {
    std::cout << "\n=== Динамическое создание объектов ===" << std::endl;
    
    auto gameFactory = std::make_unique<AdvancedGameObjectFactory>();
    auto uiFactory = std::make_unique<StandardUIElementFactory>();
    
    // Создаем объекты на основе пользовательского ввода
    std::vector<std::string> gameObjectTypes = {"player", "goblin", "orc", "merchant"};
    std::vector<std::string> uiElementTypes = {"button", "textbox", "label"};
    
    std::cout << "\nСоздание игровых объектов:" << std::endl;
    for (size_t i = 0; i < gameObjectTypes.size(); ++i) {
        try {
            auto obj = gameFactory->createGameObject(gameObjectTypes[i], "Dynamic_" + std::to_string(i));
            obj->render();
        } catch (const std::exception& e) {
            std::cout << "Ошибка создания игрового объекта: " << e.what() << std::endl;
        }
    }
    
    std::cout << "\nСоздание UI элементов:" << std::endl;
    for (size_t i = 0; i < uiElementTypes.size(); ++i) {
        try {
            auto element = uiFactory->createElement(uiElementTypes[i], "dynamic_" + std::to_string(i), 
                                                   10 + i * 110, 10, 100, 30);
            element->render();
        } catch (const std::exception& e) {
            std::cout << "Ошибка создания UI элемента: " << e.what() << std::endl;
        }
    }
}

// ============================================================================
// ОСНОВНАЯ ФУНКЦИЯ
// ============================================================================

int main() {
    std::cout << "🎮 Продвинутые примеры Factory Method" << std::endl;
    std::cout << std::string(50, '=') << std::endl;
    
    demonstrateGameObjectFactory();
    demonstrateUIElementFactory();
    demonstrateDynamicCreation();
    
    std::cout << "\n✅ Демонстрация продвинутых примеров завершена!" << std::endl;
    std::cout << "\n🎯 Ключевые выводы:" << std::endl;
    std::cout << "• Factory Method обеспечивает гибкость создания объектов" << std::endl;
    std::cout << "• Разные фабрики могут создавать объекты с разными настройками" << std::endl;
    std::cout << "• Легко добавлять новые типы объектов без изменения существующего кода" << std::endl;
    std::cout << "• Паттерн особенно полезен для игровых движков и UI фреймворков" << std::endl;
    std::cout << "• Всегда обрабатывайте ошибки создания объектов" << std::endl;
    
    return 0;
}
