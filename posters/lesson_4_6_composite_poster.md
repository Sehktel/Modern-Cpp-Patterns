# 📊 Плакат: Composite Pattern (Компоновщик)

## 🎯 Назначение паттерна Composite

```
┌─────────────────────────────────────────────────────────────────┐
│                        COMPOSITE PATTERN                       │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│  🌳 СОЗДАЕТ ДРЕВОВИДНУЮ СТРУКТУРУ ИЗ ПРОСТЫХ И СОСТАВНЫХ      │
│    ОБЪЕКТОВ                                                    │
│  🔄 ОБРАЩАЕТСЯ С ГРУППАМИ ОБЪЕКТОВ КАК С ОДНИМ ОБЪЕКТОМ       │
│  🎯 РЕАЛИЗУЕТ ЕДИНЫЙ ИНТЕРФЕЙС ДЛЯ ЛИСТЬЕВ И КОНТЕЙНЕРОВ      │
└─────────────────────────────────────────────────────────────────┘

Применение:
┌─────────────────┬───────────────────────────────────────────────┐
│   UI Components │ • Создание иерархии элементов интерфейса     │
│                 │   (панели, кнопки, меню)                     │
├─────────────────┼───────────────────────────────────────────────┤
│   File System   │ • Представление файлов и папок как единой    │
│                 │   структуры                                  │
├─────────────────┼───────────────────────────────────────────────┤
│   Graphics      │ • Создание сложных графических объектов      │
│                 │   из простых примитивов                      │
├─────────────────┼───────────────────────────────────────────────┤
│   Organization  │ • Структура организаций (отделы, сотрудники)  │
├─────────────────┼───────────────────────────────────────────────┤
│   Menu Systems  │ • Создание иерархических меню                │
└─────────────────┴───────────────────────────────────────────────┘
```

## 🔄 Проблема без Composite

```
┌─────────────────────────────────────────────────────────────────┐
│                    ПРОБЛЕМА: ДУБЛИРОВАНИЕ КОДА                │
└─────────────────────────────────────────────────────────────────┘

❌ ПЛОХО - Разные интерфейсы для листьев и контейнеров:
┌─────────────────────────────────────────────────────────────────┐
│ class File {                                                    │
│ private:                                                        │
│     std::string name_;                                          │
│     size_t size_;                                               │
│                                                                 │
│ public:                                                         │
│     File(const std::string& name, size_t size)                 │
│         : name_(name), size_(size) {}                          │
│                                                                 │
│     size_t getSize() const { return size_; }                   │
│     std::string getName() const { return name_; }               │
│ };                                                              │
│                                                                 │
│ class Directory {                                              │
│ private:                                                        │
│     std::string name_;                                          │
│     std::vector<std::unique_ptr<File>> files_;                 │
│     std::vector<std::unique_ptr<Directory>> directories_;      │
│                                                                 │
│ public:                                                         │
│     Directory(const std::string& name) : name_(name) {}        │
│                                                                 │
│     void addFile(std::unique_ptr<File> file) {                 │
│         files_.push_back(std::move(file));                      │
│     }                                                           │
│                                                                 │
│     void addDirectory(std::unique_ptr<Directory> dir) {         │
│         directories_.push_back(std::move(dir));                 │
│     }                                                           │
│                                                                 │
│     size_t getSize() const {                                    │
│         size_t totalSize = 0;                                   │
│         for (const auto& file : files_) {                       │
│             totalSize += file->getSize();                       │
│         }                                                       │
│         for (const auto& dir : directories_) {                  │
│             totalSize += dir->getSize();                        │
│         }                                                       │
│         return totalSize;                                       │
│     }                                                           │
│                                                                 │
│     std::string getName() const { return name_; }               │
│ };                                                              │
│                                                                 │
│ // Проблемы:                                                    │
│ // • Разные интерфейсы для File и Directory                    │
│ // • Дублирование кода для общих операций                      │
│ // • Сложность работы с иерархией                              │
│ // • Нарушение принципа единообразия                           │
└─────────────────────────────────────────────────────────────────┘
```

## ✅ Решение с Composite

```
┌─────────────────────────────────────────────────────────────────┐
│                    РЕШЕНИЕ: ЕДИНЫЙ ИНТЕРФЕЙС                   │
└─────────────────────────────────────────────────────────────────┘

✅ ХОРОШО - Единый интерфейс для всех компонентов:
┌─────────────────────────────────────────────────────────────────┐
│ class FileSystemComponent {                                     │
│ public:                                                         │
│     virtual ~FileSystemComponent() = default;                  │
│     virtual size_t getSize() const = 0;                         │
│     virtual std::string getName() const = 0;                   │
│     virtual void add(std::unique_ptr<FileSystemComponent> component) { │
│         throw std::runtime_error("Cannot add to leaf component"); │
│     }                                                           │
│     virtual void remove(FileSystemComponent* component) {       │
│         throw std::runtime_error("Cannot remove from leaf component"); │
│     }                                                           │
│     virtual void print(int depth = 0) const = 0;               │
│ };                                                              │
│                                                                 │
│ class File : public FileSystemComponent {                      │
│ private:                                                        │
│     std::string name_;                                          │
│     size_t size_;                                               │
│                                                                 │
│ public:                                                         │
│     File(const std::string& name, size_t size)                  │
│         : name_(name), size_(size) {}                          │
│                                                                 │
│     size_t getSize() const override { return size_; }           │
│     std::string getName() const override { return name_; }       │
│                                                                 │
│     void print(int depth = 0) const override {                 │
│         std::string indent(depth * 2, ' ');                    │
│         std::cout << indent << "📄 " << name_                  │
│                   << " (" << size_ << " bytes)" << std::endl;  │
│     }                                                           │
│ };                                                              │
│                                                                 │
│ class Directory : public FileSystemComponent {                 │
│ private:                                                        │
│     std::string name_;                                          │
│     std::vector<std::unique_ptr<FileSystemComponent>> children_; │
│                                                                 │
│ public:                                                         │
│     Directory(const std::string& name) : name_(name) {}        │
│                                                                 │
│     size_t getSize() const override {                           │
│         size_t totalSize = 0;                                   │
│         for (const auto& child : children_) {                   │
│             totalSize += child->getSize();                      │
│         }                                                       │
│         return totalSize;                                       │
│     }                                                           │
│                                                                 │
│     std::string getName() const override { return name_; }      │
│                                                                 │
│     void add(std::unique_ptr<FileSystemComponent> component) override { │
│         children_.push_back(std::move(component));             │
│     }                                                           │
│                                                                 │
│     void remove(FileSystemComponent* component) override {      │
│         children_.erase(                                        │
│             std::remove_if(children_.begin(), children_.end(),  │
│                 [component](const std::unique_ptr<FileSystemComponent>& child) { │
│                     return child.get() == component;           │
│                 }), children_.end());                           │
│     }                                                           │
│                                                                 │
│     void print(int depth = 0) const override {                 │
│         std::string indent(depth * 2, ' ');                    │
│         std::cout << indent << "📁 " << name_ << std::endl;     │
│         for (const auto& child : children_) {                   │
│             child->print(depth + 1);                            │
│         }                                                       │
│     }                                                           │
│ };                                                              │
└─────────────────────────────────────────────────────────────────┘

Использование:
┌─────────────────────────────────────────────────────────────────┐
│ // Создание файловой системы                                   │
│ auto root = std::make_unique<Directory>("root");                │
│ auto documents = std::make_unique<Directory>("Documents");       │
│ auto images = std::make_unique<Directory>("Images");             │
│                                                                 │
│ // Добавление файлов                                           │
│ documents->add(std::make_unique<File>("report.txt", 1024));     │
│ documents->add(std::make_unique<File>("notes.md", 512));        │
│ images->add(std::make_unique<File>("photo.jpg", 2048));          │
│                                                                 │
│ // Добавление директорий                                       │
│ root->add(std::move(documents));                                │
│ root->add(std::move(images));                                   │
│                                                                 │
│ // Единообразная работа с компонентами                         │
│ root->print();                                                  │
│ std::cout << "Total size: " << root->getSize() << " bytes" << std::endl; │
└─────────────────────────────────────────────────────────────────┘

Преимущества:
✅ Единый интерфейс для всех компонентов
✅ Рекурсивная обработка структуры
✅ Легкое добавление новых типов компонентов
✅ Соблюдение принципов SOLID
```

## 🏗️ Структура Composite

```
┌─────────────────────────────────────────────────────────────────┐
│                    СТРУКТУРА ПАТТЕРНА                          │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│                    Component (Компонент)                       │
│  ┌─────────────────────────────────────────────────────────────┐│
│  │ class FileSystemComponent {                                 ││
│  │ public:                                                     ││
│  │     virtual ~FileSystemComponent() = default;              ││
│  │     virtual size_t getSize() const = 0;                   ││
│  │     virtual std::string getName() const = 0;               ││
│  │     virtual void add(std::unique_ptr<FileSystemComponent>); ││
│  │     virtual void remove(FileSystemComponent*);             ││
│  │     virtual void print(int depth = 0) const = 0;          ││
│  │ };                                                          ││
│  └─────────────────────────────────────────────────────────────┘│
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│                    Leaf (Лист)                                  │
│  ┌─────────────────────────────────────────────────────────────┐│
│  │ class File : public FileSystemComponent {                  ││
│  │ private:                                                    ││
│  │     std::string name_;                                      ││
│  │     size_t size_;                                           ││
│  │                                                             ││
│  │ public:                                                     ││
│  │     File(const std::string& name, size_t size);            ││
│  │     size_t getSize() const override;                       ││
│  │     std::string getName() const override;                   ││
│  │     void print(int depth = 0) const override;              ││
│  │ };                                                          ││
│  └─────────────────────────────────────────────────────────────┘│
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│                    Composite (Компоновщик)                     │
│  ┌─────────────────────────────────────────────────────────────┐│
│  │ class Directory : public FileSystemComponent {             ││
│  │ private:                                                    ││
│  │     std::string name_;                                      ││
│  │     std::vector<std::unique_ptr<FileSystemComponent>> children_; ││
│  │                                                             ││
│  │ public:                                                     ││
│  │     Directory(const std::string& name);                    ││
│  │     size_t getSize() const override;                       ││
│  │     std::string getName() const override;                   ││
│  │     void add(std::unique_ptr<FileSystemComponent>) override; ││
│  │     void remove(FileSystemComponent*) override;            ││
│  │     void print(int depth = 0) const override;              ││
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
│   1. Клиент     │ auto root = std::make_unique<Directory>("root"); │
│      создает     │ auto file = std::make_unique<File>("test.txt", 1024); │
│      компоненты  │                                               │
├─────────────────┼───────────────────────────────────────────────┤
│   2. Клиент     │ root->add(std::move(file));                   │
│      добавляет   │                                               │
│      компоненты  │                                               │
├─────────────────┼───────────────────────────────────────────────┤
│   3. Клиент     │ size_t totalSize = root->getSize();           │
│      вызывает    │                                               │
│      операции    │                                               │
├─────────────────┼───────────────────────────────────────────────┤
│   4. Composite  │ Directory::getSize() вызывает                 │
│      делегирует  │ child->getSize() для каждого дочернего       │
│      операцию    │ элемента                                      │
├─────────────────┼───────────────────────────────────────────────┤
│   5. Leaf       │ File::getSize() возвращает размер файла      │
│      возвращает  │                                               │
│      результат   │                                               │
└─────────────────┴───────────────────────────────────────────────┘

Диаграмма взаимодействий:
┌─────────────────────────────────────────────────────────────────┐
│  Client ──┐                                                    │
│           │                                                    │
│           ▼                                                    │
│  Directory ──┐                                                 │
│               │                                                 │
│               ▼                                                 │
│  File ──┐                                                       │
│          │                                                       │
│          ▼                                                       │
│  getSize() = 1024                                               │
└─────────────────────────────────────────────────────────────────┘
```

## 🎨 Варианты реализации

### 1. Classic Composite (Классический)
```
┌─────────────────────────────────────────────────────────────────┐
│                    CLASSIC COMPOSITE                           │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│ class UIComponent {                                             │
│ public:                                                         │
│     virtual ~UIComponent() = default;                          │
│     virtual void render() const = 0;                           │
│     virtual void add(std::unique_ptr<UIComponent> component) {  │
│         throw std::runtime_error("Cannot add to leaf");         │
│     }                                                           │
│     virtual void remove(UIComponent* component) {              │
│         throw std::runtime_error("Cannot remove from leaf");     │
│     }                                                           │
│     virtual std::vector<UIComponent*> getChildren() const {     │
│         return {};                                              │
│     }                                                           │
│ };                                                              │
│                                                                 │
│ class Button : public UIComponent {                             │
│ private:                                                        │
│     std::string text_;                                          │
│                                                                 │
│ public:                                                         │
│     Button(const std::string& text) : text_(text) {}           │
│                                                                 │
│     void render() const override {                              │
│         std::cout << "[Button: " << text_ << "]" << std::endl;  │
│     }                                                           │
│ };                                                              │
│                                                                 │
│ class Panel : public UIComponent {                              │
│ private:                                                        │
│     std::string name_;                                          │
│     std::vector<std::unique_ptr<UIComponent>> children_;        │
│                                                                 │
│ public:                                                         │
│     Panel(const std::string& name) : name_(name) {}             │
│                                                                 │
│     void render() const override {                              │
│         std::cout << "Panel: " << name_ << std::endl;          │
│         for (const auto& child : children_) {                   │
│             child->render();                                    │
│         }                                                       │
│     }                                                           │
│                                                                 │
│     void add(std::unique_ptr<UIComponent> component) override { │
│         children_.push_back(std::move(component));               │
│     }                                                           │
│                                                                 │
│     void remove(UIComponent* component) override {              │
│         children_.erase(                                        │
│             std::remove_if(children_.begin(), children_.end(),  │
│                 [component](const std::unique_ptr<UIComponent>& child) { │
│                     return child.get() == component;            │
│                 }), children_.end());                           │
│     }                                                           │
│                                                                 │
│     std::vector<UIComponent*> getChildren() const override {    │
│         std::vector<UIComponent*> result;                       │
│         for (const auto& child : children_) {                   │
│             result.push_back(child.get());                      │
│         }                                                       │
│         return result;                                          │
│     }                                                           │
│ };                                                              │
└─────────────────────────────────────────────────────────────────┘
```

### 2. Template Composite
```
┌─────────────────────────────────────────────────────────────────┐
│                    TEMPLATE COMPOSITE                          │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│ template<typename T>                                             │
│ class Composite {                                                │
│ private:                                                        │
│     std::string name_;                                          │
│     std::vector<std::unique_ptr<T>> children_;                  │
│                                                                 │
│ public:                                                         │
│     Composite(const std::string& name) : name_(name) {}         │
│                                                                 │
│     void add(std::unique_ptr<T> component) {                   │
│         children_.push_back(std::move(component));               │
│     }                                                           │
│                                                                 │
│     void remove(T* component) {                                 │
│         children_.erase(                                        │
│             std::remove_if(children_.begin(), children_.end(),  │
│                 [component](const std::unique_ptr<T>& child) {  │
│                     return child.get() == component;            │
│                 }), children_.end());                           │
│     }                                                           │
│                                                                 │
│     template<typename Func>                                     │
│     void forEach(Func func) const {                             │
│         for (const auto& child : children_) {                   │
│             func(*child);                                       │
│         }                                                       │
│     }                                                           │
│                                                                 │
│     const std::string& getName() const { return name_; }        │
│     size_t getChildCount() const { return children_.size(); }    │
│ };                                                              │
│                                                                 │
│ // Использование                                               │
│ class Drawable {                                                │
│ public:                                                         │
│     virtual void draw() const = 0;                             │
│     virtual ~Drawable() = default;                              │
│ };                                                              │
│                                                                 │
│ class Circle : public Drawable {                               │
│ public:                                                         │
│     void draw() const override {                                │
│         std::cout << "Drawing circle" << std::endl;             │
│     }                                                           │
│ };                                                              │
│                                                                 │
│ Composite<Drawable> group("Shapes");                            │
│ group.add(std::make_unique<Circle>());                          │
│ group.forEach([](const Drawable& d) { d.draw(); });             │
└─────────────────────────────────────────────────────────────────┘
```

### 3. Functional Composite
```
┌─────────────────────────────────────────────────────────────────┐
│                    FUNCTIONAL COMPOSITE                        │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│ class FunctionalComposite {                                     │
│ private:                                                        │
│     std::string name_;                                          │
│     std::vector<std::function<void()>> operations_;            │
│                                                                 │
│ public:                                                         │
│     FunctionalComposite(const std::string& name) : name_(name) {} │
│                                                                 │
│     void addOperation(std::function<void()> operation) {        │
│         operations_.push_back(operation);                       │
│     }                                                           │
│                                                                 │
│     void execute() const {                                      │
│         std::cout << "Executing: " << name_ << std::endl;      │
│         for (const auto& operation : operations_) {             │
│             operation();                                        │
│         }                                                       │
│     }                                                           │
│                                                                 │
│     void executeWith(std::function<void(const std::string&)> func) const { │
│         func(name_);                                            │
│         for (const auto& operation : operations_) {             │
│             operation();                                        │
│         }                                                       │
│     }                                                           │
│ };                                                              │
│                                                                 │
│ // Использование                                               │
│ FunctionalComposite workflow("Data Processing");                │
│ workflow.addOperation([]() {                                   │
│     std::cout << "Step 1: Load data" << std::endl;             │
│ });                                                             │
│ workflow.addOperation([]() {                                   │
│     std::cout << "Step 2: Process data" << std::endl;           │
│ });                                                             │
│ workflow.addOperation([]() {                                   │
│     std::cout << "Step 3: Save results" << std::endl;          │
│ });                                                             │
│                                                                 │
│ workflow.execute();                                             │
└─────────────────────────────────────────────────────────────────┘
```

## 🔄 Сравнение подходов

```
┌─────────────────────────────────────────────────────────────────┐
│                    СРАВНЕНИЕ ПОДХОДОВ                          │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────┬─────────────────┬─────────────────┬─────────────────┐
│   Classic       │   Template      │   Functional    │   Hybrid        │
│   Composite     │   Composite     │   Composite     │   Composite     │
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

## 🎯 Когда использовать Composite

```
┌─────────────────────────────────────────────────────────────────┐
│                КОГДА ИСПОЛЬЗОВАТЬ COMPOSITE                    │
└─────────────────────────────────────────────────────────────────┘

✅ ХОРОШИЕ СЛУЧАИ:
┌─────────────────┬───────────────────────────────────────────────┐
│ Иерархические   │ • Нужно представить древовидную структуру    │
│ структуры       │ • Объекты имеют родительско-дочерние связи   │
├─────────────────┼───────────────────────────────────────────────┤
│ Единообразие    │ • Нужен единый интерфейс для всех объектов   │
│                 │ • Клиент должен работать с группой как с одним│
├─────────────────┼───────────────────────────────────────────────┤
│ Рекурсивные     │ • Нужны рекурсивные операции                 │
│ операции        │ • Операции должны применяться ко всей структуре│
├─────────────────┼───────────────────────────────────────────────┤
│ Динамическое    │ • Структура может изменяться во время выполнения│
│ изменение       │ • Нужно добавлять/удалять компоненты         │
├─────────────────┼───────────────────────────────────────────────┤
│ Прозрачность    │ • Клиент не должен различать листья и контейнеры│
│                 │ • Единообразная работа с компонентами        │
└─────────────────┴───────────────────────────────────────────────┘

❌ ПЛОХИЕ СЛУЧАИ:
┌─────────────────┬───────────────────────────────────────────────┐
│ Простые случаи  │ • Когда можно использовать простые структуры │
├─────────────────┼───────────────────────────────────────────────┤
│ Производительность│ • Критична производительность               │
├─────────────────┼───────────────────────────────────────────────┤
│ Сложность       │ • Добавляется ненужная сложность             │
├─────────────────┼───────────────────────────────────────────────┤
│ Альтернативы    │ • Есть лучшие альтернативы (Visitor)        │
└─────────────────┴───────────────────────────────────────────────┘
```

## 🧪 Тестирование Composite

```
┌─────────────────────────────────────────────────────────────────┐
│                    ТЕСТИРОВАНИЕ COMPOSITE                     │
└─────────────────────────────────────────────────────────────────┘

Преимущества для тестирования:
┌─────────────────┬───────────────────────────────────────────────┐
│ Моки            │ ✅ Легко создавать моки для компонентов      │
│                 │ class MockComponent : public FileSystemComponent { │
│                 │     MOCK_METHOD(size_t, getSize, (), (const)); │
│                 │     MOCK_METHOD(std::string, getName, (), (const)); │
│                 │     MOCK_METHOD(void, print, (int), (const)); │
│                 │ };                                            │
├─────────────────┼───────────────────────────────────────────────┤
│ Изоляция        │ ✅ Легко изолировать тесты                   │
│                 │ auto mockComponent = std::make_unique<MockComponent>(); │
│                 │ Directory dir("test");                        │
│                 │ dir.add(std::move(mockComponent));            │
├─────────────────┼───────────────────────────────────────────────┤
│ Управление      │ ✅ Полный контроль над компонентами          │
│ зависимостями   │ EXPECT_CALL(*mockComponent, getSize())        │
│                 │     .WillOnce(Return(1024));                 │
└─────────────────┴───────────────────────────────────────────────┘

Пример теста:
┌─────────────────────────────────────────────────────────────────┐
│ TEST(DirectoryTest, CalculateTotalSize) {                       │
│     auto file1 = std::make_unique<File>("file1.txt", 1024);      │
│     auto file2 = std::make_unique<File>("file2.txt", 2048);      │
│     auto subdir = std::make_unique<Directory>("subdir");         │
│     auto file3 = std::make_unique<File>("file3.txt", 512);       │
│                                                                 │
│     subdir->add(std::move(file3));                              │
│                                                                 │
│     Directory root("root");                                     │
│     root.add(std::move(file1));                                 │
│     root.add(std::move(file2));                                 │
│     root.add(std::move(subdir));                                │
│                                                                 │
│     EXPECT_EQ(root.getSize(), 3584); // 1024 + 2048 + 512      │
│ }                                                               │
└─────────────────────────────────────────────────────────────────┘
```

## 🔗 Связь с другими паттернами

```
┌─────────────────────────────────────────────────────────────────┐
│              СВЯЗЬ С ДРУГИМИ ПАТТЕРНАМИ                        │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────┬───────────────────────────────────────────────┐
│ Iterator        │ • Composite может использовать Iterator     │
│                 │ • Iterator для обхода структуры Composite    │
│                 │ • Composite - структура                      │
│                 │ • Iterator - обход                          │
├─────────────────┼───────────────────────────────────────────────┤
│ Visitor         │ • Composite может использовать Visitor      │
│                 │ • Visitor для операций над Composite        │
│                 │ • Composite - структура                      │
│                 │ • Visitor - операции                         │
├─────────────────┼───────────────────────────────────────────────┤
│ Decorator       │ • Composite может использовать Decorator     │
│                 │ • Decorator для добавления функциональности  │
│                 │ • Composite - структура                      │
│                 │ • Decorator - расширение                    │
├─────────────────┼───────────────────────────────────────────────┤
│ Builder         │ • Composite может использовать Builder       │
│                 │ • Builder для создания сложных структур     │
│                 │ • Composite - структура                      │
│                 │ • Builder - создание                        │
├─────────────────┼───────────────────────────────────────────────┤
│ Command         │ • Composite может использовать Command       │
│                 │ • Command для операций над Composite         │
│                 │ • Composite - структура                      │
│                 │ • Command - операции                         │
└─────────────────┴───────────────────────────────────────────────┘
```

## 💡 Практические рекомендации

```
┌─────────────────────────────────────────────────────────────────┐
│                  ПРАКТИЧЕСКИЕ РЕКОМЕНДАЦИИ                     │
└─────────────────────────────────────────────────────────────────┘

1. 🎯 Выбор реализации:
   • Classic Composite - для большинства случаев
   • Template Composite - для типизированных случаев
   • Functional Composite - для функционального стиля
   • Hybrid Composite - для сложных систем

2. 🔧 Принципы проектирования:
   • Соблюдайте Single Responsibility Principle
   • Используйте единый интерфейс для всех компонентов
   • Инкапсулируйте логику работы с детьми
   • Обеспечьте прозрачность для клиента

3. 🧪 Тестирование:
   • Создавайте интерфейсы для компонентов
   • Используйте моки в тестах
   • Тестируйте рекурсивные операции
   • Проверяйте корректность работы с детьми

4. ⚡ Производительность:
   • Рассмотрите кэширование результатов
   • Используйте move semantics
   • Избегайте ненужных копирований
   • Профилируйте критические участки

5. 📚 Документация:
   • Документируйте иерархическую структуру
   • Описывайте операции над компонентами
   • Приводите примеры использования
   • Объясняйте причины выбора паттерна

6. 🔄 Рефакторинг:
   • Начинайте с простых composite'ов
   • Постепенно добавляйте сложность
   • Используйте интерфейсы для абстракции
   • Планируйте расширяемость
```

## 🎓 Ключевые выводы

```
┌─────────────────────────────────────────────────────────────────┐
│                      КЛЮЧЕВЫЕ ВЫВОДЫ                          │
└─────────────────────────────────────────────────────────────────┘

✅ Composite - мощный инструмент:
   • Создает древовидные структуры
   • Обеспечивает единый интерфейс
   • Поддерживает рекурсивные операции
   • Следует принципам SOLID

✅ Выбирайте подходящую реализацию:
   • Classic Composite - для большинства случаев
   • Template Composite - для типизированных случаев
   • Functional Composite - для функционального стиля
   • Hybrid Composite - для сложных систем

✅ Используйте правильно:
   • Когда нужна древовидная структура
   • Когда важен единый интерфейс
   • Когда нужны рекурсивные операции
   • Когда важна прозрачность

✅ Избегайте злоупотребления:
   • Не используйте для простых случаев
   • Не добавляйте сложность без необходимости
   • Не забывайте об обработке ошибок
   • Не нарушайте принципы SOLID

✅ Помните о компромиссах:
   • Единообразие vs производительность
   • Рекурсия vs итерация
   • Прозрачность vs функциональность
   • Тестируемость vs производительность
```

---
*Composite - это инструмент для создания древовидных структур. 
Используйте его для единообразной работы с группами объектов!*
