// C++17/20 vs C++23: Command Pattern
// C++23: std::generator для command iteration, std::jthread для async execution

#include <iostream>
#include <queue>
#include <memory>
#include <thread>

#if __cplusplus >= 202302L
    #include <expected>
    #include <print>
    #include <generator>
    #define HAS_CPP23 1
    using std::expected; using std::unexpected;
#else
    #define HAS_CPP23 0
    template<typename T, typename E> struct expected { T v; bool has_value() const { return true; } T& operator*() { return v; } };
#endif

class Command { public: virtual ~Command() = default; virtual void execute() = 0; };
class PrintCommand : public Command { 
    int value_;
public: 
    PrintCommand(int v) : value_(v) {}
    void execute() override { std::cout << "Execute: " << value_ << "\n"; } 
};

namespace cpp17 {
    class CommandQueue {
        std::queue<std::unique_ptr<Command>> commands_;
    public:
        void enqueue(std::unique_ptr<Command> cmd) {
            commands_.push(std::move(cmd));
        }
        
        void executeAll() {
            while (!commands_.empty()) {
                commands_.front()->execute();
                commands_.pop();
            }
        }
        
        // ❌ std::thread для async
        void executeAsync() {
            std::thread t([this]() {
                executeAll();
            });
            t.join();
        }
    };
}

namespace cpp23 {
    enum class QueueError { Empty, Full };
    
    class CommandQueue {
        std::queue<std::unique_ptr<Command>> commands_;
    public:
        void enqueue(std::unique_ptr<Command> cmd) {
            commands_.push(std::move(cmd));
        }
        
#if HAS_CPP23
        // ✅ Generator для lazy iteration
        std::generator<Command*> iterate() {
            while (!commands_.empty()) {
                co_yield commands_.front().get();
                commands_.front()->execute();
                commands_.pop();
            }
        }
        
        // ✅ jthread для async execution
        void executeAsync() {
            std::jthread t([this](std::stop_token stoken) {
                while (!commands_.empty() && !stoken.stop_requested()) {
                    commands_.front()->execute();
                    commands_.pop();
                }
            });
            // ✅ Auto-join
        }
#endif
    };
}

int main() {
    cpp23::CommandQueue queue;
    queue.enqueue(std::make_unique<PrintCommand>(1));
    queue.enqueue(std::make_unique<PrintCommand>(2));
    
#if HAS_CPP23
    std::println("C++23: Generator для command iteration");
    for (auto* cmd : queue.iterate()) {
        // Lazy iteration
    }
#endif
    
    std::cout << "✅ C++23: generator + jthread для commands\n";
    return 0;
}

