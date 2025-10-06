# Урок 2.2: DRY, KISS, YAGNI принципы
# Демонстрация принципов Don't Repeat Yourself, Keep It Simple Stupid, You Aren't Gonna Need It

cmake_minimum_required(VERSION 3.10)

# Исполняемые файлы для демонстрации принципов
add_executable(dry_example dry_example.cpp)
target_link_libraries(dry_example common_utils)

add_executable(kiss_example kiss_example.cpp)
target_link_libraries(kiss_example common_utils)

add_executable(yagni_example yagni_example.cpp)
target_link_libraries(yagni_example common_utils)

add_executable(principles_comparison principles_comparison.cpp)
target_link_libraries(principles_comparison common_utils)

# Добавляем в общий target модуля
add_dependencies(principles_all dry_example kiss_example yagni_example principles_comparison)
