/**
 * @file kiss_example.cpp
 * @brief Детальная демонстрация принципа KISS (Keep It Simple Stupid)
 * @author Senior C++ Developer
 * @date 2024
 * 
 * Этот файл демонстрирует различные аспекты принципа KISS:
 * - Выявление избыточной сложности
 * - Методы упрощения кода
 * - Компромиссы при применении KISS
 */

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <memory>
#include <functional>

/**
 * @brief Математическая формализация KISS
 * 
 * KISS можно формализовать следующим образом:
 * Complexity(Code) = Σ(Complexity(Component))
 * где Complexity(Component) → min при сохранении функциональности
 * 
 * Другими словами: простота должна быть ключевой целью,
 * и ненужная сложность должна быть устранена.
 */

// ============================================================================
// ПРИМЕР 1: ИЗБЫТОЧНАЯ СЛОЖНОСТЬ В СИСТЕМЕ КОНФИГУРАЦИИ
// ============================================================================

/**
 * @brief Нарушение KISS: Сложная иерархия для простой задачи
 * 
 * Этот код нарушает KISS, создавая избыточную сложность для простой
 * задачи хранения конфигурации.
 */
class BadConfigNode {
public:
    enum class NodeType {
        STRING, INTEGER, BOOLEAN, ARRAY, OBJECT, NULL_VALUE
    };
    
private:
    NodeType type;
    std::string stringValue;
    int intValue;
    bool boolValue;
    std::vector<std::unique_ptr<BadConfigNode>> arrayValue;
    std::map<std::string, std::unique_ptr<BadConfigNode>> objectValue;
    
public:
    BadConfigNode(NodeType nodeType) : type(nodeType) {}
    
    void setString(const std::string& value) {
        if (type == NodeType::STRING) {
            stringValue = value;
        }
    }
    
    void setInt(int value) {
        if (type == NodeType::INTEGER) {
            intValue = value;
        }
    }
    
    void setBool(bool value) {
        if (type == NodeType::BOOLEAN) {
            boolValue = value;
        }
    }
    
    void addArrayElement(std::unique_ptr<BadConfigNode> element) {
        if (type == NodeType::ARRAY) {
            arrayValue.push_back(std::move(element));
        }
    }
    
    void setObjectProperty(const std::string& key, std::unique_ptr<BadConfigNode> value) {
        if (type == NodeType::OBJECT) {
            objectValue[key] = std::move(value);
        }
    }
    
    std::string getString() const {
        if (type == NodeType::STRING) {
            return stringValue;
        }
        return "";
    }
    
    int getInt() const {
        if (type == NodeType::INTEGER) {
            return intValue;
        }
        return 0;
    }
    
    bool getBool() const {
        if (type == NodeType::BOOLEAN) {
            return boolValue;
        }
        return false;
    }
    
    size_t getArraySize() const {
        if (type == NodeType::ARRAY) {
            return arrayValue.size();
        }
        return 0;
    }
    
    BadConfigNode* getArrayElement(size_t index) const {
        if (type == NodeType::ARRAY && index < arrayValue.size()) {
            return arrayValue[index].get();
        }
        return nullptr;
    }
    
    BadConfigNode* getObjectProperty(const std::string& key) const {
        if (type == NodeType::OBJECT) {
            auto it = objectValue.find(key);
            if (it != objectValue.end()) {
                return it->second.get();
            }
        }
        return nullptr;
    }
    
    NodeType getType() const { return type; }
};

class BadConfigManager {
private:
    std::unique_ptr<BadConfigNode> root;
    
public:
    BadConfigManager() {
        root = std::make_unique<BadConfigNode>(BadConfigNode::NodeType::OBJECT);
    }
    
    void setString(const std::string& path, const std::string& value) {
        setValue(path, [value](BadConfigNode* node) {
            node->setString(value);
        });
    }
    
    void setInt(const std::string& path, int value) {
        setValue(path, [value](BadConfigNode* node) {
            node->setInt(value);
        });
    }
    
    void setBool(const std::string& path, bool value) {
        setValue(path, [value](BadConfigNode* node) {
            node->setBool(value);
        });
    }
    
    std::string getString(const std::string& path) const {
        BadConfigNode* node = getNode(path);
        return node ? node->getString() : "";
    }
    
    int getInt(const std::string& path) const {
        BadConfigNode* node = getNode(path);
        return node ? node->getInt() : 0;
    }
    
    bool getBool(const std::string& path) const {
        BadConfigNode* node = getNode(path);
        return node ? node->getBool() : false;
    }
    
private:
    void setValue(const std::string& path, std::function<void(BadConfigNode*)> setter) {
        std::vector<std::string> parts = splitPath(path);
        BadConfigNode* current = root.get();
        
        for (size_t i = 0; i < parts.size() - 1; ++i) {
            BadConfigNode* next = current->getObjectProperty(parts[i]);
            if (!next) {
                auto newNode = std::make_unique<BadConfigNode>(BadConfigNode::NodeType::OBJECT);
                next = newNode.get();
                current->setObjectProperty(parts[i], std::move(newNode));
            }
            current = next;
        }
        
        setter(current);
    }
    
    BadConfigNode* getNode(const std::string& path) const {
        std::vector<std::string> parts = splitPath(path);
        BadConfigNode* current = root.get();
        
        for (const auto& part : parts) {
            current = current->getObjectProperty(part);
            if (!current) {
                return nullptr;
            }
        }
        
        return current;
    }
    
    std::vector<std::string> splitPath(const std::string& path) const {
        std::vector<std::string> parts;
        std::string current;
        
        for (char c : path) {
            if (c == '.') {
                if (!current.empty()) {
                    parts.push_back(current);
                    current.clear();
                }
            } else {
                current += c;
            }
        }
        
        if (!current.empty()) {
            parts.push_back(current);
        }
        
        return parts;
    }
};

/**
 * @brief Соблюдение KISS: Простая система конфигурации
 * 
 * Этот код соблюдает KISS, используя простые структуры данных
 * для хранения конфигурации.
 */
class GoodConfig {
private:
    std::map<std::string, std::string> config;
    
public:
    void set(const std::string& key, const std::string& value) {
        config[key] = value;
    }
    
    void set(const std::string& key, int value) {
        config[key] = std::to_string(value);
    }
    
    void set(const std::string& key, bool value) {
        config[key] = value ? "true" : "false";
    }
    
    std::string getString(const std::string& key, const std::string& defaultValue = "") const {
        auto it = config.find(key);
        return it != config.end() ? it->second : defaultValue;
    }
    
    int getInt(const std::string& key, int defaultValue = 0) const {
        auto it = config.find(key);
        if (it != config.end()) {
            try {
                return std::stoi(it->second);
            } catch (...) {
                return defaultValue;
            }
        }
        return defaultValue;
    }
    
    bool getBool(const std::string& key, bool defaultValue = false) const {
        auto it = config.find(key);
        if (it != config.end()) {
            return it->second == "true";
        }
        return defaultValue;
    }
    
    bool hasKey(const std::string& key) const {
        return config.find(key) != config.end();
    }
    
    void remove(const std::string& key) {
        config.erase(key);
    }
    
    void printAll() const {
        std::cout << "\n📋 Конфигурация:\n";
        for (const auto& pair : config) {
            std::cout << "  " << pair.first << " = " << pair.second << "\n";
        }
    }
    
    size_t size() const {
        return config.size();
    }
};

// ============================================================================
// ПРИМЕР 2: ИЗБЫТОЧНАЯ СЛОЖНОСТЬ В СИСТЕМЕ УВЕДОМЛЕНИЙ
// ============================================================================

/**
 * @brief Нарушение KISS: Сложная система уведомлений
 */
class BadNotificationEvent {
public:
    enum class Priority {
        LOW, MEDIUM, HIGH, CRITICAL
    };
    
    enum class Type {
        EMAIL, SMS, PUSH, IN_APP, WEBHOOK
    };
    
private:
    std::string id;
    std::string title;
    std::string message;
    Priority priority;
    Type type;
    std::map<std::string, std::string> metadata;
    std::chrono::system_clock::time_point timestamp;
    std::chrono::system_clock::time_point expiresAt;
    int retryCount;
    int maxRetries;
    
public:
    BadNotificationEvent(const std::string& eventId, const std::string& eventTitle,
                        const std::string& eventMessage, Priority eventPriority,
                        Type eventType, int maxRetryAttempts = 3)
        : id(eventId), title(eventTitle), message(eventMessage),
          priority(eventPriority), type(eventType), retryCount(0),
          maxRetries(maxRetryAttempts) {
        timestamp = std::chrono::system_clock::now();
        expiresAt = timestamp + std::chrono::hours(24);
    }
    
    void setMetadata(const std::string& key, const std::string& value) {
        metadata[key] = value;
    }
    
    std::string getMetadata(const std::string& key) const {
        auto it = metadata.find(key);
        return it != metadata.end() ? it->second : "";
    }
    
    bool isExpired() const {
        return std::chrono::system_clock::now() > expiresAt;
    }
    
    bool canRetry() const {
        return retryCount < maxRetries;
    }
    
    void incrementRetry() {
        retryCount++;
    }
    
    // Множество геттеров...
    std::string getId() const { return id; }
    std::string getTitle() const { return title; }
    std::string getMessage() const { return message; }
    Priority getPriority() const { return priority; }
    Type getType() const { return type; }
    int getRetryCount() const { return retryCount; }
    int getMaxRetries() const { return maxRetries; }
};

class BadNotificationProcessor {
private:
    std::vector<std::unique_ptr<BadNotificationEvent>> queue;
    
public:
    void addNotification(std::unique_ptr<BadNotificationEvent> notification) {
        queue.push_back(std::move(notification));
    }
    
    void processNotifications() {
        for (auto it = queue.begin(); it != queue.end();) {
            auto& notification = *it;
            
            if (notification->isExpired()) {
                std::cout << "⏰ Уведомление " << notification->getId() << " истекло\n";
                it = queue.erase(it);
                continue;
            }
            
            if (processNotification(*notification)) {
                std::cout << "✅ Уведомление " << notification->getId() << " отправлено\n";
                it = queue.erase(it);
            } else {
                if (notification->canRetry()) {
                    notification->incrementRetry();
                    std::cout << "🔄 Повторная попытка для " << notification->getId() 
                              << " (попытка " << notification->getRetryCount() << ")\n";
                    ++it;
                } else {
                    std::cout << "❌ Уведомление " << notification->getId() << " не удалось отправить\n";
                    it = queue.erase(it);
                }
            }
        }
    }
    
private:
    bool processNotification(const BadNotificationEvent& notification) {
        // Сложная логика обработки в зависимости от типа
        switch (notification.getType()) {
            case BadNotificationEvent::Type::EMAIL:
                return processEmailNotification(notification);
            case BadNotificationEvent::Type::SMS:
                return processSMSNotification(notification);
            case BadNotificationEvent::Type::PUSH:
                return processPushNotification(notification);
            case BadNotificationEvent::Type::IN_APP:
                return processInAppNotification(notification);
            case BadNotificationEvent::Type::WEBHOOK:
                return processWebhookNotification(notification);
            default:
                return false;
        }
    }
    
    bool processEmailNotification(const BadNotificationEvent& notification) {
        std::cout << "📧 Отправка email: " << notification.getTitle() << "\n";
        return true; // Симуляция
    }
    
    bool processSMSNotification(const BadNotificationEvent& notification) {
        std::cout << "📱 Отправка SMS: " << notification.getTitle() << "\n";
        return true; // Симуляция
    }
    
    bool processPushNotification(const BadNotificationEvent& notification) {
        std::cout << "🔔 Отправка push: " << notification.getTitle() << "\n";
        return true; // Симуляция
    }
    
    bool processInAppNotification(const BadNotificationEvent& notification) {
        std::cout << "📱 Отправка in-app: " << notification.getTitle() << "\n";
        return true; // Симуляция
    }
    
    bool processWebhookNotification(const BadNotificationEvent& notification) {
        std::cout << "🔗 Отправка webhook: " << notification.getTitle() << "\n";
        return true; // Симуляция
    }
};

/**
 * @brief Соблюдение KISS: Простая система уведомлений
 */
class SimpleNotification {
private:
    std::string message;
    std::string recipient;
    
public:
    SimpleNotification(const std::string& msg, const std::string& rec) 
        : message(msg), recipient(rec) {}
    
    void send() {
        std::cout << "📤 Отправка уведомления для " << recipient << ": " << message << "\n";
        // Простая логика отправки
    }
    
    const std::string& getMessage() const { return message; }
    const std::string& getRecipient() const { return recipient; }
};

class SimpleNotificationService {
private:
    std::vector<SimpleNotification> notifications;
    
public:
    void addNotification(const std::string& message, const std::string& recipient) {
        notifications.emplace_back(message, recipient);
    }
    
    void sendAll() {
        for (const auto& notification : notifications) {
            notification.send();
        }
        notifications.clear();
    }
    
    size_t pendingCount() const {
        return notifications.size();
    }
};

// ============================================================================
// ПРИМЕР 3: ИЗБЫТОЧНАЯ СЛОЖНОСТЬ В СИСТЕМЕ КЭШИРОВАНИЯ
// ============================================================================

/**
 * @brief Нарушение KISS: Сложная система кэширования
 */
template<typename K, typename V>
class BadCacheNode {
public:
    enum class State {
        VALID, EXPIRED, INVALIDATED, LOCKED
    };
    
private:
    K key;
    V value;
    State state;
    std::chrono::system_clock::time_point createdAt;
    std::chrono::system_clock::time_point lastAccessed;
    std::chrono::seconds ttl;
    int accessCount;
    double priority;
    
public:
    BadCacheNode(const K& nodeKey, const V& nodeValue, std::chrono::seconds timeToLive)
        : key(nodeKey), value(nodeValue), state(State::VALID),
          createdAt(std::chrono::system_clock::now()),
          lastAccessed(std::chrono::system_clock::now()),
          ttl(timeToLive), accessCount(0), priority(1.0) {}
    
    bool isExpired() const {
        auto now = std::chrono::system_clock::now();
        return now - createdAt > ttl;
    }
    
    void updateAccess() {
        lastAccessed = std::chrono::system_clock::now();
        accessCount++;
    }
    
    void setPriority(double newPriority) {
        priority = newPriority;
    }
    
    // Множество геттеров...
    const K& getKey() const { return key; }
    const V& getValue() const { return value; }
    State getState() const { return state; }
    int getAccessCount() const { return accessCount; }
    double getPriority() const { return priority; }
    
    void setState(State newState) { state = newState; }
};

template<typename K, typename V>
class BadCache {
private:
    std::map<K, std::unique_ptr<BadCacheNode<K, V>>> cache;
    size_t maxSize;
    std::chrono::seconds defaultTtl;
    
public:
    BadCache(size_t maximumSize = 1000, std::chrono::seconds defaultTimeToLive = std::chrono::hours(1))
        : maxSize(maximumSize), defaultTtl(defaultTimeToLive) {}
    
    void set(const K& key, const V& value, std::chrono::seconds ttl = std::chrono::seconds(0)) {
        if (ttl.count() == 0) {
            ttl = defaultTtl;
        }
        
        auto node = std::make_unique<BadCacheNode<K, V>>(key, value, ttl);
        cache[key] = std::move(node);
        
        if (cache.size() > maxSize) {
            evictLeastUsed();
        }
    }
    
    V get(const K& key, const V& defaultValue = V{}) {
        auto it = cache.find(key);
        if (it != cache.end()) {
            auto& node = it->second;
            if (!node->isExpired()) {
                node->updateAccess();
                return node->getValue();
            } else {
                cache.erase(it);
            }
        }
        return defaultValue;
    }
    
    bool exists(const K& key) const {
        auto it = cache.find(key);
        return it != cache.end() && !it->second->isExpired();
    }
    
private:
    void evictLeastUsed() {
        // Сложная логика вытеснения
        auto it = std::min_element(cache.begin(), cache.end(),
            [](const auto& a, const auto& b) {
                return a.second->getAccessCount() < b.second->getAccessCount();
            });
        
        if (it != cache.end()) {
            cache.erase(it);
        }
    }
};

/**
 * @brief Соблюдение KISS: Простая система кэширования
 */
template<typename K, typename V>
class SimpleCache {
private:
    std::map<K, V> cache;
    size_t maxSize;
    
public:
    SimpleCache(size_t maximumSize = 100) : maxSize(maximumSize) {}
    
    void set(const K& key, const V& value) {
        cache[key] = value;
        
        if (cache.size() > maxSize) {
            // Простое вытеснение - удаляем первый элемент
            cache.erase(cache.begin());
        }
    }
    
    V get(const K& key, const V& defaultValue = V{}) {
        auto it = cache.find(key);
        return it != cache.end() ? it->second : defaultValue;
    }
    
    bool exists(const K& key) const {
        return cache.find(key) != cache.end();
    }
    
    void remove(const K& key) {
        cache.erase(key);
    }
    
    void clear() {
        cache.clear();
    }
    
    size_t size() const {
        return cache.size();
    }
};

// ============================================================================
// ДЕМОНСТРАЦИЯ ПРИНЦИПА
// ============================================================================

void demonstrateBadKISS() {
    std::cout << "❌ НАРУШЕНИЕ KISS - Избыточная сложность:\n";
    std::cout << std::string(50, '-') << "\n";
    
    std::cout << "Сложная система конфигурации:\n";
    BadConfigManager badConfig;
    badConfig.setString("database.host", "localhost");
    badConfig.setInt("database.port", 5432);
    badConfig.setBool("debug.enabled", true);
    
    std::cout << "DB Host: " << badConfig.getString("database.host") << "\n";
    std::cout << "DB Port: " << badConfig.getInt("database.port") << "\n";
    std::cout << "Debug: " << badConfig.getBool("debug.enabled") << "\n";
    
    std::cout << "\nСложная система уведомлений:\n";
    BadNotificationProcessor badProcessor;
    badProcessor.addNotification(std::make_unique<BadNotificationEvent>(
        "msg1", "Test Title", "Test Message", 
        BadNotificationEvent::Priority::HIGH,
        BadNotificationEvent::Type::EMAIL, 3
    ));
    badProcessor.processNotifications();
    
    std::cout << "\nСложная система кэширования:\n";
    BadCache<std::string, std::string> badCache(5);
    badCache.set("key1", "value1", std::chrono::seconds(3600));
    badCache.set("key2", "value2", std::chrono::seconds(3600));
    std::cout << "Key1: " << badCache.get("key1") << "\n";
    std::cout << "Key2: " << badCache.get("key2") << "\n";
}

void demonstrateGoodKISS() {
    std::cout << "\n✅ СОБЛЮДЕНИЕ KISS - Простота:\n";
    std::cout << std::string(50, '-') << "\n";
    
    std::cout << "Простая система конфигурации:\n";
    GoodConfig goodConfig;
    goodConfig.set("database.host", "localhost");
    goodConfig.set("database.port", 5432);
    goodConfig.set("debug.enabled", true);
    
    std::cout << "DB Host: " << goodConfig.getString("database.host") << "\n";
    std::cout << "DB Port: " << goodConfig.getInt("database.port") << "\n";
    std::cout << "Debug: " << goodConfig.getBool("debug.enabled") << "\n";
    goodConfig.printAll();
    
    std::cout << "\nПростая система уведомлений:\n";
    SimpleNotificationService simpleService;
    simpleService.addNotification("Привет!", "user@example.com");
    simpleService.addNotification("Как дела?", "admin@example.com");
    std::cout << "Ожидающих уведомлений: " << simpleService.pendingCount() << "\n";
    simpleService.sendAll();
    
    std::cout << "\nПростая система кэширования:\n";
    SimpleCache<std::string, std::string> simpleCache(5);
    simpleCache.set("key1", "value1");
    simpleCache.set("key2", "value2");
    simpleCache.set("key3", "value3");
    std::cout << "Key1: " << simpleCache.get("key1") << "\n";
    std::cout << "Key2: " << simpleCache.get("key2") << "\n";
    std::cout << "Размер кэша: " << simpleCache.size() << "\n";
}

void analyzeTradeOffs() {
    std::cout << "\n🔬 АНАЛИЗ КОМПРОМИССОВ KISS:\n";
    std::cout << std::string(50, '-') << "\n";
    
    std::cout << "📊 ПРЕИМУЩЕСТВА:\n";
    std::cout << "• Легкость понимания и отладки\n";
    std::cout << "• Быстрая разработка\n";
    std::cout << "• Меньше багов\n";
    std::cout << "• Простота тестирования\n";
    std::cout << "• Легкость сопровождения\n\n";
    
    std::cout << "⚠️ НЕДОСТАТКИ:\n";
    std::cout << "• Возможные ограничения функциональности\n";
    std::cout << "• Необходимость рефакторинга при росте требований\n";
    std::cout << "• Потенциальная неэффективность\n";
    std::cout << "• Меньше возможностей для оптимизации\n\n";
    
    std::cout << "🎯 РЕКОМЕНДАЦИИ:\n";
    std::cout << "• Начинайте с простого решения\n";
    std::cout << "• Усложняйте только при необходимости\n";
    std::cout << "• Избегайте преждевременной оптимизации\n";
    std::cout << "• Применяйте принцип YAGNI\n";
}

int main() {
    std::cout << "🎯 ДЕТАЛЬНАЯ ДЕМОНСТРАЦИЯ ПРИНЦИПА KISS (Keep It Simple Stupid)\n";
    std::cout << "Автор: Senior C++ Developer\n";
    std::cout << "Цель: Углубленное изучение принципа простоты\n\n";
    
    demonstrateBadKISS();
    demonstrateGoodKISS();
    analyzeTradeOffs();
    
    std::cout << "\n📚 МАТЕМАТИЧЕСКОЕ ОБОСНОВАНИЕ:\n";
    std::cout << "KISS: Complexity(Code) = Σ(Complexity(Component))\n";
    std::cout << "где Complexity(Component) → min при сохранении функциональности\n\n";
    
    std::cout << "💡 КЛЮЧЕВЫЕ ВЫВОДЫ:\n";
    std::cout << "1. Простота должна быть ключевой целью\n";
    std::cout << "2. Ненужная сложность должна быть устранена\n";
    std::cout << "3. KISS улучшает читаемость и maintainability\n";
    std::cout << "4. Применение требует баланса с другими принципами\n\n";
    
    std::cout << "🔬 Принципы - это инструменты для мышления о проблемах!\n";
    
    return 0;
}
