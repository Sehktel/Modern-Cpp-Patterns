/**
 * @file multi_level_cache.cpp
 * @brief Многоуровневый кэш для Cache-Aside Pattern
 * 
 * Реализован многоуровневый кэш с поддержкой:
 * - L1 кэш (in-memory)
 * - L2 кэш (Redis-like)
 * - Стратегии промотирования
 * - Консистентность между уровнями
 */

#include <iostream>
#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <chrono>
#include <optional>
#include <queue>
#include <atomic>
#include <thread>
#include <list>

// Политики вытеснения кэша
enum class EvictionPolicy {
    LRU,  // Least Recently Used
    LFU,  // Least Frequently Used
    FIFO  // First In First Out
};

// Запись кэша с метаданными
template<typename T>
struct CacheEntry {
    T value;
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point last_accessed;
    size_t access_count;
    size_t size_bytes;
    
    CacheEntry(T val, size_t size = 0) 
        : value(std::move(val)), 
          created_at(std::chrono::system_clock::now()),
          last_accessed(std::chrono::system_clock::now()),
          access_count(1),
          size_bytes(size) {}
    
    void touch() {
        last_accessed = std::chrono::system_clock::now();
        access_count++;
    }
};

// Базовый интерфейс для уровня кэша
template<typename K, typename V>
class CacheLevel {
public:
    virtual ~CacheLevel() = default;
    virtual std::optional<V> get(const K& key) = 0;
    virtual void put(const K& key, const V& value, size_t size = 0) = 0;
    virtual void remove(const K& key) = 0;
    virtual void clear() = 0;
    virtual size_t size() const = 0;
    virtual std::string getName() const = 0;
    virtual void printStats() const = 0;
};

// L1 кэш - быстрый in-memory кэш с LRU вытеснением
template<typename K, typename V>
class L1Cache : public CacheLevel<K, V> {
private:
    size_t max_size_;
    size_t max_memory_bytes_;
    std::atomic<size_t> current_memory_{0};
    
    // Используем list для LRU порядка и map для быстрого доступа
    std::list<K> lru_list_;
    std::unordered_map<K, typename std::list<K>::iterator> lru_map_;
    std::unordered_map<K, CacheEntry<V>> cache_;
    
    mutable std::mutex mutex_;
    
    // Статистика
    std::atomic<size_t> hits_{0};
    std::atomic<size_t> misses_{0};
    std::atomic<size_t> evictions_{0};
    
public:
    L1Cache(size_t max_size, size_t max_memory_mb = 100) 
        : max_size_(max_size), 
          max_memory_bytes_(max_memory_mb * 1024 * 1024) {
        std::cout << "L1 Cache создан (макс. размер: " << max_size 
                  << ", макс. память: " << max_memory_mb << " MB)" << std::endl;
    }
    
    std::optional<V> get(const K& key) override {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = cache_.find(key);
        if (it != cache_.end()) {
            // Обновляем LRU
            touchLRU(key);
            it->second.touch();
            hits_.fetch_add(1);
            return it->second.value;
        }
        
        misses_.fetch_add(1);
        return std::nullopt;
    }
    
    void put(const K& key, const V& value, size_t size = 0) override {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // Если ключ уже существует, обновляем
        auto it = cache_.find(key);
        if (it != cache_.end()) {
            size_t old_size = it->second.size_bytes;
            current_memory_.fetch_sub(old_size);
            
            it->second = CacheEntry<V>(value, size);
            current_memory_.fetch_add(size);
            touchLRU(key);
            return;
        }
        
        // Вытесняем, если нужно
        while ((cache_.size() >= max_size_ || 
                current_memory_.load() + size > max_memory_bytes_) && 
               !lru_list_.empty()) {
            evictLRU();
        }
        
        // Добавляем новую запись
        cache_.emplace(key, CacheEntry<V>(value, size));
        lru_list_.push_front(key);
        lru_map_[key] = lru_list_.begin();
        current_memory_.fetch_add(size);
    }
    
    void remove(const K& key) override {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = cache_.find(key);
        if (it != cache_.end()) {
            current_memory_.fetch_sub(it->second.size_bytes);
            cache_.erase(it);
            
            auto lru_it = lru_map_.find(key);
            if (lru_it != lru_map_.end()) {
                lru_list_.erase(lru_it->second);
                lru_map_.erase(lru_it);
            }
        }
    }
    
    void clear() override {
        std::lock_guard<std::mutex> lock(mutex_);
        cache_.clear();
        lru_list_.clear();
        lru_map_.clear();
        current_memory_.store(0);
    }
    
    size_t size() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return cache_.size();
    }
    
    std::string getName() const override {
        return "L1 Cache";
    }
    
    void printStats() const override {
        size_t total = hits_.load() + misses_.load();
        double hit_rate = total > 0 ? (100.0 * hits_.load() / total) : 0.0;
        
        std::cout << "\n=== " << getName() << " Statistics ===" << std::endl;
        std::cout << "Размер: " << size() << " / " << max_size_ << std::endl;
        std::cout << "Память: " << (current_memory_.load() / 1024) << " KB / " 
                  << (max_memory_bytes_ / 1024 / 1024) << " MB" << std::endl;
        std::cout << "Hits: " << hits_.load() << std::endl;
        std::cout << "Misses: " << misses_.load() << std::endl;
        std::cout << "Hit Rate: " << hit_rate << "%" << std::endl;
        std::cout << "Evictions: " << evictions_.load() << std::endl;
        std::cout << "================================" << std::endl;
    }
    
private:
    void touchLRU(const K& key) {
        // Перемещаем ключ в начало LRU списка
        auto it = lru_map_.find(key);
        if (it != lru_map_.end()) {
            lru_list_.erase(it->second);
            lru_list_.push_front(key);
            lru_map_[key] = lru_list_.begin();
        }
    }
    
    void evictLRU() {
        if (lru_list_.empty()) return;
        
        // Удаляем последний элемент (наименее недавно использованный)
        K key = lru_list_.back();
        lru_list_.pop_back();
        lru_map_.erase(key);
        
        auto it = cache_.find(key);
        if (it != cache_.end()) {
            current_memory_.fetch_sub(it->second.size_bytes);
            cache_.erase(it);
            evictions_.fetch_add(1);
            
            std::cout << "L1 Cache: вытеснен ключ " << key << std::endl;
        }
    }
};

// L2 кэш - более медленный, но более вместительный кэш (имитация Redis)
template<typename K, typename V>
class L2Cache : public CacheLevel<K, V> {
private:
    size_t max_size_;
    std::unordered_map<K, CacheEntry<V>> cache_;
    mutable std::mutex mutex_;
    
    // Статистика
    std::atomic<size_t> hits_{0};
    std::atomic<size_t> misses_{0};
    std::atomic<size_t> evictions_{0};
    
    // Симуляция сетевой задержки
    std::chrono::milliseconds latency_;
    
public:
    L2Cache(size_t max_size, std::chrono::milliseconds latency = std::chrono::milliseconds(10)) 
        : max_size_(max_size), latency_(latency) {
        std::cout << "L2 Cache создан (макс. размер: " << max_size 
                  << ", задержка: " << latency.count() << " ms)" << std::endl;
    }
    
    std::optional<V> get(const K& key) override {
        // Имитация сетевой задержки
        std::this_thread::sleep_for(latency_);
        
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = cache_.find(key);
        if (it != cache_.end()) {
            it->second.touch();
            hits_.fetch_add(1);
            return it->second.value;
        }
        
        misses_.fetch_add(1);
        return std::nullopt;
    }
    
    void put(const K& key, const V& value, size_t size = 0) override {
        // Имитация сетевой задержки
        std::this_thread::sleep_for(latency_);
        
        std::lock_guard<std::mutex> lock(mutex_);
        
        // Простое FIFO вытеснение
        if (cache_.size() >= max_size_ && cache_.find(key) == cache_.end()) {
            // Находим самую старую запись
            auto oldest_it = cache_.begin();
            for (auto it = cache_.begin(); it != cache_.end(); ++it) {
                if (it->second.created_at < oldest_it->second.created_at) {
                    oldest_it = it;
                }
            }
            
            if (oldest_it != cache_.end()) {
                std::cout << "L2 Cache: вытеснен ключ " << oldest_it->first << std::endl;
                cache_.erase(oldest_it);
                evictions_.fetch_add(1);
            }
        }
        
        cache_[key] = CacheEntry<V>(value, size);
    }
    
    void remove(const K& key) override {
        std::lock_guard<std::mutex> lock(mutex_);
        cache_.erase(key);
    }
    
    void clear() override {
        std::lock_guard<std::mutex> lock(mutex_);
        cache_.clear();
    }
    
    size_t size() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return cache_.size();
    }
    
    std::string getName() const override {
        return "L2 Cache (Redis-like)";
    }
    
    void printStats() const override {
        size_t total = hits_.load() + misses_.load();
        double hit_rate = total > 0 ? (100.0 * hits_.load() / total) : 0.0;
        
        std::cout << "\n=== " << getName() << " Statistics ===" << std::endl;
        std::cout << "Размер: " << size() << " / " << max_size_ << std::endl;
        std::cout << "Hits: " << hits_.load() << std::endl;
        std::cout << "Misses: " << misses_.load() << std::endl;
        std::cout << "Hit Rate: " << hit_rate << "%" << std::endl;
        std::cout << "Evictions: " << evictions_.load() << std::endl;
        std::cout << "================================" << std::endl;
    }
};

// Многоуровневый кэш с промотированием
template<typename K, typename V>
class MultiLevelCache {
private:
    std::shared_ptr<L1Cache<K, V>> l1_cache_;
    std::shared_ptr<L2Cache<K, V>> l2_cache_;
    
    // Функция загрузки данных из основного хранилища
    std::function<V(const K&)> data_loader_;
    
    // Статистика
    std::atomic<size_t> l1_hits_{0};
    std::atomic<size_t> l2_hits_{0};
    std::atomic<size_t> misses_{0};
    std::atomic<size_t> promotions_{0};
    
public:
    MultiLevelCache(size_t l1_size, size_t l2_size, 
                    std::function<V(const K&)> loader,
                    size_t l1_memory_mb = 100)
        : l1_cache_(std::make_shared<L1Cache<K, V>>(l1_size, l1_memory_mb)),
          l2_cache_(std::make_shared<L2Cache<K, V>>(l2_size)),
          data_loader_(std::move(loader)) {
        std::cout << "Многоуровневый кэш создан" << std::endl;
    }
    
    // Получение значения с автоматическим промотированием
    V get(const K& key) {
        // 1. Пытаемся получить из L1
        auto l1_result = l1_cache_->get(key);
        if (l1_result.has_value()) {
            l1_hits_.fetch_add(1);
            std::cout << "L1 HIT: " << key << std::endl;
            return l1_result.value();
        }
        
        // 2. Пытаемся получить из L2
        auto l2_result = l2_cache_->get(key);
        if (l2_result.has_value()) {
            l2_hits_.fetch_add(1);
            std::cout << "L2 HIT: " << key << " (промотирование в L1)" << std::endl;
            
            // Промотирование в L1
            l1_cache_->put(key, l2_result.value());
            promotions_.fetch_add(1);
            
            return l2_result.value();
        }
        
        // 3. Загружаем из основного хранилища
        misses_.fetch_add(1);
        std::cout << "MISS: " << key << " (загрузка из БД)" << std::endl;
        
        V value = data_loader_(key);
        
        // Сохраняем на всех уровнях
        l2_cache_->put(key, value);
        l1_cache_->put(key, value);
        
        return value;
    }
    
    // Запись значения на всех уровнях
    void put(const K& key, const V& value) {
        l1_cache_->put(key, value);
        l2_cache_->put(key, value);
    }
    
    // Инвалидация на всех уровнях
    void invalidate(const K& key) {
        std::cout << "Инвалидация ключа: " << key << std::endl;
        l1_cache_->remove(key);
        l2_cache_->remove(key);
    }
    
    // Очистка всех уровней
    void clear() {
        l1_cache_->clear();
        l2_cache_->clear();
    }
    
    // Вывод статистики
    void printStats() const {
        size_t total = l1_hits_.load() + l2_hits_.load() + misses_.load();
        double overall_hit_rate = total > 0 
            ? (100.0 * (l1_hits_.load() + l2_hits_.load()) / total) 
            : 0.0;
        
        std::cout << "\n=== Multi-Level Cache Statistics ===" << std::endl;
        std::cout << "L1 Hits: " << l1_hits_.load() << std::endl;
        std::cout << "L2 Hits: " << l2_hits_.load() << std::endl;
        std::cout << "Total Misses: " << misses_.load() << std::endl;
        std::cout << "Overall Hit Rate: " << overall_hit_rate << "%" << std::endl;
        std::cout << "Promotions (L2->L1): " << promotions_.load() << std::endl;
        std::cout << "=====================================" << std::endl;
        
        l1_cache_->printStats();
        l2_cache_->printStats();
    }
};

// Имитация базы данных
class Database {
private:
    std::unordered_map<std::string, std::string> data_;
    mutable std::mutex mutex_;
    std::atomic<size_t> queries_{0};
    
public:
    Database() {
        // Заполняем тестовыми данными
        for (int i = 0; i < 1000; ++i) {
            std::string key = "user_" + std::to_string(i);
            std::string value = "UserData_" + std::to_string(i);
            data_[key] = value;
        }
        std::cout << "База данных создана с " << data_.size() << " записями" << std::endl;
    }
    
    std::string query(const std::string& key) {
        // Имитация медленного запроса к БД
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        
        queries_.fetch_add(1);
        
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = data_.find(key);
        if (it != data_.end()) {
            return it->second;
        }
        
        return "NOT_FOUND";
    }
    
    size_t getQueryCount() const {
        return queries_.load();
    }
};

// Демонстрация работы многоуровневого кэша
void demonstrateMultiLevelCache() {
    std::cout << "\n=== Демонстрация многоуровневого кэша ===" << std::endl;
    
    Database db;
    
    // Создаем многоуровневый кэш
    MultiLevelCache<std::string, std::string> cache(
        10,  // L1 size
        50,  // L2 size
        [&db](const std::string& key) {
            return db.query(key);
        },
        1  // L1 memory limit in MB
    );
    
    // Паттерн доступа: сначала холодный кэш, затем горячие данные
    std::vector<std::string> keys;
    for (int i = 0; i < 20; ++i) {
        keys.push_back("user_" + std::to_string(i));
    }
    
    std::cout << "\n--- Первичный доступ (холодный кэш) ---" << std::endl;
    for (const auto& key : keys) {
        auto value = cache.get(key);
    }
    
    std::cout << "\n--- Повторный доступ к горячим данным ---" << std::endl;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 5; ++j) {
            auto value = cache.get("user_" + std::to_string(j));
        }
    }
    
    std::cout << "\n--- Доступ к новым данным (вытеснение L1) ---" << std::endl;
    for (int i = 20; i < 35; ++i) {
        auto value = cache.get("user_" + std::to_string(i));
    }
    
    std::cout << "\n--- Возврат к старым горячим данным (промотирование) ---" << std::endl;
    for (int i = 0; i < 5; ++i) {
        auto value = cache.get("user_" + std::to_string(i));
    }
    
    cache.printStats();
    std::cout << "\nВсего запросов к БД: " << db.getQueryCount() << std::endl;
}

// Демонстрация инвалидации
void demonstrateCacheInvalidation() {
    std::cout << "\n=== Демонстрация инвалидации кэша ===" << std::endl;
    
    Database db;
    
    MultiLevelCache<std::string, std::string> cache(
        5,  // L1 size
        10,  // L2 size
        [&db](const std::string& key) {
            return db.query(key);
        }
    );
    
    // Загружаем данные
    std::cout << "\n--- Загрузка данных ---" << std::endl;
    for (int i = 0; i < 5; ++i) {
        std::string key = "user_" + std::to_string(i);
        auto value = cache.get(key);
    }
    
    // Инвалидируем некоторые ключи
    std::cout << "\n--- Инвалидация ---" << std::endl;
    cache.invalidate("user_1");
    cache.invalidate("user_3");
    
    // Повторный доступ
    std::cout << "\n--- Повторный доступ после инвалидации ---" << std::endl;
    for (int i = 0; i < 5; ++i) {
        std::string key = "user_" + std::to_string(i);
        auto value = cache.get(key);
    }
    
    cache.printStats();
}

int main() {
    std::cout << "=== Multi-Level Cache Pattern ===" << std::endl;
    
    try {
        demonstrateMultiLevelCache();
        demonstrateCacheInvalidation();
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\n=== Демонстрация завершена ===" << std::endl;
    return 0;
}
