# Урок 2.3: Композиция vs Наследование
# Демонстрация принципов "Favor Composition over Inheritance" и анализа компромиссов

cmake_minimum_required(VERSION 3.10)

# Исполняемые файлы для демонстрации принципов
add_executable(inheritance_example inheritance_example.cpp)
target_link_libraries(inheritance_example common_utils)

add_executable(composition_example composition_example.cpp)
target_link_libraries(composition_example common_utils)

add_executable(comparison_analysis comparison_analysis.cpp)
target_link_libraries(comparison_analysis common_utils)

add_executable(real_world_examples real_world_examples.cpp)
target_link_libraries(real_world_examples common_utils)

# Добавляем в общий target модуля
add_dependencies(principles_all inheritance_example composition_example comparison_analysis real_world_examples)
