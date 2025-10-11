// C++17/20 vs C++23: Command Queue
// C++23: std::jthread, std::generator, std::expected

#include <iostream>
#include <queue>
#include <thread>
#include <mutex>

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
    void execute() override { std::cout << "Command: " << value_ << "\n"; }
};

namespace cpp17 {
    class CommandQueue {
        std::queue<std::unique_ptr<Command>> queue_;
        std::thread worker_;
        bool running_ = true;
    public:
        CommandQueue() {
            worker_ = std::thread([this]() {  // ❌ Manual thread
                while (running_) {
                    // Process commands
                }
            });
        }
        
        ~CommandQueue() {
            running_ = false;
            worker_.join();  // ❌ Manual join
        }
    };
}

namespace cpp23 {
    enum class QueueError { Full, Empty };
    
    class CommandQueue {
        std::queue<std::unique_ptr<Command>> queue_;
#if HAS_CPP23
        std::jthread worker_;  // ✅ Auto-join
#else
        std::thread worker_;
        bool running_ = true;
#endif
        const size_t MAX_SIZE = 1000;
        
    public:
        CommandQueue() {
#if HAS_CPP23
            worker_ = std::jthread([this](std::stop_token stoken) {
                while (!stoken.stop_requested()) {
                    // Process commands
                }
            });
#endif
        }
        
        // ✅ expected для queue operations
        expected<void, QueueError> enqueue(std::unique_ptr<Command> cmd) {
            if (queue_.size() >= MAX_SIZE) {
                return unexpected(QueueError::Full);
            }
            queue_.push(std::move(cmd));
            return {};
        }
        
#if HAS_CPP23
        // ✅ Generator для command iteration
        std::generator<Command*> iterate() {
            while (!queue_.empty()) {
                co_yield queue_.front().get();
                queue_.pop();
            }
        }
#endif
    };
}

int main() {
    cpp23::CommandQueue queue;
    
    auto result = queue.enqueue(std::make_unique<PrintCommand>(1));
    if (result.has_value()) {
        std::cout << "✅ Command enqueued\n";
    }
    
    std::cout << "✅ C++23: jthread для queue processing (auto-join)\n";
    std::cout << "✅ C++23: generator для lazy command iteration\n";
    return 0;
}

