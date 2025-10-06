# Модуль 8: Паттерны высоконагруженных систем
cmake_minimum_required(VERSION 3.10)

# Урок 8.1: Cache-Aside Pattern
add_executable(lesson_8_1_cache_aside
    lesson_8_1_cache_aside/cache_aside_pattern.cpp
    lesson_8_1_cache_aside/multi_level_cache.cpp
)

target_link_libraries(lesson_8_1_cache_aside
    PRIVATE common_utils
)

# Урок 8.2: Circuit Breaker Pattern
add_executable(lesson_8_2_circuit_breaker
    lesson_8_2_circuit_breaker/circuit_breaker_pattern.cpp
    lesson_8_2_circuit_breaker/resilient_client.cpp
)

target_link_libraries(lesson_8_2_circuit_breaker
    PRIVATE common_utils
)

# Урок 8.3: Bulkhead Pattern
add_executable(lesson_8_3_bulkhead
    lesson_8_3_bulkhead/bulkhead_pattern.cpp
    lesson_8_3_bulkhead/resource_isolation.cpp
)

target_link_libraries(lesson_8_3_bulkhead
    PRIVATE common_utils
)

# Урок 8.4: Saga Pattern
add_executable(lesson_8_4_saga
    lesson_8_4_saga/saga_pattern.cpp
    lesson_8_4_saga/distributed_transactions.cpp
)

target_link_libraries(lesson_8_4_saga
    PRIVATE common_utils
)
