#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <random>
#include <algorithm>
#include <cmath>

/**
 * @file game_strategies.cpp
 * @brief Практические примеры Strategy паттерна в игровой разработке
 * 
 * Этот файл демонстрирует использование Strategy паттерна
 * в контексте игровой разработки и ИИ.
 */

// ============================================================================
// ИГРОВЫЕ СТРАТЕГИИ ИИ
// ============================================================================

/**
 * @brief Интерфейс стратегии ИИ
 */
class AIStrategy {
public:
    virtual ~AIStrategy() = default;
    virtual std::string getAction(const std::string& situation) = 0;
    virtual std::string getStrategyName() const = 0;
    virtual int getAggressiveness() const = 0; // 0-100
};

/**
 * @brief Агрессивная стратегия
 */
class AggressiveStrategy : public AIStrategy {
private:
    std::mt19937 rng_;
    
public:
    AggressiveStrategy() : rng_(std::random_device{}()) {}
    
    std::string getAction(const std::string& situation) override {
        std::vector<std::string> actions = {
            "Атаковать ближайшего врага",
            "Использовать мощное заклинание",
            "Броситься в бой",
            "Применить боевой крик",
            "Активировать берсерк"
        };
        
        std::uniform_int_distribution<int> dist(0, static_cast<int>(actions.size()) - 1);
        return actions[dist(rng_)];
    }
    
    std::string getStrategyName() const override {
        return "Агрессивная";
    }
    
    int getAggressiveness() const override {
        return 90;
    }
};

/**
 * @brief Защитная стратегия
 */
class DefensiveStrategy : public AIStrategy {
private:
    std::mt19937 rng_;
    
public:
    DefensiveStrategy() : rng_(std::random_device{}()) {}
    
    std::string getAction(const std::string& situation) override {
        std::vector<std::string> actions = {
            "Поднять щит",
            "Отступить к безопасному месту",
            "Использовать защитное заклинание",
            "Лечить союзников",
            "Ждать подходящего момента"
        };
        
        std::uniform_int_distribution<int> dist(0, static_cast<int>(actions.size()) - 1);
        return actions[dist(rng_)];
    }
    
    std::string getStrategyName() const override {
        return "Защитная";
    }
    
    int getAggressiveness() const override {
        return 20;
    }
};

/**
 * @brief Сбалансированная стратегия
 */
class BalancedStrategy : public AIStrategy {
private:
    std::mt19937 rng_;
    
public:
    BalancedStrategy() : rng_(std::random_device{}()) {}
    
    std::string getAction(const std::string& situation) override {
        std::vector<std::string> actions = {
            "Анализировать ситуацию",
            "Координировать с союзниками",
            "Использовать тактические способности",
            "Адаптироваться к противнику",
            "Поддерживать баланс атаки и защиты"
        };
        
        std::uniform_int_distribution<int> dist(0, static_cast<int>(actions.size()) - 1);
        return actions[dist(rng_)];
    }
    
    std::string getStrategyName() const override {
        return "Сбалансированная";
    }
    
    int getAggressiveness() const override {
        return 50;
    }
};

/**
 * @brief Случайная стратегия
 */
class RandomStrategy : public AIStrategy {
private:
    std::mt19937 rng_;
    
public:
    RandomStrategy() : rng_(std::random_device{}()) {}
    
    std::string getAction(const std::string& situation) override {
        std::vector<std::string> actions = {
            "Сделать что-то неожиданное",
            "Попробовать новую тактику",
            "Экспериментировать",
            "Действовать интуитивно",
            "Импровизировать"
        };
        
        std::uniform_int_distribution<int> dist(0, static_cast<int>(actions.size()) - 1);
        return actions[dist(rng_)];
    }
    
    std::string getStrategyName() const override {
        return "Случайная";
    }
    
    int getAggressiveness() const override {
        return 50; // Случайная агрессивность
    }
};

/**
 * @brief Игровой ИИ
 */
class GameAI {
private:
    std::unique_ptr<AIStrategy> strategy_;
    std::string aiName_;
    int health_;
    int mana_;
    
public:
    GameAI(const std::string& name, int health = 100, int mana = 100)
        : aiName_(name), health_(health), mana_(mana) {}
    
    void setStrategy(std::unique_ptr<AIStrategy> strategy) {
        strategy_ = std::move(strategy);
    }
    
    void act(const std::string& situation) {
        if (strategy_) {
            std::string action = strategy_->getAction(situation);
            std::cout << "🤖 " << aiName_ << " (" << strategy_->getStrategyName() 
                      << "): " << action << std::endl;
        } else {
            std::cout << "🤖 " << aiName_ << ": Стратегия не установлена!" << std::endl;
        }
    }
    
    void takeDamage(int damage) {
        health_ -= damage;
        std::cout << "💔 " << aiName_ << " получает " << damage 
                  << " урона. Здоровье: " << health_ << std::endl;
    }
    
    void heal(int amount) {
        health_ += amount;
        std::cout << "💚 " << aiName_ << " восстанавливает " << amount 
                  << " здоровья. Здоровье: " << health_ << std::endl;
    }
    
    int getHealth() const { return health_; }
    int getMana() const { return mana_; }
    const std::string& getName() const { return aiName_; }
    
    std::string getCurrentStrategyName() const {
        return strategy_ ? strategy_->getStrategyName() : "No Strategy";
    }
};

// ============================================================================
// СТРАТЕГИИ ДВИЖЕНИЯ
// ============================================================================

/**
 * @brief Интерфейс стратегии движения
 */
class MovementStrategy {
public:
    virtual ~MovementStrategy() = default;
    virtual std::pair<int, int> calculateNextPosition(int currentX, int currentY, 
                                                     int targetX, int targetY) = 0;
    virtual std::string getMovementType() const = 0;
};

/**
 * @brief Прямолинейное движение
 */
class LinearMovement : public MovementStrategy {
public:
    std::pair<int, int> calculateNextPosition(int currentX, int currentY, 
                                            int targetX, int targetY) override {
        int dx = targetX - currentX;
        int dy = targetY - currentY;
        
        // Нормализуем направление
        if (dx != 0) dx = dx / std::abs(dx);
        if (dy != 0) dy = dy / std::abs(dy);
        
        return {currentX + dx, currentY + dy};
    }
    
    std::string getMovementType() const override {
        return "Прямолинейное";
    }
};

/**
 * @brief Движение по диагонали
 */
class DiagonalMovement : public MovementStrategy {
public:
    std::pair<int, int> calculateNextPosition(int currentX, int currentY, 
                                            int targetX, int targetY) override {
        int dx = targetX - currentX;
        int dy = targetY - currentY;
        
        // Движение по диагонали
        if (dx != 0 && dy != 0) {
            dx = dx / std::abs(dx);
            dy = dy / std::abs(dy);
        } else if (dx != 0) {
            dx = dx / std::abs(dx);
            dy = 0;
        } else if (dy != 0) {
            dx = 0;
            dy = dy / std::abs(dy);
        }
        
        return {currentX + dx, currentY + dy};
    }
    
    std::string getMovementType() const override {
        return "Диагональное";
    }
};

/**
 * @brief Зигзагообразное движение
 */
class ZigzagMovement : public MovementStrategy {
private:
    bool horizontalFirst_ = true;
    
public:
    std::pair<int, int> calculateNextPosition(int currentX, int currentY, 
                                            int targetX, int targetY) override {
        int dx = targetX - currentX;
        int dy = targetY - currentY;
        
        if (horizontalFirst_) {
            if (dx != 0) {
                horizontalFirst_ = false;
                return {currentX + (dx / std::abs(dx)), currentY};
            } else if (dy != 0) {
                horizontalFirst_ = false;
                return {currentX, currentY + (dy / std::abs(dy))};
            }
        } else {
            if (dy != 0) {
                horizontalFirst_ = true;
                return {currentX, currentY + (dy / std::abs(dy))};
            } else if (dx != 0) {
                horizontalFirst_ = true;
                return {currentX + (dx / std::abs(dx)), currentY};
            }
        }
        
        return {currentX, currentY};
    }
    
    std::string getMovementType() const override {
        return "Зигзагообразное";
    }
};

/**
 * @brief Игровой объект с движением
 */
class GameObject {
private:
    std::unique_ptr<MovementStrategy> movementStrategy_;
    std::string name_;
    int x_, y_;
    
public:
    GameObject(const std::string& name, int x = 0, int y = 0)
        : name_(name), x_(x), y_(y) {}
    
    void setMovementStrategy(std::unique_ptr<MovementStrategy> strategy) {
        movementStrategy_ = std::move(strategy);
    }
    
    void moveTo(int targetX, int targetY) {
        if (movementStrategy_) {
            auto [newX, newY] = movementStrategy_->calculateNextPosition(x_, y_, targetX, targetY);
            std::cout << "🎮 " << name_ << " (" << movementStrategy_->getMovementType() 
                      << ") перемещается с (" << x_ << "," << y_ << ") на (" 
                      << newX << "," << newY << ")" << std::endl;
            x_ = newX;
            y_ = newY;
        } else {
            std::cout << "🎮 " << name_ << ": Стратегия движения не установлена!" << std::endl;
        }
    }
    
    std::pair<int, int> getPosition() const {
        return {x_, y_};
    }
    
    const std::string& getName() const {
        return name_;
    }
};

// ============================================================================
// СТРАТЕГИИ АТАКИ
// ============================================================================

/**
 * @brief Интерфейс стратегии атаки
 */
class AttackStrategy {
public:
    virtual ~AttackStrategy() = default;
    virtual int calculateDamage(int baseDamage, int level) = 0;
    virtual std::string getAttackType() const = 0;
    virtual double getCriticalChance() const = 0;
};

/**
 * @brief Физическая атака
 */
class PhysicalAttack : public AttackStrategy {
public:
    int calculateDamage(int baseDamage, int level) override {
        return baseDamage + (level * 2);
    }
    
    std::string getAttackType() const override {
        return "Физическая";
    }
    
    double getCriticalChance() const override {
        return 0.15; // 15% шанс критического удара
    }
};

/**
 * @brief Магическая атака
 */
class MagicalAttack : public AttackStrategy {
public:
    int calculateDamage(int baseDamage, int level) override {
        return static_cast<int>(baseDamage * 1.5) + (level * 3);
    }
    
    std::string getAttackType() const override {
        return "Магическая";
    }
    
    double getCriticalChance() const override {
        return 0.25; // 25% шанс критического удара
    }
};

/**
 * @brief Атака ядом
 */
class PoisonAttack : public AttackStrategy {
public:
    int calculateDamage(int baseDamage, int level) override {
        return baseDamage + (level * 1); // Меньше урона, но с эффектом
    }
    
    std::string getAttackType() const override {
        return "Ядовитая";
    }
    
    double getCriticalChance() const override {
        return 0.10; // 10% шанс критического удара
    }
};

/**
 * @brief Игровой персонаж
 */
class GameCharacter {
private:
    std::unique_ptr<AttackStrategy> attackStrategy_;
    std::string name_;
    int level_;
    int baseDamage_;
    
public:
    GameCharacter(const std::string& name, int level = 1, int baseDamage = 10)
        : name_(name), level_(level), baseDamage_(baseDamage) {}
    
    void setAttackStrategy(std::unique_ptr<AttackStrategy> strategy) {
        attackStrategy_ = std::move(strategy);
    }
    
    int attack() {
        if (attackStrategy_) {
            int damage = attackStrategy_->calculateDamage(baseDamage_, level_);
            std::cout << "⚔️ " << name_ << " (" << attackStrategy_->getAttackType() 
                      << ") атакует с уроном " << damage << std::endl;
            return damage;
        } else {
            std::cout << "⚔️ " << name_ << ": Стратегия атаки не установлена!" << std::endl;
            return 0;
        }
    }
    
    std::string getAttackType() const {
        return attackStrategy_ ? attackStrategy_->getAttackType() : "No Strategy";
    }
    
    double getCriticalChance() const {
        return attackStrategy_ ? attackStrategy_->getCriticalChance() : 0.0;
    }
    
    const std::string& getName() const {
        return name_;
    }
};

// ============================================================================
// ДЕМОНСТРАЦИОННЫЕ ФУНКЦИИ
// ============================================================================

/**
 * @brief Демонстрация стратегий ИИ
 */
void demonstrateAIStrategies() {
    std::cout << "\n=== Стратегии ИИ ===" << std::endl;
    
    GameAI ai1("Воин", 120, 50);
    GameAI ai2("Маг", 80, 150);
    GameAI ai3("Разбойник", 100, 80);
    GameAI ai4("Странник", 90, 100);
    
    // Устанавливаем разные стратегии
    ai1.setStrategy(std::make_unique<AggressiveStrategy>());
    ai2.setStrategy(std::make_unique<DefensiveStrategy>());
    ai3.setStrategy(std::make_unique<BalancedStrategy>());
    ai4.setStrategy(std::make_unique<RandomStrategy>());
    
    std::vector<GameAI*> ais = {&ai1, &ai2, &ai3, &ai4};
    
    // Симулируем несколько ситуаций
    std::vector<std::string> situations = {
        "Враг приближается",
        "Союзник ранен",
        "Обнаружен сокровище",
        "Ловушка активирована"
    };
    
    for (const auto& situation : situations) {
        std::cout << "\n--- Ситуация: " << situation << " ---" << std::endl;
        for (auto* ai : ais) {
            ai->act(situation);
        }
    }
}

/**
 * @brief Демонстрация стратегий движения
 */
void demonstrateMovementStrategies() {
    std::cout << "\n=== Стратегии движения ===" << std::endl;
    
    GameObject player("Игрок", 0, 0);
    GameObject enemy("Враг", 5, 5);
    GameObject npc("NPC", 3, 3);
    
    // Устанавливаем разные стратегии движения
    player.setMovementStrategy(std::make_unique<LinearMovement>());
    enemy.setMovementStrategy(std::make_unique<DiagonalMovement>());
    npc.setMovementStrategy(std::make_unique<ZigzagMovement>());
    
    std::vector<GameObject*> objects = {&player, &enemy, &npc};
    
    // Симулируем движение к цели
    int targetX = 10, targetY = 10;
    std::cout << "Цель: (" << targetX << "," << targetY << ")" << std::endl;
    
    for (int step = 0; step < 5; ++step) {
        std::cout << "\n--- Шаг " << (step + 1) << " ---" << std::endl;
        for (auto* obj : objects) {
            obj->moveTo(targetX, targetY);
        }
    }
}

/**
 * @brief Демонстрация стратегий атаки
 */
void demonstrateAttackStrategies() {
    std::cout << "\n=== Стратегии атаки ===" << std::endl;
    
    GameCharacter warrior("Воин", 5, 15);
    GameCharacter mage("Маг", 8, 12);
    GameCharacter assassin("Убийца", 6, 18);
    
    // Устанавливаем разные стратегии атаки
    warrior.setAttackStrategy(std::make_unique<PhysicalAttack>());
    mage.setAttackStrategy(std::make_unique<MagicalAttack>());
    assassin.setAttackStrategy(std::make_unique<PoisonAttack>());
    
    std::vector<GameCharacter*> characters = {&warrior, &mage, &assassin};
    
    // Симулируем несколько атак
    for (int round = 1; round <= 3; ++round) {
        std::cout << "\n--- Раунд " << round << " ---" << std::endl;
        for (auto* character : characters) {
            character->attack();
        }
    }
    
    // Показываем характеристики
    std::cout << "\n--- Характеристики персонажей ---" << std::endl;
    for (auto* character : characters) {
        std::cout << character->getName() << ":" << std::endl;
        std::cout << "  Тип атаки: " << character->getAttackType() << std::endl;
        std::cout << "  Шанс критического удара: " << (character->getCriticalChance() * 100) << "%" << std::endl;
    }
}

/**
 * @brief Демонстрация динамической смены стратегий
 */
void demonstrateDynamicStrategyChange() {
    std::cout << "\n=== Динамическая смена стратегий ===" << std::endl;
    
    GameAI adaptiveAI("Адаптивный ИИ", 100, 100);
    
    // Начинаем с агрессивной стратегии
    adaptiveAI.setStrategy(std::make_unique<AggressiveStrategy>());
    std::cout << "Начальная стратегия: " << adaptiveAI.getCurrentStrategyName() << std::endl;
    adaptiveAI.act("Враг обнаружен");
    
    // Меняем на защитную при низком здоровье
    adaptiveAI.takeDamage(80);
    std::cout << "\nЗдоровье низкое, меняем стратегию на защитную..." << std::endl;
    adaptiveAI.setStrategy(std::make_unique<DefensiveStrategy>());
    adaptiveAI.act("Враг приближается");
    
    // Лечимся и меняем на сбалансированную
    adaptiveAI.heal(60);
    std::cout << "\nЗдоровье восстановлено, меняем на сбалансированную стратегию..." << std::endl;
    adaptiveAI.setStrategy(std::make_unique<BalancedStrategy>());
    adaptiveAI.act("Ситуация стабилизировалась");
}

// ============================================================================
// ОСНОВНАЯ ФУНКЦИЯ
// ============================================================================

int main() {
    std::cout << "🎮 Стратегии в игровой разработке" << std::endl;
    std::cout << std::string(50, '=') << std::endl;
    
    demonstrateAIStrategies();
    demonstrateMovementStrategies();
    demonstrateAttackStrategies();
    demonstrateDynamicStrategyChange();
    
    std::cout << "\n✅ Демонстрация игровых стратегий завершена!" << std::endl;
    std::cout << "\n🎯 Ключевые выводы:" << std::endl;
    std::cout << "• Strategy паттерн идеально подходит для игрового ИИ" << std::endl;
    std::cout << "• Различные стратегии движения создают разнообразие поведения" << std::endl;
    std::cout << "• Стратегии атаки позволяют создавать уникальных персонажей" << std::endl;
    std::cout << "• Динамическая смена стратегий делает ИИ более адаптивным" << std::endl;
    std::cout << "• Паттерн упрощает добавление новых типов поведения" << std::endl;
    
    return 0;
}
