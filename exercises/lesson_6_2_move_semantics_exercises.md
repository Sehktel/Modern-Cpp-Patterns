# 🏋️ Упражнения: Move Semantics

## 📋 Задание 1: Modern Move Semantics с Templates

### Описание
Создайте современную реализацию Move Semantics с использованием C++17:

### Требования
1. ✅ Используйте `std::move` для перемещения объектов
2. ✅ Примените `std::forward` для perfect forwarding
3. ✅ Используйте `auto` и `decltype` для вывода типов
4. ✅ Добавьте поддержку variadic templates
5. ✅ Реализуйте type-safe перемещение

### Современная архитектура
```cpp
#include <memory>
#include <variant>
#include <optional>
#include <unordered_map>
#include <functional>
#include <string>
#include <vector>

// Базовый класс с move semantics
class MoveableObject {
protected:
    std::string name_;
    bool moved_;
    
public:
    explicit MoveableObject(const std::string& name) : name_(name), moved_(false) {
        std::cout << "MoveableObject " << name_ << " created" << std::endl;
    }
    
    virtual ~MoveableObject() {
        if (!moved_) {
            std::cout << "MoveableObject " << name_ << " destroyed" << std::endl;
        }
    }
    
    // Move constructor
    MoveableObject(MoveableObject&& other) noexcept 
        : name_(std::move(other.name_)), moved_(other.moved_) {
        other.moved_ = true;
        std::cout << "MoveableObject " << name_ << " moved from " << other.name_ << std::endl;
    }
    
    // Move assignment operator
    MoveableObject& operator=(MoveableObject&& other) noexcept {
        if (this != &other) {
            name_ = std::move(other.name_);
            moved_ = other.moved_;
            other.moved_ = true;
            std::cout << "MoveableObject " << name_ << " move assigned from " << other.name_ << std::endl;
        }
        return *this;
    }
    
    // Запрещаем копирование
    MoveableObject(const MoveableObject&) = delete;
    MoveableObject& operator=(const MoveableObject&) = delete;
    
    std::string getName() const {
        return name_;
    }
    
    bool isMoved() const {
        return moved_;
    }
};

// Конкретные перемещаемые объекты
class DataContainer : public MoveableObject {
private:
    std::vector<int> data_;
    
public:
    DataContainer(const std::string& name, const std::vector<int>& data) 
        : MoveableObject(name), data_(data) {
        std::cout << "DataContainer " << name_ << " created with " << data_.size() << " elements" << std::endl;
    }
    
    // Move constructor
    DataContainer(DataContainer&& other) noexcept 
        : MoveableObject(std::move(other)), data_(std::move(other.data_)) {
        std::cout << "DataContainer " << name_ << " moved with " << data_.size() << " elements" << std::endl;
    }
    
    // Move assignment operator
    DataContainer& operator=(DataContainer&& other) noexcept {
        if (this != &other) {
            MoveableObject::operator=(std::move(other));
            data_ = std::move(other.data_);
            std::cout << "DataContainer " << name_ << " move assigned with " << data_.size() << " elements" << std::endl;
        }
        return *this;
    }
    
    const std::vector<int>& getData() const {
        return data_;
    }
    
    void addData(int value) {
        data_.push_back(value);
    }
    
    size_t getSize() const {
        return data_.size();
    }
};

class StringBuffer : public MoveableObject {
private:
    std::string buffer_;
    
public:
    StringBuffer(const std::string& name, const std::string& buffer) 
        : MoveableObject(name), buffer_(buffer) {
        std::cout << "StringBuffer " << name_ << " created with " << buffer_.size() << " characters" << std::endl;
    }
    
    // Move constructor
    StringBuffer(StringBuffer&& other) noexcept 
        : MoveableObject(std::move(other)), buffer_(std::move(other.buffer_)) {
        std::cout << "StringBuffer " << name_ << " moved with " << buffer_.size() << " characters" << std::endl;
    }
    
    // Move assignment operator
    StringBuffer& operator=(StringBuffer&& other) noexcept {
        if (this != &other) {
            MoveableObject::operator=(std::move(other));
            buffer_ = std::move(other.buffer_);
            std::cout << "StringBuffer " << name_ << " move assigned with " << buffer_.size() << " characters" << std::endl;
        }
        return *this;
    }
    
    const std::string& getBuffer() const {
        return buffer_;
    }
    
    void append(const std::string& text) {
        buffer_ += text;
    }
    
    size_t getSize() const {
        return buffer_.size();
    }
};

// Современный менеджер перемещения
class ModernMoveManager {
private:
    std::unordered_map<std::string, std::unique_ptr<MoveableObject>> objects_;
    
public:
    ModernMoveManager() = default;
    
    // Добавление объекта с perfect forwarding
    template<typename T, typename... Args>
    void addObject(const std::string& name, Args&&... args) {
        static_assert(std::is_base_of_v<MoveableObject, T>, "T must be derived from MoveableObject");
        
        auto object = std::make_unique<T>(std::forward<Args>(args)...);
        objects_[name] = std::move(object);
    }
    
    // Перемещение объекта
    std::unique_ptr<MoveableObject> moveObject(const std::string& name) {
        auto it = objects_.find(name);
        if (it != objects_.end()) {
            auto object = std::move(it->second);
            objects_.erase(it);
            return object;
        }
        return nullptr;
    }
    
    // Получение объекта
    MoveableObject* getObject(const std::string& name) const {
        auto it = objects_.find(name);
        return it != objects_.end() ? it->second.get() : nullptr;
    }
    
    // Перемещение всех объектов
    std::vector<std::unique_ptr<MoveableObject>> moveAllObjects() {
        std::vector<std::unique_ptr<MoveableObject>> movedObjects;
        
        for (auto& pair : objects_) {
            movedObjects.push_back(std::move(pair.second));
        }
        
        objects_.clear();
        return movedObjects;
    }
    
    // Получение информации об объектах
    std::string getObjectsInfo() const {
        std::string info = "Objects:\n";
        for (const auto& pair : objects_) {
            info += "  " + pair.first + ": " + pair.second->getName() + "\n";
        }
        return info;
    }
    
    // Получение количества объектов
    size_t getObjectCount() const {
        return objects_.size();
    }
};

// Фабрика перемещаемых объектов
class MoveableObjectFactory {
private:
    using ObjectVariant = std::variant<
        std::unique_ptr<DataContainer>,
        std::unique_ptr<StringBuffer>
    >;
    
    using CreatorFunction = std::function<ObjectVariant(const std::string&, const std::string&)>;
    std::unordered_map<std::string, CreatorFunction> creators_;
    
public:
    void registerObject(const std::string& type) {
        if (type == "data") {
            creators_[type] = [](const std::string& name, const std::string& param) -> ObjectVariant {
                std::vector<int> data = {1, 2, 3, 4, 5};
                return std::make_unique<DataContainer>(name, data);
            };
        } else if (type == "string") {
            creators_[type] = [](const std::string& name, const std::string& param) -> ObjectVariant {
                return std::make_unique<StringBuffer>(name, param);
            };
        }
    }
    
    std::optional<ObjectVariant> createObject(const std::string& type, const std::string& name, const std::string& param) {
        auto it = creators_.find(type);
        if (it != creators_.end()) {
            return it->second(name, param);
        }
        return std::nullopt;
    }
    
    std::vector<std::string> getAvailableTypes() const {
        std::vector<std::string> types;
        for (const auto& pair : creators_) {
            types.push_back(pair.first);
        }
        return types;
    }
};
```

### Тестовый случай
```cpp
int main() {
    MoveableObjectFactory factory;
    
    // Регистрируем объекты
    factory.registerObject("data");
    factory.registerObject("string");
    
    // Создаем менеджер перемещения
    ModernMoveManager manager;
    
    // Добавляем объекты
    manager.addObject<DataContainer>("data1", std::vector<int>{1, 2, 3, 4, 5});
    manager.addObject<StringBuffer>("string1", "Hello, World!");
    
    std::cout << "Initial objects:" << std::endl;
    std::cout << manager.getObjectsInfo() << std::endl;
    
    // Перемещаем объект
    std::cout << "\n=== Moving Object ===" << std::endl;
    auto movedObject = manager.moveObject("data1");
    if (movedObject) {
        std::cout << "Object moved: " << movedObject->getName() << std::endl;
    }
    
    std::cout << "\nRemaining objects:" << std::endl;
    std::cout << manager.getObjectsInfo() << std::endl;
    
    // Перемещаем все объекты
    std::cout << "\n=== Moving All Objects ===" << std::endl;
    auto movedObjects = manager.moveAllObjects();
    std::cout << "Moved " << movedObjects.size() << " objects" << std::endl;
    
    std::cout << "\nRemaining objects:" << std::endl;
    std::cout << manager.getObjectsInfo() << std::endl;
    
    return 0;
}
```

---

## 📋 Задание 2: Multi-Level Move Semantics System

### Описание
Создайте систему многоуровневых Move Semantics для работы с разными типами объектов:

### Требования
1. **Multiple Levels**: Поддержка множественных уровней перемещения
2. **Type Safety**: Проверка типов на этапе компиляции
3. **Error Handling**: Обработка ошибок перемещения
4. **Validation**: Валидация перемещения

### Реализация
```cpp
#include <any>
#include <type_traits>
#include <stdexcept>

// Базовый интерфейс для перемещаемого объекта
class IMoveable {
public:
    virtual ~IMoveable() = default;
    virtual std::string getName() const = 0;
    virtual std::string getDescription() const = 0;
    virtual bool isValid() const = 0;
    virtual void moveTo(const std::string& destination) = 0;
    virtual void onMove() = 0;
};

// Конкретные перемещаемые объекты
class FileObject : public IMoveable {
private:
    std::string name_;
    std::string filePath_;
    bool moved_;
    
public:
    FileObject(const std::string& name, const std::string& filePath) 
        : name_(name), filePath_(filePath), moved_(false) {
        std::cout << "FileObject " << name_ << " created at " << filePath_ << std::endl;
    }
    
    ~FileObject() {
        if (!moved_) {
            std::cout << "FileObject " << name_ << " destroyed" << std::endl;
        }
    }
    
    std::string getName() const override {
        return name_;
    }
    
    std::string getDescription() const override {
        return "File object at " + filePath_;
    }
    
    bool isValid() const override {
        return !filePath_.empty() && !moved_;
    }
    
    void moveTo(const std::string& destination) override {
        if (!isValid()) {
            throw std::runtime_error("FileObject is not valid for moving");
        }
        
        std::cout << "FileObject " << name_ << " moved from " << filePath_ << " to " << destination << std::endl;
        filePath_ = destination;
        onMove();
    }
    
    void onMove() override {
        moved_ = true;
        std::cout << "FileObject " << name_ << " move completed" << std::endl;
    }
    
    std::string getFilePath() const {
        return filePath_;
    }
    
    bool isMoved() const {
        return moved_;
    }
};

class DatabaseObject : public IMoveable {
private:
    std::string name_;
    std::string tableName_;
    bool moved_;
    
public:
    DatabaseObject(const std::string& name, const std::string& tableName) 
        : name_(name), tableName_(tableName), moved_(false) {
        std::cout << "DatabaseObject " << name_ << " created in table " << tableName_ << std::endl;
    }
    
    ~DatabaseObject() {
        if (!moved_) {
            std::cout << "DatabaseObject " << name_ << " destroyed" << std::endl;
        }
    }
    
    std::string getName() const override {
        return name_;
    }
    
    std::string getDescription() const override {
        return "Database object in table " + tableName_;
    }
    
    bool isValid() const override {
        return !tableName_.empty() && !moved_;
    }
    
    void moveTo(const std::string& destination) override {
        if (!isValid()) {
            throw std::runtime_error("DatabaseObject is not valid for moving");
        }
        
        std::cout << "DatabaseObject " << name_ << " moved from " << tableName_ << " to " << destination << std::endl;
        tableName_ = destination;
        onMove();
    }
    
    void onMove() override {
        moved_ = true;
        std::cout << "DatabaseObject " << name_ << " move completed" << std::endl;
    }
    
    std::string getTableName() const {
        return tableName_;
    }
    
    bool isMoved() const {
        return moved_;
    }
};

class NetworkObject : public IMoveable {
private:
    std::string name_;
    std::string url_;
    bool moved_;
    
public:
    NetworkObject(const std::string& name, const std::string& url) 
        : name_(name), url_(url), moved_(false) {
        std::cout << "NetworkObject " << name_ << " created at " << url_ << std::endl;
    }
    
    ~NetworkObject() {
        if (!moved_) {
            std::cout << "NetworkObject " << name_ << " destroyed" << std::endl;
        }
    }
    
    std::string getName() const override {
        return name_;
    }
    
    std::string getDescription() const override {
        return "Network object at " + url_;
    }
    
    bool isValid() const override {
        return !url_.empty() && !moved_;
    }
    
    void moveTo(const std::string& destination) override {
        if (!isValid()) {
            throw std::runtime_error("NetworkObject is not valid for moving");
        }
        
        std::cout << "NetworkObject " << name_ << " moved from " << url_ << " to " << destination << std::endl;
        url_ = destination;
        onMove();
    }
    
    void onMove() override {
        moved_ = true;
        std::cout << "NetworkObject " << name_ << " move completed" << std::endl;
    }
    
    std::string getUrl() const {
        return url_;
    }
    
    bool isMoved() const {
        return moved_;
    }
};

// Многоуровневый менеджер перемещения
class MultiLevelMoveManager {
private:
    std::vector<std::unique_ptr<IMoveable>> objects_;
    std::vector<std::string> moveHistory_;
    
public:
    MultiLevelMoveManager() = default;
    
    // Добавление объекта
    void addObject(std::unique_ptr<IMoveable> object) {
        if (!object) {
            std::cerr << "Cannot add null object" << std::endl;
            return;
        }
        
        if (!object->isValid()) {
            std::cerr << "Object is not valid" << std::endl;
            return;
        }
        
        objects_.push_back(std::move(object));
    }
    
    // Перемещение объекта по индексу
    bool moveObject(size_t index, const std::string& destination) {
        if (index >= objects_.size()) {
            std::cerr << "Object index out of range" << std::endl;
            return false;
        }
        
        try {
            objects_[index]->moveTo(destination);
            moveHistory_.push_back(objects_[index]->getName() + " -> " + destination);
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error moving object: " << e.what() << std::endl;
            return false;
        }
    }
    
    // Перемещение всех объектов
    bool moveAllObjects(const std::string& destination) {
        bool allSuccessful = true;
        
        for (size_t i = 0; i < objects_.size(); ++i) {
            if (!moveObject(i, destination)) {
                allSuccessful = false;
            }
        }
        
        return allSuccessful;
    }
    
    // Получение информации об объектах
    std::string getObjectsInfo() const {
        std::string info = "Objects:\n";
        for (size_t i = 0; i < objects_.size(); ++i) {
            info += "  " + std::to_string(i) + ": " + objects_[i]->getName() + 
                   " - " + objects_[i]->getDescription() + "\n";
        }
        return info;
    }
    
    // Получение истории перемещений
    std::vector<std::string> getMoveHistory() const {
        return moveHistory_;
    }
    
    // Получение количества объектов
    size_t getObjectCount() const {
        return objects_.size();
    }
    
    // Очистка объектов
    void clearObjects() {
        objects_.clear();
        moveHistory_.clear();
    }
};

// Фабрика перемещаемых объектов
class MoveableObjectFactory {
private:
    using ObjectVariant = std::variant<
        std::unique_ptr<FileObject>,
        std::unique_ptr<DatabaseObject>,
        std::unique_ptr<NetworkObject>
    >;
    
    using CreatorFunction = std::function<ObjectVariant(const std::string&, const std::string&)>;
    std::unordered_map<std::string, CreatorFunction> creators_;
    
public:
    void registerObject(const std::string& type) {
        if (type == "file") {
            creators_[type] = [](const std::string& name, const std::string& param) -> ObjectVariant {
                return std::make_unique<FileObject>(name, param);
            };
        } else if (type == "database") {
            creators_[type] = [](const std::string& name, const std::string& param) -> ObjectVariant {
                return std::make_unique<DatabaseObject>(name, param);
            };
        } else if (type == "network") {
            creators_[type] = [](const std::string& name, const std::string& param) -> ObjectVariant {
                return std::make_unique<NetworkObject>(name, param);
            };
        }
    }
    
    std::optional<ObjectVariant> createObject(const std::string& type, const std::string& name, const std::string& param) {
        auto it = creators_.find(type);
        if (it != creators_.end()) {
            return it->second(name, param);
        }
        return std::nullopt;
    }
    
    std::vector<std::string> getAvailableTypes() const {
        std::vector<std::string> types;
        for (const auto& pair : creators_) {
            types.push_back(pair.first);
        }
        return types;
    }
};
```

### Пример использования
```cpp
int main() {
    MoveableObjectFactory factory;
    
    // Регистрируем объекты
    factory.registerObject("file");
    factory.registerObject("database");
    factory.registerObject("network");
    
    // Создаем многоуровневый менеджер перемещения
    MultiLevelMoveManager manager;
    
    // Добавляем объекты
    if (auto object1 = factory.createObject("file", "file1", "/tmp/file1.txt")) {
        std::visit([&manager](auto& obj) {
            manager.addObject(std::move(obj));
        }, *object1);
    }
    
    if (auto object2 = factory.createObject("database", "db1", "users")) {
        std::visit([&manager](auto& obj) {
            manager.addObject(std::move(obj));
        }, *object2);
    }
    
    if (auto object3 = factory.createObject("network", "net1", "https://api.example.com")) {
        std::visit([&manager](auto& obj) {
            manager.addObject(std::move(obj));
        }, *object3);
    }
    
    // Показываем информацию об объектах
    std::cout << manager.getObjectsInfo() << std::endl;
    
    // Перемещаем объекты по одному
    std::cout << "=== Moving Objects One by One ===" << std::endl;
    for (size_t i = 0; i < manager.getObjectCount(); ++i) {
        std::cout << "Moving object " << i << "..." << std::endl;
        if (manager.moveObject(i, "destination_" + std::to_string(i))) {
            std::cout << "Object " << i << " moved successfully" << std::endl;
        } else {
            std::cout << "Object " << i << " move failed" << std::endl;
        }
    }
    
    // Показываем историю перемещений
    auto history = manager.getMoveHistory();
    std::cout << "\nMove history:" << std::endl;
    for (const auto& move : history) {
        std::cout << "  " << move << std::endl;
    }
    
    return 0;
}
```

---

## 🎯 Критерии оценки

### Отлично (5 баллов)
- ✅ Все требования выполнены
- ✅ Современные C++17 возможности
- ✅ Правильная архитектура
- ✅ Хорошая валидация
- ✅ Полная документация

### Хорошо (4 балла)
- ✅ Большинство требований выполнено
- ✅ Современные возможности использованы
- ✅ Код работает корректно

### Удовлетворительно (3 балла)
- ✅ Основные требования выполнены
- ✅ Код компилируется и работает
- ❌ Не все возможности использованы

### Неудовлетворительно (2 балла)
- ❌ Код не компилируется или работает неправильно
- ❌ Не выполнены основные требования

---

## 💡 Подсказки

1. **Move Semantics**: Используйте move semantics для эффективного перемещения объектов
2. **Perfect Forwarding**: Применяйте perfect forwarding для передачи параметров
3. **Error Handling**: Обрабатывайте ошибки перемещения
4. **Validation**: Валидируйте объекты перед перемещением
5. **Testing**: Создавайте тесты для проверки перемещения

---

*Помните: Move Semantics обеспечивают эффективное перемещение объектов без копирования!*
