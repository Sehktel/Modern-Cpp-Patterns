cmake_minimum_required(VERSION 3.10)
project(CPP_Patterns_Course)

# Устанавливаем стандарт C++17 для всех целей
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Включаем все предупреждения
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Включаем отладочную информацию
set(CMAKE_BUILD_TYPE Debug)

# Ищем и подключаем библиотеку Threads
find_package(Threads REQUIRED)

# Добавляем поддиректории для каждого модуля
add_subdirectory(01-basics)
add_subdirectory(02-principles)
add_subdirectory(03-creational)
add_subdirectory(04-structural)
add_subdirectory(05-behavioral)
add_subdirectory(06-modern-cpp)
add_subdirectory(07-concurrency)
add_subdirectory(08-high-load)
add_subdirectory(09-performance)
add_subdirectory(exercises)

# Создаем общую библиотеку с утилитами
add_subdirectory(common)
