# CMakeLists.txt для модуля Observer с анализом безопасности
cmake_minimum_required(VERSION 3.16)

# Настройка стандарта C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Подключение общих утилит (если есть)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../../common)
    add_subdirectory(../../common common)
endif()

# ----------------------------------------------------------------------------
# ОСНОВНЫЕ ЦЕЛИ ДЛЯ ИЗУЧЕНИЯ ПАТТЕРНА
# ----------------------------------------------------------------------------

# Классический Observer
add_executable(observer_pattern observer_pattern.cpp)
if(TARGET common)
    target_link_libraries(observer_pattern common)
endif()

# Современный Observer
add_executable(modern_observer modern_observer.cpp)
if(TARGET common)
    target_link_libraries(modern_observer common)
endif()

# ----------------------------------------------------------------------------
# ЦЕЛИ ДЛЯ АНАЛИЗА БЕЗОПАСНОСТИ
# ----------------------------------------------------------------------------

# Уязвимые реализации Observer
add_executable(observer_vulnerabilities observer_vulnerabilities.cpp)
if(TARGET common)
    target_link_libraries(observer_vulnerabilities common)
endif()

# Версия с ThreadSanitizer для обнаружения race conditions
add_executable(observer_vulnerabilities_tsan observer_vulnerabilities.cpp)
target_compile_options(observer_vulnerabilities_tsan PRIVATE 
    -fsanitize=thread -fno-omit-frame-pointer -g -O1)
target_link_options(observer_vulnerabilities_tsan PRIVATE -fsanitize=thread)
if(TARGET common)
    target_link_libraries(observer_vulnerabilities_tsan common)
endif()

# Версия с AddressSanitizer для обнаружения use-after-free
add_executable(observer_vulnerabilities_asan observer_vulnerabilities.cpp)
target_compile_options(observer_vulnerabilities_asan PRIVATE 
    -fsanitize=address -fno-omit-frame-pointer -g -O1)
target_link_options(observer_vulnerabilities_asan PRIVATE -fsanitize=address)
if(TARGET common)
    target_link_libraries(observer_vulnerabilities_asan common)
endif()

# Версия с UndefinedBehaviorSanitizer для обнаружения undefined behavior
add_executable(observer_vulnerabilities_ubsan observer_vulnerabilities.cpp)
target_compile_options(observer_vulnerabilities_ubsan PRIVATE 
    -fsanitize=undefined -fno-omit-frame-pointer -g)
target_link_options(observer_vulnerabilities_ubsan PRIVATE -fsanitize=undefined)
if(TARGET common)
    target_link_libraries(observer_vulnerabilities_ubsan common)
endif()

# Версия с MemorySanitizer (если поддерживается компилятором)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_executable(observer_vulnerabilities_msan observer_vulnerabilities.cpp)
    target_compile_options(observer_vulnerabilities_msan PRIVATE 
        -fsanitize=memory -fno-omit-frame-pointer -g -O1)
    target_link_options(observer_vulnerabilities_msan PRIVATE -fsanitize=memory)
    if(TARGET common)
        target_link_libraries(observer_vulnerabilities_msan common)
    endif()
endif()

# Безопасные альтернативы
add_executable(secure_observer_alternatives secure_observer_alternatives.cpp)
if(TARGET common)
    target_link_libraries(secure_observer_alternatives common)
endif()

# Эксплойты
add_executable(observer_exploits exploits/observer_exploits.cpp)
if(TARGET common)
    target_link_libraries(observer_exploits common)
endif()

# Версия эксплойтов с санитайзерами для демонстрации
add_executable(observer_exploits_asan exploits/observer_exploits.cpp)
target_compile_options(observer_exploits_asan PRIVATE 
    -fsanitize=address -fno-omit-frame-pointer -g -O1)
target_link_options(observer_exploits_asan PRIVATE -fsanitize=address)

# ----------------------------------------------------------------------------
# КАСТОМНЫЕ ТАРГЕТЫ ДЛЯ АНАЛИЗА
# ----------------------------------------------------------------------------

# Статический анализ с Clang
add_custom_target(analyze_observer
    COMMAND clang++ --analyze -Xanalyzer -analyzer-output=text 
        ${CMAKE_CURRENT_SOURCE_DIR}/observer_vulnerabilities.cpp
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running Clang Static Analyzer on observer_vulnerabilities.cpp"
)

# Статический анализ с Cppcheck
add_custom_target(cppcheck_observer
    COMMAND cppcheck --enable=all --inconclusive --std=c++17
        --suppress=missingIncludeSystem
        ${CMAKE_CURRENT_SOURCE_DIR}/observer_vulnerabilities.cpp
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running Cppcheck on observer_vulnerabilities.cpp"
)

# Valgrind Memcheck
add_custom_target(valgrind_observer
    COMMAND valgrind --tool=memcheck --leak-check=full 
        --show-leak-kinds=all --track-origins=yes
        $<TARGET_FILE:observer_vulnerabilities>
    DEPENDS observer_vulnerabilities
    COMMENT "Running Valgrind Memcheck"
)

# Valgrind Helgrind (thread errors)
add_custom_target(helgrind_observer
    COMMAND valgrind --tool=helgrind $<TARGET_FILE:observer_vulnerabilities>
    DEPENDS observer_vulnerabilities
    COMMENT "Running Valgrind Helgrind"
)

# Запуск всех санитайзеров
add_custom_target(run_all_sanitizers
    COMMAND echo "=== ThreadSanitizer ===" && $<TARGET_FILE:observer_vulnerabilities_tsan> || true
    COMMAND echo "=== AddressSanitizer ===" && $<TARGET_FILE:observer_vulnerabilities_asan> || true
    COMMAND echo "=== UndefinedBehaviorSanitizer ===" && $<TARGET_FILE:observer_vulnerabilities_ubsan> || true
    DEPENDS observer_vulnerabilities_tsan observer_vulnerabilities_asan observer_vulnerabilities_ubsan
    COMMENT "Running all sanitizers"
)

# ----------------------------------------------------------------------------
# ИНСТРУКЦИИ ПО ИСПОЛЬЗОВАНИЮ
# ----------------------------------------------------------------------------

message(STATUS "")
message(STATUS "=== МОДУЛЬ OBSERVER С АНАЛИЗОМ БЕЗОПАСНОСТИ ===")
message(STATUS "")
message(STATUS "Основные цели:")
message(STATUS "  make observer_pattern          - Классический Observer")
message(STATUS "  make modern_observer           - Современный Observer")
message(STATUS "")
message(STATUS "Анализ безопасности:")
message(STATUS "  make observer_vulnerabilities       - Уязвимые реализации")
message(STATUS "  make observer_vulnerabilities_tsan  - С ThreadSanitizer")
message(STATUS "  make observer_vulnerabilities_asan  - С AddressSanitizer")
message(STATUS "  make observer_vulnerabilities_ubsan - С UndefinedBehaviorSanitizer")
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(STATUS "  make observer_vulnerabilities_msan  - С MemorySanitizer")
endif()
message(STATUS "")
message(STATUS "Безопасные альтернативы:")
message(STATUS "  make secure_observer_alternatives  - Безопасные реализации")
message(STATUS "")
message(STATUS "Эксплойты:")
message(STATUS "  make observer_exploits             - Практические эксплойты")
message(STATUS "  make observer_exploits_asan        - Эксплойты с ASan")
message(STATUS "")
message(STATUS "Запуск анализа:")
message(STATUS "  ./observer_vulnerabilities_tsan    # Обнаружение race conditions")
message(STATUS "  ./observer_vulnerabilities_asan    # Обнаружение use-after-free")
message(STATUS "  ./observer_vulnerabilities_ubsan   # Обнаружение undefined behavior")
message(STATUS "")
message(STATUS "Кастомные таргеты:")
message(STATUS "  make analyze_observer              # Clang Static Analyzer")
message(STATUS "  make cppcheck_observer             # Cppcheck")
message(STATUS "  make valgrind_observer             # Valgrind Memcheck")
message(STATUS "  make helgrind_observer             # Valgrind Helgrind")
message(STATUS "  make run_all_sanitizers            # Все санитайзеры")
message(STATUS "")
message(STATUS "Полезные команды:")
message(STATUS "  # Динамический анализ с Valgrind:")
message(STATUS "  valgrind --tool=helgrind ./observer_vulnerabilities")
message(STATUS "  valgrind --tool=memcheck --leak-check=full ./observer_vulnerabilities")
message(STATUS "")
message(STATUS "  # Статический анализ:")
message(STATUS "  clang++ --analyze observer_vulnerabilities.cpp")
message(STATUS "  cppcheck --enable=all observer_vulnerabilities.cpp")
message(STATUS "")
