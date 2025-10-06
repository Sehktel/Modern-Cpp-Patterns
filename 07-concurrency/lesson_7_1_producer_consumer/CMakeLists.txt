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

# Устанавливаем свойства для C++17
set_target_properties(lesson_7_1_producer_consumer lesson_7_1_modern_producer_consumer
    PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
