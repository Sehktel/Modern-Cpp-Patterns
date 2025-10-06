/**
 * @file cache_aside_pattern.cpp
 * @brief Демонстрация Cache-Aside Pattern
 * 
 * Реализован Cache-Aside Pattern с поддержкой:
 * - Базовый кэш с TTL
 * - Многоуровневый кэш (L1/L2)
 * - Стратегии инвалидации
 * - LRU/LFU кэш
 */

#include <iostream>
#include <unordered_map>
#include <map>
#include <list>
#include <mutex>
#include <chrono>
#include <thread>
#include <memory>
#include <functional>
#include <atomic>
#include <random>
#include <string>
#include <vector>

// Базовый интерфейс для кэша
template<typename Key, typename Value>
class CacheInterface {
public:
    virtual ~CacheInterface() = default;
    virtual bool get(const Key& key, Value& value) = 0;
    virtual void put(const Key& key, const Value& value) = 0;
    virtual void remove(const Key& key) = 0;
    virtual void clear() = 0;
    virtual size_t size() const = 0;
    virtual bool contains(const Key& key) const = 0;
};

// Элемент кэша с временными метками
template<typename Value>
struct CacheEntry {
    Value value;
    std::chrono::steady_clock::time_point created_at;
    std::chrono::steady_clock::time_point last_accessed;
    std::chrono::milliseconds ttl;
    
    CacheEntry(const Value& val, std::chrono::milliseconds ttl_duration)
        : value(val), ttl(ttl_duration) {
        auto now = std::chrono::steady_clock::now();
        created_at = now;
        last_accessed = now;
    }
    
    bool isExpired() const {
        auto now = std::chrono::steady_clock::now();
        return (now - created_at) > ttl;
    }
    
    void updateAccess() {
        last_accessed = std::chrono::steady_clock::now();
    }
};

// LRU кэш с TTL
template<typename Key, typename Value>
class LRUCache : public CacheInterface<Key, Value> {
private:
    size_t capacity_;
    std::list<std::pair<Key, Value>> items_;
    std::unordered_map<Key, typename std::list<std::pair<Key, Value>>::iterator> cache_map_;
    std::mutex mutex_;
    
    // TTL поддержка
    std::unordered_map<Key, CacheEntry<Value>> ttl_entries_;
    
public:
    LRUCache(size_t capacity) : capacity_(capacity) {
        std::cout << "LRU Cache создан с емкостью " << capacity << std::endl;
    }
    
    bool get(const Key& key, Value& value) override {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = cache_map_.find(key);
        if (it == cache_map_.end()) {
            return false;
        }
        
        // Проверяем TTL
        auto ttl_it = ttl_entries_.find(key);
        if (ttl_it != ttl_entries_.end() && ttl_it->second.isExpired()) {
            // Элемент истек, удаляем
            items_.erase(it->second);
            cache_map_.erase(it);
            ttl_entries_.erase(ttl_it);
            return false;
        }
        
        // Обновляем время доступа
        if (ttl_it != ttl_entries_.end()) {
            ttl_it->second.updateAccess();
        }
        
        // Перемещаем в начало списка (самый недавно использованный)
        items_.splice(items_.begin(), items_, it->second);
        value = it->second->second;
        
        return true;
    }
    
    void put(const Key& key, const Value& value) override {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = cache_map_.find(key);
        if (it != cache_map_.end()) {
            // Обновляем существующий элемент
            it->second->second = value;
            items_.splice(items_.begin(), items_, it->second);
            ttl_entries_[key].updateAccess();
            return;
        }
        
        // Проверяем емкость
        if (items_.size() >= capacity_) {
            // Удаляем самый старый элемент
            auto last = items_.back();
            cache_map_.erase(last.first);
            ttl_entries_.erase(last.first);
            items_.pop_back();
        }
        
        // Добавляем новый элемент
        items_.emplace_front(key, value);
        cache_map_[key] = items_.begin();
        ttl_entries_[key] = CacheEntry<Value>(value, std::chrono::minutes(5)); // TTL 5 минут
    }
    
    void remove(const Key& key) override {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = cache_map_.find(key);
        if (it != cache_map_.end()) {
            items_.erase(it->second);
            cache_map_.erase(it);
            ttl_entries_.erase(key);
        }
    }
    
    void clear() override {
        std::lock_guard<std::mutex> lock(mutex_);
        items_.clear();
        cache_map_.clear();
        ttl_entries_.clear();
    }
    
    size_t size() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return items_.size();
    }
    
    bool contains(const Key& key) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return cache_map_.find(key) != cache_map_.end();
    }
    
    void printStats() const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::cout << "LRU Cache: размер=" << items_.size() 
                  << ", емкость=" << capacity_ << std::endl;
    }
};

// LFU кэш с TTL
template<typename Key, typename Value>
class LFUCache : public CacheInterface<Key, Value> {
private:
    size_t capacity_;
    std::unordered_map<Key, Value> cache_;
    std::unordered_map<Key, int> frequencies_;
    std::unordered_map<int, std::list<Key>> frequency_lists_;
    std::unordered_map<Key, typename std::list<Key>::iterator> key_iterators_;
    std::mutex mutex_;
    
    // TTL поддержка
    std::unordered_map<Key, CacheEntry<Value>> ttl_entries_;
    
    int min_frequency_;
    
public:
    LFUCache(size_t capacity) : capacity_(capacity), min_frequency_(0) {
        std::cout << "LFU Cache создан с емкостью " << capacity << std::endl;
    }
    
    bool get(const Key& key, Value& value) override {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = cache_.find(key);
        if (it == cache_.end()) {
            return false;
        }
        
        // Проверяем TTL
        auto ttl_it = ttl_entries_.find(key);
        if (ttl_it != ttl_entries_.end() && ttl_it->second.isExpired()) {
            removeKey(key);
            return false;
        }
        
        // Обновляем частоту использования
        updateFrequency(key);
        value = it->second;
        
        return true;
    }
    
    void put(const Key& key, const Value& value) override {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (cache_.find(key) != cache_.end()) {
            // Обновляем существующий элемент
            cache_[key] = value;
            updateFrequency(key);
            ttl_entries_[key].updateAccess();
            return;
        }
        
        // Проверяем емкость
        if (cache_.size() >= capacity_) {
            evictLFU();
        }
        
        // Добавляем новый элемент
        cache_[key] = value;
        frequencies_[key] = 1;
        frequency_lists_[1].push_back(key);
        key_iterators_[key] = --frequency_lists_[1].end();
        min_frequency_ = 1;
        ttl_entries_[key] = CacheEntry<Value>(value, std::chrono::minutes(5));
    }
    
    void remove(const Key& key) override {
        std::lock_guard<std::mutex> lock(mutex_);
        removeKey(key);
    }
    
    void clear() override {
        std::lock_guard<std::mutex> lock(mutex_);
        cache_.clear();
        frequencies_.clear();
        frequency_lists_.clear();
        key_iterators_.clear();
        ttl_entries_.clear();
        min_frequency_ = 0;
    }
    
    size_t size() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return cache_.size();
    }
    
    bool contains(const Key& key) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return cache_.find(key) != cache_.end();
    }
    
    void printStats() const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::cout << "LFU Cache: размер=" << cache_.size() 
                  << ", емкость=" << capacity_ << std::endl;
    }
    
private:
    void updateFrequency(const Key& key) {
        int freq = frequencies_[key];
        frequencies_[key]++;
        
        // Удаляем из старого списка частот
        frequency_lists_[freq].erase(key_iterators_[key]);
        
        // Добавляем в новый список частот
        frequency_lists_[freq + 1].push_back(key);
        key_iterators_[key] = --frequency_lists_[freq + 1].end();
        
        // Обновляем минимальную частоту
        if (frequency_lists_[min_frequency_].empty()) {
            min_frequency_++;
        }
    }
    
    void evictLFU() {
        auto& min_freq_list = frequency_lists_[min_frequency_];
        Key key_to_remove = min_freq_list.front();
        min_freq_list.pop_front();
        
        removeKey(key_to_remove);
    }
    
    void removeKey(const Key& key) {
        auto it = cache_.find(key);
        if (it != cache_.end()) {
            int freq = frequencies_[key];
            frequency_lists_[freq].erase(key_iterators_[key]);
            
            cache_.erase(it);
            frequencies_.erase(key);
            key_iterators_.erase(key);
            ttl_entries_.erase(key);
        }
    }
};

// Многоуровневый кэш (L1 + L2)
template<typename Key, typename Value>
class MultiLevelCache : public CacheInterface<Key, Value> {
private:
    std::unique_ptr<CacheInterface<Key, Value>> l1_cache_;  // Быстрый, маленький
    std::unique_ptr<CacheInterface<Key, Value>> l2_cache_; // Медленный, большой
    std::mutex mutex_;
    
    // Статистика
    std::atomic<size_t> l1_hits_{0};
    std::atomic<size_t> l2_hits_{0};
    std::atomic<size_t> misses_{0};
    
public:
    MultiLevelCache(size_t l1_capacity, size_t l2_capacity) {
        l1_cache_ = std::make_unique<LRUCache<Key, Value>>(l1_capacity);
        l2_cache_ = std::make_unique<LFUCache<Key, Value>>(l2_capacity);
        
        std::cout << "MultiLevel Cache создан: L1=" << l1_capacity 
                  << ", L2=" << l2_capacity << std::endl;
    }
    
    bool get(const Key& key, Value& value) override {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // Проверяем L1 кэш
        if (l1_cache_->get(key, value)) {
            l1_hits_.fetch_add(1);
            return true;
        }
        
        // Проверяем L2 кэш
        if (l2_cache_->get(key, value)) {
            l2_hits_.fetch_add(1);
            // Продвигаем в L1 кэш
            l1_cache_->put(key, value);
            return true;
        }
        
        misses_.fetch_add(1);
        return false;
    }
    
    void put(const Key& key, const Value& value) override {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // Записываем в оба кэша
        l1_cache_->put(key, value);
        l2_cache_->put(key, value);
    }
    
    void remove(const Key& key) override {
        std::lock_guard<std::mutex> lock(mutex_);
        l1_cache_->remove(key);
        l2_cache_->remove(key);
    }
    
    void clear() override {
        std::lock_guard<std::mutex> lock(mutex_);
        l1_cache_->clear();
        l2_cache_->clear();
    }
    
    size_t size() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return l1_cache_->size() + l2_cache_->size();
    }
    
    bool contains(const Key& key) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return l1_cache_->contains(key) || l2_cache_->contains(key);
    }
    
    void printStats() const {
        std::cout << "\n=== MultiLevel Cache Statistics ===" << std::endl;
        std::cout << "L1 Hits: " << l1_hits_.load() << std::endl;
        std::cout << "L2 Hits: " << l2_hits_.load() << std::endl;
        std::cout << "Misses: " << misses_.load() << std::endl;
        
        size_t total_requests = l1_hits_.load() + l2_hits_.load() + misses_.load();
        if (total_requests > 0) {
            double hit_rate = (double)(l1_hits_.load() + l2_hits_.load()) / total_requests * 100;
            std::cout << "Hit Rate: " << hit_rate << "%" << std::endl;
        }
        
        l1_cache_->printStats();
        l2_cache_->printStats();
        std::cout << "================================" << std::endl;
    }
};

// Стратегии инвалидации кэша
enum class InvalidationStrategy {
    TIME_BASED,     // По времени
    EVENT_BASED,    // По событиям
    MANUAL          // Ручная
};

// Кэш с различными стратегиями инвалидации
template<typename Key, typename Value>
class SmartCache : public CacheInterface<Key, Value> {
private:
    std::unique_ptr<CacheInterface<Key, Value>> cache_;
    InvalidationStrategy strategy_;
    std::chrono::milliseconds ttl_;
    std::mutex mutex_;
    
    // Event-based инвалидация
    std::unordered_map<Key, std::vector<std::string>> key_tags_;
    std::unordered_map<std::string, std::vector<Key>> tag_keys_;
    
public:
    SmartCache(size_t capacity, InvalidationStrategy strategy, std::chrono::milliseconds ttl)
        : strategy_(strategy), ttl_(ttl) {
        cache_ = std::make_unique<LRUCache<Key, Value>>(capacity);
        std::cout << "Smart Cache создан с стратегией инвалидации: " 
                  << static_cast<int>(strategy) << std::endl;
    }
    
    bool get(const Key& key, Value& value) override {
        std::lock_guard<std::mutex> lock(mutex_);
        return cache_->get(key, value);
    }
    
    void put(const Key& key, const Value& value) override {
        std::lock_guard<std::mutex> lock(mutex_);
        cache_->put(key, value);
    }
    
    void putWithTags(const Key& key, const Value& value, const std::vector<std::string>& tags) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        cache_->put(key, value);
        key_tags_[key] = tags;
        
        for (const auto& tag : tags) {
            tag_keys_[tag].push_back(key);
        }
    }
    
    void remove(const Key& key) override {
        std::lock_guard<std::mutex> lock(mutex_);
        cache_->remove(key);
        
        // Удаляем теги
        auto it = key_tags_.find(key);
        if (it != key_tags_.end()) {
            for (const auto& tag : it->second) {
                auto& keys = tag_keys_[tag];
                keys.erase(std::remove(keys.begin(), keys.end(), key), keys.end());
            }
            key_tags_.erase(it);
        }
    }
    
    void invalidateByTag(const std::string& tag) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = tag_keys_.find(tag);
        if (it != tag_keys_.end()) {
            for (const auto& key : it->second) {
                cache_->remove(key);
            }
            tag_keys_.erase(it);
        }
    }
    
    void clear() override {
        std::lock_guard<std::mutex> lock(mutex_);
        cache_->clear();
        key_tags_.clear();
        tag_keys_.clear();
    }
    
    size_t size() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return cache_->size();
    }
    
    bool contains(const Key& key) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return cache_->contains(key);
    }
    
    void printStats() const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::cout << "Smart Cache: размер=" << cache_->size() 
                  << ", тегов=" << tag_keys_.size() << std::endl;
    }
};

// Демонстрация базового LRU кэша
void demonstrateLRUCache() {
    std::cout << "\n=== Демонстрация LRU Cache ===" << std::endl;
    
    LRUCache<std::string, std::string> cache(3);
    
    // Добавляем элементы
    cache.put("key1", "value1");
    cache.put("key2", "value2");
    cache.put("key3", "value3");
    
    std::string value;
    if (cache.get("key1", value)) {
        std::cout << "Найден key1: " << value << std::endl;
    }
    
    // Добавляем еще один элемент (key1 должен остаться, key2 удалится)
    cache.put("key4", "value4");
    
    std::cout << "После добавления key4:" << std::endl;
    cache.printStats();
    
    if (cache.get("key2", value)) {
        std::cout << "key2 найден: " << value << std::endl;
    } else {
        std::cout << "key2 не найден (вытеснен)" << std::endl;
    }
}

// Демонстрация LFU кэша
void demonstrateLFUCache() {
    std::cout << "\n=== Демонстрация LFU Cache ===" << std::endl;
    
    LFUCache<std::string, std::string> cache(3);
    
    cache.put("key1", "value1");
    cache.put("key2", "value2");
    cache.put("key3", "value3");
    
    // Используем key1 несколько раз
    std::string value;
    cache.get("key1", value);
    cache.get("key1", value);
    cache.get("key1", value);
    
    // Используем key2 один раз
    cache.get("key2", value);
    
    std::cout << "После частого использования key1:" << std::endl;
    cache.printStats();
    
    // Добавляем новый элемент (key3 должен удалиться как наименее используемый)
    cache.put("key4", "value4");
    
    if (cache.get("key3", value)) {
        std::cout << "key3 найден" << std::endl;
    } else {
        std::cout << "key3 не найден (вытеснен как наименее используемый)" << std::endl;
    }
}

// Демонстрация многоуровневого кэша
void demonstrateMultiLevelCache() {
    std::cout << "\n=== Демонстрация MultiLevel Cache ===" << std::endl;
    
    MultiLevelCache<std::string, std::string> cache(2, 5);
    
    // Заполняем кэш
    cache.put("key1", "value1");
    cache.put("key2", "value2");
    cache.put("key3", "value3");
    cache.put("key4", "value4");
    cache.put("key5", "value5");
    
    std::string value;
    
    // Тестируем доступ
    std::cout << "Тестируем доступ к элементам:" << std::endl;
    for (int i = 1; i <= 5; ++i) {
        std::string key = "key" + std::to_string(i);
        if (cache.get(key, value)) {
            std::cout << "Найден " << key << ": " << value << std::endl;
        } else {
            std::cout << key << " не найден" << std::endl;
        }
    }
    
    cache.printStats();
}

// Демонстрация умного кэша с тегами
void demonstrateSmartCache() {
    std::cout << "\n=== Демонстрация Smart Cache с тегами ===" << std::endl;
    
    SmartCache<std::string, std::string> cache(5, InvalidationStrategy::EVENT_BASED, 
                                                std::chrono::minutes(10));
    
    // Добавляем элементы с тегами
    cache.putWithTags("user1", "John Doe", {"users", "active"});
    cache.putWithTags("user2", "Jane Smith", {"users", "inactive"});
    cache.putWithTags("product1", "Laptop", {"products", "electronics"});
    cache.putWithTags("product2", "Book", {"products", "books"});
    
    std::cout << "Кэш заполнен элементами с тегами" << std::endl;
    cache.printStats();
    
    // Инвалидируем по тегу
    std::cout << "Инвалидируем все продукты..." << std::endl;
    cache.invalidateByTag("products");
    
    std::string value;
    if (cache.get("product1", value)) {
        std::cout << "product1 найден: " << value << std::endl;
    } else {
        std::cout << "product1 не найден (инвалидирован)" << std::endl;
    }
    
    if (cache.get("user1", value)) {
        std::cout << "user1 найден: " << value << std::endl;
    }
    
    cache.printStats();
}

// Демонстрация производительности кэша
void demonstrateCachePerformance() {
    std::cout << "\n=== Демонстрация производительности кэша ===" << std::endl;
    
    MultiLevelCache<int, std::string> cache(100, 1000);
    
    // Заполняем кэш
    for (int i = 0; i < 500; ++i) {
        cache.put(i, "value_" + std::to_string(i));
    }
    
    // Тестируем производительность
    auto start = std::chrono::high_resolution_clock::now();
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 999);
    
    std::string value;
    int hits = 0;
    
    for (int i = 0; i < 10000; ++i) {
        int key = dis(gen);
        if (cache.get(key, value)) {
            hits++;
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Выполнено 10000 операций за " << duration.count() << " микросекунд" << std::endl;
    std::cout << "Hits: " << hits << "/10000" << std::endl;
    
    cache.printStats();
}

int main() {
    std::cout << "=== Cache-Aside Pattern ===" << std::endl;
    
    try {
        demonstrateLRUCache();
        demonstrateLFUCache();
        demonstrateMultiLevelCache();
        demonstrateSmartCache();
        demonstrateCachePerformance();
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\n=== Демонстрация завершена ===" << std::endl;
    return 0;
}
