# Урок 9.2: Flyweight Pattern (Приспособленец)

## 🎯 Цель урока
Изучить паттерн Flyweight - структурный паттерн для минимизации потребления памяти через разделение общего состояния между множеством объектов. Понять, как поддерживать большое количество мелких объектов эффективно.

## 📚 Что изучаем

### 1. Паттерн Flyweight
- **Определение**: Разделение intrinsic (разделяемого) и extrinsic (уникального) состояния объектов
- **Назначение**: Экономия памяти, поддержка большого количества объектов
- **Применение**: Текстовые редакторы, игровые движки, графические системы, UI frameworks

### 2. Ключевые концепции
- **Intrinsic State**: Разделяемое состояние (font, texture, material)
- **Extrinsic State**: Уникальное состояние (position, velocity, rotation)
- **Flyweight Factory**: Управление разделяемыми объектами
- **Sharing**: Множество объектов → один Flyweight

### 3. Когда использовать
- Много объектов с дублирующимся состоянием
- Память - критичный ресурс
- Intrinsic state > Extrinsic state
- High sharing ratio (100+ объектов на 1 flyweight)

## 🔍 Ключевые концепции

### Базовая реализация

```cpp
#include <iostream>
#include <memory>
#include <unordered_map>
#include <string>
#include <mutex>

// INTRINSIC STATE (Разделяемое) - Flyweight
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
    
    std::string getKey() const {
        return font_family_ + "_" + std::to_string(font_size_) + 
               "_" + color_ + "_" + std::to_string(bold_) + 
               "_" + std::to_string(italic_);
    }
    
    void applyStyle() const {
        // Применить стиль
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
    std::shared_ptr<CharacterStyle> getStyle(
        const std::string& font, int size,
        const std::string& color, bool bold, bool italic) {
        
        std::lock_guard<std::mutex> lock(mutex_);
        
        CharacterStyle temp(font, size, color, bold, italic);
        std::string key = temp.getKey();
        
        // Ищем существующий
        auto it = styles_.find(key);
        if (it != styles_.end()) {
            return it->second;  // Возвращаем существующий
        }
        
        // Создаем новый
        auto style = std::make_shared<CharacterStyle>(
            font, size, color, bold, italic);
        styles_[key] = style;
        
        return style;
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
    std::shared_ptr<CharacterStyle> style_;  // Разделяемое!
    int position_x_;
    int position_y_;
    
public:
    Character(char ch, std::shared_ptr<CharacterStyle> style, int x, int y)
        : character_(ch), style_(std::move(style)), 
          position_x_(x), position_y_(y) {}
    
    void render() const {
        style_->applyStyle();
        std::cout << character_;
    }
    
    size_t getMemorySize() const {
        return sizeof(*this);  // Только уникальное состояние
    }
};
```

### Экономия памяти

```cpp
// БЕЗ Flyweight
struct CharacterWithoutFlyweight {
    char character;           // 1 byte
    std::string font_family;  // ~40 bytes
    int font_size;            // 4 bytes
    std::string color;        // ~40 bytes
    bool bold;                // 1 byte
    bool italic;              // 1 byte
    int position_x;           // 4 bytes
    int position_y;           // 4 bytes
};
// Итого: ~95 bytes на символ

// 10,000 символов × 95 bytes = 950 KB

// С Flyweight
struct CharacterWithFlyweight {
    char character;                           // 1 byte
    std::shared_ptr<CharacterStyle> style;   // 8 bytes (указатель)
    int position_x;                           // 4 bytes
    int position_y;                           // 4 bytes
};
// Итого: ~17 bytes на символ + разделяемые стили

// 10,000 символов × 17 bytes + 5 стилей × 86 bytes = 170 KB

// ЭКОНОМИЯ: 950 KB → 170 KB = 82% меньше! 🚀
```

## 🎓 Best Practices

### ✅ DO (Рекомендуется)

1. **Делайте Flyweight IMMUTABLE**
   ```cpp
   class CharacterStyle {
   private:
       const std::string font_;  // const!
       const int size_;          // const!
   };
   ```

2. **Используйте для heavy objects**
   - Текстуры (8 MB каждая)
   - Материалы, шейдеры
   - Шрифты, стили

3. **std::shared_ptr для управления**
   - Автоматический reference counting
   - Thread-safe
   - Автоматическая очистка

4. **Мониторьте sharing ratio**
   - objects / flyweights > 10
   - Если < 10, Flyweight не окупается

### ❌ DON'T (Не рекомендуется)

1. **НЕ используйте для легковесных объектов**
   - int, char, small structs
   - Overhead > выгода

2. **НЕ делайте Flyweight mutable**
   - Изменение → все объекты изменяются
   - Трудно отследить bugs

3. **НЕ храните extrinsic state в Flyweight**
   - Position в Flyweight ❌
   - Не будет разделяться правильно

## 📊 Примеры из реального мира

### Текстовый редактор
```
10,000 символов, 5 стилей
БЕЗ: 10,000 × 95 bytes = 950 KB
С:   10,000 × 17 bytes + 5 × 86 = 170 KB
Экономия: 82%
```

### Игровой движок (Particles)
```
100,000 частиц, 10 типов
БЕЗ: 100,000 × 8.15 MB = 815 GB (невозможно!)
С:   100,000 × 36 bytes + 10 × 8.15 MB = 85 MB
Экономия: 99.99%! (9,500x меньше)
```

### Tile Map
```
1000×1000 тайлов, 50 типов
БЕЗ: 1M × 200 bytes = 200 MB
С:   1M × 12 bytes + 50 × 200 = 12 MB
Экономия: 94%
```

## 📁 Файлы урока

- `flyweight_pattern.cpp` - Базовая реализация
- `memory_optimization.cpp` - Анализ экономии памяти, сравнения
- `flyweight_vulnerabilities.cpp` - Уязвимости
- `secure_flyweight_alternatives.cpp` - Безопасные альтернативы
- `SECURITY_ANALYSIS.md` - Анализ безопасности

## 🔗 Связанные паттерны

- **Object Pool**: Переиспользование объектов (lifecycle)
- **Factory**: Создание flyweights
- **Singleton**: Factory часто singleton
- **Composite**: Flyweight для листовых узлов

## 🚀 Практическое применение

### Используется в
- **Java String Interning**: Разделение строк
- **Game Engines**: Particles, tiles, sprites
- **Web Browsers**: DOM nodes, CSS styles
- **Text Editors**: Character formatting

### Когда использовать
✅ Много объектов с дублирующимся состоянием  
✅ Память - bottleneck  
✅ Четкое разделение intrinsic/extrinsic  
✅ Immutable shared state

### Когда НЕ использовать
❌ Мало объектов (< 100)  
❌ Уникальное state > разделяемое  
❌ Mutable shared state  
❌ Low sharing ratio (< 10)

---

**Автор**: Senior C++ Developer  
**Дата**: 2025-10-07  
**Версия**: 1.0
