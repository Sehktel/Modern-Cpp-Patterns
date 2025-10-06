# Модуль 2: Принципы проектирования
# SOLID, DRY, KISS, YAGNI, Композиция vs Наследование, Dependency Injection

cmake_minimum_required(VERSION 3.10)

# Настройка стандарта C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Настройка компилятора
if(MSVC)
    add_compile_options(/W4 /utf-8)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Подключение общих утилит
add_subdirectory(${CMAKE_SOURCE_DIR}/common common)

# Подпапки с уроками
add_subdirectory(lesson_2_1_solid)
add_subdirectory(lesson_2_2_dry_kiss_yagni)
add_subdirectory(lesson_2_3_composition_inheritance)
add_subdirectory(lesson_2_4_dependency_injection)

# Общий target для всех уроков модуля
add_custom_target(principles_all)

# Связывание с общими утилитами
target_link_libraries(principles_all common_utils)
