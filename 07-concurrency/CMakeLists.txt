# Модуль 7: Паттерны многопоточности
cmake_minimum_required(VERSION 3.10)

# Урок 7.1: Producer-Consumer Pattern
add_executable(lesson_7_1_producer_consumer
    lesson_7_1_producer_consumer/producer_consumer_pattern.cpp
    lesson_7_1_producer_consumer/modern_producer_consumer.cpp
)

target_link_libraries(lesson_7_1_producer_consumer
    PRIVATE common_utils
    PRIVATE Threads::Threads
)

# Урок 7.2: Thread Pool Pattern
add_executable(lesson_7_2_thread_pool
    lesson_7_2_thread_pool/thread_pool_pattern.cpp
    lesson_7_2_thread_pool/async_thread_pool.cpp
)

target_link_libraries(lesson_7_2_thread_pool
    PRIVATE common_utils
    PRIVATE Threads::Threads
)

# Урок 7.3: Actor Model Pattern
add_executable(lesson_7_3_actor_model
    lesson_7_3_actor_model/actor_model_pattern.cpp
    lesson_7_3_actor_model/message_passing.cpp
)

target_link_libraries(lesson_7_3_actor_model
    PRIVATE common_utils
    PRIVATE Threads::Threads
)

# Урок 7.4: Reactor Pattern
add_executable(lesson_7_4_reactor
    lesson_7_4_reactor/reactor_pattern.cpp
    lesson_7_4_reactor/event_loop.cpp
)

target_link_libraries(lesson_7_4_reactor
    PRIVATE common_utils
    PRIVATE Threads::Threads
)
