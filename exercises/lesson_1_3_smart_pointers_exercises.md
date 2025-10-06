# 🏋️ Упражнения: Smart Pointers (Умные указатели)

## 📋 Задание 1: Переход от raw pointers к smart pointers

### Описание
Перепишите legacy код, заменив raw pointers на соответствующие smart pointers.

### Исходный код (проблемный)
```cpp
#include <iostream>
#include <vector>
#include <string>

class Node {
public:
    std::string data;
    Node* left;
    Node* right;
    
    Node(const std::string& d) : data(d), left(nullptr), right(nullptr) {}
    
    ~Node() {
        delete left;   // Потенциальная проблема: что если left == right?
        delete right;
    }
    
    void insert(const std::string& value) {
        if (value < data) {
            if (left == nullptr) {
                left = new Node(value);
            } else {
                left->insert(value);
            }
        } else {
            if (right == nullptr) {
                right = new Node(value);
            } else {
                right->insert(value);
            }
        }
    }
    
    void print() const {
        if (left) left->print();
        std::cout << data << " ";
        if (right) right->print();
    }
};

class BinaryTree {
private:
    Node* root;
    
public:
    BinaryTree() : root(nullptr) {}
    
    ~BinaryTree() {
        delete root; // Потенциальная утечка памяти
    }
    
    void insert(const std::string& value) {
        if (root == nullptr) {
            root = new Node(value);
        } else {
            root->insert(value);
        }
    }
    
    void print() const {
        if (root) {
            root->print();
            std::cout << std::endl;
        }
    }
};
```

### Требования к решению
1. ✅ Замените все raw pointers на `std::unique_ptr`
2. ✅ Убедитесь, что нет утечек памяти
3. ✅ Исправьте проблему двойного удаления
4. ✅ Добавьте move семантику где необходимо
5. ✅ Используйте `std::make_unique` для создания объектов

### Ожидаемый результат
```cpp
// Современная версия с smart pointers
class ModernNode {
    // Ваша реализация здесь
};

class ModernBinaryTree {
    // Ваша реализация здесь
};
```

---

## 📋 Задание 2: Система управления пользователями с shared_ptr

### Описание
Создайте систему управления пользователями, где пользователи могут иметь друзей (циклические ссылки).

### Требования
1. **User класс**: Представляет пользователя с именем и списком друзей
2. **UserManager класс**: Управляет всеми пользователями
3. **Циклические ссылки**: Используйте `std::weak_ptr` для предотвращения циклических ссылок
4. **Разделяемое владение**: Используйте `std::shared_ptr` для пользователей

### Интерфейс
```cpp
class User {
public:
    explicit User(const std::string& name);
    ~User();
    
    const std::string& getName() const;
    
    void addFriend(std::shared_ptr<User> friend_user);
    void removeFriend(const std::string& friend_name);
    
    void printFriends() const;
    size_t getFriendCount() const;
    
    // Получаем weak_ptr для безопасного доступа к друзьям
    std::vector<std::weak_ptr<User>> getFriends() const;
    
private:
    std::string name_;
    std::vector<std::weak_ptr<User>> friends_; // Используем weak_ptr!
};

class UserManager {
public:
    UserManager();
    ~UserManager();
    
    std::shared_ptr<User> createUser(const std::string& name);
    std::shared_ptr<User> findUser(const std::string& name) const;
    
    void makeFriends(const std::string& name1, const std::string& name2);
    void removeUser(const std::string& name);
    
    void printAllUsers() const;
    size_t getUserCount() const;
    
private:
    std::vector<std::shared_ptr<User>> users_;
};
```

### Тестовый случай
```cpp
int main() {
    UserManager manager;
    
    // Создаем пользователей
    auto alice = manager.createUser("Алиса");
    auto bob = manager.createUser("Боб");
    auto charlie = manager.createUser("Чарли");
    
    std::cout << "Создано пользователей: " << manager.getUserCount() << std::endl;
    
    // Добавляем друзей
    manager.makeFriends("Алиса", "Боб");
    manager.makeFriends("Боб", "Чарли");
    manager.makeFriends("Алиса", "Чарли");
    
    // Проверяем друзей
    alice->printFriends();
    bob->printFriends();
    charlie->printFriends();
    
    // Демонстрируем безопасный доступ через weak_ptr
    auto friends = alice->getFriends();
    for (const auto& weak_friend : friends) {
        if (auto friend_ptr = weak_friend.lock()) {
            std::cout << "Друг Алисы: " << friend_ptr->getName() << std::endl;
        } else {
            std::cout << "Друг был удален" << std::endl;
        }
    }
    
    // Удаляем пользователя (друзья должны остаться)
    manager.removeUser("Боб");
    std::cout << "После удаления Боба: " << manager.getUserCount() << std::endl;
    
    // Проверяем, что циклические ссылки не мешают удалению
    alice->printFriends();
    
    return 0;
}
```

---

## 📋 Задание 3: Кэш с автоматическим управлением памятью

### Описание
Создайте систему кэширования с автоматическим удалением неиспользуемых элементов.

### Требования
1. **Cache класс**: Хранит пары ключ-значение с ограниченным размером
2. **LRU политика**: Удаляет наименее недавно использованные элементы
3. **Автоматическое управление**: Использует smart pointers для управления памятью
4. **Thread safety**: Безопасен для многопоточного использования

### Интерфейс
```cpp
template<typename Key, typename Value>
class LRUCache {
public:
    explicit LRUCache(size_t maxSize);
    ~LRUCache();
    
    // Основные операции
    void put(const Key& key, std::shared_ptr<Value> value);
    std::shared_ptr<Value> get(const Key& key);
    
    // Управление кэшем
    void remove(const Key& key);
    void clear();
    
    // Информация о кэше
    size_t size() const;
    size_t maxSize() const;
    bool contains(const Key& key) const;
    
    // Статистика
    size_t getHitCount() const;
    size_t getMissCount() const;
    double getHitRate() const;
    
private:
    struct CacheNode {
        Key key;
        std::shared_ptr<Value> value;
        std::shared_ptr<CacheNode> prev;
        std::shared_ptr<CacheNode> next;
        
        CacheNode(const Key& k, std::shared_ptr<Value> v) 
            : key(k), value(v), prev(nullptr), next(nullptr) {}
    };
    
    size_t maxSize_;
    size_t hitCount_;
    size_t missCount_;
    
    std::shared_ptr<CacheNode> head_;
    std::shared_ptr<CacheNode> tail_;
    std::unordered_map<Key, std::weak_ptr<CacheNode>> cacheMap_;
    mutable std::mutex mutex_;
    
    void moveToHead(std::shared_ptr<CacheNode> node);
    void removeNode(std::shared_ptr<CacheNode> node);
    void addToHead(std::shared_ptr<CacheNode> node);
};
```

### Тестовый случай
```cpp
int main() {
    LRUCache<std::string, std::string> cache(3);
    
    // Добавляем элементы
    cache.put("key1", std::make_shared<std::string>("value1"));
    cache.put("key2", std::make_shared<std::string>("value2"));
    cache.put("key3", std::make_shared<std::string>("value3"));
    
    std::cout << "Размер кэша: " << cache.size() << std::endl;
    
    // Получаем элемент (должен стать MRU)
    auto value = cache.get("key1");
    if (value) {
        std::cout << "Получено: " << *value << std::endl;
    }
    
    // Добавляем новый элемент (key2 должен быть удален как LRU)
    cache.put("key4", std::make_shared<std::string>("value4"));
    
    std::cout << "Размер кэша после добавления key4: " << cache.size() << std::endl;
    
    // Проверяем, что key2 удален
    auto value2 = cache.get("key2");
    if (!value2) {
        std::cout << "key2 был удален из кэша (LRU)" << std::endl;
    }
    
    // Проверяем статистику
    std::cout << "Hit rate: " << cache.getHitRate() << std::endl;
    std::cout << "Hits: " << cache.getHitCount() << ", Misses: " << cache.getMissCount() << std::endl;
    
    return 0;
}
```

---

## 📋 Задание 4: Система плагинов с weak_ptr

### Описание
Создайте систему плагинов, где плагины могут ссылаться друг на друга без создания циклических зависимостей.

### Требования
1. **Plugin класс**: Базовый класс для всех плагинов
2. **PluginManager класс**: Управляет загрузкой и выгрузкой плагинов
3. **Зависимости**: Плагины могут зависеть от других плагинов
4. **Безопасная выгрузка**: Используйте weak_ptr для предотвращения циклических ссылок

### Интерфейс
```cpp
class Plugin {
public:
    explicit Plugin(const std::string& name);
    virtual ~Plugin();
    
    const std::string& getName() const;
    virtual void initialize() = 0;
    virtual void shutdown() = 0;
    virtual void execute() = 0;
    
    // Добавляем зависимость (используем weak_ptr)
    void addDependency(std::weak_ptr<Plugin> dependency);
    std::vector<std::weak_ptr<Plugin>> getDependencies() const;
    
    // Проверяем, что все зависимости доступны
    bool areDependenciesAvailable() const;
    
protected:
    std::string name_;
    std::vector<std::weak_ptr<Plugin>> dependencies_;
};

class PluginManager {
public:
    PluginManager();
    ~PluginManager();
    
    template<typename T, typename... Args>
    std::shared_ptr<T> loadPlugin(const std::string& name, Args&&... args);
    
    void unloadPlugin(const std::string& name);
    void unloadAll();
    
    std::shared_ptr<Plugin> getPlugin(const std::string& name) const;
    std::vector<std::string> getLoadedPlugins() const;
    
    void executeAll();
    
private:
    std::unordered_map<std::string, std::shared_ptr<Plugin>> plugins_;
    std::mutex mutex_;
};
```

### Примеры плагинов
```cpp
class DatabasePlugin : public Plugin {
public:
    DatabasePlugin() : Plugin("Database") {}
    
    void initialize() override {
        std::cout << "Инициализация базы данных..." << std::endl;
    }
    
    void shutdown() override {
        std::cout << "Закрытие соединения с базой данных..." << std::endl;
    }
    
    void execute() override {
        std::cout << "Выполнение операций с базой данных..." << std::endl;
    }
};

class LoggerPlugin : public Plugin {
public:
    LoggerPlugin() : Plugin("Logger") {}
    
    void initialize() override {
        std::cout << "Инициализация логгера..." << std::endl;
    }
    
    void shutdown() override {
        std::cout << "Закрытие логгера..." << std::endl;
    }
    
    void execute() override {
        std::cout << "Запись в лог..." << std::endl;
    }
};

class WebServerPlugin : public Plugin {
public:
    WebServerPlugin() : Plugin("WebServer") {}
    
    void initialize() override {
        std::cout << "Инициализация веб-сервера..." << std::endl;
    }
    
    void shutdown() override {
        std::cout << "Остановка веб-сервера..." << std::endl;
    }
    
    void execute() override {
        std::cout << "Обработка HTTP запросов..." << std::endl;
    }
};
```

### Тестовый случай
```cpp
int main() {
    PluginManager manager;
    
    // Загружаем плагины
    auto dbPlugin = manager.loadPlugin<DatabasePlugin>();
    auto loggerPlugin = manager.loadPlugin<LoggerPlugin>();
    auto webPlugin = manager.loadPlugin<WebServerPlugin>();
    
    // Настраиваем зависимости
    webPlugin->addDependency(dbPlugin);
    webPlugin->addDependency(loggerPlugin);
    
    std::cout << "Загружено плагинов: " << manager.getLoadedPlugins().size() << std::endl;
    
    // Проверяем зависимости
    if (webPlugin->areDependenciesAvailable()) {
        std::cout << "Все зависимости WebServer доступны" << std::endl;
    }
    
    // Выполняем все плагины
    manager.executeAll();
    
    // Выгружаем плагины (порядок важен!)
    manager.unloadPlugin("WebServer");
    manager.unloadPlugin("Logger");
    manager.unloadPlugin("Database");
    
    std::cout << "Все плагины выгружены" << std::endl;
    
    return 0;
}
```

---

## 📋 Задание 5: Комплексная система управления ресурсами

### Описание
Создайте систему управления ресурсами для игры, демонстрирующую все типы smart pointers.

### Требования
1. **Resource класс**: Базовый класс для всех ресурсов
2. **Texture, Sound, Model классы**: Конкретные типы ресурсов
3. **ResourceManager класс**: Управляет всеми ресурсами
4. **Scene класс**: Использует ресурсы для отображения сцены

### Архитектура
```cpp
class Resource {
public:
    explicit Resource(const std::string& path);
    virtual ~Resource();
    
    const std::string& getPath() const;
    bool isLoaded() const;
    virtual void load() = 0;
    virtual void unload() = 0;
    
protected:
    std::string path_;
    bool loaded_;
};

class Texture : public Resource {
public:
    explicit Texture(const std::string& path);
    ~Texture();
    
    void load() override;
    void unload() override;
    
    // Симуляция работы с текстурой
    void bind() const;
    void unbind() const;
};

class Sound : public Resource {
public:
    explicit Sound(const std::string& path);
    ~Sound();
    
    void load() override;
    void unload() override;
    
    void play() const;
    void stop() const;
};

class Model : public Resource {
public:
    explicit Model(const std::string& path);
    ~Model();
    
    void load() override;
    void unload() override;
    
    void render() const;
};

class ResourceManager {
public:
    ResourceManager();
    ~ResourceManager();
    
    // Загрузка ресурсов (возвращаем shared_ptr для разделяемого владения)
    std::shared_ptr<Texture> loadTexture(const std::string& path);
    std::shared_ptr<Sound> loadSound(const std::string& path);
    std::shared_ptr<Model> loadModel(const std::string& path);
    
    // Получение ресурсов
    std::shared_ptr<Texture> getTexture(const std::string& path) const;
    std::shared_ptr<Sound> getSound(const std::string& path) const;
    std::shared_ptr<Model> getModel(const std::string& path) const;
    
    // Управление памятью
    void unloadUnusedResources();
    void unloadAllResources();
    
    // Статистика
    size_t getLoadedResourceCount() const;
    size_t getTotalMemoryUsage() const;
    
private:
    std::unordered_map<std::string, std::shared_ptr<Texture>> textures_;
    std::unordered_map<std::string, std::shared_ptr<Sound>> sounds_;
    std::unordered_map<std::string, std::shared_ptr<Model>> models_;
    mutable std::mutex mutex_;
};

class Scene {
public:
    explicit Scene(std::shared_ptr<ResourceManager> resourceManager);
    ~Scene();
    
    void addTexture(const std::string& path);
    void addSound(const std::string& path);
    void addModel(const std::string& path);
    
    void render();
    void playBackgroundMusic();
    
private:
    std::shared_ptr<ResourceManager> resourceManager_;
    std::vector<std::weak_ptr<Texture>> textures_; // weak_ptr для избежания циклических ссылок
    std::vector<std::weak_ptr<Sound>> sounds_;
    std::vector<std::weak_ptr<Model>> models_;
};
```

### Тестовый случай
```cpp
int main() {
    // Создаем менеджер ресурсов
    auto resourceManager = std::make_shared<ResourceManager>();
    
    // Создаем сцену
    Scene scene(resourceManager);
    
    // Добавляем ресурсы в сцену
    scene.addTexture("textures/grass.png");
    scene.addTexture("textures/stone.png");
    scene.addSound("sounds/background.mp3");
    scene.addModel("models/tree.obj");
    
    std::cout << "Загружено ресурсов: " << resourceManager->getLoadedResourceCount() << std::endl;
    
    // Рендерим сцену
    scene.render();
    scene.playBackgroundMusic();
    
    // Симулируем изменение сцены
    scene.addTexture("textures/water.png");
    scene.addModel("models/house.obj");
    
    std::cout << "После добавления новых ресурсов: " << resourceManager->getLoadedResourceCount() << std::endl;
    
    // Очищаем неиспользуемые ресурсы
    resourceManager->unloadUnusedResources();
    
    std::cout << "После очистки неиспользуемых ресурсов: " << resourceManager->getLoadedResourceCount() << std::endl;
    
    return 0;
}
```

---

## 🎯 Критерии оценки

### Отлично (5 баллов)
- ✅ Правильно используются все типы smart pointers
- ✅ Нет утечек памяти и циклических ссылок
- ✅ Move семантика реализована корректно
- ✅ Thread safety обеспечена где необходимо
- ✅ Код следует современным стандартам C++

### Хорошо (4 балла)
- ✅ Smart pointers используются правильно в большинстве случаев
- ✅ Основные проблемы с памятью решены
- ✅ Есть небольшие недочеты в использовании weak_ptr
- ✅ Код работает стабильно

### Удовлетворительно (3 балла)
- ✅ Базовое использование smart pointers
- ✅ Код компилируется и работает
- ❌ Есть проблемы с управлением памятью
- ❌ Не все типы smart pointers используются правильно

### Неудовлетворительно (2 балла)
- ❌ Smart pointers не используются или используются неправильно
- ❌ Есть утечки памяти
- ❌ Код не компилируется или работает неправильно

---

## 💡 Подсказки

1. **unique_ptr**: Используйте для единоличного владения ресурсом
2. **shared_ptr**: Используйте для разделяемого владения
3. **weak_ptr**: Используйте для слабых ссылок и предотвращения циклических зависимостей
4. **make_unique/make_shared**: Предпочитайте эти функции вместо new
5. **RAII**: Smart pointers автоматически управляют памятью

---

*Smart pointers - это современный и безопасный способ управления памятью в C++. Освойте их использование, и ваши программы станут намного надежнее!*
