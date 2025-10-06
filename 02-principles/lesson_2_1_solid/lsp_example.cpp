/**
 * @file lsp_example.cpp
 * @brief Детальная демонстрация Liskov Substitution Principle (LSP)
 * @author Senior C++ Developer
 * @date 2024
 * 
 * Этот файл содержит углубленный анализ LSP с математическим обоснованием
 * и практическими примерами из реальной разработки.
 */

#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <stdexcept>
#include <cmath>

/**
 * @brief Математическая формализация LSP
 * 
 * LSP можно формализовать следующим образом:
 * ∀D ⊆ B: ∀d ∈ D, ∀b ∈ B: behavior(d) ⊆ behavior(b)
 * где behavior(x) = {методы, которые может выполнить x}
 * 
 * Другими словами: объекты производного класса должны быть заменяемы
 * объектами базового класса без нарушения ожидаемого поведения.
 */

// ============================================================================
// ПРИМЕР 1: КЛАССИЧЕСКИЙ ПРИМЕР С ПРЯМОУГОЛЬНИКОМ И КВАДРАТОМ
// ============================================================================

/**
 * @brief Нарушение LSP: Классический анти-паттерн Rectangle-Square
 * 
 * Этот пример демонстрирует нарушение LSP, когда Square наследуется от Rectangle,
 * но изменяет поведение базового класса.
 */
class BadRectangle {
protected:
    int width;
    int height;
    
public:
    BadRectangle(int w, int h) : width(w), height(h) {}
    
    virtual void setWidth(int w) {
        width = w;
        std::cout << "📐 Установлена ширина: " << w << "\n";
    }
    
    virtual void setHeight(int h) {
        height = h;
        std::cout << "📐 Установлена высота: " << h << "\n";
    }
    
    virtual int getWidth() const { return width; }
    virtual int getHeight() const { return height; }
    
    virtual int getArea() const {
        return width * height;
    }
    
    virtual int getPerimeter() const {
        return 2 * (width + height);
    }
    
    virtual std::string getType() const {
        return "Rectangle";
    }
};

class BadSquare : public BadRectangle {
public:
    BadSquare(int side) : BadRectangle(side, side) {}
    
    // Нарушение LSP: изменение поведения базового класса
    void setWidth(int w) override {
        width = w;
        height = w;  // Квадрат всегда имеет равные стороны
        std::cout << "⬜ Установлена сторона квадрата: " << w << "\n";
    }
    
    void setHeight(int h) override {
        width = h;   // Нарушение LSP: изменяем width при изменении height
        height = h;
        std::cout << "⬜ Установлена сторона квадрата: " << h << "\n";
    }
    
    std::string getType() const override {
        return "Square";
    }
};

/**
 * @brief Функция, которая демонстрирует нарушение LSP
 * 
 * Эта функция ожидает, что изменение ширины прямоугольника не повлияет на высоту,
 * но с Square это не так.
 */
void demonstrateLSPViolation(BadRectangle& rect) {
    std::cout << "\n🔍 Тестирование LSP для " << rect.getType() << ":\n";
    
    // Сохраняем исходные размеры
    int originalWidth = rect.getWidth();
    int originalHeight = rect.getHeight();
    int originalArea = rect.getArea();
    
    std::cout << "Исходные размеры: " << originalWidth << "x" << originalHeight 
              << ", площадь: " << originalArea << "\n";
    
    // Изменяем только ширину
    rect.setWidth(10);
    
    int newWidth = rect.getWidth();
    int newHeight = rect.getHeight();
    int newArea = rect.getArea();
    
    std::cout << "После setWidth(10): " << newWidth << "x" << newHeight 
              << ", площадь: " << newArea << "\n";
    
    // Проверяем инварианты LSP
    bool widthChanged = (newWidth != originalWidth);
    bool heightPreserved = (newHeight == originalHeight);
    bool behaviorConsistent = widthChanged && heightPreserved;
    
    std::cout << "✅ Ширина изменилась: " << (widthChanged ? "Да" : "Нет") << "\n";
    std::cout << "✅ Высота сохранилась: " << (heightPreserved ? "Да" : "Нет") << "\n";
    std::cout << "❌ Поведение согласовано: " << (behaviorConsistent ? "Да" : "Нет") << "\n";
    
    if (!behaviorConsistent) {
        std::cout << "🚨 НАРУШЕНИЕ LSP: Изменение ширины повлияло на высоту!\n";
    }
}

// ============================================================================
// ПРИМЕР 2: СИСТЕМА ФАЙЛОВ
// ============================================================================

/**
 * @brief Нарушение LSP: Неправильная иерархия файлов
 */
class BadFile {
protected:
    std::string name;
    size_t size;
    
public:
    BadFile(const std::string& fileName, size_t fileSize) 
        : name(fileName), size(fileSize) {}
    
    virtual ~BadFile() = default;
    
    virtual void read() {
        std::cout << "📖 Чтение файла: " << name << " (размер: " << size << " байт)\n";
    }
    
    virtual void write(const std::string& data) {
        std::cout << "✍️ Запись в файл: " << name << " (данные: " << data << ")\n";
    }
    
    virtual void deleteFile() {
        std::cout << "🗑️ Удаление файла: " << name << "\n";
    }
    
    virtual std::string getName() const { return name; }
    virtual size_t getSize() const { return size; }
};

class BadReadOnlyFile : public BadFile {
public:
    BadReadOnlyFile(const std::string& fileName, size_t fileSize) 
        : BadFile(fileName, fileSize) {}
    
    // Нарушение LSP: изменение поведения базового класса
    void write(const std::string& data) override {
        throw std::runtime_error("Невозможно записать в файл только для чтения: " + name);
    }
    
    void deleteFile() override {
        throw std::runtime_error("Невозможно удалить файл только для чтения: " + name);
    }
};

/**
 * @brief Функция, которая демонстрирует нарушение LSP в системе файлов
 */
void demonstrateFileLSPViolation(BadFile& file) {
    std::cout << "\n🔍 Тестирование файла: " << file.getName() << "\n";
    
    try {
        file.read();
        file.write("test data");
        file.deleteFile();
        std::cout << "✅ Все операции выполнены успешно\n";
    } catch (const std::exception& e) {
        std::cout << "❌ Ошибка: " << e.what() << "\n";
        std::cout << "🚨 НАРУШЕНИЕ LSP: Файл не может быть заменен базовым типом!\n";
    }
}

// ============================================================================
// ПРИМЕР 3: ПРАВИЛЬНАЯ РЕАЛИЗАЦИЯ LSP
// ============================================================================

/**
 * @brief Соблюдение LSP: Правильная иерархия геометрических фигур
 */
class Shape {
public:
    virtual ~Shape() = default;
    virtual double getArea() const = 0;
    virtual double getPerimeter() const = 0;
    virtual std::string getType() const = 0;
    virtual void printInfo() const = 0;
};

class Rectangle : public Shape {
private:
    double width;
    double height;
    
public:
    Rectangle(double w, double h) : width(w), height(h) {}
    
    double getArea() const override {
        return width * height;
    }
    
    double getPerimeter() const override {
        return 2 * (width + height);
    }
    
    std::string getType() const override {
        return "Rectangle";
    }
    
    void printInfo() const override {
        std::cout << "📐 Прямоугольник: " << width << "x" << height 
                  << ", площадь: " << getArea() << ", периметр: " << getPerimeter() << "\n";
    }
    
    // Методы специфичные для прямоугольника
    void setWidth(double w) { width = w; }
    void setHeight(double h) { height = h; }
    double getWidth() const { return width; }
    double getHeight() const { return height; }
};

class Square : public Shape {
private:
    double side;
    
public:
    Square(double s) : side(s) {}
    
    double getArea() const override {
        return side * side;
    }
    
    double getPerimeter() const override {
        return 4 * side;
    }
    
    std::string getType() const override {
        return "Square";
    }
    
    void printInfo() const override {
        std::cout << "⬜ Квадрат: сторона " << side 
                  << ", площадь: " << getArea() << ", периметр: " << getPerimeter() << "\n";
    }
    
    // Методы специфичные для квадрата
    void setSide(double s) { side = s; }
    double getSide() const { return side; }
};

class Circle : public Shape {
private:
    double radius;
    
public:
    Circle(double r) : radius(r) {}
    
    double getArea() const override {
        return M_PI * radius * radius;
    }
    
    double getPerimeter() const override {
        return 2 * M_PI * radius;
    }
    
    std::string getType() const override {
        return "Circle";
    }
    
    void printInfo() const override {
        std::cout << "⭕ Круг: радиус " << radius 
                  << ", площадь: " << getArea() << ", периметр: " << getPerimeter() << "\n";
    }
    
    // Методы специфичные для круга
    void setRadius(double r) { radius = r; }
    double getRadius() const { return radius; }
};

/**
 * @brief Функция, которая демонстрирует соблюдение LSP
 * 
 * Эта функция может работать с любой фигурой через базовый интерфейс Shape,
 * и все фигуры ведут себя согласованно.
 */
void demonstrateGoodLSP(const Shape& shape) {
    std::cout << "\n🔍 Тестирование LSP для " << shape.getType() << ":\n";
    shape.printInfo();
    
    // Все фигуры поддерживают базовые операции
    double area = shape.getArea();
    double perimeter = shape.getPerimeter();
    
    std::cout << "✅ Площадь: " << area << "\n";
    std::cout << "✅ Периметр: " << perimeter << "\n";
    std::cout << "✅ LSP соблюден: фигура может быть заменена базовым типом\n";
}

// ============================================================================
// ПРИМЕР 4: СИСТЕМА УВЕДОМЛЕНИЙ
// ============================================================================

/**
 * @brief Правильная реализация LSP для системы уведомлений
 */
class NotificationChannel {
public:
    virtual ~NotificationChannel() = default;
    virtual bool send(const std::string& message) = 0;
    virtual std::string getChannelType() const = 0;
    virtual bool isAvailable() const = 0;
    virtual void configure() = 0;
};

class EmailChannel : public NotificationChannel {
private:
    std::string email;
    bool configured;
    
public:
    EmailChannel(const std::string& emailAddr) : email(emailAddr), configured(false) {}
    
    bool send(const std::string& message) override {
        if (!isAvailable()) {
            std::cout << "❌ Email канал недоступен\n";
            return false;
        }
        
        std::cout << "📧 Отправка email на " << email << ": " << message << "\n";
        return true;
    }
    
    std::string getChannelType() const override {
        return "Email";
    }
    
    bool isAvailable() const override {
        return configured && !email.empty() && email.find('@') != std::string::npos;
    }
    
    void configure() override {
        configured = true;
        std::cout << "⚙️ Email канал настроен для " << email << "\n";
    }
};

class SMSChannel : public NotificationChannel {
private:
    std::string phoneNumber;
    bool configured;
    
public:
    SMSChannel(const std::string& phone) : phoneNumber(phone), configured(false) {}
    
    bool send(const std::string& message) override {
        if (!isAvailable()) {
            std::cout << "❌ SMS канал недоступен\n";
            return false;
        }
        
        std::cout << "📱 Отправка SMS на " << phoneNumber << ": " << message << "\n";
        return true;
    }
    
    std::string getChannelType() const override {
        return "SMS";
    }
    
    bool isAvailable() const override {
        return configured && phoneNumber.length() >= 10;
    }
    
    void configure() override {
        configured = true;
        std::cout << "⚙️ SMS канал настроен для " << phoneNumber << "\n";
    }
};

class PushNotificationChannel : public NotificationChannel {
private:
    std::string deviceToken;
    bool configured;
    
public:
    PushNotificationChannel(const std::string& token) : deviceToken(token), configured(false) {}
    
    bool send(const std::string& message) override {
        if (!isAvailable()) {
            std::cout << "❌ Push канал недоступен\n";
            return false;
        }
        
        std::cout << "🔔 Отправка push-уведомления на устройство " 
                  << deviceToken.substr(0, 8) << "...: " << message << "\n";
        return true;
    }
    
    std::string getChannelType() const override {
        return "Push Notification";
    }
    
    bool isAvailable() const override {
        return configured && deviceToken.length() >= 32;
    }
    
    void configure() override {
        configured = true;
        std::cout << "⚙️ Push канал настроен для устройства " 
                  << deviceToken.substr(0, 8) << "...\n";
    }
};

class NotificationService {
public:
    static void sendNotification(std::unique_ptr<NotificationChannel> channel, 
                                const std::string& message) {
        if (!channel) {
            std::cout << "❌ Канал уведомлений не предоставлен\n";
            return;
        }
        
        std::cout << "\n🔔 Отправка уведомления через " << channel->getChannelType() << ":\n";
        
        // Все каналы ведут себя согласованно благодаря LSP
        channel->configure();
        
        if (channel->isAvailable()) {
            bool success = channel->send(message);
            std::cout << "Результат: " << (success ? "✅ Успешно" : "❌ Ошибка") << "\n";
        } else {
            std::cout << "❌ Канал недоступен\n";
        }
    }
};

// ============================================================================
// ПРИМЕР 5: СИСТЕМА КОЛЛЕКЦИЙ
// ============================================================================

/**
 * @brief Правильная реализация LSP для системы коллекций
 */
template<typename T>
class Collection {
public:
    virtual ~Collection() = default;
    virtual void add(const T& item) = 0;
    virtual bool remove(const T& item) = 0;
    virtual bool contains(const T& item) const = 0;
    virtual size_t size() const = 0;
    virtual bool isEmpty() const = 0;
    virtual void clear() = 0;
    virtual std::string getType() const = 0;
};

template<typename T>
class List : public Collection<T> {
private:
    std::vector<T> items;
    
public:
    void add(const T& item) override {
        items.push_back(item);
        std::cout << "➕ Добавлен элемент в список: " << item << "\n";
    }
    
    bool remove(const T& item) override {
        auto it = std::find(items.begin(), items.end(), item);
        if (it != items.end()) {
            items.erase(it);
            std::cout << "➖ Удален элемент из списка: " << item << "\n";
            return true;
        }
        return false;
    }
    
    bool contains(const T& item) const override {
        return std::find(items.begin(), items.end(), item) != items.end();
    }
    
    size_t size() const override {
        return items.size();
    }
    
    bool isEmpty() const override {
        return items.empty();
    }
    
    void clear() override {
        items.clear();
        std::cout << "🗑️ Список очищен\n";
    }
    
    std::string getType() const override {
        return "List";
    }
};

template<typename T>
class Set : public Collection<T> {
private:
    std::vector<T> items;
    
    bool itemExists(const T& item) const {
        return std::find(items.begin(), items.end(), item) != items.end();
    }
    
public:
    void add(const T& item) override {
        if (!itemExists(item)) {
            items.push_back(item);
            std::cout << "➕ Добавлен элемент в множество: " << item << "\n";
        } else {
            std::cout << "⚠️ Элемент уже существует в множестве: " << item << "\n";
        }
    }
    
    bool remove(const T& item) override {
        auto it = std::find(items.begin(), items.end(), item);
        if (it != items.end()) {
            items.erase(it);
            std::cout << "➖ Удален элемент из множества: " << item << "\n";
            return true;
        }
        return false;
    }
    
    bool contains(const T& item) const override {
        return itemExists(item);
    }
    
    size_t size() const override {
        return items.size();
    }
    
    bool isEmpty() const override {
        return items.empty();
    }
    
    void clear() override {
        items.clear();
        std::cout << "🗑️ Множество очищено\n";
    }
    
    std::string getType() const override {
        return "Set";
    }
};

template<typename T>
class Stack : public Collection<T> {
private:
    std::vector<T> items;
    
public:
    void add(const T& item) override {
        items.push_back(item);
        std::cout << "⬆️ Элемент добавлен в стек: " << item << "\n";
    }
    
    bool remove(const T& item) override {
        if (!items.empty() && items.back() == item) {
            items.pop_back();
            std::cout << "⬇️ Элемент удален из стека: " << item << "\n";
            return true;
        }
        std::cout << "⚠️ Можно удалить только верхний элемент стека\n";
        return false;
    }
    
    bool contains(const T& item) const override {
        return std::find(items.begin(), items.end(), item) != items.end();
    }
    
    size_t size() const override {
        return items.size();
    }
    
    bool isEmpty() const override {
        return items.empty();
    }
    
    void clear() override {
        items.clear();
        std::cout << "🗑️ Стек очищен\n";
    }
    
    std::string getType() const override {
        return "Stack";
    }
    
    // Дополнительные методы специфичные для стека
    T top() const {
        if (items.empty()) {
            throw std::runtime_error("Стек пуст");
        }
        return items.back();
    }
    
    void pop() {
        if (!items.empty()) {
            items.pop_back();
        }
    }
};

template<typename T>
void demonstrateCollectionLSP(std::unique_ptr<Collection<T>> collection, 
                             const std::vector<T>& testItems) {
    std::cout << "\n🔍 Тестирование LSP для " << collection->getType() << ":\n";
    
    // Все коллекции поддерживают базовые операции
    std::cout << "Размер: " << collection->size() << "\n";
    std::cout << "Пуста: " << (collection->isEmpty() ? "Да" : "Нет") << "\n";
    
    // Добавление элементов
    for (const auto& item : testItems) {
        collection->add(item);
    }
    
    std::cout << "Размер после добавления: " << collection->size() << "\n";
    
    // Проверка содержимого
    for (const auto& item : testItems) {
        bool contains = collection->contains(item);
        std::cout << "Содержит '" << item << "': " << (contains ? "Да" : "Нет") << "\n";
    }
    
    // Очистка
    collection->clear();
    std::cout << "Размер после очистки: " << collection->size() << "\n";
    std::cout << "✅ LSP соблюден: коллекция может быть заменена базовым типом\n";
}

// ============================================================================
// ДЕМОНСТРАЦИЯ ПРИНЦИПА
// ============================================================================

void demonstrateBadLSP() {
    std::cout << "❌ НАРУШЕНИЕ LSP - Классический пример Rectangle-Square:\n";
    std::cout << std::string(60, '-') << "\n";
    
    BadRectangle rect(5, 10);
    BadSquare square(5);
    
    std::cout << "Тестирование Rectangle:\n";
    demonstrateLSPViolation(rect);
    
    std::cout << "\nТестирование Square:\n";
    demonstrateLSPViolation(square);
    
    std::cout << "\n❌ НАРУШЕНИЕ LSP - Система файлов:\n";
    std::cout << std::string(40, '-') << "\n";
    
    BadFile regularFile("document.txt", 1024);
    BadReadOnlyFile readOnlyFile("readonly.txt", 512);
    
    demonstrateFileLSPViolation(regularFile);
    demonstrateFileLSPViolation(readOnlyFile);
}

void demonstrateGoodLSP() {
    std::cout << "\n✅ СОБЛЮДЕНИЕ LSP - Правильная иерархия фигур:\n";
    std::cout << std::string(50, '-') << "\n";
    
    Rectangle rect(5, 10);
    Square square(5);
    Circle circle(3);
    
    demonstrateGoodLSP(rect);
    demonstrateGoodLSP(square);
    demonstrateGoodLSP(circle);
    
    std::cout << "\n✅ СОБЛЮДЕНИЕ LSP - Система уведомлений:\n";
    std::cout << std::string(40, '-') << "\n";
    
    NotificationService::sendNotification(
        std::make_unique<EmailChannel>("user@example.com"),
        "Ваш заказ подтвержден"
    );
    
    NotificationService::sendNotification(
        std::make_unique<SMSChannel>("+1234567890"),
        "Код подтверждения: 123456"
    );
    
    NotificationService::sendNotification(
        std::make_unique<PushNotificationChannel>("abc123def456ghi789jkl012mno345pqr678"),
        "Новое сообщение в чате"
    );
    
    std::cout << "\n✅ СОБЛЮДЕНИЕ LSP - Система коллекций:\n";
    std::cout << std::string(40, '-') << "\n";
    
    std::vector<std::string> testItems = {"apple", "banana", "cherry", "apple"};
    
    demonstrateCollectionLSP(
        std::make_unique<List<std::string>>(),
        testItems
    );
    
    demonstrateCollectionLSP(
        std::make_unique<Set<std::string>>(),
        testItems
    );
    
    demonstrateCollectionLSP(
        std::make_unique<Stack<std::string>>(),
        testItems
    );
}

void analyzeTradeOffs() {
    std::cout << "\n🔬 АНАЛИЗ КОМПРОМИССОВ LSP:\n";
    std::cout << std::string(50, '-') << "\n";
    
    std::cout << "📊 ПРЕИМУЩЕСТВА:\n";
    std::cout << "• Корректность наследования и полиморфизма\n";
    std::cout << "• Возможность безопасной замены объектов\n";
    std::cout << "• Улучшенная предсказуемость поведения\n";
    std::cout << "• Снижение количества багов в runtime\n";
    std::cout << "• Лучшая поддержка принципа открытости/закрытости\n\n";
    
    std::cout << "⚠️ НЕДОСТАТКИ:\n";
    std::cout << "• Необходимость тщательного проектирования иерархий\n";
    std::cout << "• Возможные ограничения на наследование\n";
    std::cout << "• Необходимость создания дополнительных абстракций\n";
    std::cout << "• Потенциальное усложнение архитектуры\n\n";
    
    std::cout << "🎯 РЕКОМЕНДАЦИИ:\n";
    std::cout << "• Используйте композицию вместо наследования где это возможно\n";
    std::cout << "• Соблюдайте контракты базовых классов\n";
    std::cout << "• Избегайте изменения поведения в производных классах\n";
    std::cout << "• Применяйте принцип "is-a" vs "has-a" для проверки наследования\n";
}

int main() {
    std::cout << "🎯 ДЕТАЛЬНАЯ ДЕМОНСТРАЦИЯ LISKOV SUBSTITUTION PRINCIPLE (LSP)\n";
    std::cout << "Автор: Senior C++ Developer\n";
    std::cout << "Цель: Углубленное изучение принципа подстановки Лисков\n\n";
    
    demonstrateBadLSP();
    demonstrateGoodLSP();
    analyzeTradeOffs();
    
    std::cout << "\n📚 МАТЕМАТИЧЕСКОЕ ОБОСНОВАНИЕ:\n";
    std::cout << "LSP: ∀D ⊆ B: ∀d ∈ D, ∀b ∈ B: behavior(d) ⊆ behavior(b)\n";
    std::cout << "где behavior(x) = {методы, которые может выполнить x}\n\n";
    
    std::cout << "💡 КЛЮЧЕВЫЕ ВЫВОДЫ:\n";
    std::cout << "1. Объекты производного класса должны быть заменяемы объектами базового класса\n";
    std::cout << "2. Поведение производного класса не должно нарушать контракты базового класса\n";
    std::cout << "3. LSP обеспечивает корректность наследования и полиморфизма\n";
    std::cout << "4. Применение требует тщательного проектирования иерархий\n\n";
    
    std::cout << "🔬 Принципы - это инструменты для мышления о проблемах!\n";
    
    return 0;
}
