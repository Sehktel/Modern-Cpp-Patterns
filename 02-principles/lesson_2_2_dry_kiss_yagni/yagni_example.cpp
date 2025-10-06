/**
 * @file yagni_example.cpp
 * @brief Детальная демонстрация принципа YAGNI (You Aren't Gonna Need It)
 * @author Senior C++ Developer
 * @date 2024
 * 
 * Этот файл демонстрирует различные аспекты принципа YAGNI:
 * - Выявление преждевременной оптимизации
 * - Методы отложения ненужной функциональности
 * - Компромиссы при применении YAGNI
 */

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <memory>
#include <functional>
#include <chrono>
#include <thread>

/**
 * @brief Математическая формализация YAGNI
 * 
 * YAGNI можно формализовать следующим образом:
 * ∀F ∈ Features: F ∈ CurrentRequirements ∨ F ∈ ImmediateNeeds
 * где CurrentRequirements = {f | f требуется сейчас}
 *     ImmediateNeeds = {f | f потребуется в ближайшем будущем}
 * 
 * Другими словами: не добавляйте функциональность,
 * пока она не понадобится.
 */

// ============================================================================
// ПРИМЕР 1: ПРЕЖДЕВРЕМЕННАЯ ОПТИМИЗАЦИЯ В СИСТЕМЕ УВЕДОМЛЕНИЙ
// ============================================================================

/**
 * @brief Нарушение YAGNI: Сложная система уведомлений "на будущее"
 * 
 * Этот код нарушает YAGNI, реализуя сложную функциональность,
 * которая может никогда не понадобиться.
 */
class BadNotificationSystem {
public:
    enum class NotificationType {
        EMAIL, SMS, PUSH, IN_APP, WEBHOOK, SLACK, DISCORD, TELEGRAM, WHATSAPP
    };
    
    enum class Priority {
        LOW, NORMAL, HIGH, URGENT, CRITICAL
    };
    
    enum class DeliveryStatus {
        PENDING, SENT, DELIVERED, READ, FAILED, RETRYING, CANCELLED
    };
    
    struct NotificationTemplate {
        std::string id;
        std::string subject;
        std::string body;
        NotificationType type;
        Priority priority;
        std::map<std::string, std::string> variables;
        std::chrono::seconds retryInterval;
        int maxRetries;
        std::chrono::seconds ttl;
    };
    
    struct NotificationRecipient {
        std::string id;
        std::string email;
        std::string phone;
        std::string pushToken;
        std::map<NotificationType, bool> preferences;
        std::chrono::system_clock::time_point lastActive;
        std::string timezone;
        std::string language;
    };
    
private:
    std::vector<NotificationTemplate> templates;
    std::vector<NotificationRecipient> recipients;
    std::map<std::string, DeliveryStatus> deliveryStatuses;
    
public:
    void addTemplate(const NotificationTemplate& template) {
        templates.push_back(template);
        std::cout << "📝 Добавлен шаблон уведомления: " << template.id << "\n";
    }
    
    void addRecipient(const NotificationRecipient& recipient) {
        recipients.push_back(recipient);
        std::cout << "👤 Добавлен получатель: " << recipient.id << "\n";
    }
    
    void sendNotification(const std::string& templateId, 
                         const std::vector<std::string>& recipientIds,
                         const std::map<std::string, std::string>& variables) {
        
        // Сложная логика поиска шаблона
        auto templateIt = std::find_if(templates.begin(), templates.end(),
            [&templateId](const NotificationTemplate& t) {
                return t.id == templateId;
            });
        
        if (templateIt == templates.end()) {
            std::cout << "❌ Шаблон не найден: " << templateId << "\n";
            return;
        }
        
        const NotificationTemplate& template = *templateIt;
        
        // Сложная логика отправки каждому получателю
        for (const auto& recipientId : recipientIds) {
            auto recipientIt = std::find_if(recipients.begin(), recipients.end(),
                [&recipientId](const NotificationRecipient& r) {
                    return r.id == recipientId;
                });
            
            if (recipientIt != recipients.end()) {
                sendToRecipient(template, *recipientIt, variables);
            }
        }
    }
    
    void setDeliveryStatus(const std::string& notificationId, DeliveryStatus status) {
        deliveryStatuses[notificationId] = status;
        std::cout << "📊 Статус уведомления " << notificationId << " изменен на " 
                  << static_cast<int>(status) << "\n";
    }
    
    void scheduleNotification(const std::string& templateId,
                             const std::vector<std::string>& recipientIds,
                             const std::chrono::system_clock::time_point& scheduledTime,
                             const std::map<std::string, std::string>& variables) {
        
        std::cout << "⏰ Уведомление запланировано на " 
                  << std::chrono::duration_cast<std::chrono::seconds>(
                         scheduledTime.time_since_epoch()).count() << "\n";
        
        // Сложная логика планирования...
    }
    
    void configureRetryPolicy(const std::string& templateId, 
                             std::chrono::seconds interval, 
                             int maxRetries) {
        auto it = std::find_if(templates.begin(), templates.end(),
            [&templateId](const NotificationTemplate& t) {
                return t.id == templateId;
            });
        
        if (it != templates.end()) {
            it->retryInterval = interval;
            it->maxRetries = maxRetries;
            std::cout << "🔄 Политика повторов настроена для " << templateId << "\n";
        }
    }
    
    void setRecipientPreferences(const std::string& recipientId,
                                NotificationType type, 
                                bool enabled) {
        auto it = std::find_if(recipients.begin(), recipients.end(),
            [&recipientId](const NotificationRecipient& r) {
                return r.id == recipientId;
            });
        
        if (it != recipients.end()) {
            it->preferences[type] = enabled;
            std::cout << "⚙️ Настройки получателя " << recipientId 
                      << " обновлены для типа " << static_cast<int>(type) << "\n";
        }
    }
    
    void generateAnalytics() {
        std::cout << "📈 Генерация аналитики уведомлений...\n";
        // Сложная логика аналитики...
    }
    
    void exportRecipients(const std::string& filename) {
        std::cout << "📤 Экспорт получателей в " << filename << "\n";
        // Сложная логика экспорта...
    }
    
    void importTemplates(const std::string& filename) {
        std::cout << "📥 Импорт шаблонов из " << filename << "\n";
        // Сложная логика импорта...
    }
    
private:
    void sendToRecipient(const NotificationTemplate& template,
                        const NotificationRecipient& recipient,
                        const std::map<std::string, std::string>& variables) {
        
        std::cout << "📤 Отправка уведомления " << template.id 
                  << " получателю " << recipient.id << "\n";
        
        // Сложная логика отправки в зависимости от типа и предпочтений...
    }
};

/**
 * @brief Соблюдение YAGNI: Простая система уведомлений
 * 
 * Этот код соблюдает YAGNI, реализуя только необходимую функциональность.
 */
class SimpleNotificationSystem {
private:
    std::vector<std::string> recipients;
    
public:
    void addRecipient(const std::string& email) {
        recipients.push_back(email);
        std::cout << "👤 Добавлен получатель: " << email << "\n";
    }
    
    void sendNotification(const std::string& message) {
        std::cout << "📤 Отправка уведомления:\n";
        std::cout << "Сообщение: " << message << "\n";
        
        for (const auto& recipient : recipients) {
            std::cout << "  → " << recipient << "\n";
        }
    }
    
    size_t getRecipientCount() const {
        return recipients.size();
    }
};

// ============================================================================
// ПРИМЕР 2: ПРЕЖДЕВРЕМЕННАЯ ОПТИМИЗАЦИЯ В СИСТЕМЕ КЭШИРОВАНИЯ
// ============================================================================

/**
 * @brief Нарушение YAGNI: Сложная система кэширования с множеством стратегий
 */
template<typename K, typename V>
class BadAdvancedCache {
public:
    enum class EvictionPolicy {
        LRU, LFU, FIFO, RANDOM, TTL_BASED, PRIORITY_BASED
    };
    
    enum class PersistenceMode {
        MEMORY_ONLY, FILE_BASED, DATABASE_BASED, DISTRIBUTED
    };
    
    struct CacheStatistics {
        size_t hits;
        size_t misses;
        size_t evictions;
        double hitRatio;
        std::chrono::milliseconds averageAccessTime;
    };
    
private:
    std::map<K, V> cache;
    std::map<K, std::chrono::system_clock::time_point> accessTimes;
    std::map<K, int> accessCounts;
    std::map<K, std::chrono::system_clock::time_point> expirationTimes;
    std::map<K, double> priorities;
    
    size_t maxSize;
    EvictionPolicy policy;
    PersistenceMode persistence;
    CacheStatistics stats;
    bool compressionEnabled;
    bool encryptionEnabled;
    
public:
    BadAdvancedCache(size_t size, EvictionPolicy evictionPolicy, 
                     PersistenceMode persistenceMode = PersistenceMode::MEMORY_ONLY)
        : maxSize(size), policy(evictionPolicy), persistence(persistenceMode),
          compressionEnabled(false), encryptionEnabled(false) {
        
        stats = {0, 0, 0, 0.0, std::chrono::milliseconds(0)};
        std::cout << "🗄️ Создан продвинутый кэш с политикой " 
                  << static_cast<int>(policy) << "\n";
    }
    
    void set(const K& key, const V& value, 
             std::chrono::seconds ttl = std::chrono::seconds(0),
             double priority = 1.0) {
        
        auto start = std::chrono::high_resolution_clock::now();
        
        // Сложная логика установки с учетом всех параметров
        cache[key] = value;
        accessTimes[key] = std::chrono::system_clock::now();
        accessCounts[key] = 1;
        priorities[key] = priority;
        
        if (ttl.count() > 0) {
            expirationTimes[key] = std::chrono::system_clock::now() + ttl;
        }
        
        if (cache.size() > maxSize) {
            evict();
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        stats.averageAccessTime = duration;
        
        std::cout << "💾 Значение установлено в кэш: " << key << "\n";
    }
    
    V get(const K& key, const V& defaultValue = V{}) {
        auto start = std::chrono::high_resolution_clock::now();
        
        auto it = cache.find(key);
        if (it != cache.end()) {
            // Проверка TTL
            auto expIt = expirationTimes.find(key);
            if (expIt != expirationTimes.end()) {
                if (std::chrono::system_clock::now() > expIt->second) {
                    remove(key);
                    stats.misses++;
                    return defaultValue;
                }
            }
            
            // Обновление статистики доступа
            accessTimes[key] = std::chrono::system_clock::now();
            accessCounts[key]++;
            
            stats.hits++;
            stats.hitRatio = static_cast<double>(stats.hits) / (stats.hits + stats.misses);
            
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            stats.averageAccessTime = duration;
            
            std::cout << "📖 Значение получено из кэша: " << key << "\n";
            return it->second;
        }
        
        stats.misses++;
        stats.hitRatio = static_cast<double>(stats.hits) / (stats.hits + stats.misses);
        return defaultValue;
    }
    
    void remove(const K& key) {
        cache.erase(key);
        accessTimes.erase(key);
        accessCounts.erase(key);
        expirationTimes.erase(key);
        priorities.erase(key);
        stats.evictions++;
        std::cout << "🗑️ Значение удалено из кэша: " << key << "\n";
    }
    
    CacheStatistics getStatistics() const {
        return stats;
    }
    
    void enableCompression(bool enable) {
        compressionEnabled = enable;
        std::cout << "🗜️ Сжатие " << (enable ? "включено" : "выключено") << "\n";
    }
    
    void enableEncryption(bool enable) {
        encryptionEnabled = enable;
        std::cout << "🔐 Шифрование " << (enable ? "включено" : "выключено") << "\n";
    }
    
    void setEvictionPolicy(EvictionPolicy newPolicy) {
        policy = newPolicy;
        std::cout << "⚙️ Политика вытеснения изменена на " 
                  << static_cast<int>(newPolicy) << "\n";
    }
    
    void persistToFile(const std::string& filename) {
        std::cout << "💾 Сохранение кэша в файл: " << filename << "\n";
        // Сложная логика сериализации...
    }
    
    void loadFromFile(const std::string& filename) {
        std::cout << "📂 Загрузка кэша из файла: " << filename << "\n";
        // Сложная логика десериализации...
    }
    
private:
    void evict() {
        switch (policy) {
            case EvictionPolicy::LRU:
                evictLRU();
                break;
            case EvictionPolicy::LFU:
                evictLFU();
                break;
            case EvictionPolicy::FIFO:
                evictFIFO();
                break;
            case EvictionPolicy::RANDOM:
                evictRandom();
                break;
            case EvictionPolicy::TTL_BASED:
                evictTTL();
                break;
            case EvictionPolicy::PRIORITY_BASED:
                evictPriority();
                break;
        }
    }
    
    void evictLRU() {
        auto oldest = std::min_element(accessTimes.begin(), accessTimes.end(),
            [](const auto& a, const auto& b) {
                return a.second < b.second;
            });
        
        if (oldest != accessTimes.end()) {
            remove(oldest->first);
        }
    }
    
    void evictLFU() {
        auto leastUsed = std::min_element(accessCounts.begin(), accessCounts.end(),
            [](const auto& a, const auto& b) {
                return a.second < b.second;
            });
        
        if (leastUsed != accessCounts.end()) {
            remove(leastUsed->first);
        }
    }
    
    void evictFIFO() {
        if (!accessTimes.empty()) {
            auto first = accessTimes.begin();
            remove(first->first);
        }
    }
    
    void evictRandom() {
        if (!cache.empty()) {
            auto it = cache.begin();
            std::advance(it, rand() % cache.size());
            remove(it->first);
        }
    }
    
    void evictTTL() {
        auto expired = std::find_if(expirationTimes.begin(), expirationTimes.end(),
            [](const auto& pair) {
                return std::chrono::system_clock::now() > pair.second;
            });
        
        if (expired != expirationTimes.end()) {
            remove(expired->first);
        }
    }
    
    void evictPriority() {
        auto lowestPriority = std::min_element(priorities.begin(), priorities.end(),
            [](const auto& a, const auto& b) {
                return a.second < b.second;
            });
        
        if (lowestPriority != priorities.end()) {
            remove(lowestPriority->first);
        }
    }
};

/**
 * @brief Соблюдение YAGNI: Простая система кэширования
 * 
 * Этот код соблюдает YAGNI, реализуя только необходимую функциональность.
 */
template<typename K, typename V>
class SimpleCache {
private:
    std::map<K, V> cache;
    size_t maxSize;
    
public:
    SimpleCache(size_t size = 100) : maxSize(size) {
        std::cout << "🗄️ Создан простой кэш размером " << size << "\n";
    }
    
    void set(const K& key, const V& value) {
        cache[key] = value;
        
        if (cache.size() > maxSize) {
            // Простое вытеснение - удаляем первый элемент
            cache.erase(cache.begin());
        }
        
        std::cout << "💾 Значение установлено: " << key << "\n";
    }
    
    V get(const K& key, const V& defaultValue = V{}) {
        auto it = cache.find(key);
        if (it != cache.end()) {
            std::cout << "📖 Значение найдено в кэше: " << key << "\n";
            return it->second;
        }
        
        std::cout << "❌ Значение не найдено в кэше: " << key << "\n";
        return defaultValue;
    }
    
    bool exists(const K& key) const {
        return cache.find(key) != cache.end();
    }
    
    size_t size() const {
        return cache.size();
    }
};

// ============================================================================
// ПРИМЕР 3: ПРЕЖДЕВРЕМЕННАЯ ОПТИМИЗАЦИЯ В СИСТЕМЕ ЛОГИРОВАНИЯ
// ============================================================================

/**
 * @brief Нарушение YAGNI: Сложная система логирования
 */
class BadLogger {
public:
    enum class LogLevel {
        TRACE, DEBUG, INFO, WARN, ERROR, FATAL
    };
    
    enum class OutputFormat {
        PLAIN_TEXT, JSON, XML, BINARY, CUSTOM
    };
    
    enum class CompressionType {
        NONE, GZIP, BZIP2, LZ4, SNAPPY
    };
    
    struct LogEntry {
        LogLevel level;
        std::string message;
        std::string source;
        std::string threadId;
        std::chrono::system_clock::time_point timestamp;
        std::map<std::string, std::string> metadata;
        std::string correlationId;
    };
    
private:
    std::vector<std::function<void(const LogEntry&)>> handlers;
    std::map<std::string, LogLevel> categoryLevels;
    OutputFormat format;
    CompressionType compression;
    bool asyncEnabled;
    size_t bufferSize;
    std::chrono::seconds flushInterval;
    
public:
    BadLogger(OutputFormat outputFormat = OutputFormat::PLAIN_TEXT,
              CompressionType compType = CompressionType::NONE,
              bool async = false)
        : format(outputFormat), compression(compType), asyncEnabled(async),
          bufferSize(1024), flushInterval(std::chrono::seconds(5)) {
        
        std::cout << "📝 Создан продвинутый логгер с форматом " 
                  << static_cast<int>(format) << "\n";
    }
    
    void addHandler(std::function<void(const LogEntry&)> handler) {
        handlers.push_back(handler);
        std::cout << "➕ Добавлен обработчик логов\n";
    }
    
    void setCategoryLevel(const std::string& category, LogLevel level) {
        categoryLevels[category] = level;
        std::cout << "📊 Уровень для категории " << category 
                  << " установлен на " << static_cast<int>(level) << "\n";
    }
    
    void log(LogLevel level, const std::string& message, 
             const std::string& category = "default") {
        
        LogEntry entry;
        entry.level = level;
        entry.message = message;
        entry.source = category;
        entry.timestamp = std::chrono::system_clock::now();
        
        // Проверка уровня логирования для категории
        auto it = categoryLevels.find(category);
        if (it != categoryLevels.end() && level < it->second) {
            return; // Не логируем, если уровень слишком низкий
        }
        
        // Обработка через все обработчики
        for (const auto& handler : handlers) {
            handler(entry);
        }
        
        std::cout << "📝 Лог записан: [" << static_cast<int>(level) 
                  << "] " << message << "\n";
    }
    
    void setOutputFormat(OutputFormat newFormat) {
        format = newFormat;
        std::cout << "🎨 Формат вывода изменен на " << static_cast<int>(newFormat) << "\n";
    }
    
    void setCompression(CompressionType compType) {
        compression = compType;
        std::cout << "🗜️ Сжатие изменено на " << static_cast<int>(compType) << "\n";
    }
    
    void enableAsync(bool enable) {
        asyncEnabled = enable;
        std::cout << "⚡ Асинхронное логирование " 
                  << (enable ? "включено" : "выключено") << "\n";
    }
    
    void setBufferSize(size_t size) {
        bufferSize = size;
        std::cout << "📦 Размер буфера установлен на " << size << "\n";
    }
    
    void setFlushInterval(std::chrono::seconds interval) {
        flushInterval = interval;
        std::cout << "⏰ Интервал сброса установлен на " 
                  << interval.count() << " секунд\n";
    }
};

/**
 * @brief Соблюдение YAGNI: Простая система логирования
 * 
 * Этот код соблюдает YAGNI, реализуя только необходимую функциональность.
 */
class SimpleLogger {
public:
    void log(const std::string& message) {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        
        std::cout << "[" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") 
                  << "] " << message << "\n";
    }
    
    void error(const std::string& message) {
        log("ERROR: " + message);
    }
    
    void info(const std::string& message) {
        log("INFO: " + message);
    }
};

// ============================================================================
// ДЕМОНСТРАЦИЯ ПРИНЦИПА
// ============================================================================

void demonstrateBadYAGNI() {
    std::cout << "❌ НАРУШЕНИЕ YAGNI - Преждевременная оптимизация:\n";
    std::cout << std::string(50, '-') << "\n";
    
    std::cout << "Сложная система уведомлений:\n";
    BadNotificationSystem badSystem;
    
    BadNotificationSystem::NotificationTemplate template1;
    template1.id = "welcome";
    template1.subject = "Добро пожаловать";
    template1.body = "Добро пожаловать в наше приложение!";
    template1.type = BadNotificationSystem::NotificationType::EMAIL;
    template1.priority = BadNotificationSystem::Priority::NORMAL;
    template1.maxRetries = 3;
    
    badSystem.addTemplate(template1);
    badSystem.sendNotification("welcome", {"user1@example.com"}, {});
    badSystem.generateAnalytics(); // Функция, которая может не понадобиться
    badSystem.exportRecipients("recipients.csv"); // Функция, которая может не понадобиться
    
    std::cout << "\nСложная система кэширования:\n";
    BadAdvancedCache<std::string, std::string> badCache(5, 
        BadAdvancedCache<std::string, std::string>::EvictionPolicy::LRU);
    
    badCache.set("key1", "value1", std::chrono::seconds(3600), 1.0);
    badCache.set("key2", "value2", std::chrono::seconds(3600), 2.0);
    badCache.enableCompression(true); // Функция, которая может не понадобиться
    badCache.enableEncryption(true); // Функция, которая может не понадобиться
    badCache.persistToFile("cache.dat"); // Функция, которая может не понадобиться
    
    std::cout << "\nСложная система логирования:\n";
    BadLogger badLogger(BadLogger::OutputFormat::JSON, 
                       BadLogger::CompressionType::GZIP, true);
    
    badLogger.setCategoryLevel("database", BadLogger::LogLevel::INFO);
    badLogger.setBufferSize(2048); // Функция, которая может не понадобиться
    badLogger.setFlushInterval(std::chrono::seconds(10)); // Функция, которая может не понадобиться
    badLogger.log(BadLogger::LogLevel::INFO, "Приложение запущено", "system");
}

void demonstrateGoodYAGNI() {
    std::cout << "\n✅ СОБЛЮДЕНИЕ YAGNI - Только необходимое:\n";
    std::cout << std::string(50, '-') << "\n";
    
    std::cout << "Простая система уведомлений:\n";
    SimpleNotificationSystem simpleSystem;
    simpleSystem.addRecipient("user@example.com");
    simpleSystem.addRecipient("admin@example.com");
    simpleSystem.sendNotification("Приложение запущено");
    std::cout << "Получателей: " << simpleSystem.getRecipientCount() << "\n";
    
    std::cout << "\nПростая система кэширования:\n";
    SimpleCache<std::string, std::string> simpleCache(5);
    simpleCache.set("key1", "value1");
    simpleCache.set("key2", "value2");
    simpleCache.set("key3", "value3");
    std::cout << "Размер кэша: " << simpleCache.size() << "\n";
    std::cout << "Key1: " << simpleCache.get("key1") << "\n";
    
    std::cout << "\nПростая система логирования:\n";
    SimpleLogger simpleLogger;
    simpleLogger.info("Приложение запущено");
    simpleLogger.error("Ошибка подключения к базе данных");
    simpleLogger.log("Обычное сообщение");
}

void analyzeTradeOffs() {
    std::cout << "\n🔬 АНАЛИЗ КОМПРОМИССОВ YAGNI:\n";
    std::cout << std::string(50, '-') << "\n";
    
    std::cout << "📊 ПРЕИМУЩЕСТВА:\n";
    std::cout << "• Быстрая разработка\n";
    std::cout << "• Меньше неиспользуемого кода\n";
    std::cout << "• Простота понимания\n";
    std::cout << "• Меньше багов\n";
    std::cout << "• Экономия времени и ресурсов\n\n";
    
    std::cout << "⚠️ НЕДОСТАТКИ:\n";
    std::cout << "• Возможная необходимость рефакторинга\n";
    std::cout << "• Потенциальные ограничения архитектуры\n";
    std::cout << "• Необходимость прогнозирования будущих потребностей\n";
    std::cout << "• Возможные технические долги\n\n";
    
    std::cout << "🎯 РЕКОМЕНДАЦИИ:\n";
    std::cout << "• Реализуйте только текущие требования\n";
    std::cout << "• Откладывайте оптимизацию до измерения\n";
    std::cout << "• Используйте простые решения\n";
    std::cout << "• Планируйте архитектуру для будущего роста\n";
}

int main() {
    std::cout << "🎯 ДЕТАЛЬНАЯ ДЕМОНСТРАЦИЯ ПРИНЦИПА YAGNI (You Aren't Gonna Need It)\n";
    std::cout << "Автор: Senior C++ Developer\n";
    std::cout << "Цель: Углубленное изучение принципа откладывания ненужной функциональности\n\n";
    
    demonstrateBadYAGNI();
    demonstrateGoodYAGNI();
    analyzeTradeOffs();
    
    std::cout << "\n📚 МАТЕМАТИЧЕСКОЕ ОБОСНОВАНИЕ:\n";
    std::cout << "YAGNI: ∀F ∈ Features: F ∈ CurrentRequirements ∨ F ∈ ImmediateNeeds\n";
    std::cout << "где CurrentRequirements = {f | f требуется сейчас}\n";
    std::cout << "     ImmediateNeeds = {f | f потребуется в ближайшем будущем}\n\n";
    
    std::cout << "💡 КЛЮЧЕВЫЕ ВЫВОДЫ:\n";
    std::cout << "1. Не добавляйте функциональность, пока она не понадобится\n";
    std::cout << "2. YAGNI ускоряет разработку и снижает сложность\n";
    std::cout << "3. Применение требует баланса с планированием архитектуры\n";
    std::cout << "4. Современные практики поддерживают инкрементальную разработку\n\n";
    
    std::cout << "🔬 Принципы - это инструменты для мышления о проблемах!\n";
    
    return 0;
}
