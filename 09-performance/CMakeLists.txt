# Модуль 9: Паттерны производительности
cmake_minimum_required(VERSION 3.10)

# Урок 9.1: Object Pool Pattern
add_executable(lesson_9_1_object_pool
    lesson_9_1_object_pool/object_pool_pattern.cpp
    lesson_9_1_object_pool/resource_pool.cpp
)

target_link_libraries(lesson_9_1_object_pool
    PRIVATE common_utils
)

# Урок 9.2: Flyweight Pattern (расширенный)
add_executable(lesson_9_2_flyweight
    lesson_9_2_flyweight/flyweight_pattern.cpp
    lesson_9_2_flyweight/memory_optimization.cpp
)

target_link_libraries(lesson_9_2_flyweight
    PRIVATE common_utils
)

# Урок 9.3: Command Queue Pattern
add_executable(lesson_9_3_command_queue
    lesson_9_3_command_queue/command_queue_pattern.cpp
    lesson_9_3_command_queue/batch_processing.cpp
)

target_link_libraries(lesson_9_3_command_queue
    PRIVATE common_utils
)
