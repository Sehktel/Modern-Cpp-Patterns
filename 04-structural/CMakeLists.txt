# Модуль 4: Структурные паттерны

# Урок 4.1: Adapter Pattern
add_executable(adapter_demo
    lesson_4_1_adapter/adapter_pattern.cpp
    lesson_4_1_adapter/modern_adapter.cpp
)

target_link_libraries(adapter_demo common_utils)

# Урок 4.2: Decorator Pattern
add_executable(decorator_demo
    lesson_4_2_decorator/decorator_pattern.cpp
    lesson_4_2_decorator/stream_decorators.cpp
)

target_link_libraries(decorator_demo common_utils)

# Урок 4.3: Facade Pattern
add_executable(facade_demo
    lesson_4_3_facade/facade_pattern.cpp
    lesson_4_3_facade/subsystem_manager.cpp
)

target_link_libraries(facade_demo common_utils)

# Урок 4.4: Proxy Pattern
add_executable(proxy_demo
    lesson_4_4_proxy/proxy_pattern.cpp
    lesson_4_4_proxy/smart_proxy.cpp
)

target_link_libraries(proxy_demo common_utils)
