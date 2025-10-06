# Регистр технического долга (TODO)

Дата формирования: 2025-10-06

Назначение: консолидированный список всех TODO в репозитории с кратким описанием местоположения и планируемого действия. Используем этот файл для планирования работ и регулярной сверки прогресса.

Формат записей:
- Файл — краткое положение/контекст — планируемое действие

## Плакаты (posters)
- posters/lesson_7_2_thread_pool_poster.md — заглушка плаката — Написать и оформить плакат
- posters/lesson_7_4_reactor_poster.md — заглушка плаката — Написать и оформить плакат
- posters/lesson_8_2_circuit_breaker_poster.md — заглушка плаката — Написать и оформить плакат
- posters/lesson_8_3_bulkhead_poster.md — заглушка плаката — Написать и оформить плакат
- posters/lesson_8_4_saga_poster.md — заглушка плаката — Написать и оформить плакат
- posters/lesson_9_1_object_pool_poster.md — заглушка плаката — Написать и оформить плакат
- posters/lesson_9_2_flyweight_poster.md — заглушка плаката — Написать и оформить плакат
- posters/lesson_9_3_command_queue_poster.md — заглушка плаката — Написать и оформить плакат

## Упражнения (exercises)
- exercises/lesson_7_2_thread_pool_exercises.md — заглушка упражнений — Подготовить задания и ответы
- exercises/lesson_7_3_actor_model_exercises.md — заглушка упражнений — Подготовить задания и ответы
- exercises/lesson_7_4_reactor_exercises.md — заглушка упражнений — Подготовить задания и ответы
- exercises/lesson_8_1_cache_aside_exercises.md — заглушка упражнений — Подготовить задания и ответы
- exercises/lesson_8_2_circuit_breaker_exercises.md — заглушка упражнений — Подготовить задания и ответы
- exercises/lesson_8_3_bulkhead_exercises.md — заглушка упражнений — Подготовить задания и ответы
- exercises/lesson_8_4_saga_exercises.md — заглушка упражнений — Подготовить задания и ответы
- exercises/lesson_9_1_object_pool_exercises.md — заглушка упражнений — Подготовить задания и ответы
- exercises/lesson_9_2_flyweight_exercises.md — заглушка упражнений — Подготовить задания и ответы
- exercises/lesson_9_3_command_queue_exercises.md — заглушка упражнений — Подготовить задания и ответы

## 07-concurrency
- 07-concurrency/lesson_7_2_thread_pool/async_thread_pool.cpp — комментарий-заглушка — Реализовать асинхронный Thread Pool (интерфейсы, задачи, очередь, воркеры)
- 07-concurrency/lesson_7_3_actor_model/message_passing.cpp — комментарий-заглушка — Реализовать расширенные примеры передачи сообщений (почтовые ящики, маршрутизация)
- 07-concurrency/lesson_7_4_reactor/reactor_pattern.cpp — комментарий-заглушка — Реализовать Reactor Pattern (мультиплексирование событий)
- 07-concurrency/lesson_7_4_reactor/event_loop.cpp — комментарий-заглушка — Реализовать Event Loop (цикл, источники событий, обработчики)
- 07-concurrency/lesson_7_4_reactor/README.md — разделы TODO — Заполнить описание, примеры и упражнения

## 08-high-load
- 08-high-load/lesson_8_1_cache_aside/multi_level_cache.cpp — комментарий-заглушка — Реализовать многоуровневый кэш (L1/L2, инвалидация)
- 08-high-load/lesson_8_2_circuit_breaker/resilient_client.cpp — комментарий-заглушка — Реализовать клиент с Circuit Breaker (состояния, метрики, таймауты)
- 08-high-load/lesson_8_3_bulkhead/bulkhead_pattern.cpp — комментарий-заглушка — Реализовать Bulkhead Pattern (ограничения на пул/очереди)
- 08-high-load/lesson_8_3_bulkhead/resource_isolation.cpp — комментарий-заглушка — Реализовать изоляцию ресурсов (сегментация потоков/лимиты)
- 08-high-load/lesson_8_3_bulkhead/README.md — разделы TODO — Заполнить описание, примеры и упражнения
- 08-high-load/lesson_8_4_saga/distributed_transactions.cpp — комментарий-заглушка — Реализовать распределённые транзакции (саги/компенсации)
- 08-high-load/lesson_8_4_saga/README.md — разделы TODO — Заполнить описание, примеры и упражнения

## 09-performance
- 09-performance/lesson_9_2_flyweight/memory_optimization.cpp — комментарий-заглушка — Реализовать оптимизацию памяти через Flyweight (шары состояний)
- 09-performance/lesson_9_2_flyweight/README.md — разделы TODO — Заполнить примеры и упражнения
- 09-performance/lesson_9_3_command_queue/command_queue_pattern.cpp — комментарий-заглушка — Реализовать Command Queue (команды, очередь, воркеры, ретраи)
- 09-performance/lesson_9_3_command_queue/batch_processing.cpp — комментарий-заглушка — Реализовать батчинг (агрегация, флаш, метрики)
- 09-performance/lesson_9_3_command_queue/README.md — разделы TODO — Заполнить описание, примеры и упражнения

## Метаданные курса
- COURSE_STRUCTURE_COMPLETE.md — множество маркеров TODO — Синхронизировать статусы с фактическим прогрессом, обновить список после реализации и удалить маркеры

## Security Posters (плакаты безопасности)
- 03-creational/lesson_3_1_singleton — плакат безопасности — Создать плакат с уязвимостями Singleton (race conditions, memory leaks, double-free), инструментами анализа и защитными мерами
- 03-creational/lesson_3_2_factory_method — плакат безопасности — Создать плакат с уязвимостями Factory Method (buffer overflow, type confusion), эксплоитами и безопасными альтернативами
- 03-creational/lesson_3_3_abstract_factory — плакат безопасности — Создать плакат с уязвимостями Abstract Factory (memory leaks, use-after-free), инструментами анализа и рекомендациями
- 03-creational/lesson_3_4_builder — плакат безопасности — Создать плакат с уязвимостями Builder (buffer overflow, integer overflow), эксплоитами и защитными мерами
- 04-structural/lesson_4_1_adapter — плакат безопасности — Создать плакат с уязвимостями Adapter (type confusion, buffer overflow, use-after-free), инструментами анализа и безопасными альтернативами
- 04-structural/lesson_4_2_decorator — плакат безопасности — Создать плакат с уязвимостями Decorator (stack overflow, memory leak, use-after-free), эксплоитами и рекомендациями
- 04-structural/lesson_4_3_facade — плакат безопасности — Создать плакат с уязвимостями Facade (information disclosure, buffer overflow, use-after-free), инструментами анализа и защитными мерами
- 04-structural/lesson_4_4_proxy — плакат безопасности — Создать плакат с уязвимостями Proxy (authentication bypass, buffer overflow, use-after-free), эксплоитами и безопасными альтернативами
- 05-behavioral/lesson_5_1_observer — плакат безопасности — Создать плакат с уязвимостями Observer (race conditions, use-after-free), инструментами анализа и рекомендациями
- 05-behavioral/lesson_5_2_strategy — плакат безопасности — Создать плакат с уязвимостями Strategy (type confusion, undefined behavior), эксплоитами и защитными мерами
- 05-behavioral/lesson_5_3_command — плакат безопасности — Создать плакат с уязвимостями Command (queue overflow, race conditions), инструментами анализа и безопасными альтернативами
- 05-behavioral/lesson_5_4_state — плакат безопасности — Создать плакат с уязвимостями State (invalid state, TOCTOU), эксплоитами и рекомендациями
- 06-modern-cpp/lesson_6_1_smart_pointers — плакат безопасности — Создать плакат с уязвимостями Smart Pointers (double-free, memory leaks, use-after-free), инструментами анализа и защитными мерами
- 06-modern-cpp/lesson_6_2_move_semantics — плакат безопасности — Создать плакат с уязвимостями Move Semantics (dangling references, after-move use), эксплоитами и безопасными альтернативами
- 06-modern-cpp/lesson_6_3_crtp — плакат безопасности — Создать плакат с уязвимостями CRTP (undefined behavior, ODR violations, type erasure), инструментами анализа и рекомендациями
- 06-modern-cpp/lesson_6_4_type_erasure — плакат безопасности — Создать плакат с уязвимостями Type Erasure (type confusion, allocation issues), эксплоитами и защитными мерами
- 07-concurrency/lesson_7_1_producer_consumer — плакат безопасности — Создать плакат с уязвимостями Producer-Consumer (race conditions, buffer overflow, deadlocks), инструментами анализа (TSan) и безопасными альтернативами
- 07-concurrency/lesson_7_2_thread_pool — плакат безопасности — Создать плакат с уязвимостями Thread Pool (starvation, race conditions, double-submit), эксплоитами и рекомендациями
- 07-concurrency/lesson_7_3_actor_model — плакат безопасности — Создать плакат с уязвимостями Actor Model (message corruption, ordering issues), инструментами анализа (TSan) и защитными мерами
- 07-concurrency/lesson_7_4_reactor — плакат безопасности — Создать плакат с уязвимостями Reactor (event-loop issues, race conditions, overflow), эксплоитами и безопасными альтернативами
- 08-high-load/lesson_8_1_cache_aside — плакат безопасности — Создать плакат с уязвимостями Cache-Aside (stale reads, cache poisoning), инструментами анализа и рекомендациями
- 08-high-load/lesson_8_2_circuit_breaker — плакат безопасности — Создать плакат с уязвимостями Circuit Breaker (state desync, thrashing), эксплоитами и защитными мерами
- 08-high-load/lesson_8_3_bulkhead — плакат безопасности — Создать плакат с уязвимостями Bulkhead (resource exhaustion, DoS), инструментами анализа и безопасными альтернативами
- 08-high-load/lesson_8_4_saga — плакат безопасности — Создать плакат с уязвимостями Saga (compensation ordering, rollback races), эксплоитами и рекомендациями
- 09-performance/lesson_9_1_object_pool — плакат безопасности — Создать плакат с уязвимостями Object Pool (reuse-after-free, fragmentation), инструментами анализа и защитными мерами
- 09-performance/lesson_9_2_flyweight — плакат безопасности — Создать плакат с уязвимостями Flyweight (shared state leakage, aliasing), эксплоитами и безопасными альтернативами
- 09-performance/lesson_9_3_command_queue — плакат безопасности — Создать плакат с уязвимостями Command Queue (batching issues, queue overflow, race conditions), инструментами анализа и рекомендациями

## Security Integration (интеграция безопасности)
- 03-creational/lesson_3_1_singleton — пакет заглушек (SECURITY_ANALYSIS.md, singleton_vulnerabilities.cpp, exploits/singleton_exploits.cpp, secure_singleton_alternatives.cpp, CMakeLists.txt) — Проверить и дополнить анализ, снять TODO, расширить примеры/эксплоиты, добавить юнит‑тесты и санитайзеры в сборке
- 03-creational/lesson_3_2_factory_method — пакет заглушек (SECURITY_ANALYSIS.md, factory_method_vulnerabilities.cpp, exploits/factory_method_exploits.cpp, secure_factory_method_alternatives.cpp, CMakeLists.txt) — Проверить и дополнить анализ, снять TODO, расширить примеры/эксплоиты, добавить юнит‑тесты и санитайзеры в сборке
- 03-creational/lesson_3_3_abstract_factory — пакет заглушек (SECURITY_ANALYSIS.md, abstract_factory_vulnerabilities.cpp, exploits/abstract_factory_exploits.cpp, secure_abstract_factory_alternatives.cpp, CMakeLists.txt) — Проверить и дополнить анализ, снять TODO, расширить примеры/эксплоиты, добавить юнит‑тесты и санитайзеры в сборке
- 03-creational/lesson_3_4_builder — пакет заглушек (SECURITY_ANALYSIS.md, builder_vulnerabilities.cpp, exploits/builder_exploits.cpp, secure_builder_alternatives.cpp, CMakeLists.txt) — Проверить и дополнить анализ, снять TODO, расширить примеры/эксплоиты, добавить юнит‑тесты и санитайзеры в сборке
- 04-structural/lesson_4_1_adapter — пакет заглушек (SECURITY_ANALYSIS.md, adapter_vulnerabilities.cpp, exploits/adapter_exploits.cpp, secure_adapter_alternatives.cpp, CMakeLists.txt) — Проверить и дополнить анализ, снять TODO, исправить кодировку, расширить эксплоиты, добавить юнит‑тесты и санитайзеры в сборке
- 04-structural/lesson_4_2_decorator — пакет заглушек (SECURITY_ANALYSIS.md, decorator_vulnerabilities.cpp, exploits/decorator_exploits.cpp, secure_decorator_alternatives.cpp, CMakeLists.txt) — Проверить и дополнить анализ, снять TODO, расширить эксплоиты, добавить юнит‑тесты и санитайзеры в сборке
- 04-structural/lesson_4_3_facade — пакет заглушек (SECURITY_ANALYSIS.md, facade_vulnerabilities.cpp, exploits/facade_exploits.cpp, secure_facade_alternatives.cpp, CMakeLists.txt) — Проверить и дополнить анализ, снять TODO, расширить эксплоиты, добавить юнит‑тесты и санитайзеры в сборке
- 04-structural/lesson_4_4_proxy — пакет заглушек (SECURITY_ANALYSIS.md, proxy_vulnerabilities.cpp, exploits/proxy_exploits.cpp, secure_proxy_alternatives.cpp, CMakeLists.txt) — Проверить и дополнить анализ, снять TODO, расширить эксплоиты, добавить юнит‑тесты и санитайзеры в сборке

- 05-behavioral/lesson_5_1_observer — пакет заглушек (SECURITY_ANALYSIS.md, observer_vulnerabilities.cpp, exploits/observer_exploits.cpp, secure_observer_alternatives.cpp, CMakeLists.txt) — Проверить и дополнить анализ, снять TODO, усилить race/use-after-free сценарии, добавить юнит‑тесты и санитайзеры
- 05-behavioral/lesson_5_2_strategy — пакет заглушек (SECURITY_ANALYSIS.md, strategy_vulnerabilities.cpp, exploits/strategy_exploits.cpp, secure_strategy_alternatives.cpp, CMakeLists.txt) — Проверить и дополнить анализ, снять TODO, расширить type confusion/UB кейсы, добавить юнит‑тесты и санитайзеры
- 05-behavioral/lesson_5_3_command — пакет заглушек (SECURITY_ANALYSIS.md, command_vulnerabilities.cpp, exploits/command_exploits.cpp, secure_command_alternatives.cpp, CMakeLists.txt) — Проверить и дополнить анализ, снять TODO, расширить queue/race/overflow кейсы, добавить юнит‑тесты и санитайзеры
- 05-behavioral/lesson_5_4_state — пакет заглушек (SECURITY_ANALYSIS.md, state_vulnerabilities.cpp, exploits/state_exploits.cpp, secure_state_alternatives.cpp, CMakeLists.txt) — Проверить и дополнить анализ, снять TODO, расширить invalid-state/TOCTOU кейсы, добавить юнит‑тесты и санитайзеры

- 06-modern-cpp/lesson_6_1_smart_pointers — пакет заглушек (SECURITY_ANALYSIS.md, smart_pointers_vulnerabilities.cpp, exploits/smart_pointers_exploits.cpp, secure_smart_pointers_alternatives.cpp, CMakeLists.txt) — Проверить и дополнить анализ, снять TODO, расширить double-free/leak/uaf кейсы, добавить юнит‑тесты и санитайзеры
- 06-modern-cpp/lesson_6_2_move_semantics — пакет заглушек (SECURITY_ANALYSIS.md, move_semantics_vulnerabilities.cpp, exploits/move_semantics_exploits.cpp, secure_move_semantics_alternatives.cpp, CMakeLists.txt) — Проверить и дополнить анализ, снять TODO, расширить dangling/after-move use кейсы, добавить юнит‑тесты и санитайзеры
- 06-modern-cpp/lesson_6_3_crtp — пакет заглушек (SECURITY_ANALYSIS.md, crtp_vulnerabilities.cpp, exploits/crtp_exploits.cpp, secure_crtp_alternatives.cpp, CMakeLists.txt) — Проверить и дополнить анализ, снять TODO, расширить UB/ODR/type erasure кейсы, добавить юнит‑тесты и санитайзеры
- 06-modern-cpp/lesson_6_4_type_erasure — пакет заглушек (SECURITY_ANALYSIS.md, type_erasure_vulnerabilities.cpp, exploits/type_erasure_exploits.cpp, secure_type_erasure_alternatives.cpp, CMakeLists.txt) — Проверить и дополнить анализ, снять TODO, расширить type confusion/alloc кейсы, добавить юнит‑тесты и санитайзеры

- 07-concurrency/lesson_7_1_producer_consumer — пакет заглушек (SECURITY_ANALYSIS.md, producer_consumer_vulnerabilities.cpp, exploits/producer_consumer_exploits.cpp, secure_producer_consumer_alternatives.cpp, CMakeLists.txt) — Проверить и дополнить анализ, снять TODO, расширить race/overflow/deadlock кейсы, добавить юнит‑тесты и санитайзеры (TSan)
- 07-concurrency/lesson_7_2_thread_pool — пакет заглушек (SECURITY_ANALYSIS.md, thread_pool_vulnerabilities.cpp, exploits/thread_pool_exploits.cpp, secure_thread_pool_alternatives.cpp, CMakeLists.txt) — Проверить и дополнить анализ, снять TODO, расширить starvation/race/double-submit кейсы, добавить юнит‑тесты и санитайзеры (TSan)
- 07-concurrency/lesson_7_3_actor_model — пакет заглушек (SECURITY_ANALYSIS.md, actor_model_vulnerabilities.cpp, exploits/actor_model_exploits.cpp, secure_actor_model_alternatives.cpp, CMakeLists.txt) — Проверить и дополнить анализ, снять TODO, расширить message corruption/ordering кейсы, добавить юнит‑тесты и санитайзеры (TSan)
- 07-concurrency/lesson_7_4_reactor — пакет заглушек (SECURITY_ANALYSIS.md, reactor_vulnerabilities.cpp, exploits/reactor_exploits.cpp, secure_reactor_alternatives.cpp, CMakeLists.txt) — Проверить и дополнить анализ, снять TODO, расширить event-loop/race/overflow кейсы, добавить юнит‑тесты и санитайзеры (TSan/ASan)

- 08-high-load/lesson_8_1_cache_aside — пакет заглушек (SECURITY_ANALYSIS.md, cache_aside_vulnerabilities.cpp, exploits/cache_aside_exploits.cpp, secure_cache_aside_alternatives.cpp, CMakeLists.txt) — Проверить и дополнить анализ, снять TODO, расширить stale read/poisoning кейсы, добавить юнит‑тесты и санитайзеры
- 08-high-load/lesson_8_2_circuit_breaker — пакет заглушек (SECURITY_ANALYSIS.md, circuit_breaker_vulnerabilities.cpp, exploits/circuit_breaker_exploits.cpp, secure_circuit_breaker_alternatives.cpp, CMakeLists.txt) — Проверить и дополнить анализ, снять TODO, расширить state desync/thrashing кейсы, добавить юнит‑тесты и санитайзеры
- 08-high-load/lesson_8_3_bulkhead — пакет заглушек (SECURITY_ANALYSIS.md, bulkhead_vulnerabilities.cpp, exploits/bulkhead_exploits.cpp, secure_bulkhead_alternatives.cpp, CMakeLists.txt) — Проверить и дополнить анализ, снять TODO, расширить resource exhaustion/DoS кейсы, добавить юнит‑тесты и санитайзеры
- 08-high-load/lesson_8_4_saga — пакет заглушек (SECURITY_ANALYSIS.md, saga_vulnerabilities.cpp, exploits/saga_exploits.cpp, secure_saga_alternatives.cpp, CMakeLists.txt) — Проверить и дополнить анализ, снять TODO, расширить compensation ordering/rollback race кейсы, добавить юнит‑тесты и санитайзеры

- 09-performance/lesson_9_1_object_pool — пакет заглушек (SECURITY_ANALYSIS.md, object_pool_vulnerabilities.cpp, exploits/object_pool_exploits.cpp, secure_object_pool_alternatives.cpp, CMakeLists.txt) — Проверить и дополнить анализ, снять TODO, расширить reuse-after-free/fragmentation кейсы, добавить юнит‑тесты и санитайзеры
- 09-performance/lesson_9_2_flyweight — пакет заглушек (SECURITY_ANALYSIS.md, flyweight_vulnerabilities.cpp, exploits/flyweight_exploits.cpp, secure_flyweight_alternatives.cpp, CMakeLists.txt) — Проверить и дополнить анализ, снять TODO, расширить shared state leakage/aliasing кейсы, добавить юнит‑тесты и санитайзеры
- 09-performance/lesson_9_3_command_queue — пакет заглушек (SECURITY_ANALYSIS.md, command_queue_vulnerabilities.cpp, exploits/command_queue_exploits.cpp, secure_command_queue_alternatives.cpp, CMakeLists.txt) — Проверить и дополнить анализ, снять TODO, расширить batching/queue overflow/race кейсы, добавить юнит‑тесты и санитайзеры

---

Процесс ведения реестра:
- При закрытии пункта удалить соответствующую строку здесь и удалить/обновить TODO в исходном файле.
- В PR обязательно ссылаться на строку из этого файла.


