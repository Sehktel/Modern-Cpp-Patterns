# 🏋️ Упражнения: Smart Pointers

## 📋 Задание 1: Modern Smart Pointers с Templates

### Описание
Создайте современную реализацию Smart Pointers с использованием C++17:

### Требования
1. ✅ Используйте `std::unique_ptr` для единоличного владения
2. ✅ Примените `std::shared_ptr` для разделяемого владения
3. ✅ Используйте `std::weak_ptr` для слабых ссылок
4. ✅ Добавьте поддержку variadic templates
5. ✅ Реализуйте type-safe управление памятью

### Современная архитектура
```cpp
#include <memory>
#include <variant>
#include <optional>
#include <unordered_map>
#include <functional>
#include <string>
#include <vector>

// Базовый класс ресурса
class Resource {
protected:
    std::string name_;
    bool isOpen_;
    
public:
    explicit Resource(const std::string& name) : name_(name), isOpen_(false) {
        std::cout << "Resource " << name_ << " created" << std::endl;
    }
    
    virtual ~Resource() {
        if (isOpen_) {
            close();
        }
        std::cout << "Resource " << name_ << " destroyed" << std::endl;
    }
    
    virtual void open() {
        if (!isOpen_) {
            isOpen_ = true;
            std::cout << "Resource " << name_ << " opened" << std::endl;
        }
    }
    
    virtual void close() {
        if (isOpen_) {
            isOpen_ = false;
            std::cout << "Resource " << name_ << " closed" << std::endl;
        }
    }
    
    virtual bool isOpen() const {
        return isOpen_;
    }
    
    virtual std::string getName() const {
        return name_;
    }
    
    // Запрещаем копирование
    Resource(const Resource&) = delete;
    Resource& operator=(const Resource&) = delete;
    
    // Разрешаем перемещение
    Resource(Resource&& other) noexcept : name_(std::move(other.name_)), isOpen_(other.isOpen_) {
        other.isOpen_ = false;
        std::cout << "Resource " << name_ << " moved" << std::endl;
    }
    
    Resource& operator=(Resource&& other) noexcept {
        if (this != &other) {
            name_ = std::move(other.name_);
            isOpen_ = other.isOpen_;
            other.isOpen_ = false;
            std::cout << "Resource " << name_ << " moved assigned" << std::endl;
        }
        return *this;
    }
};

// Конкретные ресурсы
class FileResource : public Resource {
private:
    std::string filePath_;
    
public:
    FileResource(const std::string& name, const std::string& filePath) 
        : Resource(name), filePath_(filePath) {}
    
    void open() override {
        Resource::open();
        std::cout << "File " << filePath_ << " opened" << std::endl;
    }
    
    void close() override {
        Resource::close();
        std::cout << "File " << filePath_ << " closed" << std::endl;
    }
    
    std::string getFilePath() const {
        return filePath_;
    }
};

class DatabaseResource : public Resource {
private:
    std::string connectionString_;
    
public:
    DatabaseResource(const std::string& name, const std::string& connectionString) 
        : Resource(name), connectionString_(connectionString) {}
    
    void open() override {
        Resource::open();
        std::cout << "Database connection " << connectionString_ << " established" << std::endl;
    }
    
    void close() override {
        Resource::close();
        std::cout << "Database connection " << connectionString_ << " closed" << std::endl;
    }
    
    std::string getConnectionString() const {
        return connectionString_;
    }
};

class NetworkResource : public Resource {
private:
    std::string url_;
    
public:
    NetworkResource(const std::string& name, const std::string& url) 
        : Resource(name), url_(url) {}
    
    void open() override {
        Resource::open();
        std::cout << "Network connection to " << url_ << " established" << std::endl;
    }
    
    void close() override {
        Resource::close();
        std::cout << "Network connection to " << url_ << " closed" << std::endl;
    }
    
    std::string getUrl() const {
        return url_;
    }
};

// Современный менеджер ресурсов
class ModernResourceManager {
private:
    // Уникальные ресурсы (единоличное владение)
    std::unordered_map<std::string, std::unique_ptr<Resource>> uniqueResources_;
    
    // Разделяемые ресурсы (разделяемое владение)
    std::unordered_map<std::string, std::shared_ptr<Resource>> sharedResources_;
    
    // Слабые ссылки на ресурсы
    std::unordered_map<std::string, std::weak_ptr<Resource>> weakResources_;
    
public:
    ModernResourceManager() = default;
    
    // Добавление уникального ресурса
    template<typename T, typename... Args>
    void addUniqueResource(const std::string& name, Args&&... args) {
        static_assert(std::is_base_of_v<Resource, T>, "T must be derived from Resource");
        
        auto resource = std::make_unique<T>(std::forward<Args>(args)...);
        uniqueResources_[name] = std::move(resource);
    }
    
    // Добавление разделяемого ресурса
    template<typename T, typename... Args>
    void addSharedResource(const std::string& name, Args&&... args) {
        static_assert(std::is_base_of_v<Resource, T>, "T must be derived from Resource");
        
        auto resource = std::make_shared<T>(std::forward<Args>(args)...);
        sharedResources_[name] = resource;
        weakResources_[name] = resource;
    }
    
    // Получение уникального ресурса
    Resource* getUniqueResource(const std::string& name) const {
        auto it = uniqueResources_.find(name);
        return it != uniqueResources_.end() ? it->second.get() : nullptr;
    }
    
    // Получение разделяемого ресурса
    std::shared_ptr<Resource> getSharedResource(const std::string& name) const {
        auto it = sharedResources_.find(name);
        return it != sharedResources_.end() ? it->second : nullptr;
    }
    
    // Получение слабой ссылки на ресурс
    std::weak_ptr<Resource> getWeakResource(const std::string& name) const {
        auto it = weakResources_.find(name);
        return it != weakResources_.end() ? it->second : std::weak_ptr<Resource>();
    }
    
    // Демонстрация слабых ссылок
    void demonstrateWeakReferences() {
        std::cout << "\n=== Demonstrating Weak References ===" << std::endl;
        
        for (const auto& pair : weakResources_) {
            const std::string& name = pair.first;
            const std::weak_ptr<Resource>& weakPtr = pair.second;
            
            if (auto sharedPtr = weakPtr.lock()) {
                std::cout << "Weak reference to " << name << " is valid" << std::endl;
                std::cout << "  Reference count: " << sharedPtr.use_count() << std::endl;
            } else {
                std::cout << "Weak reference to " << name << " is expired" << std::endl;
            }
        }
    }
    
    // Статус ресурсов
    void printResourceStatus() const {
        std::cout << "\n=== Resource Status ===" << std::endl;
        
        std::cout << "Unique Resources:" << std::endl;
        for (const auto& pair : uniqueResources_) {
            std::cout << "  " << pair.first << ": " << (pair.second->isOpen() ? "Open" : "Closed") << std::endl;
        }
        
        std::cout << "Shared Resources:" << std::endl;
        for (const auto& pair : sharedResources_) {
            std::cout << "  " << pair.first << ": " << (pair.second->isOpen() ? "Open" : "Closed") 
                      << " (ref count: " << pair.second.use_count() << ")" << std::endl;
        }
        
        std::cout << "Weak References:" << std::endl;
        for (const auto& pair : weakResources_) {
            std::cout << "  " << pair.first << ": " << (pair.second.expired() ? "Expired" : "Valid") << std::endl;
        }
    }
    
    // Открытие всех ресурсов
    void openAllResources() {
        std::cout << "\n=== Opening All Resources ===" << std::endl;
        
        for (auto& pair : uniqueResources_) {
            pair.second->open();
        }
        
        for (auto& pair : sharedResources_) {
            pair.second->open();
        }
    }
    
    // Закрытие всех ресурсов
    void closeAllResources() {
        std::cout << "\n=== Closing All Resources ===" << std::endl;
        
        for (auto& pair : uniqueResources_) {
            pair.second->close();
        }
        
        for (auto& pair : sharedResources_) {
            pair.second->close();
        }
    }
    
    // Получение количества ресурсов
    size_t getUniqueResourceCount() const {
        return uniqueResources_.size();
    }
    
    size_t getSharedResourceCount() const {
        return sharedResources_.size();
    }
    
    size_t getWeakResourceCount() const {
        return weakResources_.size();
    }
};

// Фабрика ресурсов
class ResourceFactory {
private:
    using ResourceVariant = std::variant<
        std::unique_ptr<FileResource>,
        std::unique_ptr<DatabaseResource>,
        std::unique_ptr<NetworkResource>
    >;
    
    using CreatorFunction = std::function<ResourceVariant(const std::string&, const std::string&)>;
    std::unordered_map<std::string, CreatorFunction> creators_;
    
public:
    void registerResource(const std::string& type) {
        if (type == "file") {
            creators_[type] = [](const std::string& name, const std::string& param) -> ResourceVariant {
                return std::make_unique<FileResource>(name, param);
            };
        } else if (type == "database") {
            creators_[type] = [](const std::string& name, const std::string& param) -> ResourceVariant {
                return std::make_unique<DatabaseResource>(name, param);
            };
        } else if (type == "network") {
            creators_[type] = [](const std::string& name, const std::string& param) -> ResourceVariant {
                return std::make_unique<NetworkResource>(name, param);
            };
        }
    }
    
    std::optional<ResourceVariant> createResource(const std::string& type, const std::string& name, const std::string& param) {
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
    ResourceFactory factory;
    
    // Регистрируем ресурсы
    factory.registerResource("file");
    factory.registerResource("database");
    factory.registerResource("network");
    
    // Создаем менеджер ресурсов
    ModernResourceManager manager;
    
    // Добавляем уникальные ресурсы
    manager.addUniqueResource<FileResource>("file1", "file1.txt");
    manager.addUniqueResource<DatabaseResource>("db1", "localhost:5432");
    
    // Добавляем разделяемые ресурсы
    manager.addSharedResource<NetworkResource>("net1", "https://api.example.com");
    manager.addSharedResource<FileResource>("file2", "file2.txt");
    
    // Показываем статус ресурсов
    manager.printResourceStatus();
    
    // Открываем все ресурсы
    manager.openAllResources();
    
    // Демонстрируем слабые ссылки
    manager.demonstrateWeakReferences();
    
    // Создаем дополнительные ссылки на разделяемые ресурсы
    auto sharedFile = manager.getSharedResource("file2");
    auto sharedNet = manager.getSharedResource("net1");
    
    std::cout << "\nAdditional references created" << std::endl;
    manager.printResourceStatus();
    
    // Демонстрируем слабые ссылки снова
    manager.demonstrateWeakReferences();
    
    // Закрываем все ресурсы
    manager.closeAllResources();
    
    // Показываем финальный статус
    manager.printResourceStatus();
    
    return 0;
}
```

---

## 📋 Задание 2: Multi-Level Smart Pointer System

### Описание
Создайте систему многоуровневых Smart Pointers для работы с разными типами объектов:

### Требования
1. **Multiple Levels**: Поддержка множественных уровней Smart Pointers
2. **Type Safety**: Проверка типов на этапе компиляции
3. **Error Handling**: Обработка ошибок управления памятью
4. **Validation**: Валидация Smart Pointers

### Реализация
```cpp
#include <any>
#include <type_traits>
#include <stdexcept>

// Базовый интерфейс для управляемого объекта
class ManagedObject {
public:
    virtual ~ManagedObject() = default;
    virtual std::string getName() const = 0;
    virtual std::string getDescription() const = 0;
    virtual bool isValid() const = 0;
    virtual void initialize() = 0;
    virtual void cleanup() = 0;
};

// Конкретные управляемые объекты
class MemoryBuffer : public ManagedObject {
private:
    std::string name_;
    size_t size_;
    std::unique_ptr<char[]> buffer_;
    bool initialized_;
    
public:
    MemoryBuffer(const std::string& name, size_t size) 
        : name_(name), size_(size), initialized_(false) {
        buffer_ = std::make_unique<char[]>(size_);
        std::cout << "MemoryBuffer " << name_ << " created with size " << size_ << std::endl;
    }
    
    ~MemoryBuffer() {
        if (initialized_) {
            cleanup();
        }
        std::cout << "MemoryBuffer " << name_ << " destroyed" << std::endl;
    }
    
    std::string getName() const override {
        return name_;
    }
    
    std::string getDescription() const override {
        return "Memory buffer of size " + std::to_string(size_);
    }
    
    bool isValid() const override {
        return buffer_ != nullptr && size_ > 0;
    }
    
    void initialize() override {
        if (!initialized_) {
            std::fill(buffer_.get(), buffer_.get() + size_, 0);
            initialized_ = true;
            std::cout << "MemoryBuffer " << name_ << " initialized" << std::endl;
        }
    }
    
    void cleanup() override {
        if (initialized_) {
            std::fill(buffer_.get(), buffer_.get() + size_, 0);
            initialized_ = false;
            std::cout << "MemoryBuffer " << name_ << " cleaned up" << std::endl;
        }
    }
    
    char* getBuffer() {
        return buffer_.get();
    }
    
    size_t getSize() const {
        return size_;
    }
};

class FileHandle : public ManagedObject {
private:
    std::string name_;
    std::string filePath_;
    bool opened_;
    
public:
    FileHandle(const std::string& name, const std::string& filePath) 
        : name_(name), filePath_(filePath), opened_(false) {
        std::cout << "FileHandle " << name_ << " created for " << filePath_ << std::endl;
    }
    
    ~FileHandle() {
        if (opened_) {
            cleanup();
        }
        std::cout << "FileHandle " << name_ << " destroyed" << std::endl;
    }
    
    std::string getName() const override {
        return name_;
    }
    
    std::string getDescription() const override {
        return "File handle for " + filePath_;
    }
    
    bool isValid() const override {
        return !filePath_.empty();
    }
    
    void initialize() override {
        if (!opened_) {
            opened_ = true;
            std::cout << "FileHandle " << name_ << " opened" << std::endl;
        }
    }
    
    void cleanup() override {
        if (opened_) {
            opened_ = false;
            std::cout << "FileHandle " << name_ << " closed" << std::endl;
        }
    }
    
    std::string getFilePath() const {
        return filePath_;
    }
    
    bool isOpened() const {
        return opened_;
    }
};

class NetworkSocket : public ManagedObject {
private:
    std::string name_;
    std::string address_;
    int port_;
    bool connected_;
    
public:
    NetworkSocket(const std::string& name, const std::string& address, int port) 
        : name_(name), address_(address), port_(port), connected_(false) {
        std::cout << "NetworkSocket " << name_ << " created for " << address_ << ":" << port_ << std::endl;
    }
    
    ~NetworkSocket() {
        if (connected_) {
            cleanup();
        }
        std::cout << "NetworkSocket " << name_ << " destroyed" << std::endl;
    }
    
    std::string getName() const override {
        return name_;
    }
    
    std::string getDescription() const override {
        return "Network socket for " + address_ + ":" + std::to_string(port_);
    }
    
    bool isValid() const override {
        return !address_.empty() && port_ > 0;
    }
    
    void initialize() override {
        if (!connected_) {
            connected_ = true;
            std::cout << "NetworkSocket " << name_ << " connected" << std::endl;
        }
    }
    
    void cleanup() override {
        if (connected_) {
            connected_ = false;
            std::cout << "NetworkSocket " << name_ << " disconnected" << std::endl;
        }
    }
    
    std::string getAddress() const {
        return address_;
    }
    
    int getPort() const {
        return port_;
    }
    
    bool isConnected() const {
        return connected_;
    }
};

// Многоуровневый менеджер Smart Pointers
class MultiLevelSmartPointerManager {
private:
    // Уникальные указатели
    std::unordered_map<std::string, std::unique_ptr<ManagedObject>> uniqueObjects_;
    
    // Разделяемые указатели
    std::unordered_map<std::string, std::shared_ptr<ManagedObject>> sharedObjects_;
    
    // Слабые указатели
    std::unordered_map<std::string, std::weak_ptr<ManagedObject>> weakObjects_;
    
public:
    MultiLevelSmartPointerManager() = default;
    
    // Добавление уникального объекта
    template<typename T, typename... Args>
    void addUniqueObject(const std::string& name, Args&&... args) {
        static_assert(std::is_base_of_v<ManagedObject, T>, "T must be derived from ManagedObject");
        
        auto object = std::make_unique<T>(std::forward<Args>(args)...);
        uniqueObjects_[name] = std::move(object);
    }
    
    // Добавление разделяемого объекта
    template<typename T, typename... Args>
    void addSharedObject(const std::string& name, Args&&... args) {
        static_assert(std::is_base_of_v<ManagedObject, T>, "T must be derived from ManagedObject");
        
        auto object = std::make_shared<T>(std::forward<Args>(args)...);
        sharedObjects_[name] = object;
        weakObjects_[name] = object;
    }
    
    // Получение уникального объекта
    ManagedObject* getUniqueObject(const std::string& name) const {
        auto it = uniqueObjects_.find(name);
        return it != uniqueObjects_.end() ? it->second.get() : nullptr;
    }
    
    // Получение разделяемого объекта
    std::shared_ptr<ManagedObject> getSharedObject(const std::string& name) const {
        auto it = sharedObjects_.find(name);
        return it != sharedObjects_.end() ? it->second : nullptr;
    }
    
    // Получение слабого указателя
    std::weak_ptr<ManagedObject> getWeakObject(const std::string& name) const {
        auto it = weakObjects_.find(name);
        return it != weakObjects_.end() ? it->second : std::weak_ptr<ManagedObject>();
    }
    
    // Инициализация всех объектов
    void initializeAll() {
        std::cout << "\n=== Initializing All Objects ===" << std::endl;
        
        for (auto& pair : uniqueObjects_) {
            if (pair.second && pair.second->isValid()) {
                pair.second->initialize();
            }
        }
        
        for (auto& pair : sharedObjects_) {
            if (pair.second && pair.second->isValid()) {
                pair.second->initialize();
            }
        }
    }
    
    // Очистка всех объектов
    void cleanupAll() {
        std::cout << "\n=== Cleaning Up All Objects ===" << std::endl;
        
        for (auto& pair : uniqueObjects_) {
            if (pair.second && pair.second->isValid()) {
                pair.second->cleanup();
            }
        }
        
        for (auto& pair : sharedObjects_) {
            if (pair.second && pair.second->isValid()) {
                pair.second->cleanup();
            }
        }
    }
    
    // Демонстрация слабых указателей
    void demonstrateWeakPointers() {
        std::cout << "\n=== Demonstrating Weak Pointers ===" << std::endl;
        
        for (const auto& pair : weakObjects_) {
            const std::string& name = pair.first;
            const std::weak_ptr<ManagedObject>& weakPtr = pair.second;
            
            if (auto sharedPtr = weakPtr.lock()) {
                std::cout << "Weak pointer to " << name << " is valid" << std::endl;
                std::cout << "  Reference count: " << sharedPtr.use_count() << std::endl;
                std::cout << "  Description: " << sharedPtr->getDescription() << std::endl;
            } else {
                std::cout << "Weak pointer to " << name << " is expired" << std::endl;
            }
        }
    }
    
    // Статус объектов
    void printObjectStatus() const {
        std::cout << "\n=== Object Status ===" << std::endl;
        
        std::cout << "Unique Objects:" << std::endl;
        for (const auto& pair : uniqueObjects_) {
            std::cout << "  " << pair.first << ": " << pair.second->getDescription() << std::endl;
        }
        
        std::cout << "Shared Objects:" << std::endl;
        for (const auto& pair : sharedObjects_) {
            std::cout << "  " << pair.first << ": " << pair.second->getDescription() 
                      << " (ref count: " << pair.second.use_count() << ")" << std::endl;
        }
        
        std::cout << "Weak Pointers:" << std::endl;
        for (const auto& pair : weakObjects_) {
            std::cout << "  " << pair.first << ": " << (pair.second.expired() ? "Expired" : "Valid") << std::endl;
        }
    }
    
    // Получение количества объектов
    size_t getUniqueObjectCount() const {
        return uniqueObjects_.size();
    }
    
    size_t getSharedObjectCount() const {
        return sharedObjects_.size();
    }
    
    size_t getWeakObjectCount() const {
        return weakObjects_.size();
    }
};

// Фабрика управляемых объектов
class ManagedObjectFactory {
private:
    using ObjectVariant = std::variant<
        std::unique_ptr<MemoryBuffer>,
        std::unique_ptr<FileHandle>,
        std::unique_ptr<NetworkSocket>
    >;
    
    using CreatorFunction = std::function<ObjectVariant(const std::string&, const std::string&, int)>;
    std::unordered_map<std::string, CreatorFunction> creators_;
    
public:
    void registerObject(const std::string& type) {
        if (type == "memory") {
            creators_[type] = [](const std::string& name, const std::string& param, int size) -> ObjectVariant {
                return std::make_unique<MemoryBuffer>(name, size);
            };
        } else if (type == "file") {
            creators_[type] = [](const std::string& name, const std::string& param, int port) -> ObjectVariant {
                return std::make_unique<FileHandle>(name, param);
            };
        } else if (type == "network") {
            creators_[type] = [](const std::string& name, const std::string& param, int port) -> ObjectVariant {
                return std::make_unique<NetworkSocket>(name, param, port);
            };
        }
    }
    
    std::optional<ObjectVariant> createObject(const std::string& type, const std::string& name, const std::string& param, int value = 0) {
        auto it = creators_.find(type);
        if (it != creators_.end()) {
            return it->second(name, param, value);
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
    ManagedObjectFactory factory;
    
    // Регистрируем объекты
    factory.registerObject("memory");
    factory.registerObject("file");
    factory.registerObject("network");
    
    // Создаем многоуровневый менеджер Smart Pointers
    MultiLevelSmartPointerManager manager;
    
    // Добавляем уникальные объекты
    manager.addUniqueObject<MemoryBuffer>("buffer1", 1024);
    manager.addUniqueObject<FileHandle>("file1", "test.txt");
    
    // Добавляем разделяемые объекты
    manager.addSharedObject<NetworkSocket>("socket1", "localhost", 8080);
    manager.addSharedObject<MemoryBuffer>("buffer2", 2048);
    
    // Показываем статус объектов
    manager.printObjectStatus();
    
    // Инициализируем все объекты
    manager.initializeAll();
    
    // Демонстрируем слабые указатели
    manager.demonstrateWeakPointers();
    
    // Создаем дополнительные ссылки на разделяемые объекты
    auto sharedBuffer = manager.getSharedObject("buffer2");
    auto sharedSocket = manager.getSharedObject("socket1");
    
    std::cout << "\nAdditional references created" << std::endl;
    manager.printObjectStatus();
    
    // Демонстрируем слабые указатели снова
    manager.demonstrateWeakPointers();
    
    // Очищаем все объекты
    manager.cleanupAll();
    
    // Показываем финальный статус
    manager.printObjectStatus();
    
    return 0;
}
```

---

## 📋 Задание 3: Smart Pointers с Configuration

### Описание
Создайте Smart Pointers, которые поддерживают конфигурацию и могут быть легко настраиваемыми:

### Требования
1. **Configuration Support**: Поддержка различных конфигураций
2. **Style Configuration**: Настройка стилей через конфигурацию
3. **Dynamic Loading**: Динамическая загрузка конфигураций
4. **Validation**: Валидация конфигурации

### Реализация
```cpp
#include <any>
#include <type_traits>
#include <stdexcept>

// Конфигурация Smart Pointer
struct SmartPointerConfig {
    std::string name;
    std::string version;
    std::unordered_map<std::string, std::string> settings;
    std::unordered_map<std::string, std::any> customSettings;
    
    SmartPointerConfig(const std::string& n, const std::string& v)
        : name(n), version(v) {}
    
    void setSetting(const std::string& key, const std::string& value) {
        settings[key] = value;
    }
    
    std::string getSetting(const std::string& key) const {
        auto it = settings.find(key);
        return it != settings.end() ? it->second : "";
    }
    
    template<typename T>
    void setCustomSetting(const std::string& key, const T& value) {
        customSettings[key] = value;
    }
    
    template<typename T>
    std::optional<T> getCustomSetting(const std::string& key) const {
        auto it = customSettings.find(key);
        if (it != customSettings.end()) {
            try {
                return std::any_cast<T>(it->second);
            } catch (const std::bad_any_cast&) {
                return std::nullopt;
            }
        }
        return std::nullopt;
    }
};

// Базовый класс с конфигурацией
class ConfigurableSmartPointer {
protected:
    SmartPointerConfig config_;
    
public:
    ConfigurableSmartPointer(const SmartPointerConfig& config) : config_(config) {}
    
    virtual void validateConfig() const {
        if (config_.name.empty()) {
            throw std::invalid_argument("Smart pointer name cannot be empty");
        }
        if (config_.version.empty()) {
            throw std::invalid_argument("Smart pointer version cannot be empty");
        }
    }
    
    std::string getName() const {
        return config_.name;
    }
    
    std::string getVersion() const {
        return config_.version;
    }
    
    std::string getSetting(const std::string& key) const {
        return config_.getSetting(key);
    }
    
    template<typename T>
    std::optional<T> getCustomSetting(const std::string& key) const {
        return config_.getCustomSetting<T>(key);
    }
};

// Конфигурируемый Smart Pointer
class ConfigurableResourceManager : public ConfigurableSmartPointer {
private:
    std::string resourceType_;
    bool enabled_;
    std::unordered_map<std::string, std::unique_ptr<Resource>> resources_;
    
public:
    ConfigurableResourceManager(const SmartPointerConfig& config) 
        : ConfigurableSmartPointer(config), enabled_(true) {
        validateConfig();
        
        // Загружаем тип ресурса из конфигурации
        resourceType_ = config_.getSetting("resource.type");
        if (resourceType_.empty()) {
            resourceType_ = "default";
        }
        
        // Загружаем состояние включен/выключен
        if (auto enabled = config_.getCustomSetting<bool>("enabled")) {
            enabled_ = *enabled;
        }
    }
    
    // Добавление ресурса
    template<typename T, typename... Args>
    void addResource(const std::string& name, Args&&... args) {
        if (!enabled_) {
            std::cout << "Resource manager is disabled" << std::endl;
            return;
        }
        
        static_assert(std::is_base_of_v<Resource, T>, "T must be derived from Resource");
        
        auto resource = std::make_unique<T>(std::forward<Args>(args)...);
        resources_[name] = std::move(resource);
        
        std::cout << "Resource " << name << " added to " << config_.name << std::endl;
    }
    
    // Получение ресурса
    Resource* getResource(const std::string& name) const {
        auto it = resources_.find(name);
        return it != resources_.end() ? it->second.get() : nullptr;
    }
    
    // Удаление ресурса
    void removeResource(const std::string& name) {
        auto it = resources_.find(name);
        if (it != resources_.end()) {
            resources_.erase(it);
            std::cout << "Resource " << name << " removed from " << config_.name << std::endl;
        }
    }
    
    // Открытие всех ресурсов
    void openAllResources() {
        if (!enabled_) {
            std::cout << "Resource manager is disabled" << std::endl;
            return;
        }
        
        std::cout << "Opening all resources in " << config_.name << std::endl;
        
        for (auto& pair : resources_) {
            if (pair.second && pair.second->isValid()) {
                pair.second->open();
            }
        }
    }
    
    // Закрытие всех ресурсов
    void closeAllResources() {
        if (!enabled_) {
            std::cout << "Resource manager is disabled" << std::endl;
            return;
        }
        
        std::cout << "Closing all resources in " << config_.name << std::endl;
        
        for (auto& pair : resources_) {
            if (pair.second && pair.second->isValid()) {
                pair.second->close();
            }
        }
    }
    
    // Получение информации о ресурсах
    std::string getResourcesInfo() const {
        std::string info = "Resources in " + config_.name + ":\n";
        
        for (const auto& pair : resources_) {
            info += "  " + pair.first + ": " + pair.second->getName() + 
                   " (" + (pair.second->isOpen() ? "Open" : "Closed") + ")\n";
        }
        
        return info;
    }
    
    // Дополнительные методы
    void setEnabled(bool enabled) {
        enabled_ = enabled;
    }
    
    bool isEnabled() const {
        return enabled_;
    }
    
    void setResourceType(const std::string& type) {
        resourceType_ = type;
    }
    
    std::string getResourceType() const {
        return resourceType_;
    }
    
    size_t getResourceCount() const {
        return resources_.size();
    }
    
    void display() const {
        std::cout << "Configurable Resource Manager: " << config_.name << " v" << config_.version << std::endl;
        std::cout << "  Resource type: " << resourceType_ << std::endl;
        std::cout << "  Enabled: " << (enabled_ ? "Yes" : "No") << std::endl;
        std::cout << "  Resource count: " << getResourceCount() << std::endl;
        std::cout << "  Settings:" << std::endl;
        for (const auto& setting : config_.settings) {
            std::cout << "    " << setting.first << ": " << setting.second << std::endl;
        }
    }
};

// Конфигурируемая фабрика Smart Pointers
class ConfigurableSmartPointerFactory {
private:
    SmartPointerConfig config_;
    std::string smartPointerType_;
    
public:
    ConfigurableSmartPointerFactory(const SmartPointerConfig& config, const std::string& type)
        : config_(config), smartPointerType_(type) {}
    
    // Методы для настройки конфигурации
    ConfigurableSmartPointerFactory& setSetting(const std::string& key, const std::string& value) {
        config_.setSetting(key, value);
        return *this;
    }
    
    template<typename T>
    ConfigurableSmartPointerFactory& setCustomSetting(const std::string& key, const T& value) {
        config_.setCustomSetting(key, value);
        return *this;
    }
    
    // Создание Smart Pointer
    std::unique_ptr<ConfigurableSmartPointer> build() {
        if (smartPointerType_ == "resource_manager") {
            return std::make_unique<ConfigurableResourceManager>(config_);
        }
        throw std::invalid_argument("Unknown smart pointer type: " + smartPointerType_);
    }
    
    // Создание Smart Pointer с валидацией
    std::unique_ptr<ConfigurableSmartPointer> buildSafe() {
        try {
            return build();
        } catch (const std::exception& e) {
            std::cerr << "Error building smart pointer: " << e.what() << std::endl;
            return nullptr;
        }
    }
    
    // Получение текущей конфигурации
    SmartPointerConfig getConfig() const {
        return config_;
    }
    
    std::string getSmartPointerType() const {
        return smartPointerType_;
    }
};

// Менеджер конфигураций
class SmartPointerConfigManager {
private:
    std::unordered_map<std::string, SmartPointerConfig> configs_;
    
public:
    void registerConfig(const std::string& name, const SmartPointerConfig& config) {
        configs_[name] = config;
    }
    
    std::optional<SmartPointerConfig> getConfig(const std::string& name) const {
        auto it = configs_.find(name);
        if (it != configs_.end()) {
            return it->second;
        }
        return std::nullopt;
    }
    
    std::vector<std::string> getAvailableConfigs() const {
        std::vector<std::string> configNames;
        for (const auto& pair : configs_) {
            configNames.push_back(pair.first);
        }
        return configNames;
    }
    
    std::unique_ptr<ConfigurableSmartPointerFactory> createFactory(
        const std::string& configName, 
        const std::string& smartPointerType
    ) {
        auto config = getConfig(configName);
        if (config) {
            return std::make_unique<ConfigurableSmartPointerFactory>(*config, smartPointerType);
        }
        return nullptr;
    }
};
```

### Пример использования
```cpp
int main() {
    SmartPointerConfigManager configManager;
    
    // Регистрируем конфигурации
    SmartPointerConfig fileManagerConfig("File Manager", "1.0");
    fileManagerConfig.setSetting("resource.type", "file");
    fileManagerConfig.setSetting("resource.path", "/tmp");
    fileManagerConfig.setSetting("logging.level", "INFO");
    fileManagerConfig.setCustomSetting("enabled", true);
    fileManagerConfig.setCustomSetting("max.resources", 10);
    
    SmartPointerConfig networkManagerConfig("Network Manager", "2.0");
    networkManagerConfig.setSetting("resource.type", "network");
    networkManagerConfig.setSetting("resource.timeout", "30");
    networkManagerConfig.setSetting("logging.level", "DEBUG");
    networkManagerConfig.setCustomSetting("enabled", true);
    networkManagerConfig.setCustomSetting("max.connections", 5);
    
    configManager.registerConfig("file", fileManagerConfig);
    configManager.registerConfig("network", networkManagerConfig);
    
    // Создаем фабрику для файлового менеджера
    auto fileManagerFactory = configManager.createFactory("file", "resource_manager");
    if (fileManagerFactory) {
        fileManagerFactory->setSetting("logging.level", "WARN");
        fileManagerFactory->setCustomSetting("enabled", true);
        
        if (auto smartPointer = fileManagerFactory->buildSafe()) {
            auto resourceManager = static_cast<ConfigurableResourceManager*>(smartPointer.get());
            resourceManager->display();
            
            // Добавляем ресурсы
            resourceManager->addResource<FileResource>("file1", "test1.txt");
            resourceManager->addResource<FileResource>("file2", "test2.txt");
            
            // Показываем информацию о ресурсах
            std::cout << resourceManager->getResourcesInfo() << std::endl;
            
            // Открываем все ресурсы
            resourceManager->openAllResources();
            
            // Закрываем все ресурсы
            resourceManager->closeAllResources();
        }
    }
    
    // Создаем фабрику для сетевого менеджера
    auto networkManagerFactory = configManager.createFactory("network", "resource_manager");
    if (networkManagerFactory) {
        networkManagerFactory->setSetting("logging.level", "TRACE");
        networkManagerFactory->setCustomSetting("enabled", true);
        
        if (auto smartPointer = networkManagerFactory->buildSafe()) {
            auto resourceManager = static_cast<ConfigurableResourceManager*>(smartPointer.get());
            resourceManager->display();
            
            // Добавляем ресурсы
            resourceManager->addResource<NetworkResource>("socket1", "localhost", 8080);
            resourceManager->addResource<NetworkResource>("socket2", "localhost", 9090);
            
            // Показываем информацию о ресурсах
            std::cout << resourceManager->getResourcesInfo() << std::endl;
            
            // Открываем все ресурсы
            resourceManager->openAllResources();
            
            // Закрываем все ресурсы
            resourceManager->closeAllResources();
        }
    }
    
    // Показываем доступные конфигурации
    auto configs = configManager.getAvailableConfigs();
    std::cout << "\nAvailable configurations: ";
    for (const auto& config : configs) {
        std::cout << config << " ";
    }
    std::cout << std::endl;
    
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

1. **Memory Management**: Используйте Smart Pointers для автоматического управления памятью
2. **Ownership**: Правильно определяйте владение ресурсами
3. **Error Handling**: Обрабатывайте ошибки управления памятью
4. **Configuration**: Используйте конфигурацию для настройки Smart Pointers
5. **Testing**: Создавайте тесты для проверки Smart Pointers

---

*Помните: Smart Pointers обеспечивают автоматическое управление памятью и безопасность!*
