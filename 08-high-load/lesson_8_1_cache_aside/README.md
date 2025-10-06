# Урок 8.1: Cache-Aside Pattern (Кэш-вне-приложения)

## 🎯 Цель урока
Изучить паттерн Cache-Aside - один из фундаментальных паттернов для высоконагруженных систем. Понять, как эффективно управлять кэшем данных для улучшения производительности и снижения нагрузки на основное хранилище.

## 📚 Что изучаем

### 1. Паттерн Cache-Aside
- **Определение**: Приложение само управляет кэшем, загружая и сохраняя данные
- **Назначение**: Улучшение производительности за счет кэширования часто используемых данных
- **Применение**: Базы данных, веб-приложения, API, распределенные системы

### 2. Ключевые компоненты
- **Cache**: Быстрое хранилище (Redis, Memcached, in-memory)
- **Primary Storage**: Основное хранилище (база данных, файловая система)
- **Application Logic**: Логика управления кэшем
- **Cache Policies**: Стратегии кэширования (LRU, LFU, TTL)

### 3. Алгоритм работы
1. **Read**: Проверяем кэш → если нет, загружаем из хранилища → сохраняем в кэш
2. **Write**: Обновляем хранилище → инвалидируем или обновляем кэш

## 🔍 Ключевые концепции

### Базовая реализация
```cpp
#include <unordered_map>
#include <mutex>
#include <chrono>
#include <memory>

template<typename Key, typename Value>
class CacheAsideCache {
private:
    struct CacheEntry {
        Value value;
        std::chrono::system_clock::time_point timestamp;
        std::chrono::seconds ttl;
        
        bool isExpired() const {
            return std::chrono::system_clock::now() > timestamp + ttl;
        }
    };
    
    std::unordered_map<Key, CacheEntry> cache_;
    mutable std::mutex mutex_;
    std::chrono::seconds defaultTTL_;
    
public:
    explicit CacheAsideCache(std::chrono::seconds defaultTTL = std::chrono::seconds(300))
        : defaultTTL_(defaultTTL) {}
    
    // Получение данных из кэша
    std::optional<Value> get(const Key& key) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = cache_.find(key);
        if (it != cache_.end()) {
            if (it->second.isExpired()) {
                cache_.erase(it);
                return std::nullopt;
            }
            return it->second.value;
        }
        
        return std::nullopt;
    }
    
    // Сохранение данных в кэш
    void put(const Key& key, const Value& value, 
             std::chrono::seconds ttl = std::chrono::seconds(-1)) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        CacheEntry entry;
        entry.value = value;
        entry.timestamp = std::chrono::system_clock::now();
        entry.ttl = (ttl.count() < 0) ? defaultTTL_ : ttl;
        
        cache_[key] = std::move(entry);
    }
    
    // Инвалидация кэша
    void invalidate(const Key& key) {
        std::lock_guard<std::mutex> lock(mutex_);
        cache_.erase(key);
    }
    
    // Очистка истекших записей
    void cleanup() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = cache_.begin();
        while (it != cache_.end()) {
            if (it->second.isExpired()) {
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
};
```

## 🛠️ Практические примеры

### Кэш для пользовательских данных
```cpp
class UserCache {
private:
    CacheAsideCache<std::string, User> cache_;
    Database& database_;
    
public:
    UserCache(Database& db) : database_(db) {}
    
    std::optional<User> getUser(const std::string& userId) {
        // Пытаемся получить из кэша
        auto cachedUser = cache_.get(userId);
        if (cachedUser) {
            std::cout << "Cache HIT для пользователя: " << userId << std::endl;
            return *cachedUser;
        }
        
        // Кэш промах - загружаем из базы данных
        std::cout << "Cache MISS для пользователя: " << userId << std::endl;
        
        auto user = database_.getUser(userId);
        if (user) {
            // Сохраняем в кэш на 5 минут
            cache_.put(userId, *user, std::chrono::minutes(5));
        }
        
        return user;
    }
    
    void updateUser(const std::string& userId, const User& user) {
        // Обновляем в базе данных
        database_.updateUser(userId, user);
        
        // Инвалидируем кэш
        cache_.invalidate(userId);
        
        std::cout << "Пользователь " << userId << " обновлен, кэш инвалидирован" << std::endl;
    }
    
    void deleteUser(const std::string& userId) {
        // Удаляем из базы данных
        database_.deleteUser(userId);
        
        // Инвалидируем кэш
        cache_.invalidate(userId);
        
        std::cout << "Пользователь " << userId << " удален, кэш инвалидирован" << std::endl;
    }
};
```

### Многоуровневый кэш
```cpp
class MultiLevelCache {
private:
    CacheAsideCache<std::string, std::string> l1Cache_; // Быстрый in-memory кэш
    CacheAsideCache<std::string, std::string> l2Cache_; // Медленный кэш (Redis)
    Database& database_;
    
public:
    MultiLevelCache(Database& db) : database_(db) {}
    
    std::optional<std::string> get(const std::string& key) {
        // L1 Cache (самый быстрый)
        auto l1Result = l1Cache_.get(key);
        if (l1Result) {
            std::cout << "L1 Cache HIT для ключа: " << key << std::endl;
            return *l1Result;
        }
        
        // L2 Cache
        auto l2Result = l2Cache_.get(key);
        if (l2Result) {
            std::cout << "L2 Cache HIT для ключа: " << key << std::endl;
            
            // Промотируем в L1 кэш
            l1Cache_.put(key, *l2Result, std::chrono::minutes(1));
            return *l2Result;
        }
        
        // Cache miss - загружаем из базы данных
        std::cout << "Cache MISS для ключа: " << key << std::endl;
        
        auto value = database_.get(key);
        if (value) {
            // Сохраняем во все уровни кэша
            l1Cache_.put(key, *value, std::chrono::minutes(1));
            l2Cache_.put(key, *value, std::chrono::minutes(10));
        }
        
        return value;
    }
    
    void put(const std::string& key, const std::string& value) {
        // Обновляем базу данных
        database_.put(key, value);
        
        // Обновляем все уровни кэша
        l1Cache_.put(key, value, std::chrono::minutes(1));
        l2Cache_.put(key, value, std::chrono::minutes(10));
        
        std::cout << "Ключ " << key << " обновлен во всех кэшах" << std::endl;
    }
    
    void invalidate(const std::string& key) {
        // Инвалидируем все уровни кэша
        l1Cache_.invalidate(key);
        l2Cache_.invalidate(key);
        
        std::cout << "Ключ " << key << " инвалидирован во всех кэшах" << std::endl;
    }
};
```

## 🎯 Практические упражнения

### Упражнение 1: LRU Cache
Реализуйте кэш с политикой LRU (Least Recently Used).

### Упражнение 2: Write-Through Cache
Создайте кэш с синхронной записью в основное хранилище.

### Упражнение 3: Write-Behind Cache
Реализуйте кэш с асинхронной записью (write-behind).

### Упражнение 4: Distributed Cache
Создайте распределенный кэш с консистентностью.

## 💡 Важные принципы

1. **Cache Hit Ratio**: Стремитесь к высокому проценту попаданий в кэш
2. **TTL Management**: Правильно управляйте временем жизни записей
3. **Cache Invalidation**: Своевременно инвалидируйте устаревшие данные
4. **Memory Management**: Контролируйте использование памяти
5. **Consistency**: Обеспечивайте консистентность между кэшем и хранилищем
