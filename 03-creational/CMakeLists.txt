# Модуль 3: Креационные паттерны

# Урок 3.1: Singleton Pattern
add_executable(singleton_demo
    lesson_3_1_singleton/singleton_pattern.cpp
    lesson_3_1_singleton/modern_singleton.cpp
    lesson_3_1_singleton/thread_safe_singleton.cpp
)

target_link_libraries(singleton_demo common_utils)

# Урок 3.2: Factory Method Pattern
add_executable(factory_method_demo
    lesson_3_2_factory_method/factory_method.cpp
    lesson_3_2_factory_method/product_factory.cpp
)

target_link_libraries(factory_method_demo common_utils)

# Урок 3.3: Abstract Factory Pattern
add_executable(abstract_factory_demo
    lesson_3_3_abstract_factory/abstract_factory.cpp
    lesson_3_3_abstract_factory/ui_factory.cpp
)

target_link_libraries(abstract_factory_demo common_utils)

# Урок 3.4: Builder Pattern
add_executable(builder_demo
    lesson_3_4_builder/builder_pattern.cpp
    lesson_3_4_builder/fluent_builder.cpp
)

target_link_libraries(builder_demo common_utils)
