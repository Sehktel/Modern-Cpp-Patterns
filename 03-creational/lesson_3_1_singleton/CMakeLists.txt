# CMakeLists.txt для модуля Singleton с анализом безопасности
cmake_minimum_required(VERSION 3.16)

# Настройка стандарта C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Подключение общих утилит
add_subdirectory(../../common common)

# ----------------------------------------------------------------------------
# ОСНОВНЫЕ ЦЕЛИ ДЛЯ ИЗУЧЕНИЯ ПАТТЕРНА
# ----------------------------------------------------------------------------

# Классический Singleton
add_executable(singleton_pattern singleton_pattern.cpp)
target_link_libraries(singleton_pattern common)

# Современный Singleton
add_executable(modern_singleton modern_singleton.cpp)
target_link_libraries(modern_singleton common)

# ----------------------------------------------------------------------------
# ЦЕЛИ ДЛЯ АНАЛИЗА БЕЗОПАСНОСТИ
# ----------------------------------------------------------------------------

# Уязвимые реализации Singleton
add_executable(singleton_vulnerabilities singleton_vulnerabilities.cpp)
target_link_libraries(singleton_vulnerabilities common)

# Версия с ThreadSanitizer для обнаружения race conditions
add_executable(singleton_vulnerabilities_tsan singleton_vulnerabilities.cpp)
target_compile_options(singleton_vulnerabilities_tsan PRIVATE -fsanitize=thread -fno-omit-frame-pointer -g)
target_link_options(singleton_vulnerabilities_tsan PRIVATE -fsanitize=thread)
target_link_libraries(singleton_vulnerabilities_tsan common)

# Версия с AddressSanitizer для обнаружения use-after-free
add_executable(singleton_vulnerabilities_asan singleton_vulnerabilities.cpp)
target_compile_options(singleton_vulnerabilities_asan PRIVATE -fsanitize=address -fno-omit-frame-pointer -g)
target_link_options(singleton_vulnerabilities_asan PRIVATE -fsanitize=address)
target_link_libraries(singleton_vulnerabilities_asan common)

# Версия с UndefinedBehaviorSanitizer для обнаружения integer overflow
add_executable(singleton_vulnerabilities_ubsan singleton_vulnerabilities.cpp)
target_compile_options(singleton_vulnerabilities_ubsan PRIVATE -fsanitize=undefined -fno-omit-frame-pointer -g)
target_link_options(singleton_vulnerabilities_ubsan PRIVATE -fsanitize=undefined)
target_link_libraries(singleton_vulnerabilities_ubsan common)

# ----------------------------------------------------------------------------
# ИНСТРУКЦИИ ПО ИСПОЛЬЗОВАНИЮ
# ----------------------------------------------------------------------------

message(STATUS "=== МОДУЛЬ SINGLETON С АНАЛИЗОМ БЕЗОПАСНОСТИ ===")
message(STATUS "")
message(STATUS "Основные цели:")
message(STATUS "  make singleton_pattern          - Классический Singleton")
message(STATUS "  make modern_singleton           - Современный Singleton")
message(STATUS "")
message(STATUS "Анализ безопасности:")
message(STATUS "  make singleton_vulnerabilities  - Уязвимые реализации")
message(STATUS "  make singleton_vulnerabilities_tsan  - С ThreadSanitizer")
message(STATUS "  make singleton_vulnerabilities_asan  - С AddressSanitizer")
message(STATUS "  make singleton_vulnerabilities_ubsan  - С UndefinedBehaviorSanitizer")
message(STATUS "")
message(STATUS "Запуск анализа:")
message(STATUS "  ./singleton_vulnerabilities_tsan    # Обнаружение race conditions")
message(STATUS "  ./singleton_vulnerabilities_asan     # Обнаружение use-after-free")
message(STATUS "  ./singleton_vulnerabilities_ubsan    # Обнаружение undefined behavior")
message(STATUS "")
message(STATUS "Статический анализ:")
message(STATUS "  clang --analyze singleton_vulnerabilities.cpp")
message(STATUS "  cppcheck --enable=all singleton_vulnerabilities.cpp")
message(STATUS "")
message(STATUS "Динамический анализ с Valgrind:")
message(STATUS "  valgrind --tool=helgrind ./singleton_vulnerabilities")
message(STATUS "  valgrind --tool=memcheck ./singleton_vulnerabilities")

