# Урок 2.4: Dependency Injection
# Демонстрация современных подходов к внедрению зависимостей в C++

cmake_minimum_required(VERSION 3.10)

# Исполняемые файлы для демонстрации DI
add_executable(di_basics di_basics.cpp)
target_link_libraries(di_basics common_utils)

add_executable(di_container di_container.cpp)
target_link_libraries(di_container common_utils)

add_executable(di_modern di_modern.cpp)
target_link_libraries(di_modern common_utils)

add_executable(di_testing di_testing.cpp)
target_link_libraries(di_testing common_utils)

# Добавляем в общий target модуля
add_dependencies(principles_all di_basics di_container di_modern di_testing)
