# Модуль 5: Поведенческие паттерны

# Урок 5.1: Observer Pattern
add_executable(observer_demo
    lesson_5_1_observer/observer_pattern.cpp
    lesson_5_1_observer/modern_observer.cpp
)

target_link_libraries(observer_demo common_utils)

# Урок 5.2: Strategy Pattern
add_executable(strategy_demo
    lesson_5_2_strategy/strategy_pattern.cpp
    lesson_5_2_strategy/game_strategies.cpp
)

target_link_libraries(strategy_demo common_utils)

# Урок 5.3: Command Pattern
add_executable(command_demo
    lesson_5_3_command/command_pattern.cpp
    lesson_5_3_command/undo_redo.cpp
)

target_link_libraries(command_demo common_utils)

# Урок 5.4: State Pattern
add_executable(state_demo
    lesson_5_4_state/state_pattern.cpp
    lesson_5_4_state/finite_state_machine.cpp
)

target_link_libraries(state_demo common_utils)
