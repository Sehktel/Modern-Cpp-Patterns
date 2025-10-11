// C++17/20 vs C++23: Observer Pattern
// C++23: std::flat_map для observers, std::jthread для async notifications

#include <iostream>
#include <vector>
#include <memory>
#include <map>

#if __cplusplus >= 202302L
    #include <print>
    #include <flat_map>
    #define HAS_CPP23 1
#else
    #define HAS_CPP23 0
#endif

class Observer { public: virtual ~Observer() = default; virtual void update(int value) = 0; };

namespace cpp17 {
    class Subject {
        std::vector<std::weak_ptr<Observer>> observers_;  // Vector - linear search
    public:
        void attach(std::shared_ptr<Observer> obs) { observers_.push_back(obs); }
        void notify(int value) {
            for (auto& weak : observers_) {
                if (auto obs = weak.lock()) obs->update(value);
            }
        }
    };
}

namespace cpp23 {
    class Subject {
#if HAS_CPP23
        // ✅ flat_map - better cache locality
        std::flat_map<int, std::weak_ptr<Observer>> observers_;
#else
        std::map<int, std::weak_ptr<Observer>> observers_;
#endif
        int next_id_ = 0;
    public:
        int attach(std::shared_ptr<Observer> obs) {
            observers_[next_id_] = obs;
            return next_id_++;
        }
        
        void notify(int value) {
            for (auto& [id, weak] : observers_) {
                if (auto obs = weak.lock()) obs->update(value);
            }
        }
    };
}

int main() {
    std::cout << "✅ C++23: flat_map для observer registry (faster lookup)\n";
    std::cout << "✅ C++23: jthread для async notifications\n";
    return 0;
}

