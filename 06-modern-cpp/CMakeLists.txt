# Модуль 6: Современные C++ паттерны

# Урок 6.1: Smart Pointers
add_executable(smart_pointers_demo
    lesson_6_1_smart_pointers/smart_pointers.cpp
    lesson_6_1_smart_pointers/memory_management.cpp
)

target_link_libraries(smart_pointers_demo common_utils)

# Урок 6.2: Move Semantics
add_executable(move_semantics_demo
    lesson_6_2_move_semantics/move_semantics.cpp
    lesson_6_2_move_semantics/perfect_forwarding.cpp
)

target_link_libraries(move_semantics_demo common_utils)

# Урок 6.3: CRTP (Curiously Recurring Template Pattern)
add_executable(crtp_demo
    lesson_6_3_crtp/crtp_pattern.cpp
    lesson_6_3_crtp/polymorphic_copying.cpp
)

target_link_libraries(crtp_demo common_utils)

# Урок 6.4: Type Erasure
add_executable(type_erasure_demo
    lesson_6_4_type_erasure/type_erasure.cpp
    lesson_6_4_type_erasure/std_variant_any.cpp
)

target_link_libraries(type_erasure_demo common_utils)
