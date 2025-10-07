# 🏋️ Упражнения: Flyweight Pattern

## 📋 Задание 1: Character Flyweight для текстового редактора

### Описание
Реализуйте Flyweight Pattern для символов в текстовом редакторе с разделением стилей и позиций.

### Требования
1. ✅ CharacterStyle как Flyweight (font, size, color)
2. ✅ Character с уникальным состоянием (char, position)
3. ✅ CharacterStyleFactory для управления
4. ✅ Метрики: экономия памяти, количество стилей
5. ✅ Сравнение с/без Flyweight

### Шаблон для реализации
```cpp
#include <iostream>
#include <memory>
#include <unordered_map>
#include <string>
#include <mutex>

// INTRINSIC STATE (Разделяемое)
class CharacterStyle {
private:
    std::string font_family_;
    int font_size_;
    std::string color_;
    bool bold_;
    bool italic_;
    
public:
    CharacterStyle(const std::string& font, int size, 
                  const std::string& color, bool bold, bool italic)
        : font_family_(font), font_size_(size), color_(color),
          bold_(bold), italic_(italic) {}
    
    // TODO: Реализуйте getKey() для уникальной идентификации
    std::string getKey() const {
        // Ваш код здесь
        return "";
    }
    
    size_t getMemorySize() const {
        return sizeof(*this) + font_family_.capacity() + color_.capacity();
    }
};

// Flyweight Factory
class CharacterStyleFactory {
private:
    std::unordered_map<std::string, std::shared_ptr<CharacterStyle>> styles_;
    mutable std::mutex mutex_;
    
public:
    // TODO: Реализуйте getStyle (создание или возврат существующего)
    std::shared_ptr<CharacterStyle> getStyle(
        const std::string& font, int size,
        const std::string& color, bool bold, bool italic) {
        
        std::lock_guard<std::mutex> lock(mutex_);
        
        // TODO: 1. Создайте ключ
        // TODO: 2. Проверьте наличие в cache
        // TODO: 3. Если нет - создайте новый
        // TODO: 4. Верните shared_ptr
    }
    
    size_t getStyleCount() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return styles_.size();
    }
};

// EXTRINSIC STATE (Уникальное)
class Character {
private:
    char character_;
    std::shared_ptr<CharacterStyle> style_;  // Flyweight
    int position_x_;
    int position_y_;
    
public:
    Character(char ch, std::shared_ptr<CharacterStyle> style, int x, int y)
        : character_(ch), style_(std::move(style)), 
          position_x_(x), position_y_(y) {}
    
    void render() const {
        std::cout << character_;
    }
    
    size_t getMemorySize() const {
        return sizeof(*this);  // Только уникальное состояние
    }
};
```

### Тесты
```cpp
void testMemorySavings() {
    CharacterStyleFactory factory;
    
    // Создаем 10,000 символов с ограниченным набором стилей
    std::vector<Character> document;
    
    auto style1 = factory.getStyle("Arial", 12, "Black", false, false);
    auto style2 = factory.getStyle("Arial", 14, "Red", true, false);
    
    for (int i = 0; i < 10000; ++i) {
        auto style = (i % 10 == 0) ? style2 : style1;
        document.emplace_back('A', style, i % 80, i / 80);
    }
    
    // Вычисляем экономию памяти
    size_t memory_with_flyweight = 
        document.size() * sizeof(Character) +
        factory.getStyleCount() * sizeof(CharacterStyle);
    
    size_t memory_without_flyweight = 
        document.size() * (sizeof(Character) + sizeof(CharacterStyle));
    
    double savings = 100.0 * (memory_without_flyweight - memory_with_flyweight) 
                    / memory_without_flyweight;
    
    std::cout << "Memory savings: " << savings << "%" << std::endl;
    assert(savings > 50 && "Should save at least 50% memory");
    
    std::cout << "Memory savings test PASSED ✅" << std::endl;
}
```

---

## 📋 Задание 2: Particle System Flyweight

### Описание
Реализуйте Flyweight для particle system в игровом движке.

### Требования
1. ✅ ParticleType как Flyweight (texture, shader, material)
2. ✅ Particle с уникальным состоянием (position, velocity)
3. ✅ ParticleFactory для управления типами
4. ✅ Batch rendering по типам
5. ✅ Метрики производительности

### Шаблон
```cpp
// Flyweight: Разделяемые данные частицы
struct ParticleType {
    std::string texture_path;
    std::string shader_name;
    float mass;
    float lifetime;
    
    size_t getMemorySize() const {
        // TODO: Вычислите размер (включая текстуру, шейдер)
        return 0;
    }
};

// Particle: Уникальные данные
struct Particle {
    std::shared_ptr<ParticleType> type;  // Flyweight
    float pos_x, pos_y, pos_z;
    float vel_x, vel_y, vel_z;
    float rotation;
    float age;
    
    void update(float dt) {
        // TODO: Обновление физики
        pos_x += vel_x * dt;
        pos_y += vel_y * dt;
        pos_z += vel_z * dt;
        age += dt;
    }
    
    void render() const {
        // TODO: Рендеринг используя type->texture и shader
    }
};

class ParticleSystem {
private:
    std::vector<Particle> particles_;
    std::shared_ptr<ParticleTypeFactory> type_factory_;
    
public:
    // TODO: Добавление частицы
    void emit(const std::string& type_name, float x, float y, float z) {
        auto type = type_factory_->getType(type_name);
        particles_.emplace_back(/* ... */);
    }
    
    // TODO: Batch rendering по типам
    void render() {
        // TODO: Сгруппируйте по type
        // TODO: Batch render для каждого типа
    }
};
```

---

## 📋 Задание 3: Tile Map Flyweight

### Описание
Реализуйте Flyweight для tile-based игры (карта из тайлов).

### Требования
1. ✅ TileType как Flyweight
2. ✅ Tile с позицией на карте
3. ✅ Efficient storage для большой карты (1000×1000)
4. ✅ Быстрый рендеринг
5. ✅ Serialization/Deserialization карты

### Шаблон
```cpp
enum class TerrainType {
    GRASS,
    WATER,
    MOUNTAIN,
    FOREST,
    DESERT
};

struct TileType {
    TerrainType terrain;
    std::string sprite_path;
    bool walkable;
    float movement_cost;
    
    // TODO: Реализуйте TileType
};

class TileMap {
private:
    size_t width_;
    size_t height_;
    std::vector<std::shared_ptr<TileType>> tiles_;  // width * height
    TileTypeFactory factory_;
    
public:
    TileMap(size_t width, size_t height) 
        : width_(width), height_(height) {
        tiles_.resize(width * height);
    }
    
    // TODO: Установка тайла
    void setTile(size_t x, size_t y, TerrainType type) {
        auto tile_type = factory_.getType(type);
        tiles_[y * width_ + x] = tile_type;
    }
    
    // TODO: Вычисление памяти
    size_t getMemoryUsage() const {
        // TODO: sizeof tiles array + sizeof unique TileTypes
    }
};
```

---

## 📋 Задание 4: Flyweight с Weak Pointers

### Описание
Реализуйте Flyweight Factory с автоматической очисткой неиспользуемых flyweights через `std::weak_ptr`.

### Требования
1. ✅ std::weak_ptr в factory map
2. ✅ Автоматическое удаление когда нет ссылок
3. ✅ Periodic cleanup неиспользуемых
4. ✅ Метрики: active vs garbage collected flyweights

### Шаблон
```cpp
template<typename Key, typename Flyweight>
class WeakFlyweightFactory {
private:
    std::unordered_map<Key, std::weak_ptr<Flyweight>> flyweights_;
    std::mutex mutex_;
    
public:
    std::shared_ptr<Flyweight> get(const Key& key, 
                                   std::function<std::shared_ptr<Flyweight>()> creator) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // TODO: 1. Проверить weak_ptr
        auto it = flyweights_.find(key);
        if (it != flyweights_.end()) {
            auto shared = it->second.lock();
            if (shared) {
                return shared;  // Flyweight еще жив
            }
        }
        
        // TODO: 2. Создать новый
        auto flyweight = creator();
        flyweights_[key] = flyweight;
        
        return flyweight;
    }
    
    // TODO: Очистка expired weak_ptrs
    void cleanup() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        for (auto it = flyweights_.begin(); it != flyweights_.end(); ) {
            if (it->second.expired()) {
                it = flyweights_.erase(it);
            } else {
                ++it;
            }
        }
    }
};
```

---

**Автор**: Senior C++ Developer  
**Дата**: 2025-10-07  
**Сложность**: ⭐⭐⭐⭐ (Продвинутый уровень)
