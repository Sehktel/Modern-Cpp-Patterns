# 🏋️ Упражнения: Move Semantics (Семантика перемещения)

## 📋 Задание 1: Переход от копирования к перемещению

### Описание
Перепишите класс для работы с большими массивами данных, заменив дорогостоящие копирования на эффективные перемещения.

### Исходный код (неэффективный)
```cpp
#include <iostream>
#include <vector>
#include <string>

class DataContainer {
private:
    std::vector<int> data_;
    std::string name_;
    
public:
    DataContainer(const std::string& name, size_t size) 
        : name_(name), data_(size) {
        // Заполняем массив данными
        for (size_t i = 0; i < size; ++i) {
            data_[i] = static_cast<int>(i * i);
        }
        std::cout << "Создан контейнер " << name_ << " с " << size << " элементами" << std::endl;
    }
    
    // Копирующий конструктор (дорого!)
    DataContainer(const DataContainer& other) 
        : name_(other.name_), data_(other.data_) {
        std::cout << "Копирование контейнера " << name_ << std::endl;
    }
    
    // Копирующий оператор присваивания (дорого!)
    DataContainer& operator=(const DataContainer& other) {
        if (this != &other) {
            name_ = other.name_;
            data_ = other.data_; // Дорогостоящее копирование!
            std::cout << "Присваивание контейнера " << name_ << std::endl;
        }
        return *this;
    }
    
    ~DataContainer() {
        std::cout << "Уничтожение контейнера " << name_ << std::endl;
    }
    
    const std::vector<int>& getData() const { return data_; }
    const std::string& getName() const { return name_; }
    size_t size() const { return data_.size(); }
    
    void addValue(int value) {
        data_.push_back(value);
    }
    
    int getSum() const {
        int sum = 0;
        for (int val : data_) {
            sum += val;
        }
        return sum;
    }
};
```

### Требования к решению
1. ✅ Добавьте move конструктор и move оператор присваивания
2. ✅ Используйте `std::move` для передачи владения ресурсами
3. ✅ Добавьте методы для создания временных объектов
4. ✅ Демонстрируйте разницу в производительности
5. ✅ Используйте `noexcept` где возможно

### Ожидаемый результат
```cpp
class ModernDataContainer {
    // Ваша реализация с move семантикой
};
```

---

## 📋 Задание 2: Система управления файлами с move семантикой

### Описание
Создайте систему управления файлами, где файлы могут эффективно перемещаться между директориями.

### Требования
1. **File класс**: Представляет файл с содержимым
2. **Directory класс**: Управляет коллекцией файлов
3. **Move операции**: Файлы могут перемещаться между директориями
4. **Эффективность**: Избегайте копирования больших файлов

### Интерфейс
```cpp
class File {
public:
    explicit File(const std::string& name, const std::string& content = "");
    ~File();
    
    // Запрещаем копирование
    File(const File&) = delete;
    File& operator=(const File&) = delete;
    
    // Разрешаем перемещение
    File(File&& other) noexcept;
    File& operator=(File&& other) noexcept;
    
    const std::string& getName() const;
    const std::string& getContent() const;
    size_t getSize() const;
    
    void appendContent(const std::string& content);
    void setContent(const std::string& content);
    
    // Методы для создания временных файлов
    static File createTempFile(const std::string& prefix = "temp");
    static File createFromContent(const std::string& name, const std::string& content);
    
private:
    std::string name_;
    std::string content_;
};

class Directory {
public:
    explicit Directory(const std::string& name);
    ~Directory();
    
    // Запрещаем копирование
    Directory(const Directory&) = delete;
    Directory& operator=(const Directory&) = delete;
    
    // Разрешаем перемещение
    Directory(Directory&& other) noexcept;
    Directory& operator=(Directory&& other) noexcept;
    
    const std::string& getName() const;
    
    // Добавление файлов
    void addFile(File&& file);
    void addFile(const std::string& name, const std::string& content);
    
    // Перемещение файлов между директориями
    File moveFile(const std::string& fileName);
    void moveFileFrom(Directory&& sourceDir, const std::string& fileName);
    
    // Поиск и получение файлов
    bool hasFile(const std::string& fileName) const;
    const File* getFile(const std::string& fileName) const;
    
    // Информация о директории
    size_t getFileCount() const;
    size_t getTotalSize() const;
    void listFiles() const;
    
    // Очистка
    void clear();
    
private:
    std::string name_;
    std::vector<File> files_;
};
```

### Тестовый случай
```cpp
int main() {
    // Создаем директории
    Directory docs("Documents");
    Directory temp("Temp");
    
    // Создаем файлы
    File file1 = File::createFromContent("report.txt", "Это содержимое отчета...");
    File file2 = File::createFromContent("data.csv", "name,age,city\nИван,25,Москва\nПетр,30,СПб");
    
    std::cout << "Размер file1: " << file1.getSize() << " байт" << std::endl;
    std::cout << "Размер file2: " << file2.getSize() << " байт" << std::endl;
    
    // Добавляем файлы в директорию (move семантика!)
    docs.addFile(std::move(file1));
    docs.addFile(std::move(file2));
    
    std::cout << "Файлов в Documents: " << docs.getFileCount() << std::endl;
    std::cout << "Общий размер Documents: " << docs.getTotalSize() << " байт" << std::endl;
    
    // Перемещаем файл в другую директорию
    File movedFile = docs.moveFile("report.txt");
    temp.addFile(std::move(movedFile));
    
    std::cout << "После перемещения report.txt:" << std::endl;
    std::cout << "Файлов в Documents: " << docs.getFileCount() << std::endl;
    std::cout << "Файлов в Temp: " << temp.getFileCount() << std::endl;
    
    // Перемещаем все файлы из одной директории в другую
    Directory backup("Backup");
    backup.moveFileFrom(std::move(temp), "report.txt");
    
    std::cout << "Файлов в Backup: " << backup.getFileCount() << std::endl;
    
    return 0;
}
```

---

## 📋 Задание 3: Контейнер с move-only элементами

### Описание
Создайте контейнер, который может эффективно работать с move-only типами (например, unique_ptr).

### Требования
1. **MoveOnlyContainer класс**: Контейнер для move-only элементов
2. **Эффективные операции**: Добавление, удаление, поиск
3. **Move-only элементы**: Работа с unique_ptr, File и другими move-only типами
4. **Итераторы**: Поддержка range-based for loops

### Интерфейс
```cpp
template<typename T>
class MoveOnlyContainer {
public:
    MoveOnlyContainer();
    ~MoveOnlyContainer();
    
    // Запрещаем копирование
    MoveOnlyContainer(const MoveOnlyContainer&) = delete;
    MoveOnlyContainer& operator=(const MoveOnlyContainer&) = delete;
    
    // Разрешаем перемещение
    MoveOnlyContainer(MoveOnlyContainer&& other) noexcept;
    MoveOnlyContainer& operator=(MoveOnlyContainer&& other) noexcept;
    
    // Добавление элементов
    void push_back(T&& element);
    void push_back(const T& element) = delete; // Запрещаем копирование
    
    template<typename... Args>
    void emplace_back(Args&&... args);
    
    // Удаление элементов
    void pop_back();
    void remove(size_t index);
    void clear();
    
    // Доступ к элементам
    T& operator[](size_t index);
    const T& operator[](size_t index) const;
    T& at(size_t index);
    const T& at(size_t index) const;
    
    // Информация о контейнере
    size_t size() const;
    bool empty() const;
    size_t capacity() const;
    
    // Итераторы (упрощенная версия)
    class Iterator {
    public:
        Iterator(T* ptr);
        T& operator*();
        T* operator->();
        Iterator& operator++();
        bool operator!=(const Iterator& other) const;
    private:
        T* ptr_;
    };
    
    Iterator begin();
    Iterator end();
    
    // Поиск
    template<typename Predicate>
    Iterator find_if(Predicate pred);
    
private:
    T* data_;
    size_t size_;
    size_t capacity_;
    
    void resize(size_t newCapacity);
};
```

### Тестовый случай
```cpp
int main() {
    // Контейнер для unique_ptr
    MoveOnlyContainer<std::unique_ptr<int>> intContainer;
    
    // Добавляем элементы
    intContainer.emplace_back(std::make_unique<int>(42));
    intContainer.emplace_back(std::make_unique<int>(100));
    intContainer.emplace_back(std::make_unique<int>(200));
    
    std::cout << "Размер контейнера: " << intContainer.size() << std::endl;
    
    // Используем range-based for loop
    for (const auto& ptr : intContainer) {
        std::cout << "Значение: " << *ptr << std::endl;
    }
    
    // Поиск элемента
    auto it = intContainer.find_if([](const std::unique_ptr<int>& ptr) {
        return *ptr == 100;
    });
    
    if (it != intContainer.end()) {
        std::cout << "Найден элемент со значением 100" << std::endl;
    }
    
    // Контейнер для File объектов
    MoveOnlyContainer<File> fileContainer;
    
    fileContainer.emplace_back("file1.txt", "Содержимое файла 1");
    fileContainer.emplace_back("file2.txt", "Содержимое файла 2");
    
    std::cout << "Файлов в контейнере: " << fileContainer.size() << std::endl;
    
    // Перемещаем контейнер
    MoveOnlyContainer<File> movedContainer = std::move(fileContainer);
    std::cout << "После перемещения:" << std::endl;
    std::cout << "Исходный контейнер пуст: " << fileContainer.empty() << std::endl;
    std::cout << "Новый контейнер содержит: " << movedContainer.size() << " файлов" << std::endl;
    
    return 0;
}
```

---

## 📋 Задание 4: Система очередей с move семантикой

### Описание
Создайте систему очередей задач, где задачи могут эффективно перемещаться между очередями.

### Требования
1. **Task класс**: Представляет задачу с данными
2. **TaskQueue класс**: Управляет очередью задач
3. **TaskScheduler класс**: Распределяет задачи между очередями
4. **Move операции**: Задачи перемещаются без копирования

### Интерфейс
```cpp
class Task {
public:
    explicit Task(const std::string& name, const std::string& data = "");
    ~Task();
    
    // Запрещаем копирование
    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;
    
    // Разрешаем перемещение
    Task(Task&& other) noexcept;
    Task& operator=(Task&& other) noexcept;
    
    const std::string& getName() const;
    const std::string& getData() const;
    size_t getDataSize() const;
    
    void setData(const std::string& data);
    void appendData(const std::string& data);
    
    // Выполнение задачи
    virtual void execute() = 0;
    
    // Создание различных типов задач
    static Task createIO(const std::string& name, const std::string& data);
    static Task createCPU(const std::string& name, const std::string& data);
    static Task createNetwork(const std::string& name, const std::string& data);
    
protected:
    std::string name_;
    std::string data_;
};

class TaskQueue {
public:
    explicit TaskQueue(const std::string& name);
    ~TaskQueue();
    
    // Запрещаем копирование
    TaskQueue(const TaskQueue&) = delete;
    TaskQueue& operator=(const TaskQueue&) = delete;
    
    // Разрешаем перемещение
    TaskQueue(TaskQueue&& other) noexcept;
    TaskQueue& operator=(TaskQueue&& other) noexcept;
    
    const std::string& getName() const;
    
    // Добавление задач
    void enqueue(Task&& task);
    template<typename... Args>
    void emplace(Args&&... args);
    
    // Извлечение задач
    Task dequeue();
    bool tryDequeue(Task& task);
    
    // Информация об очереди
    size_t size() const;
    bool empty() const;
    size_t getTotalDataSize() const;
    
    // Перемещение задач между очередями
    void moveTasksTo(TaskQueue&& destination, size_t count);
    void moveAllTasksTo(TaskQueue&& destination);
    
private:
    std::string name_;
    std::queue<Task> tasks_;
    size_t totalDataSize_;
};

class TaskScheduler {
public:
    TaskScheduler();
    ~TaskScheduler();
    
    // Создание очередей
    TaskQueue& createQueue(const std::string& name);
    TaskQueue* getQueue(const std::string& name);
    
    // Распределение задач
    void scheduleTask(Task&& task, const std::string& queueName);
    void balanceQueues();
    
    // Выполнение задач
    void processAllQueues();
    void processQueue(const std::string& queueName);
    
    // Статистика
    size_t getTotalTasks() const;
    size_t getQueueCount() const;
    void printStatistics() const;
    
private:
    std::unordered_map<std::string, TaskQueue> queues_;
};
```

### Тестовый случай
```cpp
int main() {
    TaskScheduler scheduler;
    
    // Создаем очереди
    auto& ioQueue = scheduler.createQueue("IO");
    auto& cpuQueue = scheduler.createQueue("CPU");
    auto& networkQueue = scheduler.createQueue("Network");
    
    // Создаем задачи
    auto ioTask = Task::createIO("read_file", "Большой файл с данными...");
    auto cpuTask = Task::createCPU("calculate", "Сложные вычисления...");
    auto networkTask = Task::createNetwork("send_data", "Отправка данных по сети...");
    
    std::cout << "Размер IO задачи: " << ioTask.getDataSize() << " байт" << std::endl;
    
    // Планируем задачи
    scheduler.scheduleTask(std::move(ioTask), "IO");
    scheduler.scheduleTask(std::move(cpuTask), "CPU");
    scheduler.scheduleTask(std::move(networkTask), "Network");
    
    std::cout << "Всего задач: " << scheduler.getTotalTasks() << std::endl;
    
    // Балансируем очереди
    scheduler.balanceQueues();
    
    // Перемещаем задачи между очередями
    if (auto cpuQueuePtr = scheduler.getQueue("CPU")) {
        if (auto ioQueuePtr = scheduler.getQueue("IO")) {
            cpuQueuePtr->moveTasksTo(std::move(*ioQueuePtr), 1);
        }
    }
    
    // Выполняем задачи
    scheduler.processAllQueues();
    
    scheduler.printStatistics();
    
    return 0;
}
```

---

## 📋 Задание 5: Комплексная система с move семантикой

### Описание
Создайте систему управления ресурсами игры с эффективным перемещением объектов.

### Требования
1. **GameObject класс**: Базовый класс для игровых объектов
2. **Scene класс**: Управляет объектами в сцене
3. **ResourceManager класс**: Управляет ресурсами
4. **Move операции**: Объекты и ресурсы перемещаются эффективно

### Архитектура
```cpp
class GameObject {
public:
    explicit GameObject(const std::string& name);
    virtual ~GameObject();
    
    // Запрещаем копирование
    GameObject(const GameObject&) = delete;
    GameObject& operator=(const GameObject&) = delete;
    
    // Разрешаем перемещение
    GameObject(GameObject&& other) noexcept;
    GameObject& operator=(GameObject&& other) noexcept;
    
    const std::string& getName() const;
    void setName(const std::string& name);
    
    virtual void update(float deltaTime) = 0;
    virtual void render() const = 0;
    
    // Позиция и трансформация
    void setPosition(float x, float y, float z);
    std::tuple<float, float, float> getPosition() const;
    
protected:
    std::string name_;
    float x_, y_, z_;
};

class Player : public GameObject {
public:
    explicit Player(const std::string& name);
    
    void update(float deltaTime) override;
    void render() const override;
    
    void move(float dx, float dy, float dz);
    void jump();
    
private:
    float velocityX_, velocityY_, velocityZ_;
    bool onGround_;
};

class Enemy : public GameObject {
public:
    explicit Enemy(const std::string& name);
    
    void update(float deltaTime) override;
    void render() const override;
    
    void setTarget(std::weak_ptr<Player> target);
    void attack();
    
private:
    std::weak_ptr<Player> target_;
    float attackCooldown_;
};

class Scene {
public:
    explicit Scene(const std::string& name);
    ~Scene();
    
    // Запрещаем копирование
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;
    
    // Разрешаем перемещение
    Scene(Scene&& other) noexcept;
    Scene& operator=(Scene&& other) noexcept;
    
    const std::string& getName() const;
    
    // Добавление объектов
    void addObject(std::unique_ptr<GameObject> object);
    template<typename T, typename... Args>
    void emplaceObject(Args&&... args);
    
    // Перемещение объектов между сценами
    std::unique_ptr<GameObject> removeObject(const std::string& name);
    void moveObjectTo(Scene&& destination, const std::string& name);
    
    // Обновление и рендеринг
    void update(float deltaTime);
    void render() const;
    
    // Поиск объектов
    GameObject* findObject(const std::string& name) const;
    std::vector<GameObject*> findObjectsByType(const std::string& type) const;
    
    // Информация о сцене
    size_t getObjectCount() const;
    void printObjectList() const;
    
private:
    std::string name_;
    std::vector<std::unique_ptr<GameObject>> objects_;
};

class GameEngine {
public:
    GameEngine();
    ~GameEngine();
    
    // Управление сценами
    Scene& createScene(const std::string& name);
    Scene* getScene(const std::string& name);
    void removeScene(const std::string& name);
    
    // Переключение сцен
    void setActiveScene(const std::string& name);
    Scene* getActiveScene() const;
    
    // Игровой цикл
    void update(float deltaTime);
    void render();
    
    // Перемещение объектов между сценами
    void moveObjectBetweenScenes(const std::string& objectName, 
                                const std::string& fromScene, 
                                const std::string& toScene);
    
    // Статистика
    size_t getTotalScenes() const;
    size_t getTotalObjects() const;
    void printEngineStatistics() const;
    
private:
    std::unordered_map<std::string, Scene> scenes_;
    Scene* activeScene_;
};
```

### Тестовый случай
```cpp
int main() {
    GameEngine engine;
    
    // Создаем сцены
    auto& mainScene = engine.createScene("MainScene");
    auto& menuScene = engine.createScene("MenuScene");
    
    // Создаем игровые объекты
    auto player = std::make_unique<Player>("Player1");
    auto enemy1 = std::make_unique<Enemy>("Enemy1");
    auto enemy2 = std::make_unique<Enemy>("Enemy2");
    
    // Добавляем объекты в сцену
    mainScene.addObject(std::move(player));
    mainScene.addObject(std::move(enemy1));
    mainScene.addObject(std::move(enemy2));
    
    std::cout << "Объектов в MainScene: " << mainScene.getObjectCount() << std::endl;
    
    // Перемещаем объект между сценами
    engine.moveObjectBetweenScenes("Enemy1", "MainScene", "MenuScene");
    
    std::cout << "После перемещения Enemy1:" << std::endl;
    std::cout << "Объектов в MainScene: " << mainScene.getObjectCount() << std::endl;
    std::cout << "Объектов в MenuScene: " << menuScene.getObjectCount() << std::endl;
    
    // Перемещаем всю сцену
    Scene movedScene = std::move(menuScene);
    std::cout << "Объектов в перемещенной сцене: " << movedScene.getObjectCount() << std::endl;
    
    // Игровой цикл
    engine.setActiveScene("MainScene");
    for (int i = 0; i < 10; ++i) {
        engine.update(0.016f); // 60 FPS
        engine.render();
    }
    
    engine.printEngineStatistics();
    
    return 0;
}
```

---

## 🎯 Критерии оценки

### Отлично (5 баллов)
- ✅ Move семантика реализована корректно
- ✅ Все move конструкторы и операторы используют noexcept
- ✅ Избегаются ненужные копирования
- ✅ Код эффективен и производителен
- ✅ Правильно используется std::move

### Хорошо (4 балла)
- ✅ Основная move семантика реализована
- ✅ Большинство операций эффективны
- ✅ Есть небольшие недочеты в использовании noexcept
- ✅ Код работает стабильно

### Удовлетворительно (3 балла)
- ✅ Базовая move семантика реализована
- ✅ Код компилируется и работает
- ❌ Есть ненужные копирования
- ❌ Не все возможности move семантики используются

### Неудовлетворительно (2 балла)
- ❌ Move семантика не реализована или реализована неправильно
- ❌ Много ненужных копирований
- ❌ Код не компилируется или работает неправильно

---

## 💡 Подсказки

1. **Move конструктор**: Принимает rvalue reference и перемещает ресурсы
2. **Move оператор**: Аналогично конструктору, но с проверкой self-assignment
3. **noexcept**: Используйте для move операций, чтобы избежать исключений
4. **std::move**: Используйте для приведения lvalue к rvalue
5. **Perfect forwarding**: Используйте для передачи параметров без копирования

---

*Move семантика - это ключ к высокопроизводительному C++. Освойте эти принципы, и ваши программы станут намного быстрее!*
