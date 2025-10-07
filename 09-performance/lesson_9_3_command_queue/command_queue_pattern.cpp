/**
 * @file command_queue_pattern.cpp
 * @brief Демонстрация Command Queue Pattern
 * 
 * Реализован Command Queue Pattern с поддержкой:
 * - Очередь команд с батчингом
 * - Обработка команд по группам
 * - Оптимизация производительности
 * - Графический движок
 */

#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <chrono>
#include <functional>

// Базовая команда
class Command {
public:
    virtual ~Command() = default;
    virtual void execute() = 0;
    virtual std::string getName() const = 0;
    virtual bool canBatch() const { return false; }  // Может ли команда батчиться
    virtual std::string getBatchKey() const { return ""; }  // Ключ для группировки
};

// Команда рендеринга спрайта
class RenderSpriteCommand : public Command {
private:
    int sprite_id_;
    int x_, y_;
    std::string texture_;
    
public:
    RenderSpriteCommand(int id, int x, int y, const std::string& texture)
        : sprite_id_(id), x_(x), y_(y), texture_(texture) {}
    
    void execute() override {
        // Имитация рендеринга спрайта
        std::cout << "  Render Sprite #" << sprite_id_ 
                  << " at (" << x_ << ", " << y_ << ")" 
                  << " texture: " << texture_ << std::endl;
        
        // Имитация затрат на рендеринг
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
    
    std::string getName() const override {
        return "RenderSprite";
    }
    
    bool canBatch() const override {
        return true;
    }
    
    std::string getBatchKey() const override {
        return "render_" + texture_;  // Группируем по текстуре
    }
    
    const std::string& getTexture() const { return texture_; }
    int getSpriteId() const { return sprite_id_; }
};

// Команда обновления физики
class UpdatePhysicsCommand : public Command {
private:
    int entity_id_;
    float delta_time_;
    
public:
    UpdatePhysicsCommand(int id, float dt)
        : entity_id_(id), delta_time_(dt) {}
    
    void execute() override {
        std::cout << "  Update Physics for entity #" << entity_id_ 
                  << " dt=" << delta_time_ << std::endl;
        
        std::this_thread::sleep_for(std::chrono::microseconds(50));
    }
    
    std::string getName() const override {
        return "UpdatePhysics";
    }
    
    bool canBatch() const override {
        return true;
    }
    
    std::string getBatchKey() const override {
        return "physics";
    }
};

// Команда воспроизведения звука
class PlaySoundCommand : public Command {
private:
    std::string sound_file_;
    float volume_;
    
public:
    PlaySoundCommand(const std::string& file, float vol)
        : sound_file_(file), volume_(vol) {}
    
    void execute() override {
        std::cout << "  Play Sound: " << sound_file_ 
                  << " volume: " << volume_ << std::endl;
        
        std::this_thread::sleep_for(std::chrono::microseconds(200));
    }
    
    std::string getName() const override {
        return "PlaySound";
    }
};

// Команда изменения состояния игры
class GameStateCommand : public Command {
private:
    std::string state_change_;
    
public:
    explicit GameStateCommand(const std::string& change)
        : state_change_(change) {}
    
    void execute() override {
        std::cout << "  Game State Change: " << state_change_ << std::endl;
        std::this_thread::sleep_for(std::chrono::microseconds(500));
    }
    
    std::string getName() const override {
        return "GameState";
    }
};

// Очередь команд с батчингом
class CommandQueue {
private:
    std::queue<std::shared_ptr<Command>> command_queue_;
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    std::atomic<bool> running_{true};
    
    // Статистика
    std::atomic<size_t> commands_processed_{0};
    std::atomic<size_t> batches_executed_{0};
    std::atomic<size_t> commands_queued_{0};
    
    // Конфигурация
    size_t max_batch_size_;
    std::chrono::milliseconds batch_timeout_;
    
public:
    CommandQueue(size_t max_batch = 50, 
                std::chrono::milliseconds timeout = std::chrono::milliseconds(16))
        : max_batch_size_(max_batch), batch_timeout_(timeout) {
        std::cout << "Command Queue создана (max batch: " << max_batch_size_ 
                  << ", timeout: " << batch_timeout_.count() << " ms)" << std::endl;
    }
    
    // Добавление команды в очередь
    void enqueue(std::shared_ptr<Command> command) {
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            command_queue_.push(std::move(command));
            commands_queued_.fetch_add(1);
        }
        condition_.notify_one();
    }
    
    // Обработка команд (без батчинга)
    void processSimple() {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        
        condition_.wait(lock, [this] {
            return !command_queue_.empty() || !running_.load();
        });
        
        if (!running_.load()) return;
        
        while (!command_queue_.empty()) {
            auto command = command_queue_.front();
            command_queue_.pop();
            
            lock.unlock();
            
            command->execute();
            commands_processed_.fetch_add(1);
            
            lock.lock();
        }
    }
    
    // Обработка команд с батчингом
    void processBatched() {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        
        // Ждем команды или таймаута
        condition_.wait_for(lock, batch_timeout_, [this] {
            return !command_queue_.empty() || !running_.load();
        });
        
        if (!running_.load() && command_queue_.empty()) return;
        
        // Собираем батч команд
        std::vector<std::shared_ptr<Command>> batch;
        std::unordered_map<std::string, std::vector<std::shared_ptr<Command>>> batched_commands;
        
        while (!command_queue_.empty() && batch.size() < max_batch_size_) {
            auto command = command_queue_.front();
            command_queue_.pop();
            
            if (command->canBatch()) {
                std::string key = command->getBatchKey();
                batched_commands[key].push_back(command);
            } else {
                batch.push_back(command);
            }
        }
        
        lock.unlock();
        
        // Выполняем сгруппированные команды
        for (const auto& pair : batched_commands) {
            if (!pair.second.empty()) {
                std::cout << "\n[BATCH] Выполнение " << pair.second.size() 
                          << " команд с ключом '" << pair.first << "'" << std::endl;
                
                for (const auto& cmd : pair.second) {
                    cmd->execute();
                    commands_processed_.fetch_add(1);
                }
                
                batches_executed_.fetch_add(1);
            }
        }
        
        // Выполняем небатчируемые команды
        for (const auto& cmd : batch) {
            std::cout << "\n[SINGLE] Выполнение команды '" << cmd->getName() << "'" << std::endl;
            cmd->execute();
            commands_processed_.fetch_add(1);
        }
    }
    
    // Остановка очереди
    void stop() {
        running_.store(false);
        condition_.notify_all();
    }
    
    // Статистика
    void printStats() const {
        std::cout << "\n=== Command Queue Statistics ===" << std::endl;
        std::cout << "Команд в очереди: " << commands_queued_.load() << std::endl;
        std::cout << "Команд обработано: " << commands_processed_.load() << std::endl;
        std::cout << "Батчей выполнено: " << batches_executed_.load() << std::endl;
        
        if (batches_executed_.load() > 0) {
            double avg_batch_size = static_cast<double>(commands_processed_.load()) / 
                                   batches_executed_.load();
            std::cout << "Средний размер батча: " << avg_batch_size << std::endl;
        }
        
        std::cout << "=================================" << std::endl;
    }
    
    bool isEmpty() const {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        return command_queue_.empty();
    }
};

// Игровой движок с Command Queue
class GameEngine {
private:
    std::shared_ptr<CommandQueue> command_queue_;
    std::thread processing_thread_;
    std::atomic<bool> running_{false};
    bool use_batching_;
    
    std::atomic<size_t> frame_count_{0};
    std::chrono::system_clock::time_point start_time_;
    
public:
    explicit GameEngine(bool use_batching = true)
        : command_queue_(std::make_shared<CommandQueue>()),
          use_batching_(use_batching) {
        std::cout << "Game Engine создан (батчинг: " 
                  << (use_batching ? "ВКЛ" : "ВЫКЛ") << ")" << std::endl;
    }
    
    ~GameEngine() {
        stop();
    }
    
    // Запуск движка
    void start() {
        running_.store(true);
        start_time_ = std::chrono::system_clock::now();
        
        processing_thread_ = std::thread([this]() {
            gameLoop();
        });
        
        std::cout << "Game Engine запущен" << std::endl;
    }
    
    // Остановка движка
    void stop() {
        if (!running_.load()) return;
        
        std::cout << "Останавливаем Game Engine..." << std::endl;
        
        running_.store(false);
        command_queue_->stop();
        
        if (processing_thread_.joinable()) {
            processing_thread_.join();
        }
        
        auto end_time = std::chrono::system_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            end_time - start_time_);
        
        double fps = (frame_count_.load() * 1000.0) / duration.count();
        
        std::cout << "Game Engine остановлен" << std::endl;
        std::cout << "Обработано кадров: " << frame_count_.load() << std::endl;
        std::cout << "Средний FPS: " << fps << std::endl;
        
        command_queue_->printStats();
    }
    
    // Отправка команды
    void submitCommand(std::shared_ptr<Command> command) {
        command_queue_->enqueue(std::move(command));
    }
    
private:
    void gameLoop() {
        std::cout << "Game Loop запущен" << std::endl;
        
        while (running_.load()) {
            auto frame_start = std::chrono::high_resolution_clock::now();
            
            // Обработка команд
            if (use_batching_) {
                command_queue_->processBatched();
            } else {
                command_queue_->processSimple();
            }
            
            frame_count_.fetch_add(1);
            
            // Ограничиваем до ~60 FPS
            auto frame_end = std::chrono::high_resolution_clock::now();
            auto frame_time = std::chrono::duration_cast<std::chrono::milliseconds>(
                frame_end - frame_start);
            
            if (frame_time < std::chrono::milliseconds(16)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(16) - frame_time);
            }
        }
        
        std::cout << "Game Loop завершен" << std::endl;
    }
};

// Демонстрация без батчинга
void demonstrateWithoutBatching() {
    std::cout << "\n=== Демонстрация БЕЗ батчинга ===" << std::endl;
    
    GameEngine engine(false);
    engine.start();
    
    // Имитация игрового процесса
    std::cout << "\n--- Генерация команд рендеринга ---" << std::endl;
    
    for (int frame = 0; frame < 3; ++frame) {
        std::cout << "\nFrame " << frame << ":" << std::endl;
        
        // Множество спрайтов с одинаковой текстурой
        for (int i = 0; i < 10; ++i) {
            engine.submitCommand(
                std::make_shared<RenderSpriteCommand>(i, i * 10, frame * 20, "player.png")
            );
        }
        
        // Физика
        for (int i = 0; i < 5; ++i) {
            engine.submitCommand(
                std::make_shared<UpdatePhysicsCommand>(i, 0.016f)
            );
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    engine.stop();
}

// Демонстрация с батчингом
void demonstrateWithBatching() {
    std::cout << "\n=== Демонстрация С батчингом ===" << std::endl;
    
    GameEngine engine(true);
    engine.start();
    
    // Имитация игрового процесса
    std::cout << "\n--- Генерация команд рендеринга ---" << std::endl;
    
    for (int frame = 0; frame < 3; ++frame) {
        std::cout << "\nFrame " << frame << ":" << std::endl;
        
        // Множество спрайтов с одинаковой текстурой
        for (int i = 0; i < 10; ++i) {
            engine.submitCommand(
                std::make_shared<RenderSpriteCommand>(i, i * 10, frame * 20, "player.png")
            );
        }
        
        // Спрайты с другой текстурой
        for (int i = 0; i < 8; ++i) {
            engine.submitCommand(
                std::make_shared<RenderSpriteCommand>(100 + i, i * 15, frame * 25, "enemy.png")
            );
        }
        
        // Физика
        for (int i = 0; i < 5; ++i) {
            engine.submitCommand(
                std::make_shared<UpdatePhysicsCommand>(i, 0.016f)
            );
        }
        
        // Некоторые звуки
        engine.submitCommand(
            std::make_shared<PlaySoundCommand>("jump.wav", 0.8f)
        );
        
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    engine.stop();
}

// Сравнение производительности
void comparePerformance() {
    std::cout << "\n=== Сравнение производительности ===" << std::endl;
    
    const int NUM_FRAMES = 60;
    const int COMMANDS_PER_FRAME = 100;
    
    // Без батчинга
    std::cout << "\n--- БЕЗ батчинга ---" << std::endl;
    
    {
        GameEngine engine(false);
        engine.start();
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int frame = 0; frame < NUM_FRAMES; ++frame) {
            for (int i = 0; i < COMMANDS_PER_FRAME; ++i) {
                engine.submitCommand(
                    std::make_shared<RenderSpriteCommand>(i, i * 10, frame * 20, "texture.png")
                );
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "Время обработки: " << duration.count() << " ms" << std::endl;
        
        engine.stop();
    }
    
    // С батчингом
    std::cout << "\n--- С батчингом ---" << std::endl;
    
    {
        GameEngine engine(true);
        engine.start();
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int frame = 0; frame < NUM_FRAMES; ++frame) {
            for (int i = 0; i < COMMANDS_PER_FRAME; ++i) {
                engine.submitCommand(
                    std::make_shared<RenderSpriteCommand>(i, i * 10, frame * 20, "texture.png")
                );
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "Время обработки: " << duration.count() << " ms" << std::endl;
        
        engine.stop();
    }
}

int main() {
    std::cout << "=== Command Queue Pattern ===" << std::endl;
    
    try {
        demonstrateWithoutBatching();
        demonstrateWithBatching();
        comparePerformance();
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\n=== Демонстрация завершена ===" << std::endl;
    return 0;
}
