#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <algorithm>
#include <stdexcept>
#include <type_traits>

/**
 * @file modern_adapter.cpp
 * @brief Современные подходы к реализации Adapter паттерна в C++
 * 
 * Этот файл демонстрирует продвинутые техники адаптации
 * с использованием современных возможностей C++17/20.
 */

// ============================================================================
// FUNCTION ADAPTER - АДАПТАЦИЯ ФУНКЦИЙ
// ============================================================================

/**
 * @brief Адаптер для функций с разными сигнатурами
 */
class FunctionAdapter {
public:
    // Адаптация функции с одним параметром
    template<typename Func>
    static auto adaptOne(Func func) {
        return [func](const std::vector<int>& args) -> void {
            if (!args.empty()) {
                func(args[0]);
            } else {
                throw std::invalid_argument("Недостаточно аргументов");
            }
        };
    }
    
    // Адаптация функции с двумя параметрами
    template<typename Func>
    static auto adaptTwo(Func func) {
        return [func](const std::vector<int>& args) -> void {
            if (args.size() >= 2) {
                func(args[0], args[1]);
            } else {
                throw std::invalid_argument("Недостаточно аргументов");
            }
        };
    }
    
    // Адаптация функции с переменным количеством параметров
    template<typename Func>
    static auto adaptVariable(Func func) {
        return [func](const std::vector<int>& args) -> void {
            if (args.size() >= 3) {
                func(args[0], args[1], args[2]);
            } else {
                throw std::invalid_argument("Недостаточно аргументов");
            }
        };
    }
    
    // Адаптация с преобразованием типов
    template<typename Func>
    static auto adaptWithConversion(Func func) {
        return [func](const std::vector<std::string>& args) -> void {
            if (!args.empty()) {
                try {
                    int value = std::stoi(args[0]);
                    func(value);
                } catch (const std::exception&) {
                    throw std::invalid_argument("Не удалось преобразовать аргумент в int");
                }
            }
        };
    }
};

// ============================================================================
// SMART ADAPTER С КЭШИРОВАНИЕМ
// ============================================================================

/**
 * @brief Умный адаптер с кэшированием результатов
 */
template<typename AdapteeType>
class SmartAdapter {
private:
    std::unique_ptr<AdapteeType> adaptee_;
    std::map<std::string, std::string> cache_;
    size_t maxCacheSize_;
    
public:
    explicit SmartAdapter(size_t maxCacheSize = 100) 
        : adaptee_(std::make_unique<AdapteeType>()), maxCacheSize_(maxCacheSize) {}
    
    std::string processRequest(const std::string& request) {
        // Проверяем кэш
        auto cacheIt = cache_.find(request);
        if (cacheIt != cache_.end()) {
            std::cout << "SmartAdapter: Возвращаем результат из кэша" << std::endl;
            return cacheIt->second;
        }
        
        // Обрабатываем запрос
        std::cout << "SmartAdapter: Обрабатываем новый запрос" << std::endl;
        std::string result = adaptee_->process(request);
        
        // Сохраняем в кэш (с проверкой размера)
        if (cache_.size() >= maxCacheSize_) {
            // Удаляем самый старый элемент (простая стратегия)
            cache_.erase(cache_.begin());
        }
        cache_[request] = result;
        
        return result;
    }
    
    void clearCache() {
        cache_.clear();
        std::cout << "SmartAdapter: Кэш очищен" << std::endl;
    }
    
    size_t getCacheSize() const {
        return cache_.size();
    }
    
    void setMaxCacheSize(size_t size) {
        maxCacheSize_ = size;
        // Если текущий кэш больше нового размера, очищаем лишние элементы
        while (cache_.size() > maxCacheSize_) {
            cache_.erase(cache_.begin());
        }
    }
};

/**
 * @brief Пример адаптируемого класса
 */
class LegacyProcessor {
public:
    std::string process(const std::string& request) {
        // Имитация обработки
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return "Processed: " + request;
    }
};

// ============================================================================
// TWO-WAY ADAPTER - ДВУНАПРАВЛЕННАЯ АДАПТАЦИЯ
// ============================================================================

/**
 * @brief Интерфейс для системы A
 */
class SystemA {
public:
    virtual ~SystemA() = default;
    virtual void methodA(const std::string& data) = 0;
};

/**
 * @brief Интерфейс для системы B
 */
class SystemB {
public:
    virtual ~SystemB() = default;
    virtual void methodB(const std::vector<int>& data) = 0;
};

/**
 * @brief Конкретная реализация системы A
 */
class ConcreteSystemA : public SystemA {
public:
    void methodA(const std::string& data) override {
        std::cout << "SystemA: Обрабатываем строку: " << data << std::endl;
    }
};

/**
 * @brief Конкретная реализация системы B
 */
class ConcreteSystemB : public SystemB {
public:
    void methodB(const std::vector<int>& data) override {
        std::cout << "SystemB: Обрабатываем вектор: ";
        for (int value : data) {
            std::cout << value << " ";
        }
        std::cout << std::endl;
    }
};

/**
 * @brief Двунаправленный адаптер
 */
class TwoWayAdapter : public SystemA, public SystemB {
private:
    std::unique_ptr<ConcreteSystemA> systemA_;
    std::unique_ptr<ConcreteSystemB> systemB_;
    
public:
    TwoWayAdapter() 
        : systemA_(std::make_unique<ConcreteSystemA>()), 
          systemB_(std::make_unique<ConcreteSystemB>()) {}
    
    // Реализация интерфейса SystemA
    void methodA(const std::string& data) override {
        std::cout << "TwoWayAdapter: Адаптируем для SystemA" << std::endl;
        systemA_->methodA(data);
    }
    
    // Реализация интерфейса SystemB
    void methodB(const std::vector<int>& data) override {
        std::cout << "TwoWayAdapter: Адаптируем для SystemB" << std::endl;
        systemB_->methodB(data);
    }
    
    // Дополнительные методы для взаимодействия между системами
    void adaptAtoB(const std::string& data) {
        std::cout << "TwoWayAdapter: Адаптируем из A в B" << std::endl;
        // Преобразуем строку в вектор целых чисел
        std::vector<int> convertedData;
        for (char c : data) {
            convertedData.push_back(static_cast<int>(c));
        }
        systemB_->methodB(convertedData);
    }
    
    void adaptBtoA(const std::vector<int>& data) {
        std::cout << "TwoWayAdapter: Адаптируем из B в A" << std::endl;
        // Преобразуем вектор в строку
        std::string convertedData;
        for (int value : data) {
            convertedData += static_cast<char>(value);
        }
        systemA_->methodA(convertedData);
    }
};

// ============================================================================
// TYPE-ERASED ADAPTER - АДАПТЕР С СТИРАНИЕМ ТИПОВ
// ============================================================================

/**
 * @brief Адаптер с использованием type erasure
 */
class TypeErasedAdapter {
private:
    struct Concept {
        virtual ~Concept() = default;
        virtual void process(const std::string& data) = 0;
        virtual std::string getResult() const = 0;
    };
    
    template<typename T>
    struct Model : Concept {
        T adaptee_;
        
        template<typename... Args>
        explicit Model(Args&&... args) : adaptee_(std::forward<Args>(args)...) {}
        
        void process(const std::string& data) override {
            adaptee_.process(data);
        }
        
        std::string getResult() const override {
            return adaptee_.getResult();
        }
    };
    
    std::unique_ptr<Concept> concept_;
    
public:
    template<typename T, typename... Args>
    explicit TypeErasedAdapter(Args&&... args) 
        : concept_(std::make_unique<Model<T>>(std::forward<Args>(args)...)) {}
    
    void process(const std::string& data) {
        concept_->process(data);
    }
    
    std::string getResult() const {
        return concept_->getResult();
    }
};

/**
 * @brief Пример адаптируемого класса для type erasure
 */
class LegacyStringProcessor {
private:
    std::string result_;
    
public:
    void process(const std::string& data) {
        result_ = "Legacy processed: " + data;
    }
    
    std::string getResult() const {
        return result_;
    }
};

/**
 * @brief Другой пример адаптируемого класса
 */
class ModernStringProcessor {
private:
    std::string result_;
    
public:
    void process(const std::string& data) {
        result_ = "Modern processed: " + data;
    }
    
    std::string getResult() const {
        return result_;
    }
};

// ============================================================================
// POLICY-BASED ADAPTER
// ============================================================================

/**
 * @brief Политика для кэширования
 */
struct NoCache {
    void cache(const std::string& key, const std::string& value) {}
    std::optional<std::string> get(const std::string& key) { return std::nullopt; }
    void clear() {}
};

struct SimpleCache {
    std::map<std::string, std::string> cache_;
    
    void cache(const std::string& key, const std::string& value) {
        cache_[key] = value;
    }
    
    std::optional<std::string> get(const std::string& key) {
        auto it = cache_.find(key);
        return (it != cache_.end()) ? std::make_optional(it->second) : std::nullopt;
    }
    
    void clear() {
        cache_.clear();
    }
};

/**
 * @brief Адаптер на основе политик
 */
template<typename AdapteeType, typename CachePolicy = NoCache>
class PolicyBasedAdapter {
private:
    AdapteeType adaptee_;
    CachePolicy cachePolicy_;
    
public:
    template<typename... Args>
    explicit PolicyBasedAdapter(Args&&... args) 
        : adaptee_(std::forward<Args>(args)...) {}
    
    std::string process(const std::string& request) {
        // Проверяем кэш
        auto cached = cachePolicy_.get(request);
        if (cached) {
            std::cout << "PolicyBasedAdapter: Результат из кэша" << std::endl;
            return *cached;
        }
        
        // Обрабатываем запрос
        std::cout << "PolicyBasedAdapter: Обрабатываем запрос" << std::endl;
        std::string result = adaptee_.process(request);
        
        // Сохраняем в кэш
        cachePolicy_.cache(request, result);
        
        return result;
    }
    
    void clearCache() {
        cachePolicy_.clear();
    }
};

// ============================================================================
// ДЕМОНСТРАЦИОННЫЕ ФУНКЦИИ
// ============================================================================

/**
 * @brief Демонстрация адаптера функций
 */
void demonstrateFunctionAdapter() {
    std::cout << "\n=== Адаптер функций ===" << std::endl;
    
    // Legacy функции
    auto legacyFunction1 = [](int x) {
        std::cout << "Legacy function 1: " << x << std::endl;
    };
    
    auto legacyFunction2 = [](int x, int y) {
        std::cout << "Legacy function 2: " << x << ", " << y << std::endl;
    };
    
    auto legacyFunction3 = [](int x, int y, int z) {
        std::cout << "Legacy function 3: " << x << ", " << y << ", " << z << std::endl;
    };
    
    // Адаптируем функции
    auto adapted1 = FunctionAdapter::adaptOne(legacyFunction1);
    auto adapted2 = FunctionAdapter::adaptTwo(legacyFunction2);
    auto adapted3 = FunctionAdapter::adaptVariable(legacyFunction3);
    
    // Используем адаптированные функции
    std::vector<int> args1 = {10};
    std::vector<int> args2 = {20, 30};
    std::vector<int> args3 = {40, 50, 60};
    
    adapted1(args1);
    adapted2(args2);
    adapted3(args3);
    
    // Адаптация с преобразованием типов
    auto legacyStringFunction = [](int value) {
        std::cout << "Legacy string function: " << value << std::endl;
    };
    
    auto adaptedString = FunctionAdapter::adaptWithConversion(legacyStringFunction);
    std::vector<std::string> stringArgs = {"123"};
    adaptedString(stringArgs);
}

/**
 * @brief Демонстрация умного адаптера
 */
void demonstrateSmartAdapter() {
    std::cout << "\n=== Умный адаптер с кэшированием ===" << std::endl;
    
    SmartAdapter<LegacyProcessor> adapter(3); // Максимум 3 элемента в кэше
    
    // Первые запросы - будут обработаны
    std::cout << "Результат 1: " << adapter.processRequest("request1") << std::endl;
    std::cout << "Результат 2: " << adapter.processRequest("request2") << std::endl;
    std::cout << "Результат 3: " << adapter.processRequest("request3") << std::endl;
    
    // Повторные запросы - будут взяты из кэша
    std::cout << "Результат 1 (из кэша): " << adapter.processRequest("request1") << std::endl;
    std::cout << "Результат 2 (из кэша): " << adapter.processRequest("request2") << std::endl;
    
    // Новый запрос - вытеснит старый из кэша
    std::cout << "Результат 4: " << adapter.processRequest("request4") << std::endl;
    
    std::cout << "Размер кэша: " << adapter.getCacheSize() << std::endl;
    
    adapter.clearCache();
    std::cout << "Размер кэша после очистки: " << adapter.getCacheSize() << std::endl;
}

/**
 * @brief Демонстрация двунаправленного адаптера
 */
void demonstrateTwoWayAdapter() {
    std::cout << "\n=== Двунаправленный адаптер ===" << std::endl;
    
    TwoWayAdapter adapter;
    
    // Используем как SystemA
    adapter.methodA("Hello from SystemA");
    
    // Используем как SystemB
    adapter.methodB({1, 2, 3, 4, 5});
    
    // Адаптируем из A в B
    adapter.adaptAtoB("ABC");
    
    // Адаптируем из B в A
    adapter.adaptBtoA({72, 101, 108, 108, 111}); // "Hello" в ASCII
}

/**
 * @brief Демонстрация адаптера с type erasure
 */
void demonstrateTypeErasedAdapter() {
    std::cout << "\n=== Адаптер с type erasure ===" << std::endl;
    
    // Создаем адаптеры для разных типов
    TypeErasedAdapter legacyAdapter(LegacyStringProcessor{});
    TypeErasedAdapter modernAdapter(ModernStringProcessor{});
    
    std::vector<TypeErasedAdapter*> adapters = {&legacyAdapter, &modernAdapter};
    
    std::string testData = "test data";
    
    for (size_t i = 0; i < adapters.size(); ++i) {
        std::cout << "\nАдаптер " << (i + 1) << ":" << std::endl;
        adapters[i]->process(testData);
        std::cout << "Результат: " << adapters[i]->getResult() << std::endl;
    }
}

/**
 * @brief Демонстрация адаптера на основе политик
 */
void demonstratePolicyBasedAdapter() {
    std::cout << "\n=== Адаптер на основе политик ===" << std::endl;
    
    // Адаптер без кэширования
    PolicyBasedAdapter<LegacyProcessor, NoCache> noCacheAdapter;
    std::cout << "Без кэширования:" << std::endl;
    std::cout << "Результат: " << noCacheAdapter.process("request1") << std::endl;
    std::cout << "Результат: " << noCacheAdapter.process("request1") << std::endl;
    
    std::cout << "\nС кэшированием:" << std::endl;
    // Адаптер с кэшированием
    PolicyBasedAdapter<LegacyProcessor, SimpleCache> cachedAdapter;
    std::cout << "Результат: " << cachedAdapter.process("request1") << std::endl;
    std::cout << "Результат: " << cachedAdapter.process("request1") << std::endl;
    
    cachedAdapter.clearCache();
    std::cout << "Кэш очищен" << std::endl;
}

// ============================================================================
// ОСНОВНАЯ ФУНКЦИЯ
// ============================================================================

int main() {
    std::cout << "🚀 Современные подходы к Adapter Pattern" << std::endl;
    std::cout << std::string(50, '=') << std::endl;
    
    demonstrateFunctionAdapter();
    demonstrateSmartAdapter();
    demonstrateTwoWayAdapter();
    demonstrateTypeErasedAdapter();
    demonstratePolicyBasedAdapter();
    
    std::cout << "\n✅ Демонстрация современных подходов завершена!" << std::endl;
    std::cout << "\n🎯 Ключевые выводы:" << std::endl;
    std::cout << "• Function Adapter адаптирует функции с разными сигнатурами" << std::endl;
    std::cout << "• Smart Adapter добавляет кэширование и оптимизацию" << std::endl;
    std::cout << "• Two-way Adapter обеспечивает двунаправленную адаптацию" << std::endl;
    std::cout << "• Type Erasure позволяет работать с разными типами единообразно" << std::endl;
    std::cout << "• Policy-based Adapter обеспечивает гибкую настройку поведения" << std::endl;
    
    return 0;
}
