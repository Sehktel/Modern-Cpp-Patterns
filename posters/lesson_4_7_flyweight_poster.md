# 📊 Плакат: Flyweight Pattern (Приспособленец)

## 🎯 Назначение паттерна Flyweight

```
┌─────────────────────────────────────────────────────────────────┐
│                        FLYWEIGHT PATTERN                       │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│  🪶 МИНИМИЗИРУЕТ ИСПОЛЬЗОВАНИЕ ПАМЯТИ ЧЕРЕЗ РАЗДЕЛЕНИЕ         │
│    ВНУТРЕННЕГО И ВНЕШНЕГО СОСТОЯНИЯ                             │
│  🔄 РАЗДЕЛЯЕТ ОБЩИЕ ДАННЫЕ МЕЖДУ МНОЖЕСТВЕННЫМИ ОБЪЕКТАМИ       │
│  🎯 ОПТИМИЗИРУЕТ ПРОИЗВОДИТЕЛЬНОСТЬ ПРИ РАБОТЕ С МНОЖЕСТВОМ    │
│    ОБЪЕКТОВ                                                     │
└─────────────────────────────────────────────────────────────────┘

Применение:
┌─────────────────┬───────────────────────────────────────────────┐
│   Text Editor   │ • Разделение символов и их форматирования    │
│                 │   (шрифт, размер, цвет)                      │
├─────────────────┼───────────────────────────────────────────────┤
│   Game Engine   │ • Разделение спрайтов и их позиций           │
│                 │   (текстура, анимация vs координаты)          │
├─────────────────┼───────────────────────────────────────────────┤
│   GUI Framework │ • Разделение виджетов и их состояний         │
│                 │   (стиль, размер vs позиция, видимость)       │
├─────────────────┼───────────────────────────────────────────────┤
│   Document      │ • Разделение символов и их позиций           │
│   Processing    │   (символ vs координаты, стиль)              │
├─────────────────┼───────────────────────────────────────────────┤
│   Web Browser   │ • Разделение DOM элементов и их стилей       │
│                 │   (CSS классы vs позиции, размеры)            │
└─────────────────┴───────────────────────────────────────────────┘
```

## 🔄 Проблема без Flyweight

```
┌─────────────────────────────────────────────────────────────────┐
│                    ПРОБЛЕМА: ИЗБЫТОК ПАМЯТИ                    │
└─────────────────────────────────────────────────────────────────┘

❌ ПЛОХО - Дублирование данных:
┌─────────────────────────────────────────────────────────────────┐
│ class Character {                                               │
│ private:                                                        │
│     char symbol_;                                               │
│     std::string font_;                                          │
│     int size_;                                                  │
│     std::string color_;                                         │
│     int x_, y_;                                                 │
│                                                                 │
│ public:                                                         │
│     Character(char symbol, const std::string& font,            │
│               int size, const std::string& color,               │
│               int x, int y)                                     │
│         : symbol_(symbol), font_(font), size_(size),            │
│           color_(color), x_(x), y_(y) {}                       │
│                                                                 │
│     void render() {                                             │
│         std::cout << "Rendering '" << symbol_ << "' at ("       │
│                   << x_ << ", " << y_ << ") with "             │
│                   << font_ << " " << size_ << "px " << color_   │
│                   << std::endl;                                 │
│     }                                                           │
│ };                                                              │
│                                                                 │
│ // Проблема: создание 1000 символов 'A' с одинаковым форматированием│
│ std::vector<std::unique_ptr<Character>> characters;            │
│ for (int i = 0; i < 1000; ++i) {                               │
│     characters.push_back(                                       │
│         std::make_unique<Character>('A', "Arial", 12, "black",  │
│                                     i % 100, i / 100));         │
│ }                                                               │
│                                                                 │
│ // Проблемы:                                                    │
│ // • Дублирование данных форматирования (font, size, color)    │
│ // • Избыточное использование памяти                           │
│ // • Медленная инициализация                                   │
│ // • Сложность изменения форматирования                        │
└─────────────────────────────────────────────────────────────────┘
```

## ✅ Решение с Flyweight

```
┌─────────────────────────────────────────────────────────────────┐
│                    РЕШЕНИЕ: РАЗДЕЛЕНИЕ СОСТОЯНИЯ               │
└─────────────────────────────────────────────────────────────────┘

✅ ХОРОШО - Разделение внутреннего и внешнего состояния:
┌─────────────────────────────────────────────────────────────────┐
│ // Внутреннее состояние (разделяемое)                           │
│ class CharacterStyle {                                          │
│ private:                                                        │
│     char symbol_;                                               │
│     std::string font_;                                          │
│     int size_;                                                  │
│     std::string color_;                                         │
│                                                                 │
│ public:                                                         │
│     CharacterStyle(char symbol, const std::string& font,       │
│                    int size, const std::string& color)          │
│         : symbol_(symbol), font_(font), size_(size), color_(color) {} │
│                                                                 │
│     void render(int x, int y) const {                           │
│         std::cout << "Rendering '" << symbol_ << "' at ("       │
│                   << x << ", " << y << ") with "               │
│                   << font_ << " " << size_ << "px " << color_   │
│                   << std::endl;                                 │
│     }                                                           │
│                                                                 │
│     char getSymbol() const { return symbol_; }                  │
│     const std::string& getFont() const { return font_; }        │
│     int getSize() const { return size_; }                       │
│     const std::string& getColor() const { return color_; }      │
│ };                                                              │
│                                                                 │
│ // Внешнее состояние (уникальное)                              │
│ class Character {                                               │
│ private:                                                        │
│     const CharacterStyle* style_;                               │
│     int x_, y_;                                                 │
│                                                                 │
│ public:                                                         │
│     Character(const CharacterStyle* style, int x, int y)        │
│         : style_(style), x_(x), y_(y) {}                        │
│                                                                 │
│     void render() const {                                       │
│         style_->render(x_, y_);                                 │
│     }                                                           │
│                                                                 │
│     int getX() const { return x_; }                             │
│     int getY() const { return y_; }                             │
│     void setPosition(int x, int y) { x_ = x; y_ = y; }         │
│ };                                                              │
│                                                                 │
│ // Фабрика для управления flyweight объектами                   │
│ class CharacterStyleFactory {                                   │
│ private:                                                        │
│     std::unordered_map<std::string, std::unique_ptr<CharacterStyle>> styles_; │
│                                                                 │
│ public:                                                         │
│     const CharacterStyle* getStyle(char symbol,                 │
│                                    const std::string& font,     │
│                                    int size,                    │
│                                    const std::string& color) {  │
│         std::string key = std::to_string(symbol) + font +       │
│                          std::to_string(size) + color;          │
│                                                                 │
│         auto it = styles_.find(key);                            │
│         if (it == styles_.end()) {                              │
│             styles_[key] = std::make_unique<CharacterStyle>(     │
│                 symbol, font, size, color);                     │
│             return styles_[key].get();                          │
│         }                                                       │
│         return it->second.get();                                │
│     }                                                           │
│                                                                 │
│     size_t getStyleCount() const { return styles_.size(); }     │
│ };                                                              │
└─────────────────────────────────────────────────────────────────┘

Использование:
┌─────────────────────────────────────────────────────────────────┐
│ CharacterStyleFactory factory;                                  │
│ std::vector<std::unique_ptr<Character>> characters;            │
│                                                                 │
│ // Создание 1000 символов 'A' с одинаковым стилем              │
│ const CharacterStyle* styleA = factory.getStyle('A', "Arial", 12, "black"); │
│ for (int i = 0; i < 1000; ++i) {                               │
│     characters.push_back(                                       │
│         std::make_unique<Character>(styleA, i % 100, i / 100)); │
│ }                                                               │
│                                                                 │
│ // Создание 500 символов 'B' с другим стилем                   │
│ const CharacterStyle* styleB = factory.getStyle('B', "Times", 14, "red"); │
│ for (int i = 0; i < 500; ++i) {                                │
│     characters.push_back(                                       │
│         std::make_unique<Character>(styleB, i % 50, i / 50));   │
│ }                                                               │
│                                                                 │
│ std::cout << "Total styles created: " << factory.getStyleCount() << std::endl; │
│ // Вывод: "Total styles created: 2" (вместо 1500!)            │
└─────────────────────────────────────────────────────────────────┘

Преимущества:
✅ Экономия памяти за счет разделения состояния
✅ Быстрая инициализация объектов
✅ Легкое изменение форматирования
✅ Соблюдение принципов SOLID
```

## 🏗️ Структура Flyweight

```
┌─────────────────────────────────────────────────────────────────┐
│                    СТРУКТУРА ПАТТЕРНА                          │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│                    Flyweight (Приспособленец)                  │
│  ┌─────────────────────────────────────────────────────────────┐│
│  │ class CharacterStyle {                                      ││
│  │ private:                                                    ││
│  │     char symbol_;                                           ││
│  │     std::string font_;                                      ││
│  │     int size_;                                              ││
│  │     std::string color_;                                     ││
│  │                                                             ││
│  │ public:                                                     ││
│  │     CharacterStyle(char symbol, const std::string& font,   ││
│  │                    int size, const std::string& color);    ││
│  │     void render(int x, int y) const;                       ││
│  │     // Геттеры для внутреннего состояния                    ││
│  │ };                                                          ││
│  └─────────────────────────────────────────────────────────────┘│
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│                    FlyweightFactory (Фабрика приспособленцев)  │
│  ┌─────────────────────────────────────────────────────────────┐│
│  │ class CharacterStyleFactory {                               ││
│  │ private:                                                    ││
│  │     std::unordered_map<std::string,                        ││
│  │                         std::unique_ptr<CharacterStyle>> styles_; ││
│  │                                                             ││
│  │ public:                                                     ││
│  │     const CharacterStyle* getStyle(char symbol,             ││
│  │                                    const std::string& font, ││
│  │                                    int size,                ││
│  │                                    const std::string& color); ││
│  │     size_t getStyleCount() const;                          ││
│  │ };                                                          ││
│  └─────────────────────────────────────────────────────────────┘│
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│                    Context (Контекст)                           │
│  ┌─────────────────────────────────────────────────────────────┐│
│  │ class Character {                                           ││
│  │ private:                                                    ││
│  │     const CharacterStyle* style_;                           ││
│  │     int x_, y_;                                             ││
│  │                                                             ││
│  │ public:                                                     ││
│  │     Character(const CharacterStyle* style, int x, int y);   ││
│  │     void render() const;                                   ││
│  │     int getX() const;                                       ││
│  │     int getY() const;                                       ││
│  │     void setPosition(int x, int y);                        ││
│  │ };                                                          ││
│  └─────────────────────────────────────────────────────────────┘│
└─────────────────────────────────────────────────────────────────┘
```

## 🔄 Последовательность взаимодействий

```
┌─────────────────────────────────────────────────────────────────┐
│                ПОСЛЕДОВАТЕЛЬНОСТЬ ВЗАИМОДЕЙСТВИЙ               │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────┬───────────────────────────────────────────────┐
│   1. Клиент     │ CharacterStyleFactory factory;                │
│      создает     │                                               │
│      фабрику    │                                               │
├─────────────────┼───────────────────────────────────────────────┤
│   2. Клиент     │ const CharacterStyle* style =                │
│      запрашивает│     factory.getStyle('A', "Arial", 12, "black"); │
│      стиль      │                                               │
├─────────────────┼───────────────────────────────────────────────┤
│   3. Фабрика    │ Проверяет существование стиля в кэше         │
│      проверяет  │ Если нет - создает новый                     │
│      кэш        │                                               │
├─────────────────┼───────────────────────────────────────────────┤
│   4. Клиент     │ Character character(style, 100, 200);        │
│      создает     │                                               │
│      объект     │                                               │
├─────────────────┼───────────────────────────────────────────────┤
│   5. Клиент     │ character.render();                          │
│      вызывает    │                                               │
│      метод      │                                               │
├─────────────────┼───────────────────────────────────────────────┤
│   6. Контекст   │ character.render() вызывает                  │
│      делегирует  │ style->render(100, 200);                      │
│      flyweight  │                                               │
└─────────────────┴───────────────────────────────────────────────┘

Диаграмма взаимодействий:
┌─────────────────────────────────────────────────────────────────┐
│  Client ──┐                                                    │
│           │                                                    │
│           ▼                                                    │
│  CharacterStyleFactory ──┐                                     │
│                           │                                     │
│                           ▼                                     │
│  CharacterStyle (cached) ──┐                                   │
│                              │                                  │
│                              ▼                                  │
│  Character.render() ──┐                                        │
│                        │                                        │
│                        ▼                                        │
│  CharacterStyle.render(100, 200)                               │
└─────────────────────────────────────────────────────────────────┘
```

## 🎨 Варианты реализации

### 1. Classic Flyweight (Классический)
```
┌─────────────────────────────────────────────────────────────────┐
│                    CLASSIC FLYWEIGHT                           │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│ class TreeType {                                                │
│ private:                                                        │
│     std::string name_;                                          │
│     std::string color_;                                         │
│     std::string texture_;                                      │
│                                                                 │
│ public:                                                         │
│     TreeType(const std::string& name, const std::string& color, │
│              const std::string& texture)                        │
│         : name_(name), color_(color), texture_(texture) {}      │
│                                                                 │
│     void render(int x, int y) const {                           │
│         std::cout << "Rendering " << name_ << " tree at ("      │
│                   << x << ", " << y << ") with " << color_      │
│                   << " color and " << texture_ << " texture"    │
│                   << std::endl;                                 │
│     }                                                           │
│                                                                 │
│     const std::string& getName() const { return name_; }        │
│     const std::string& getColor() const { return color_; }       │
│     const std::string& getTexture() const { return texture_; }   │
│ };                                                              │
│                                                                 │
│ class Tree {                                                    │
│ private:                                                        │
│     const TreeType* type_;                                     │
│     int x_, y_;                                                 │
│                                                                 │
│ public:                                                         │
│     Tree(const TreeType* type, int x, int y)                    │
│         : type_(type), x_(x), y_(y) {}                         │
│                                                                 │
│     void render() const {                                       │
│         type_->render(x_, y_);                                  │
│     }                                                           │
│                                                                 │
│     int getX() const { return x_; }                             │
│     int getY() const { return y_; }                             │
│     void setPosition(int x, int y) { x_ = x; y_ = y; }         │
│ };                                                              │
│                                                                 │
│ class TreeTypeFactory {                                         │
│ private:                                                        │
│     std::unordered_map<std::string, std::unique_ptr<TreeType>> types_; │
│                                                                 │
│ public:                                                         │
│     const TreeType* getTreeType(const std::string& name,        │
│                                 const std::string& color,       │
│                                 const std::string& texture) {   │
│         std::string key = name + color + texture;               │
│                                                                 │
│         auto it = types_.find(key);                             │
│         if (it == types_.end()) {                               │
│             types_[key] = std::make_unique<TreeType>(name, color, texture); │
│             return types_[key].get();                           │
│         }                                                       │
│         return it->second.get();                               │
│     }                                                           │
│                                                                 │
│     size_t getTypeCount() const { return types_.size(); }       │
│ };                                                              │
└─────────────────────────────────────────────────────────────────┘
```

### 2. Template Flyweight
```
┌─────────────────────────────────────────────────────────────────┐
│                    TEMPLATE FLYWEIGHT                          │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│ template<typename IntrinsicState>                               │
│ class Flyweight {                                               │
│ private:                                                        │
│     IntrinsicState state_;                                      │
│                                                                 │
│ public:                                                         │
│     Flyweight(const IntrinsicState& state) : state_(state) {}   │
│                                                                 │
│     template<typename ExtrinsicState>                           │
│     void render(const ExtrinsicState& extrinsic) const {        │
│         // Обработка с использованием внутреннего и внешнего состояния│
│         std::cout << "Rendering with intrinsic: " << state_     │
│                   << " and extrinsic: " << extrinsic << std::endl; │
│     }                                                           │
│                                                                 │
│     const IntrinsicState& getIntrinsicState() const {           │
│         return state_;                                          │
│     }                                                           │
│ };                                                              │
│                                                                 │
│ template<typename IntrinsicState>                               │
│ class FlyweightFactory {                                        │
│ private:                                                        │
│     std::unordered_map<IntrinsicState,                          │
│                        std::unique_ptr<Flyweight<IntrinsicState>>> flyweights_; │
│                                                                 │
│ public:                                                         │
│     const Flyweight<IntrinsicState>* getFlyweight(               │
│         const IntrinsicState& state) {                          │
│         auto it = flyweights_.find(state);                      │
│         if (it == flyweights_.end()) {                          │
│             flyweights_[state] = std::make_unique<Flyweight<IntrinsicState>>(state); │
│             return flyweights_[state].get();                   │
│         }                                                       │
│         return it->second.get();                               │
│     }                                                           │
│                                                                 │
│     size_t getFlyweightCount() const { return flyweights_.size(); } │
│ };                                                              │
│                                                                 │
│ // Использование                                               │
│ struct FontStyle {                                              │
│     std::string font;                                           │
│     int size;                                                   │
│     std::string color;                                          │
│                                                                 │
│     bool operator==(const FontStyle& other) const {             │
│         return font == other.font && size == other.size && color == other.color; │
│     }                                                           │
│ };                                                              │
│                                                                 │
│ struct FontStyleHash {                                          │
│     size_t operator()(const FontStyle& style) const {           │
│         return std::hash<std::string>{}(style.font) ^           │
│                std::hash<int>{}(style.size) ^                  │
│                std::hash<std::string>{}(style.color);           │
│     }                                                           │
│ };                                                              │
│                                                                 │
│ FlyweightFactory<FontStyle> factory;                            │
│ FontStyle style{"Arial", 12, "black"};                         │
│ auto flyweight = factory.getFlyweight(style);                   │
│ flyweight->render(std::make_pair(100, 200));                   │
└─────────────────────────────────────────────────────────────────┘
```

### 3. Functional Flyweight
```
┌─────────────────────────────────────────────────────────────────┐
│                    FUNCTIONAL FLYWEIGHT                        │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│ class FunctionalFlyweight {                                     │
│ private:                                                        │
│     std::string intrinsicState_;                                │
│     std::function<void(const std::string&, int, int)> renderFunc_; │
│                                                                 │
│ public:                                                         │
│     FunctionalFlyweight(const std::string& state,                │
│                        std::function<void(const std::string&, int, int)> renderFunc) │
│         : intrinsicState_(state), renderFunc_(renderFunc) {}     │
│                                                                 │
│     void render(int x, int y) const {                           │
│         renderFunc_(intrinsicState_, x, y);                     │
│     }                                                           │
│                                                                 │
│     const std::string& getIntrinsicState() const {              │
│         return intrinsicState_;                                 │
│     }                                                           │
│ };                                                              │
│                                                                 │
│ class FunctionalFlyweightFactory {                              │
│ private:                                                        │
│     std::unordered_map<std::string, std::unique_ptr<FunctionalFlyweight>> flyweights_; │
│                                                                 │
│ public:                                                         │
│     const FunctionalFlyweight* getFlyweight(                     │
│         const std::string& state,                               │
│         std::function<void(const std::string&, int, int)> renderFunc) { │
│         auto it = flyweights_.find(state);                      │
│         if (it == flyweights_.end()) {                          │
│             flyweights_[state] = std::make_unique<FunctionalFlyweight>(state, renderFunc); │
│             return flyweights_[state].get();                    │
│         }                                                       │
│         return it->second.get();                               │
│     }                                                           │
│                                                                 │
│     size_t getFlyweightCount() const { return flyweights_.size(); } │
│ };                                                              │
│                                                                 │
│ // Использование                                               │
│ FunctionalFlyweightFactory factory;                            │
│ auto renderFunc = [](const std::string& state, int x, int y) {  │
│     std::cout << "Rendering " << state << " at (" << x << ", " << y << ")" << std::endl; │
│ };                                                              │
│                                                                 │
│ auto flyweight = factory.getFlyweight("Tree", renderFunc);       │
│ flyweight->render(100, 200);                                   │
└─────────────────────────────────────────────────────────────────┘
```

## 🔄 Сравнение подходов

```
┌─────────────────────────────────────────────────────────────────┐
│                    СРАВНЕНИЕ ПОДХОДОВ                          │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────┬─────────────────┬─────────────────┬─────────────────┐
│   Classic       │   Template      │   Functional    │   Hybrid        │
│   Flyweight     │   Flyweight     │   Flyweight     │   Flyweight     │
├─────────────────┼─────────────────┼─────────────────┼─────────────────┤
│ Простота        │ Средняя          │ Высокая         │ Низкая          │
│ Гибкость        │ Высокая          │ Средняя         │ Очень высокая   │
│ Производительность│ Высокая        │ Высокая         │ Средняя         │
│ Тестируемость   │ Высокая          │ Средняя         │ Высокая         │
│ Типобезопасность│ Средняя          │ Высокая         │ Низкая          │
│ Применимость    │ Стандартные      │ Шаблонные       │ Функциональные  │
│                 │ случаи          │ случаи          │ случаи          │
└─────────────────┴─────────────────┴─────────────────┴─────────────────┘
```

## 🎯 Когда использовать Flyweight

```
┌─────────────────────────────────────────────────────────────────┐
│                КОГДА ИСПОЛЬЗОВАТЬ FLYWEIGHT                    │
└─────────────────────────────────────────────────────────────────┘

✅ ХОРОШИЕ СЛУЧАИ:
┌─────────────────┬───────────────────────────────────────────────┐
│ Множество       │ • Нужно создать много похожих объектов       │
│ объектов        │ • Объекты имеют общие характеристики          │
├─────────────────┼───────────────────────────────────────────────┤
│ Экономия памяти │ • Критична экономия памяти                   │
│                 │ • Объекты занимают много места                │
├─────────────────┼───────────────────────────────────────────────┤
│ Разделение      │ • Можно разделить состояние на внутреннее    │
│ состояния       │   и внешнее                                   │
│                 │ • Внутреннее состояние неизменно              │
├─────────────────┼───────────────────────────────────────────────┤
│ Производительность│ • Критична производительность создания     │
│                 │ • Нужно быстро создавать объекты             │
├─────────────────┼───────────────────────────────────────────────┤
│ Кэширование     │ • Нужно кэшировать объекты                   │
│                 │ • Объекты могут переиспользоваться            │
└─────────────────┴───────────────────────────────────────────────┘

❌ ПЛОХИЕ СЛУЧАИ:
┌─────────────────┬───────────────────────────────────────────────┐
│ Простые случаи  │ • Мало объектов или они сильно отличаются    │
├─────────────────┼───────────────────────────────────────────────┤
│ Сложность       │ • Добавляется ненужная сложность             │
├─────────────────┼───────────────────────────────────────────────┤
│ Производительность│ • Критична производительность доступа       │
├─────────────────┼───────────────────────────────────────────────┤
│ Альтернативы    │ • Есть лучшие альтернативы (Object Pool)    │
└─────────────────┴───────────────────────────────────────────────┘
```

## 🧪 Тестирование Flyweight

```
┌─────────────────────────────────────────────────────────────────┐
│                    ТЕСТИРОВАНИЕ FLYWEIGHT                     │
└─────────────────────────────────────────────────────────────────┘

Преимущества для тестирования:
┌─────────────────┬───────────────────────────────────────────────┐
│ Моки            │ ✅ Легко создавать моки для flyweight'ов     │
│                 │ class MockCharacterStyle : public CharacterStyle { │
│                 │     MOCK_METHOD(void, render, (int, int), (const)); │
│                 │ };                                            │
├─────────────────┼───────────────────────────────────────────────┤
│ Изоляция        │ ✅ Легко изолировать тесты                   │
│                 │ auto mockStyle = std::make_unique<MockCharacterStyle>(); │
│                 │ Character character(mockStyle.get(), 100, 200); │
├─────────────────┼───────────────────────────────────────────────┤
│ Управление      │ ✅ Полный контроль над flyweight'ами         │
│ зависимостями   │ EXPECT_CALL(*mockStyle, render(100, 200))     │
│                 │     .Times(1);                               │
└─────────────────┴───────────────────────────────────────────────┘

Пример теста:
┌─────────────────────────────────────────────────────────────────┐
│ TEST(FlyweightTest, ReuseFlyweight) {                           │
│     CharacterStyleFactory factory;                              │
│                                                                 │
│     auto style1 = factory.getStyle('A', "Arial", 12, "black");  │
│     auto style2 = factory.getStyle('A', "Arial", 12, "black");  │
│                                                                 │
│     EXPECT_EQ(style1, style2); // Должны быть одинаковыми       │
│     EXPECT_EQ(factory.getStyleCount(), 1); // Только один стиль │
│                                                                 │
│     Character char1(style1, 100, 200);                         │
│     Character char2(style2, 300, 400);                         │
│                                                                 │
│     char1.render();                                            │
│     char2.render();                                            │
│ }                                                               │
└─────────────────────────────────────────────────────────────────┘
```

## 🔗 Связь с другими паттернами

```
┌─────────────────────────────────────────────────────────────────┐
│              СВЯЗЬ С ДРУГИМИ ПАТТЕРНАМИ                        │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────┬───────────────────────────────────────────────┐
│ Factory Method  │ • Flyweight использует Factory Method       │
│                 │ • Factory Method для создания flyweight'ов   │
│                 │ • Flyweight - оптимизация                    │
│                 │ • Factory Method - создание                  │
├─────────────────┼───────────────────────────────────────────────┤
│ Singleton       │ • Flyweight может использовать Singleton     │
│                 │ • Singleton для глобального доступа к фабрике│
│                 │ • Flyweight - оптимизация                    │
│                 │ • Singleton - глобальный доступ             │
├─────────────────┼───────────────────────────────────────────────┤
│ Composite       │ • Flyweight может использоваться в Composite│
│                 │ • Composite для группировки flyweight'ов     │
│                 │ • Flyweight - оптимизация                    │
│                 │ • Composite - структура                      │
├─────────────────┼───────────────────────────────────────────────┤
│ State           │ • Flyweight может использовать State          │
│                 │ • State для изменения поведения flyweight'а  │
│                 │ • Flyweight - оптимизация                    │
│                 │ • State - поведение                          │
├─────────────────┼───────────────────────────────────────────────┤
│ Strategy        │ • Flyweight может использовать Strategy     │
│                 │ • Strategy для выбора алгоритма рендеринга  │
│                 │ • Flyweight - оптимизация                    │
│                 │ • Strategy - выбор алгоритма                │
└─────────────────┴───────────────────────────────────────────────┘
```

## 💡 Практические рекомендации

```
┌─────────────────────────────────────────────────────────────────┐
│                  ПРАКТИЧЕСКИЕ РЕКОМЕНДАЦИИ                     │
└─────────────────────────────────────────────────────────────────┘

1. 🎯 Выбор реализации:
   • Classic Flyweight - для большинства случаев
   • Template Flyweight - для типизированных случаев
   • Functional Flyweight - для функционального стиля
   • Hybrid Flyweight - для сложных систем

2. 🔧 Принципы проектирования:
   • Соблюдайте Single Responsibility Principle
   • Разделяйте внутреннее и внешнее состояние
   • Используйте фабрику для управления flyweight'ами
   • Обеспечьте неизменяемость внутреннего состояния

3. 🧪 Тестирование:
   • Создавайте интерфейсы для flyweight'ов
   • Используйте моки в тестах
   • Тестируйте переиспользование flyweight'ов
   • Проверяйте корректность разделения состояния

4. ⚡ Производительность:
   • Рассмотрите кэширование flyweight'ов
   • Используйте move semantics
   • Избегайте ненужных копирований
   • Профилируйте критические участки

5. 📚 Документация:
   • Документируйте разделение состояния
   • Описывайте критерии создания flyweight'ов
   • Приводите примеры использования
   • Объясняйте причины выбора паттерна

6. 🔄 Рефакторинг:
   • Начинайте с простых flyweight'ов
   • Постепенно добавляйте сложность
   • Используйте интерфейсы для абстракции
   • Планируйте расширяемость
```

## 🎓 Ключевые выводы

```
┌─────────────────────────────────────────────────────────────────┐
│                      КЛЮЧЕВЫЕ ВЫВОДЫ                          │
└─────────────────────────────────────────────────────────────────┘

✅ Flyweight - мощный инструмент:
   • Экономит память за счет разделения состояния
   • Ускоряет создание объектов
   • Обеспечивает переиспользование
   • Следует принципам SOLID

✅ Выбирайте подходящую реализацию:
   • Classic Flyweight - для большинства случаев
   • Template Flyweight - для типизированных случаев
   • Functional Flyweight - для функционального стиля
   • Hybrid Flyweight - для сложных систем

✅ Используйте правильно:
   • Когда нужно много похожих объектов
   • Когда критична экономия памяти
   • Когда можно разделить состояние
   • Когда важна производительность

✅ Избегайте злоупотребления:
   • Не используйте для простых случаев
   • Не добавляйте сложность без необходимости
   • Не забывайте об обработке ошибок
   • Не нарушайте принципы SOLID

✅ Помните о компромиссах:
   • Экономия памяти vs сложность
   • Производительность vs гибкость
   • Переиспользование vs безопасность
   • Тестируемость vs производительность
```

---
*Flyweight - это инструмент для оптимизации памяти. 
Используйте его для работы с множеством похожих объектов!*
