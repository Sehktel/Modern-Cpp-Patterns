# 🏋️ Упражнения: Cache-Aside Pattern

## 📋 Задание 1: Базовый LRU Cache

### Описание
Реализуйте LRU (Least Recently Used) Cache с поддержкой thread-safety и метрик.

### Требования
1. ✅ LRU вытеснение при достижении capacity
2. ✅ O(1) для get/put операций
3. ✅ Thread-safe доступ
4. ✅ Метрики: hits, misses, evictions
5. ✅ Configurable capacity и TTL

### Шаблон для реализации
```cpp
#include <iostream>
#include <unordered_map>
#include <list>
#include <mutex>
#include <chrono>
#include <optional>

template<typename K, typename V>
class LRUCache {
private:
    size_t capacity_;
    
    // TODO: Используйте list для LRU ordering
    std::list<K> lru_list_;
    
    // TODO: Map: key -> iterator в list
    std::unordered_map<K, typename std::list<K>::iterator> lru_map_;
    
    // TODO: Map: key -> value
    std::unordered_map<K, V> cache_;
    
    mutable std::mutex mutex_;
    
    // Метрики
    std::atomic<size_t> hits_{0};
    std::atomic<size_t> misses_{0};
    std::atomic<size_t> evictions_{0};
    
public:
    explicit LRUCache(size_t capacity) : capacity_(capacity) {}
    
    // TODO: Реализуйте get
    std::optional<V> get(const K& key) {
        // Ваш код здесь
        // 1. Проверить наличие в cache_
        // 2. Обновить LRU (переместить в начало)
        // 3. Обновить метрики
        return std::nullopt;
    }
    
    // TODO: Реализуйте put
    void put(const K& key, const V& value) {
        // Ваш код здесь
        // 1. Если ключ существует - обновить
        // 2. Если capacity достигнут - вытеснить LRU
        // 3. Добавить новый элемент
        // 4. Обновить LRU list
    }
    
    // TODO: Реализуйте remove
    void remove(const K& key) {
        // Ваш код здесь
    }
    
    void printStats() const {
        size_t total = hits_.load() + misses_.load();
        double hit_rate = total > 0 ? (100.0 * hits_.load() / total) : 0.0;
        
        std::cout << "Cache Stats:" << std::endl;
        std::cout << "  Hits: " << hits_.load() << std::endl;
        std::cout << "  Misses: " << misses_.load() << std::endl;
        std::cout << "  Hit Rate: " << hit_rate << "%" << std::endl;
        std::cout << "  Evictions: " << evictions_.load() << std::endl;
    }
};
```

### Тесты
```cpp
void testLRUCache() {
    LRUCache<int, std::string> cache(3);
    
    // Добавляем элементы
    cache.put(1, "one");
    cache.put(2, "two");
    cache.put(3, "three");
    
    // Проверяем hit
    auto result = cache.get(1);
    assert(result.has_value() && result.value() == "one");
    
    // Добавляем 4-й элемент (должен вытеснить 2)
    cache.put(4, "four");
    
    // 2 должен быть вытеснен
    auto result2 = cache.get(2);
    assert(!result2.has_value());
    
    cache.printStats();
    std::cout << "LRU Cache test PASSED ✅" << std::endl;
}
```

---

## 📋 Задание 2: Multi-Level Cache

### Описание
Реализуйте многоуровневый кэш (L1 in-memory + L2 persistent) с автоматическим промотированием.

### Требования
1. ✅ L1: быстрый in-memory cache
2. ✅ L2: медленный persistent cache (файл/БД)
3. ✅ Автоматическое промотирование L2 → L1
4. ✅ Write-through или write-back политика
5. ✅ Консистентность между уровнями

### Шаблон
```cpp
template<typename K, typename V>
class MultiLevelCache {
private:
    std::shared_ptr<LRUCache<K, V>> l1_cache_;
    std::shared_ptr<PersistentCache<K, V>> l2_cache_;
    
public:
    V get(const K& key) {
        // TODO: 1. Проверить L1
        // TODO: 2. Если miss в L1, проверить L2
        // TODO: 3. Если hit в L2, промотировать в L1
        // TODO: 4. Если miss везде, загрузить из DB
    }
    
    void put(const K& key, const V& value) {
        // TODO: Write-through: записать в L1 и L2
        // ИЛИ
        // TODO: Write-back: записать в L1, lazy sync в L2
    }
};
```

---

## 📋 Задание 3: Cache with TTL (Time To Live)

### Описание
Реализуйте кэш с автоматическим истечением записей по TTL.

### Требования
1. ✅ TTL для каждой записи
2. ✅ Фоновый cleaner thread для удаления expired
3. ✅ Проверка expiration при get()
4. ✅ Configurable default TTL и per-key TTL
5. ✅ Метрики: expired entries, cleanup cycles

### Шаблон
```cpp
template<typename K, typename V>
struct CacheEntry {
    V value;
    std::chrono::system_clock::time_point expires_at;
    
    bool isExpired() const {
        return std::chrono::system_clock::now() > expires_at;
    }
};

template<typename K, typename V>
class TTLCache {
private:
    std::unordered_map<K, CacheEntry<V>> cache_;
    std::chrono::milliseconds default_ttl_;
    std::thread cleaner_thread_;
    std::atomic<bool> running_{true};
    
public:
    // TODO: Реализуйте TTL cache
    
    std::optional<V> get(const K& key) {
        // TODO: Проверьте expiration
    }
    
    void put(const K& key, const V& value, 
            std::optional<std::chrono::milliseconds> ttl = std::nullopt) {
        // TODO: Установите TTL
    }
    
private:
    void cleanerLoop() {
        // TODO: Периодически удаляйте expired entries
    }
};
```

---

## 📋 Задание 4: Cache Invalidation Strategies

### Описание
Реализуйте различные стратегии инвалидации кэша.

### Требования
1. ✅ Time-based invalidation
2. ✅ Event-based invalidation
3. ✅ Version-based invalidation
4. ✅ Pattern-based invalidation (wildcard keys)
5. ✅ Cascade invalidation

### Стратегии
```cpp
enum class InvalidationStrategy {
    IMMEDIATE,      // Немедленная инвалидация
    LAZY,           // Проверка при get()
    SCHEDULED,      // По расписанию
    EVENT_DRIVEN    // При событии
};

class CacheInvalidator {
public:
    // TODO: Реализуйте разные стратегии
    
    void invalidate(const std::string& key) {
        // Immediate invalidation
    }
    
    void invalidatePattern(const std::string& pattern) {
        // Pattern-based (e.g., "user:*")
    }
    
    void scheduleInvalidation(const std::string& key, 
                             std::chrono::milliseconds delay) {
        // Delayed invalidation
    }
};
```

---

## 📋 Задание 5: Cache с Read-Through/Write-Through

### Описание
Реализуйте Cache-Aside с автоматической загрузкой данных (Read-Through) и записью (Write-Through).

### Требования
1. ✅ Read-Through: автозагрузка при cache miss
2. ✅ Write-Through: автозапись в БД при put
3. ✅ Loader function для загрузки данных
4. ✅ Writer function для сохранения данных
5. ✅ Retry logic при ошибках БД

### Шаблон
```cpp
template<typename K, typename V>
class ReadThroughCache {
private:
    std::unordered_map<K, V> cache_;
    std::function<V(const K&)> loader_;
    std::function<void(const K&, const V&)> writer_;
    
public:
    ReadThroughCache(std::function<V(const K&)> loader,
                    std::function<void(const K&, const V&)> writer)
        : loader_(std::move(loader)), writer_(std::move(writer)) {}
    
    V get(const K& key) {
        // TODO: 1. Проверить cache
        // TODO: 2. Если miss - вызвать loader_
        // TODO: 3. Сохранить в cache
        // TODO: 4. Вернуть значение
    }
    
    void put(const K& key, const V& value) {
        // TODO: 1. Записать в cache
        // TODO: 2. Записать в БД через writer_
    }
};
```

---

**Автор**: Senior C++ Developer  
**Дата**: 2025-10-07  
**Сложность**: ⭐⭐⭐⭐ (Продвинутый уровень)
