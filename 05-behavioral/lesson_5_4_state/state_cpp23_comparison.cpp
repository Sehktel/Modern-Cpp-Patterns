// C++17/20 vs C++23: State Pattern
// C++23: std::expected для state transitions, std::print

#include <iostream>
#include <memory>
#include <string>

#if __cplusplus >= 202302L
    #include <expected>
    #include <print>
    #define HAS_CPP23 1
    using std::expected; using std::unexpected;
#else
    #define HAS_CPP23 0
    template<typename T, typename E> struct expected { T v; bool has_value() const { return true; } T& operator*() { return v; } };
    template<typename E> struct unexpected { E e; };
#endif

class State { public: virtual ~State() = default; virtual void handle() = 0; };
class StateA : public State { public: void handle() override { std::cout << "State A\n"; } };
class StateB : public State { public: void handle() override { std::cout << "State B\n"; } };

namespace cpp17 {
    class Context {
        std::unique_ptr<State> state_;
    public:
        Context() : state_(std::make_unique<StateA>()) {}
        
        void setState(std::unique_ptr<State> s) {
            state_ = std::move(s);  // ❌ Может нарушить FSM
        }
        
        void request() { state_->handle(); }
    };
}

namespace cpp23 {
    enum class TransitionError { InvalidTransition, StateNotSet };
    
    class Context {
        std::unique_ptr<State> state_;
        
        // ✅ Transition validation
        bool isValidTransition(State* from, State* to) {
            // FSM validation logic
            return true;  // Simplified
        }
        
    public:
        Context() : state_(std::make_unique<StateA>()) {}
        
        // ✅ expected для validated transitions
        expected<void, TransitionError> setState(std::unique_ptr<State> new_state) {
            if (!new_state) {
                return unexpected(TransitionError::StateNotSet);
            }
            
            if (!isValidTransition(state_.get(), new_state.get())) {
#if HAS_CPP23
                std::print("Invalid state transition\n");
#endif
                return unexpected(TransitionError::InvalidTransition);
            }
            
            state_ = std::move(new_state);
            return {};
        }
        
        void request() { state_->handle(); }
    };
}

int main() {
    cpp23::Context ctx;
    
    auto result = ctx.setState(std::make_unique<StateB>());
    if (result.has_value()) {
        ctx.request();
    }
    
    std::cout << "✅ C++23: expected для validated state transitions\n";
    return 0;
}

