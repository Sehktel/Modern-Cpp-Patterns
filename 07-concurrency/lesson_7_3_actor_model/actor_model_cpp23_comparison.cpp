// C++17/20 vs C++23: Actor Model
// C++23: std::jthread, std::generator для messages, std::expected

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

struct Message { int id; std::string data; };

namespace cpp17 {
    class Actor {
        std::queue<Message> mailbox_;
        std::mutex mtx_;
        std::thread worker_;
        bool running_ = true;
    public:
        Actor() {
            worker_ = std::thread([this]() {  // ❌ Manual thread management
                while (running_) {
                    // Process messages
                }
            });
        }
        
        ~Actor() {
            running_ = false;
            worker_.join();  // ❌ Manual join
        }
        
        void send(Message msg) {
            std::lock_guard lock(mtx_);
            mailbox_.push(std::move(msg));
        }
    };
}

namespace cpp23 {
    enum class ActorError { MailboxFull, ActorStopped };
    
    class Actor {
        std::queue<Message> mailbox_;
        std::mutex mtx_;
#if HAS_CPP23
        std::jthread worker_;  // ✅ RAII thread
#else
        std::thread worker_;
        bool running_ = true;
#endif
        
    public:
        Actor() {
#if HAS_CPP23
            worker_ = std::jthread([this](std::stop_token stoken) {  // ✅ stop_token
                while (!stoken.stop_requested()) {
                    // Process messages
                }
            });
#endif
        }
        
        // ✅ expected для send validation
        expected<void, ActorError> send(Message msg) {
            std::lock_guard lock(mtx_);
            if (mailbox_.size() >= 1000) {
                return unexpected(ActorError::MailboxFull);
            }
            mailbox_.push(std::move(msg));
            return {};
        }
        
#if HAS_CPP23
        // ✅ Generator для message stream
        std::generator<Message> getMessages() {
            while (!mailbox_.empty()) {
                std::lock_guard lock(mtx_);
                if (!mailbox_.empty()) {
                    auto msg = std::move(mailbox_.front());
                    mailbox_.pop();
                    co_yield msg;
                }
            }
        }
#endif
    };
}

int main() {
    cpp23::Actor actor;
    
    auto result = actor.send({1, "Hello"});
    if (result.has_value()) {
        std::cout << "✅ Message sent\n";
    }
    
    std::cout << "✅ C++23: jthread для actor lifecycle\n";
    std::cout << "✅ C++23: generator для message streaming\n";
    return 0;
}

