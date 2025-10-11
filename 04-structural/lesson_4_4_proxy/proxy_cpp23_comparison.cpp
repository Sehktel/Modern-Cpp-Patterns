// C++17/20 vs C++23: Proxy Pattern
// C++23: std::jthread для async proxy, std::expected для auth errors

#include <iostream>
#include <thread>
#include <chrono>

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

class Subject { public: virtual ~Subject() = default; virtual void request() = 0; };
class RealSubject : public Subject { public: void request() override { std::cout << "Real request\n"; } };

namespace cpp17 {
    class AsyncProxy : public Subject {
        RealSubject real_;
        std::thread worker_;
    public:
        void request() override {
            worker_ = std::thread([this]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                real_.request();
            });
            worker_.join();  // ❌ Ручной join
        }
    };
}

namespace cpp23 {
    enum class AuthError { Unauthorized, TokenExpired };
    
    class SecureProxy : public Subject {
        RealSubject real_;
    public:
        // ✅ expected для authorization
        expected<void, AuthError> requestSecure(const std::string& token) {
            if (token.empty()) {
                return unexpected(AuthError::Unauthorized);
            }
            
            real_.request();
            return {};
        }
        
        void request() override {
#if HAS_CPP23
            // ✅ jthread - автоматический join
            std::jthread worker([this](std::stop_token stoken) {
                if (!stoken.stop_requested()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    real_.request();
                }
            });
            // ✅ Auto-join при выходе из scope
#else
            std::thread worker([this]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                real_.request();
            });
            worker.join();
#endif
        }
    };
}

int main() {
    cpp23::SecureProxy proxy;
    
    auto result = proxy.requestSecure("valid_token");
    if (result.has_value()) {
        std::cout << "✅ Authorized\n";
    }
    
    proxy.request();  // ✅ jthread auto-joins
    
    std::cout << "✅ C++23: jthread для async proxy, expected для auth\n";
    return 0;
}

