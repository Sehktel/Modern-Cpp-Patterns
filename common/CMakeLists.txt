# Общая библиотека утилит для курса
add_library(common_utils STATIC
    utils.cpp
    logger.cpp
)

target_include_directories(common_utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Устанавливаем стандарт C++17 для этой библиотеки
target_compile_features(common_utils PUBLIC cxx_std_17)
