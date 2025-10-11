// C++17/20 vs C++23: Move Semantics
// C++23: std::expected для move validation, std::mdspan для data views

#include <iostream>
#include <vector>
#include <memory>

#if __cplusplus >= 202302L
    #include <expected>
    #include <print>
    #include <mdspan>
    #define HAS_CPP23 1
    using std::expected; using std::unexpected;
#else
    #define HAS_CPP23 0
    template<typename T, typename E> struct expected { T v; bool has_value() const { return true; } T& operator*() { return v; } };
#endif

namespace cpp17 {
    class Buffer {
        std::unique_ptr<int[]> data_;
        size_t size_;
    public:
        Buffer(size_t size) : data_(std::make_unique<int[]>(size)), size_(size) {}
        
        // Move constructor
        Buffer(Buffer&& other) noexcept 
            : data_(std::move(other.data_)), size_(other.size_) {
            other.size_ = 0;
        }
        
        int* getData() { return data_.get(); }
        size_t getSize() const { return size_; }
    };
}

namespace cpp23 {
    enum class MoveError { AlreadyMoved, InvalidState };
    
    class Buffer {
        std::unique_ptr<int[]> data_;
        size_t size_;
        bool moved_from_ = false;
    public:
        Buffer(size_t size) : data_(std::make_unique<int[]>(size)), size_(size) {}
        
        Buffer(Buffer&& other) noexcept 
            : data_(std::move(other.data_)), size_(other.size_) {
            other.size_ = 0;
            other.moved_from_ = true;  // ✅ Track moved-from state
        }
        
        // ✅ expected для safe access после move
        expected<int*, MoveError> getData() {
            if (moved_from_) {
                return unexpected(MoveError::AlreadyMoved);
            }
            return data_.get();
        }
        
#if HAS_CPP23
        // ✅ mdspan для multi-dimensional view
        std::mdspan<int, std::dextents<size_t, 2>> as2D(size_t rows, size_t cols) {
            return std::mdspan<int, std::dextents<size_t, 2>>(
                data_.get(), rows, cols
            );
        }
#endif
    };
}

int main() {
    cpp23::Buffer buf(100);
    
    auto result = buf.getData();
    if (result.has_value()) {
        std::cout << "✅ Buffer accessible\n";
    }
    
    cpp23::Buffer moved = std::move(buf);
    
    // ✅ Проверка moved-from state
    auto after_move = buf.getData();
    if (!after_move.has_value()) {
        std::cout << "✅ Correctly detected use-after-move\n";
    }
    
    std::cout << "✅ C++23: expected для move validation\n";
    std::cout << "✅ C++23: mdspan для multi-dimensional views\n";
    return 0;
}

