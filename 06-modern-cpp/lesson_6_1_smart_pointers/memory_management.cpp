#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <map>
#include <unordered_map>
#include <chrono>
#include <thread>
#include <mutex>
#include <cassert>

/**
 * @file memory_management.cpp
 * @brief Продвинутые техники управления памятью с умными указателями
 * 
 * Этот файл демонстрирует сложные сценарии использования
 * умных указателей в реальных приложениях.
 */

// ============================================================================
// МЕНЕДЖЕР РЕСУРСОВ
// ============================================================================

/**
 * @brief Базовый интерфейс для ресурсов
 */
class Resource {
public:
    virtual ~Resource() = default;
    virtual std::string getName() const = 0;
    virtual bool isAvailable() const = 0;
    virtual void cleanup() = 0;
};

/**
 * @brief Ресурс базы данных
 */
class DatabaseResource : public Resource {
private:
    std::string connectionString_;
    bool connected_;
    
public:
    explicit DatabaseResource(const std::string& connectionString) 
        : connectionString_(connectionString), connected_(true) {
        std::cout << "🗄️ Подключение к БД: " << connectionString_ << std::endl;
    }
    
    ~DatabaseResource() override {
        if (connected_) {
            cleanup();
        }
    }
    
    std::string getName() const override {
        return "Database: " + connectionString_;
    }
    
    bool isAvailable() const override {
        return connected_;
    }
    
    void cleanup() override {
        if (connected_) {
            std::cout << "🔌 Отключение от БД: " << connectionString_ << std::endl;
            connected_ = false;
        }
    }
    
    void executeQuery(const std::string& query) {
        if (connected_) {
            std::cout << "📊 Выполнение запроса: " << query << std::endl;
        } else {
            throw std::runtime_error("База данных не подключена");
        }
    }
};

/**
 * @brief Ресурс файловой системы
 */
class FileSystemResource : public Resource {
private:
    std::string path_;
    bool mounted_;
    
public:
    explicit FileSystemResource(const std::string& path) 
        : path_(path), mounted_(true) {
        std::cout << "📁 Монтирование файловой системы: " << path_ << std::endl;
    }
    
    ~FileSystemResource() override {
        if (mounted_) {
            cleanup();
        }
    }
    
    std::string getName() const override {
        return "FileSystem: " + path_;
    }
    
    bool isAvailable() const override {
        return mounted_;
    }
    
    void cleanup() override {
        if (mounted_) {
            std::cout << "📤 Размонтирование файловой системы: " << path_ << std::endl;
            mounted_ = false;
        }
    }
    
    void writeFile(const std::string& filename, const std::string& content) {
        if (mounted_) {
            std::cout << "📝 Запись файла " << filename << " в " << path_ << std::endl;
        } else {
            throw std::runtime_error("Файловая система не смонтирована");
        }
    }
};

/**
 * @brief Менеджер ресурсов с умными указателями
 */
class ResourceManager {
private:
    std::map<std::string, std::unique_ptr<Resource>> resources_;
    mutable std::mutex mutex_;
    
public:
    template<typename ResourceType, typename... Args>
    void addResource(const std::string& name, Args&&... args) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto resource = std::make_unique<ResourceType>(std::forward<Args>(args)...);
        resources_[name] = std::move(resource);
        
        std::cout << "✅ Ресурс '" << name << "' добавлен" << std::endl;
    }
    
    std::unique_ptr<Resource> borrowResource(const std::string& name) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = resources_.find(name);
        if (it != resources_.end() && it->second->isAvailable()) {
            std::cout << "🔗 Ресурс '" << name << "' выдан в пользование" << std::endl;
            // Возвращаем копию (в реальном коде это может быть более сложная логика)
            return nullptr; // Упрощенная реализация
        }
        
        return nullptr;
    }
    
    void removeResource(const std::string& name) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = resources_.find(name);
        if (it != resources_.end()) {
            std::cout << "🗑️ Ресурс '" << name << "' удален" << std::endl;
            resources_.erase(it);
        }
    }
    
    void listResources() const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        std::cout << "\n📋 Доступные ресурсы:" << std::endl;
        for (const auto& [name, resource] : resources_) {
            std::cout << "  - " << name << ": " << resource->getName() 
                      << " (доступен: " << (resource->isAvailable() ? "Да" : "Нет") << ")" << std::endl;
        }
    }
    
    size_t getResourceCount() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return resources_.size();
    }
    
    ~ResourceManager() {
        std::cout << "🏁 ResourceManager уничтожается, очистка всех ресурсов..." << std::endl;
        // Все ресурсы автоматически очистятся через unique_ptr
    }
};

// ============================================================================
// КЭШ С WEAK_PTR
// ============================================================================

/**
 * @brief Кэш с автоматической очисткой неиспользуемых объектов
 */
template<typename Key, typename Value>
class WeakPtrCache {
private:
    std::unordered_map<Key, std::weak_ptr<Value>> cache_;
    mutable std::mutex mutex_;
    
public:
    std::shared_ptr<Value> get(const Key& key) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = cache_.find(key);
        if (it != cache_.end()) {
            if (auto locked = it->second.lock()) {
                std::cout << "🎯 Кэш: найден объект для ключа '" << key << "'" << std::endl;
                return locked;
            } else {
                // Объект был удален, очищаем weak_ptr
                cache_.erase(it);
                std::cout << "🧹 Кэш: очищен истекший weak_ptr для ключа '" << key << "'" << std::endl;
            }
        }
        
        return nullptr;
    }
    
    void put(const Key& key, std::shared_ptr<Value> value) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        cache_[key] = value;
        std::cout << "💾 Кэш: сохранен объект для ключа '" << key << "'" << std::endl;
    }
    
    void cleanup() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = cache_.begin();
        while (it != cache_.end()) {
            if (it->second.expired()) {
                std::cout << "🧹 Кэш: очищен истекший weak_ptr для ключа '" << it->first << "'" << std::endl;
                it = cache_.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return cache_.size();
    }
    
    void printStats() const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        size_t alive = 0, expired = 0;
        for (const auto& [key, weak] : cache_) {
            if (weak.expired()) {
                ++expired;
            } else {
                ++alive;
            }
        }
        
        std::cout << "📊 Статистика кэша: всего " << cache_.size() 
                  << ", живых " << alive << ", истекших " << expired << std::endl;
    }
};

/**
 * @brief Объект для кэширования
 */
class CacheableObject {
private:
    std::string data_;
    int id_;
    
public:
    CacheableObject(int id, const std::string& data) : data_(data), id_(id) {
        std::cout << "🏗️ CacheableObject создан: ID=" << id_ << ", данные='" << data_ << "'" << std::endl;
    }
    
    ~CacheableObject() {
        std::cout << "🗑️ CacheableObject уничтожен: ID=" << id_ << std::endl;
    }
    
    const std::string& getData() const { return data_; }
    int getId() const { return id_; }
    
    void process() {
        std::cout << "⚡ Обработка объекта ID=" << id_ << ": " << data_ << std::endl;
    }
};

// ============================================================================
// ПУЛ ОБЪЕКТОВ
// ============================================================================

/**
 * @brief Пул объектов с использованием shared_ptr
 */
template<typename ObjectType>
class ObjectPool {
private:
    std::vector<std::shared_ptr<ObjectType>> available_;
    std::vector<std::shared_ptr<ObjectType>> inUse_;
    mutable std::mutex mutex_;
    size_t maxSize_;
    
public:
    explicit ObjectPool(size_t maxSize = 10) : maxSize_(maxSize) {}
    
    template<typename... Args>
    std::shared_ptr<ObjectType> acquire(Args&&... args) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (!available_.empty()) {
            // Используем существующий объект
            auto obj = available_.back();
            available_.pop_back();
            inUse_.push_back(obj);
            
            std::cout << "🔄 Пул: переиспользован объект" << std::endl;
            return obj;
        } else if (available_.size() + inUse_.size() < maxSize_) {
            // Создаем новый объект
            auto obj = std::make_shared<ObjectType>(std::forward<Args>(args)...);
            inUse_.push_back(obj);
            
            std::cout << "🆕 Пул: создан новый объект" << std::endl;
            return obj;
        } else {
            // Пул переполнен
            std::cout << "❌ Пул: нет доступных объектов" << std::endl;
            return nullptr;
        }
    }
    
    void release(std::shared_ptr<ObjectType> obj) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = std::find(inUse_.begin(), inUse_.end(), obj);
        if (it != inUse_.end()) {
            inUse_.erase(it);
            available_.push_back(obj);
            
            std::cout << "↩️ Пул: объект возвращен" << std::endl;
        }
    }
    
    void printStats() const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        std::cout << "📊 Пул объектов: доступно " << available_.size() 
                  << ", используется " << inUse_.size() 
                  << ", максимум " << maxSize_ << std::endl;
    }
    
    size_t getAvailableCount() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return available_.size();
    }
    
    size_t getInUseCount() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return inUse_.size();
    }
};

/**
 * @brief Объект для пула
 */
class PoolObject {
private:
    int id_;
    std::string state_;
    
public:
    explicit PoolObject(int id) : id_(id), state_("создан") {
        std::cout << "🏗️ PoolObject создан: ID=" << id_ << std::endl;
    }
    
    ~PoolObject() {
        std::cout << "🗑️ PoolObject уничтожен: ID=" << id_ << std::endl;
    }
    
    void reset() {
        state_ = "сброшен";
        std::cout << "🔄 PoolObject ID=" << id_ << " сброшен для переиспользования" << std::endl;
    }
    
    void use() {
        state_ = "используется";
        std::cout << "⚡ PoolObject ID=" << id_ << " используется" << std::endl;
    }
    
    int getId() const { return id_; }
    const std::string& getState() const { return state_; }
};

// ============================================================================
// ДЕМОНСТРАЦИОННЫЕ ФУНКЦИИ
// ============================================================================

/**
 * @brief Демонстрация менеджера ресурсов
 */
void demonstrateResourceManager() {
    std::cout << "\n=== Менеджер ресурсов ===" << std::endl;
    
    ResourceManager manager;
    
    // Добавляем ресурсы
    manager.addResource<DatabaseResource>("main_db", "postgresql://localhost/mydb");
    manager.addResource<DatabaseResource>("backup_db", "postgresql://backup/mydb");
    manager.addResource<FileSystemResource>("data_fs", "/data");
    manager.addResource<FileSystemResource>("cache_fs", "/cache");
    
    // Показываем ресурсы
    manager.listResources();
    std::cout << "Всего ресурсов: " << manager.getResourceCount() << std::endl;
    
    // Удаляем один ресурс
    manager.removeResource("backup_db");
    manager.listResources();
    
    // При выходе из области видимости все ресурсы автоматически очистятся
}

/**
 * @brief Демонстрация кэша с weak_ptr
 */
void demonstrateWeakPtrCache() {
    std::cout << "\n=== Кэш с weak_ptr ===" << std::endl;
    
    WeakPtrCache<std::string, CacheableObject> cache;
    
    // Создаем и кэшируем объекты
    {
        auto obj1 = std::make_shared<CacheableObject>(1, "Данные 1");
        auto obj2 = std::make_shared<CacheableObject>(2, "Данные 2");
        auto obj3 = std::make_shared<CacheableObject>(3, "Данные 3");
        
        cache.put("key1", obj1);
        cache.put("key2", obj2);
        cache.put("key3", obj3);
        
        cache.printStats();
        
        // Получаем объекты из кэша
        auto retrieved1 = cache.get("key1");
        auto retrieved2 = cache.get("key2");
        
        if (retrieved1) {
            retrieved1->process();
        }
        if (retrieved2) {
            retrieved2->process();
        }
        
        cache.printStats();
    }
    
    // obj1, obj2, obj3 уничтожены, но weak_ptr остались
    cache.printStats();
    
    // Попытка получить объекты из кэша
    auto retrieved1 = cache.get("key1");
    if (!retrieved1) {
        std::cout << "❌ Объект для key1 не найден (уничтожен)" << std::endl;
    }
    
    // Очищаем истекшие weak_ptr
    cache.cleanup();
    cache.printStats();
}

/**
 * @brief Демонстрация пула объектов
 */
void demonstrateObjectPool() {
    std::cout << "\n=== Пул объектов ===" << std::endl;
    
    ObjectPool<PoolObject> pool(3);
    
    // Получаем объекты из пула
    std::vector<std::shared_ptr<PoolObject>> objects;
    
    for (int i = 0; i < 5; ++i) {
        auto obj = pool.acquire(i + 1);
        if (obj) {
            obj->use();
            objects.push_back(obj);
        }
        pool.printStats();
    }
    
    // Возвращаем некоторые объекты
    if (!objects.empty()) {
        pool.release(objects[0]);
        pool.release(objects[1]);
        objects.erase(objects.begin(), objects.begin() + 2);
    }
    
    pool.printStats();
    
    // Получаем еще объекты
    for (int i = 0; i < 3; ++i) {
        auto obj = pool.acquire(i + 10);
        if (obj) {
            obj->use();
            objects.push_back(obj);
        }
        pool.printStats();
    }
    
    // Все объекты автоматически вернутся в пул при уничтожении shared_ptr
    std::cout << "Возврат объектов в пул при уничтожении..." << std::endl;
}

/**
 * @brief Демонстрация циклических ссылок и их решения
 */
void demonstrateCircularReferences() {
    std::cout << "\n=== Циклические ссылки ===" << std::endl;
    
    struct Node {
        std::string name_;
        std::shared_ptr<Node> parent_;
        std::vector<std::shared_ptr<Node>> children_;
        
        explicit Node(const std::string& name) : name_(name) {
            std::cout << "🏗️ Узел создан: " << name_ << std::endl;
        }
        
        ~Node() {
            std::cout << "🗑️ Узел уничтожен: " << name_ << std::endl;
        }
        
        void addChild(std::shared_ptr<Node> child) {
            child->parent_ = shared_from_this();
            children_.push_back(child);
        }
        
        std::shared_ptr<Node> getSharedPtr() {
            return std::static_pointer_cast<Node>(shared_from_this());
        }
    };
    
    // Проблема: циклические ссылки через shared_ptr
    std::cout << "\n--- Проблема с shared_ptr ---" << std::endl;
    {
        auto parent = std::make_shared<Node>("Родитель");
        auto child = std::make_shared<Node>("Ребенок");
        
        parent->children_.push_back(child);
        child->parent_ = parent;
        
        std::cout << "Счетчик ссылок parent: " << parent.use_count() << std::endl;
        std::cout << "Счетчик ссылок child: " << child.use_count() << std::endl;
    }
    std::cout << "❌ Объекты не уничтожены из-за циклических ссылок!" << std::endl;
    
    // Решение: использование weak_ptr
    std::cout << "\n--- Решение с weak_ptr ---" << std::endl;
    {
        struct SafeNode {
            std::string name_;
            std::weak_ptr<SafeNode> parent_;
            std::vector<std::shared_ptr<SafeNode>> children_;
            
            explicit SafeNode(const std::string& name) : name_(name) {
                std::cout << "🏗️ SafeNode создан: " << name_ << std::endl;
            }
            
            ~SafeNode() {
                std::cout << "🗑️ SafeNode уничтожен: " << name_ << std::endl;
            }
            
            void addChild(std::shared_ptr<SafeNode> child) {
                child->parent_ = std::static_pointer_cast<SafeNode>(shared_from_this());
                children_.push_back(child);
            }
            
            std::shared_ptr<SafeNode> getParent() {
                return parent_.lock();
            }
        };
        
        auto parent = std::make_shared<SafeNode>("Безопасный родитель");
        auto child = std::make_shared<SafeNode>("Безопасный ребенок");
        
        parent->children_.push_back(child);
        child->parent_ = parent;
        
        std::cout << "Счетчик ссылок parent: " << parent.use_count() << std::endl;
        std::cout << "Счетчик ссылок child: " << child.use_count() << std::endl;
        
        // Проверяем доступ к родителю через weak_ptr
        if (auto parentPtr = child->getParent()) {
            std::cout << "✅ Доступ к родителю через weak_ptr: " << parentPtr->name_ << std::endl;
        }
    }
    std::cout << "✅ Объекты корректно уничтожены!" << std::endl;
}

// ============================================================================
// ОСНОВНАЯ ФУНКЦИЯ
// ============================================================================

int main() {
    std::cout << "🧠 Продвинутое управление памятью" << std::endl;
    std::cout << std::string(50, '=') << std::endl;
    
    demonstrateResourceManager();
    demonstrateWeakPtrCache();
    demonstrateObjectPool();
    demonstrateCircularReferences();
    
    std::cout << "\n✅ Демонстрация продвинутого управления памятью завершена!" << std::endl;
    std::cout << "\n🎯 Ключевые выводы:" << std::endl;
    std::cout << "• Менеджеры ресурсов автоматически управляют жизненным циклом" << std::endl;
    std::cout << "• Кэши с weak_ptr предотвращают утечки памяти" << std::endl;
    std::cout << "• Пул объектов повышает производительность за счет переиспользования" << std::endl;
    std::cout << "• weak_ptr решает проблему циклических ссылок" << std::endl;
    std::cout << "• Умные указатели обеспечивают exception safety" << std::endl;
    
    return 0;
}
