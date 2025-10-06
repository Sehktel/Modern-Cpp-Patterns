# Модуль 1: Основы современного C++

# Урок 1.1: Hello World в современном C++
add_executable(hello_world_modern
    lesson_1_1_hello_world/hello_world.cpp
    lesson_1_1_hello_world/modern_features.cpp
)

target_link_libraries(hello_world_modern common_utils)

# Урок 1.2: RAII и управление ресурсами
add_executable(raii_demo
    lesson_1_2_raii/raii_example.cpp
    lesson_1_2_raii/resource_manager.cpp
)

target_link_libraries(raii_demo common_utils)

# Урок 1.3: Smart Pointers
add_executable(smart_pointers_demo
    lesson_1_3_smart_pointers/smart_pointers.cpp
    lesson_1_3_smart_pointers/ownership_demo.cpp
)

target_link_libraries(smart_pointers_demo common_utils)

# Урок 1.4: Move Semantics
add_executable(move_semantics_demo
    lesson_1_4_move_semantics/move_semantics.cpp
    lesson_1_4_move_semantics/performance_comparison.cpp
)

target_link_libraries(move_semantics_demo common_utils)
