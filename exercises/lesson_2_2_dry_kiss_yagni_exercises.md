# 🏋️ Упражнения: DRY/KISS/YAGNI принципы

## 📋 Задание 1: Рефакторинг нарушающего DRY код

### Описание
Перепишите следующий код, нарушающий принцип DRY (Don't Repeat Yourself), устранив дублирование.

### Исходный код (нарушает DRY)
```cpp
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

// Нарушает DRY: много дублированного кода
class UserManager {
private:
    std::vector<std::string> users;
    std::vector<std::string> emails;
    std::vector<int> ages;
    
public:
    // Дублирование логики валидации
    bool addUser(const std::string& name, const std::string& email, int age) {
        // Валидация имени
        if (name.empty()) {
            std::cout << "Ошибка: имя не может быть пустым" << std::endl;
            return false;
        }
        if (name.length() < 2) {
            std::cout << "Ошибка: имя должно содержать минимум 2 символа" << std::endl;
            return false;
        }
        if (name.length() > 50) {
            std::cout << "Ошибка: имя не может содержать более 50 символов" << std::endl;
            return false;
        }
        
        // Валидация email
        if (email.empty()) {
            std::cout << "Ошибка: email не может быть пустым" << std::endl;
            return false;
        }
        if (email.find("@") == std::string::npos) {
            std::cout << "Ошибка: email должен содержать символ @" << std::endl;
            return false;
        }
        if (email.length() > 100) {
            std::cout << "Ошибка: email не может содержать более 100 символов" << std::endl;
            return false;
        }
        
        // Валидация возраста
        if (age < 0) {
            std::cout << "Ошибка: возраст не может быть отрицательным" << std::endl;
            return false;
        }
        if (age > 150) {
            std::cout << "Ошибка: возраст не может быть более 150 лет" << std::endl;
            return false;
        }
        
        // Проверка дубликатов
        for (size_t i = 0; i < users.size(); ++i) {
            if (users[i] == name) {
                std::cout << "Ошибка: пользователь с именем " << name << " уже существует" << std::endl;
                return false;
            }
        }
        
        for (size_t i = 0; i < emails.size(); ++i) {
            if (emails[i] == email) {
                std::cout << "Ошибка: пользователь с email " << email << " уже существует" << std::endl;
                return false;
            }
        }
        
        // Добавление пользователя
        users.push_back(name);
        emails.push_back(email);
        ages.push_back(age);
        
        std::cout << "Пользователь " << name << " успешно добавлен" << std::endl;
        return true;
    }
    
    // Дублирование логики поиска
    bool findUserByName(const std::string& name) {
        for (size_t i = 0; i < users.size(); ++i) {
            if (users[i] == name) {
                return true;
            }
        }
        return false;
    }
    
    bool findUserByEmail(const std::string& email) {
        for (size_t i = 0; i < emails.size(); ++i) {
            if (emails[i] == email) {
                return true;
            }
        }
        return false;
    }
    
    bool findUserByAge(int age) {
        for (size_t i = 0; i < ages.size(); ++i) {
            if (ages[i] == age) {
                return true;
            }
        }
        return false;
    }
    
    // Дублирование логики удаления
    bool removeUserByName(const std::string& name) {
        for (size_t i = 0; i < users.size(); ++i) {
            if (users[i] == name) {
                users.erase(users.begin() + i);
                emails.erase(emails.begin() + i);
                ages.erase(ages.begin() + i);
                std::cout << "Пользователь " << name << " удален" << std::endl;
                return true;
            }
        }
        std::cout << "Пользователь " << name << " не найден" << std::endl;
        return false;
    }
    
    bool removeUserByEmail(const std::string& email) {
        for (size_t i = 0; i < emails.size(); ++i) {
            if (emails[i] == email) {
                users.erase(users.begin() + i);
                emails.erase(emails.begin() + i);
                ages.erase(ages.begin() + i);
                std::cout << "Пользователь с email " << email << " удален" << std::endl;
                return true;
            }
        }
        std::cout << "Пользователь с email " << email << " не найден" << std::endl;
        return false;
    }
    
    // Дублирование логики сохранения
    bool saveToFile(const std::string& filename) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cout << "Ошибка: не удалось открыть файл " << filename << std::endl;
            return false;
        }
        
        for (size_t i = 0; i < users.size(); ++i) {
            file << users[i] << "," << emails[i] << "," << ages[i] << std::endl;
        }
        
        file.close();
        std::cout << "Данные сохранены в файл " << filename << std::endl;
        return true;
    }
    
    bool saveToCSV(const std::string& filename) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cout << "Ошибка: не удалось открыть файл " << filename << std::endl;
            return false;
        }
        
        file << "Name,Email,Age" << std::endl;
        for (size_t i = 0; i < users.size(); ++i) {
            file << users[i] << "," << emails[i] << "," << ages[i] << std::endl;
        }
        
        file.close();
        std::cout << "Данные сохранены в CSV файл " << filename << std::endl;
        return true;
    }
    
    bool saveToJSON(const std::string& filename) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cout << "Ошибка: не удалось открыть файл " << filename << std::endl;
            return false;
        }
        
        file << "[" << std::endl;
        for (size_t i = 0; i < users.size(); ++i) {
            file << "  {" << std::endl;
            file << "    \"name\": \"" << users[i] << "\"," << std::endl;
            file << "    \"email\": \"" << emails[i] << "\"," << std::endl;
            file << "    \"age\": " << ages[i] << std::endl;
            file << "  }";
            if (i < users.size() - 1) {
                file << ",";
            }
            file << std::endl;
        }
        file << "]" << std::endl;
        
        file.close();
        std::cout << "Данные сохранены в JSON файл " << filename << std::endl;
        return true;
    }
    
    void printUsers() {
        for (size_t i = 0; i < users.size(); ++i) {
            std::cout << "Пользователь " << (i + 1) << ": " << users[i] 
                      << ", " << emails[i] << ", " << ages[i] << " лет" << std::endl;
        }
    }
};
```

### Требования к решению
1. ✅ Устраните дублирование кода валидации
2. ✅ Создайте общие функции для поиска и удаления
3. ✅ Используйте шаблоны для работы с различными типами данных
4. ✅ Создайте единую систему сохранения с различными форматами
5. ✅ Примените принцип DRY везде, где возможно

### Ожидаемый результат
```cpp
// Рефакторированный код без дублирования
class User {
    // Представляет одного пользователя
};

class UserValidator {
    // Валидация пользователей
};

class UserRepository {
    // Хранение и поиск пользователей
};

class UserExporter {
    // Экспорт в различные форматы
};

class ModernUserManager {
    // Основной класс без дублирования
};
```

---

## 📋 Задание 2: Применение принципа KISS

### Описание
Упростите следующий излишне сложный код, применив принцип KISS (Keep It Simple, Stupid).

### Исходный код (слишком сложный)
```cpp
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <functional>
#include <memory>
#include <type_traits>

// Нарушает KISS: излишне сложная архитектура для простой задачи
template<typename T>
class AdvancedCalculator {
private:
    std::map<std::string, std::function<T(T, T)>> operations;
    std::vector<std::pair<std::string, T>> history;
    std::unique_ptr<std::map<std::string, T>> variables;
    std::unique_ptr<std::vector<std::function<void()>>> callbacks;
    
public:
    AdvancedCalculator() : variables(std::make_unique<std::map<std::string, T>>()) {
        callbacks = std::make_unique<std::vector<std::function<void()>>>();
        initializeOperations();
    }
    
    void initializeOperations() {
        operations["add"] = [](T a, T b) { return a + b; };
        operations["subtract"] = [](T a, T b) { return a - b; };
        operations["multiply"] = [](T a, T b) { return a * b; };
        operations["divide"] = [](T a, T b) { 
            if (b == 0) throw std::runtime_error("Division by zero");
            return a / b; 
        };
    }
    
    void registerCallback(std::function<void()> callback) {
        callbacks->push_back(callback);
    }
    
    void executeCallbacks() {
        for (auto& callback : *callbacks) {
            callback();
        }
    }
    
    T performOperation(const std::string& operation, T a, T b) {
        auto it = operations.find(operation);
        if (it == operations.end()) {
            throw std::runtime_error("Unknown operation: " + operation);
        }
        
        T result = it->second(a, b);
        history.push_back({operation, result});
        executeCallbacks();
        
        return result;
    }
    
    void setVariable(const std::string& name, T value) {
        (*variables)[name] = value;
    }
    
    T getVariable(const std::string& name) const {
        auto it = variables->find(name);
        if (it == variables->end()) {
            throw std::runtime_error("Variable not found: " + name);
        }
        return it->second;
    }
    
    void clearHistory() {
        history.clear();
    }
    
    void printHistory() const {
        for (const auto& entry : history) {
            std::cout << entry.first << ": " << entry.second << std::endl;
        }
    }
    
    void printVariables() const {
        for (const auto& var : *variables) {
            std::cout << var.first << " = " << var.second << std::endl;
        }
    }
    
    // Сложная система для простых операций
    template<typename U = T>
    typename std::enable_if<std::is_arithmetic<U>::value, U>::type
    calculateExpression(const std::string& expression) {
        // Излишне сложная реализация парсинга выражений
        std::vector<std::string> tokens;
        std::string token;
        for (char c : expression) {
            if (c == ' ' || c == '+' || c == '-' || c == '*' || c == '/') {
                if (!token.empty()) {
                    tokens.push_back(token);
                    token.clear();
                }
                if (c != ' ') {
                    tokens.push_back(std::string(1, c));
                }
            } else {
                token += c;
            }
        }
        if (!token.empty()) {
            tokens.push_back(token);
        }
        
        // Простая реализация для демонстрации
        if (tokens.size() == 3) {
            T a = std::stod(tokens[0]);
            T b = std::stod(tokens[2]);
            std::string op = tokens[1];
            return performOperation(op, a, b);
        }
        
        throw std::runtime_error("Invalid expression format");
    }
};
```

### Требования к решению
1. ✅ Упростите архитектуру до необходимого минимума
2. ✅ Уберите излишние абстракции и сложности
3. ✅ Сделайте код читаемым и понятным
4. ✅ Сохраните только необходимую функциональность
5. ✅ Примените принцип KISS

### Ожидаемый результат
```cpp
// Простой и понятный калькулятор
class SimpleCalculator {
    // Простая реализация без излишних сложностей
};
```

---

## 📋 Задание 3: Применение принципа YAGNI

### Описание
Перепишите следующий код, убрав ненужную функциональность и применив принцип YAGNI (You Aren't Gonna Need It).

### Исходный код (нарушает YAGNI)
```cpp
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <memory>
#include <thread>
#include <future>
#include <chrono>

// Нарушает YAGNI: много ненужной функциональности
class OverEngineeredTaskManager {
private:
    std::vector<std::string> tasks;
    std::map<std::string, std::string> taskDescriptions;
    std::map<std::string, int> taskPriorities;
    std::map<std::string, std::string> taskCategories;
    std::map<std::string, std::chrono::system_clock::time_point> taskDeadlines;
    std::map<std::string, std::vector<std::string>> taskTags;
    std::map<std::string, std::string> taskAssignees;
    std::map<std::string, std::string> taskStatus;
    std::vector<std::function<void(const std::string&)>> taskCallbacks;
    std::unique_ptr<std::thread> backgroundThread;
    std::atomic<bool> running;
    
public:
    OverEngineeredTaskManager() : running(false) {
        // Инициализация ненужных компонентов
        initializeDefaultCategories();
        initializeDefaultTags();
        startBackgroundProcessing();
    }
    
    ~OverEngineeredTaskManager() {
        stopBackgroundProcessing();
    }
    
    void initializeDefaultCategories() {
        taskCategories["work"] = "Рабочие задачи";
        taskCategories["personal"] = "Личные задачи";
        taskCategories["urgent"] = "Срочные задачи";
        taskCategories["low-priority"] = "Низкий приоритет";
        taskCategories["high-priority"] = "Высокий приоритет";
        taskCategories["medium-priority"] = "Средний приоритет";
    }
    
    void initializeDefaultTags() {
        // Инициализация тегов, которые могут никогда не понадобиться
    }
    
    void startBackgroundProcessing() {
        running = true;
        backgroundThread = std::make_unique<std::thread>([this]() {
            while (running) {
                // Фоновая обработка, которая может не понадобиться
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        });
    }
    
    void stopBackgroundProcessing() {
        running = false;
        if (backgroundThread && backgroundThread->joinable()) {
            backgroundThread->join();
        }
    }
    
    // Излишне сложные методы для простых операций
    void addTask(const std::string& task, 
                const std::string& description = "",
                int priority = 1,
                const std::string& category = "personal",
                const std::vector<std::string>& tags = {},
                const std::string& assignee = "",
                const std::chrono::system_clock::time_point& deadline = std::chrono::system_clock::now()) {
        
        tasks.push_back(task);
        taskDescriptions[task] = description;
        taskPriorities[task] = priority;
        taskCategories[task] = category;
        taskDeadlines[task] = deadline;
        taskTags[task] = tags;
        taskAssignees[task] = assignee;
        taskStatus[task] = "pending";
        
        // Вызов всех зарегистрированных колбэков
        for (auto& callback : taskCallbacks) {
            callback(task);
        }
    }
    
    void removeTask(const std::string& task) {
        auto it = std::find(tasks.begin(), tasks.end(), task);
        if (it != tasks.end()) {
            tasks.erase(it);
            taskDescriptions.erase(task);
            taskPriorities.erase(task);
            taskCategories.erase(task);
            taskDeadlines.erase(task);
            taskTags.erase(task);
            taskAssignees.erase(task);
            taskStatus.erase(task);
        }
    }
    
    void setTaskStatus(const std::string& task, const std::string& status) {
        taskStatus[task] = status;
    }
    
    void setTaskPriority(const std::string& task, int priority) {
        taskPriorities[task] = priority;
    }
    
    void setTaskCategory(const std::string& task, const std::string& category) {
        taskCategories[task] = category;
    }
    
    void addTaskTag(const std::string& task, const std::string& tag) {
        taskTags[task].push_back(tag);
    }
    
    void setTaskAssignee(const std::string& task, const std::string& assignee) {
        taskAssignees[task] = assignee;
    }
    
    void setTaskDeadline(const std::string& task, const std::chrono::system_clock::time_point& deadline) {
        taskDeadlines[task] = deadline;
    }
    
    void registerCallback(std::function<void(const std::string&)> callback) {
        taskCallbacks.push_back(callback);
    }
    
    // Методы для получения информации (многие могут не понадобиться)
    std::vector<std::string> getTasksByCategory(const std::string& category) const {
        std::vector<std::string> result;
        for (const auto& task : tasks) {
            auto it = taskCategories.find(task);
            if (it != taskCategories.end() && it->second == category) {
                result.push_back(task);
            }
        }
        return result;
    }
    
    std::vector<std::string> getTasksByPriority(int priority) const {
        std::vector<std::string> result;
        for (const auto& task : tasks) {
            auto it = taskPriorities.find(task);
            if (it != taskPriorities.end() && it->second == priority) {
                result.push_back(task);
            }
        }
        return result;
    }
    
    std::vector<std::string> getTasksByAssignee(const std::string& assignee) const {
        std::vector<std::string> result;
        for (const auto& task : tasks) {
            auto it = taskAssignees.find(task);
            if (it != taskAssignees.end() && it->second == assignee) {
                result.push_back(task);
            }
        }
        return result;
    }
    
    std::vector<std::string> getTasksByTag(const std::string& tag) const {
        std::vector<std::string> result;
        for (const auto& task : tasks) {
            auto it = taskTags.find(task);
            if (it != taskTags.end()) {
                const auto& tags = it->second;
                if (std::find(tags.begin(), tags.end(), tag) != tags.end()) {
                    result.push_back(task);
                }
            }
        }
        return result;
    }
    
    std::vector<std::string> getOverdueTasks() const {
        std::vector<std::string> result;
        auto now = std::chrono::system_clock::now();
        for (const auto& task : tasks) {
            auto it = taskDeadlines.find(task);
            if (it != taskDeadlines.end() && it->second < now) {
                result.push_back(task);
            }
        }
        return result;
    }
    
    void printAllTasks() const {
        for (const auto& task : tasks) {
            std::cout << "Задача: " << task << std::endl;
            std::cout << "  Описание: " << taskDescriptions.at(task) << std::endl;
            std::cout << "  Приоритет: " << taskPriorities.at(task) << std::endl;
            std::cout << "  Категория: " << taskCategories.at(task) << std::endl;
            std::cout << "  Статус: " << taskStatus.at(task) << std::endl;
            std::cout << "  Исполнитель: " << taskAssignees.at(task) << std::endl;
            std::cout << "  Теги: ";
            for (const auto& tag : taskTags.at(task)) {
                std::cout << tag << " ";
            }
            std::cout << std::endl;
            std::cout << "---" << std::endl;
        }
    }
};
```

### Требования к решению
1. ✅ Уберите ненужную функциональность
2. ✅ Оставьте только то, что действительно нужно
3. ✅ Упростите интерфейс до необходимого минимума
4. ✅ Примените принцип YAGNI
5. ✅ Сделайте код простым и понятным

### Ожидаемый результат
```cpp
// Простой менеджер задач без излишней функциональности
class SimpleTaskManager {
    // Только необходимая функциональность
};
```

---

## 📋 Задание 4: Комплексное применение DRY/KISS/YAGNI

### Описание
Создайте систему управления библиотекой, применяя все три принципа одновременно.

### Требования
1. **DRY**: Устраните дублирование кода
2. **KISS**: Сделайте архитектуру простой и понятной
3. **YAGNI**: Реализуйте только необходимую функциональность

### Функциональность (только необходимое)
1. Добавление книг
2. Поиск книг по названию
3. Выдача книг читателям
4. Возврат книг
5. Просмотр списка книг

### Интерфейс
```cpp
// Простая структура для книги
struct Book {
    std::string id;
    std::string title;
    std::string author;
    bool available;
    
    Book(const std::string& id, const std::string& title, const std::string& author)
        : id(id), title(title), author(author), available(true) {}
};

// Простая структура для читателя
struct Reader {
    std::string id;
    std::string name;
    
    Reader(const std::string& id, const std::string& name)
        : id(id), name(name) {}
};

// Простая структура для выдачи
struct Loan {
    std::string bookId;
    std::string readerId;
    std::string loanDate;
    
    Loan(const std::string& bookId, const std::string& readerId, const std::string& loanDate)
        : bookId(bookId), readerId(readerId), loanDate(loanDate) {}
};

// Простой менеджер библиотеки
class SimpleLibraryManager {
public:
    // Управление книгами
    void addBook(const std::string& id, const std::string& title, const std::string& author);
    Book* findBook(const std::string& id);
    Book* findBookByTitle(const std::string& title);
    std::vector<Book> getAllBooks() const;
    
    // Управление читателями
    void addReader(const std::string& id, const std::string& name);
    Reader* findReader(const std::string& id);
    
    // Управление выдачами
    bool borrowBook(const std::string& bookId, const std::string& readerId);
    bool returnBook(const std::string& bookId);
    
    // Информация
    void printAllBooks() const;
    void printAllReaders() const;
    void printAllLoans() const;
    
private:
    std::vector<Book> books_;
    std::vector<Reader> readers_;
    std::vector<Loan> loans_;
    
    // Вспомогательные методы (DRY)
    std::string getCurrentDate() const;
    bool isBookAvailable(const std::string& bookId) const;
    bool isReaderExists(const std::string& readerId) const;
};
```

### Тестовый случай
```cpp
int main() {
    SimpleLibraryManager library;
    
    // Добавляем книги
    library.addBook("BK001", "Война и мир", "Лев Толстой");
    library.addBook("BK002", "1984", "Джордж Оруэлл");
    library.addBook("BK003", "Мастер и Маргарита", "Михаил Булгаков");
    
    // Добавляем читателей
    library.addReader("R001", "Иван Иванов");
    library.addReader("R002", "Петр Петров");
    
    // Выдаем книги
    library.borrowBook("BK001", "R001");
    library.borrowBook("BK002", "R002");
    
    // Возвращаем книгу
    library.returnBook("BK001");
    
    // Выводим информацию
    library.printAllBooks();
    library.printAllReaders();
    library.printAllLoans();
    
    return 0;
}
```

---

## 📋 Задание 5: Рефакторинг реального кода

### Описание
Возьмите любой существующий код из вашего проекта и примените принципы DRY/KISS/YAGNI.

### Требования
1. ✅ Найдите нарушения принципов в существующем коде
2. ✅ Примените DRY для устранения дублирования
3. ✅ Примените KISS для упрощения сложных конструкций
4. ✅ Примените YAGNI для удаления ненужной функциональности
5. ✅ Документируйте изменения и их обоснование

### Шаги рефакторинга
1. **Анализ**: Найдите нарушения принципов
2. **DRY**: Устраните дублирование кода
3. **KISS**: Упростите сложные конструкции
4. **YAGNI**: Удалите ненужную функциональность
5. **Тестирование**: Убедитесь, что код работает корректно

### Критерии оценки
- ✅ Код стал проще и понятнее
- ✅ Устранено дублирование
- ✅ Удалена ненужная функциональность
- ✅ Сохранена необходимая функциональность
- ✅ Код легче поддерживать

---

## 🎯 Критерии оценки

### Отлично (5 баллов)
- ✅ Все принципы DRY/KISS/YAGNI применены корректно
- ✅ Код стал проще, понятнее и легче в поддержке
- ✅ Устранено дублирование и ненужная сложность
- ✅ Сохранена только необходимая функциональность
- ✅ Код следует современным стандартам C++

### Хорошо (4 балла)
- ✅ Большинство принципов применены
- ✅ Код в целом стал лучше
- ✅ Есть небольшие нарушения принципов
- ✅ Код работает корректно

### Удовлетворительно (3 балла)
- ✅ Основные принципы применены
- ✅ Код компилируется и работает
- ❌ Есть значительные нарушения принципов
- ❌ Код все еще сложен или содержит дублирование

### Неудовлетворительно (2 балла)
- ❌ Принципы не применены или применены неправильно
- ❌ Код не стал лучше
- ❌ Код не компилируется или работает неправильно

---

## 💡 Подсказки

1. **DRY**: Если вы копируете код, создайте функцию или класс
2. **KISS**: Простое решение лучше сложного
3. **YAGNI**: Реализуйте только то, что нужно сейчас
4. **Рефакторинг**: Делайте изменения постепенно
5. **Тестирование**: Проверяйте, что код работает после изменений

---

*DRY/KISS/YAGNI - это принципы, которые делают код лучше. Применяйте их постоянно, и ваши программы станут намного качественнее!*
