# Урок 7.1: Producer-Consumer Pattern
cmake_minimum_required(VERSION 3.10)

# Базовый Producer-Consumer
add_executable(lesson_7_1_producer_consumer
    producer_consumer_pattern.cpp
)

target_link_libraries(lesson_7_1_producer_consumer
    PRIVATE common_utils
    PRIVATE Threads::Threads
)

# Современный Producer-Consumer
add_executable(lesson_7_1_modern_producer_consumer
    modern_producer_consumer.cpp
)

target_link_libraries(lesson_7_1_modern_producer_consumer
    PRIVATE common_utils
    PRIVATE Threads::Threads
)

# C++23 Quick Comparison - краткое сравнение (109 строк)
add_executable(lesson_7_1_cpp23_quick_comparison
    producer_consumer_cpp23_quick_comparison.cpp
)

target_link_libraries(lesson_7_1_cpp23_quick_comparison
    PRIVATE common_utils
    PRIVATE Threads::Threads
)

# C++23 Full Comparison - полное сравнение (822 строки)
add_executable(lesson_7_1_cpp23_full_comparison
    producer_consumer_cpp23_full_comparison.cpp
)

target_link_libraries(lesson_7_1_cpp23_full_comparison
    PRIVATE common_utils
    PRIVATE Threads::Threads
)

# Устанавливаем свойства для C++17
set_target_properties(lesson_7_1_producer_consumer lesson_7_1_modern_producer_consumer
    PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Устанавливаем свойства для C++20 (для совместимости с C++23 фичами)
set_target_properties(lesson_7_1_cpp23_quick_comparison lesson_7_1_cpp23_full_comparison
    PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)
