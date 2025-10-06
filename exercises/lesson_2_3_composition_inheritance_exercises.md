# 🏋️ Упражнения: Composition vs Inheritance

## 📋 Задание 1: Рефакторинг наследования в композицию

### Описание
Перепишите следующий код, заменив наследование на композицию там, где это уместно.

### Исходный код (избыточное наследование)
```cpp
#include <iostream>
#include <string>
#include <vector>
#include <memory>

// Проблемное наследование: слишком глубокая иерархия
class Vehicle {
protected:
    std::string brand;
    std::string model;
    int year;
    double price;
    
public:
    Vehicle(const std::string& brand, const std::string& model, int year, double price)
        : brand(brand), model(model), year(year), price(price) {}
    
    virtual ~Vehicle() = default;
    
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void accelerate() = 0;
    virtual void brake() = 0;
    
    virtual std::string getInfo() const {
        return brand + " " + model + " (" + std::to_string(year) + ") - $" + std::to_string(price);
    }
    
    // Общие методы для всех транспортных средств
    void honk() {
        std::cout << "Beep beep!" << std::endl;
    }
    
    void turnOnLights() {
        std::cout << "Lights turned on" << std::endl;
    }
};

// Проблема: Car наследует все методы Vehicle, даже те, которые не нужны
class Car : public Vehicle {
private:
    int doors;
    std::string fuelType;
    
public:
    Car(const std::string& brand, const std::string& model, int year, double price,
        int doors, const std::string& fuelType)
        : Vehicle(brand, model, year, price), doors(doors), fuelType(fuelType) {}
    
    void start() override {
        std::cout << "Car engine started" << std::endl;
    }
    
    void stop() override {
        std::cout << "Car engine stopped" << std::endl;
    }
    
    void accelerate() override {
        std::cout << "Car accelerating" << std::endl;
    }
    
    void brake() override {
        std::cout << "Car braking" << std::endl;
    }
    
    // Специфичные для автомобиля методы
    void openTrunk() {
        std::cout << "Trunk opened" << std::endl;
    }
    
    void useAirConditioning() {
        std::cout << "Air conditioning turned on" << std::endl;
    }
    
    std::string getInfo() const override {
        return Vehicle::getInfo() + " - " + std::to_string(doors) + " doors, " + fuelType;
    }
};

// Проблема: Motorcycle наследует методы, которые не подходят для мотоцикла
class Motorcycle : public Vehicle {
private:
    bool hasWindshield;
    std::string engineType;
    
public:
    Motorcycle(const std::string& brand, const std::string& model, int year, double price,
               bool hasWindshield, const std::string& engineType)
        : Vehicle(brand, model, year, price), hasWindshield(hasWindshield), engineType(engineType) {}
    
    void start() override {
        std::cout << "Motorcycle engine started" << std::endl;
    }
    
    void stop() override {
        std::cout << "Motorcycle engine stopped" << std::endl;
    }
    
    void accelerate() override {
        std::cout << "Motorcycle accelerating" << std::endl;
    }
    
    void brake() override {
        std::cout << "Motorcycle braking" << std::endl;
    }
    
    // Специфичные для мотоцикла методы
    void putOnHelmet() {
        std::cout << "Helmet put on" << std::endl;
    }
    
    void lean() {
        std::cout << "Motorcycle leaning" << std::endl;
    }
    
    std::string getInfo() const override {
        return Vehicle::getInfo() + " - " + engineType + " engine";
    }
};

// Проблема: Truck наследует все методы Vehicle, но многие не подходят
class Truck : public Vehicle {
private:
    double cargoCapacity;
    bool hasTrailer;
    
public:
    Truck(const std::string& brand, const std::string& model, int year, double price,
          double cargoCapacity, bool hasTrailer)
        : Vehicle(brand, model, year, price), cargoCapacity(cargoCapacity), hasTrailer(hasTrailer) {}
    
    void start() override {
        std::cout << "Truck engine started" << std::endl;
    }
    
    void stop() override {
        std::cout << "Truck engine stopped" << std::endl;
    }
    
    void accelerate() override {
        std::cout << "Truck accelerating slowly" << std::endl;
    }
    
    void brake() override {
        std::cout << "Truck braking" << std::endl;
    }
    
    // Специфичные для грузовика методы
    void loadCargo(double weight) {
        std::cout << "Loading " << weight << " kg of cargo" << std::endl;
    }
    
    void unloadCargo() {
        std::cout << "Unloading cargo" << std::endl;
    }
    
    std::string getInfo() const override {
        return Vehicle::getInfo() + " - " + std::to_string(cargoCapacity) + " kg capacity";
    }
};

// Проблема: ElectricCar наследует от Car, но имеет другую логику
class ElectricCar : public Car {
private:
    double batteryCapacity;
    double currentCharge;
    
public:
    ElectricCar(const std::string& brand, const std::string& model, int year, double price,
                int doors, double batteryCapacity)
        : Car(brand, model, year, price, doors, "Electric"), 
          batteryCapacity(batteryCapacity), currentCharge(100.0) {}
    
    void start() override {
        std::cout << "Electric car started silently" << std::endl;
    }
    
    void accelerate() override {
        std::cout << "Electric car accelerating instantly" << std::endl;
        currentCharge -= 0.1;
    }
    
    // Специфичные для электромобиля методы
    void chargeBattery() {
        std::cout << "Charging battery..." << std::endl;
        currentCharge = 100.0;
    }
    
    double getBatteryLevel() const {
        return currentCharge;
    }
    
    std::string getInfo() const override {
        return Car::getInfo() + " - " + std::to_string(batteryCapacity) + " kWh battery";
    }
};
```

### Требования к решению
1. ✅ Замените наследование на композицию где это уместно
2. ✅ Создайте отдельные компоненты для различных функций
3. ✅ Используйте интерфейсы для полиморфизма
4. ✅ Сохраните необходимую функциональность
5. ✅ Сделайте код более гибким и расширяемым

### Ожидаемый результат
```cpp
// Компоненты для композиции
class Engine {
    // Двигатель
};

class Wheels {
    // Колеса
};

class Lights {
    // Фары
};

class CargoSpace {
    // Грузовое пространство
};

class Battery {
    // Батарея
};

// Основные классы с композицией
class Car {
    // Использует композицию компонентов
};

class Motorcycle {
    // Использует композицию компонентов
};

class Truck {
    // Использует композицию компонентов
};

class ElectricCar {
    // Использует композицию компонентов
};
```

---

## 📋 Задание 2: Система игровых персонажей с композицией

### Описание
Создайте систему игровых персонажей, используя композицию вместо наследования.

### Требования
1. **Компоненты**: Создайте отдельные компоненты для различных аспектов персонажа
2. **Композиция**: Персонажи состоят из компонентов
3. **Гибкость**: Легко добавлять новые типы персонажей
4. **Расширяемость**: Новые компоненты можно добавлять без изменения существующего кода

### Компоненты
```cpp
// Базовый интерфейс для всех компонентов
class Component {
public:
    virtual ~Component() = default;
    virtual std::string getType() const = 0;
};

// Компонент здоровья
class HealthComponent : public Component {
public:
    HealthComponent(int maxHealth) : maxHealth_(maxHealth), currentHealth_(maxHealth) {}
    
    std::string getType() const override { return "Health"; }
    
    void takeDamage(int damage);
    void heal(int amount);
    bool isAlive() const;
    int getCurrentHealth() const;
    int getMaxHealth() const;
    
private:
    int maxHealth_;
    int currentHealth_;
};

// Компонент маны
class ManaComponent : public Component {
public:
    ManaComponent(int maxMana) : maxMana_(maxMana), currentMana_(maxMana) {}
    
    std::string getType() const override { return "Mana"; }
    
    bool useMana(int amount);
    void restoreMana(int amount);
    int getCurrentMana() const;
    int getMaxMana() const;
    
private:
    int maxMana_;
    int currentMana_;
};

// Компонент инвентаря
class InventoryComponent : public Component {
public:
    InventoryComponent(int maxSlots) : maxSlots_(maxSlots) {}
    
    std::string getType() const override { return "Inventory"; }
    
    bool addItem(const std::string& item);
    bool removeItem(const std::string& item);
    bool hasItem(const std::string& item) const;
    std::vector<std::string> getItems() const;
    int getUsedSlots() const;
    int getMaxSlots() const;
    
private:
    std::vector<std::string> items_;
    int maxSlots_;
};

// Компонент оружия
class WeaponComponent : public Component {
public:
    WeaponComponent(const std::string& weaponName, int damage) 
        : weaponName_(weaponName), damage_(damage) {}
    
    std::string getType() const override { return "Weapon"; }
    
    int attack() const;
    void equipWeapon(const std::string& weaponName, int damage);
    std::string getWeaponName() const;
    
private:
    std::string weaponName_;
    int damage_;
};

// Компонент магии
class MagicComponent : public Component {
public:
    MagicComponent(const std::vector<std::string>& spells) : spells_(spells) {}
    
    std::string getType() const override { return "Magic"; }
    
    bool castSpell(const std::string& spellName);
    void learnSpell(const std::string& spellName);
    std::vector<std::string> getKnownSpells() const;
    
private:
    std::vector<std::string> spells_;
};

// Компонент брони
class ArmorComponent : public Component {
public:
    ArmorComponent(int defense) : defense_(defense) {}
    
    std::string getType() const override { return "Armor"; }
    
    int getDefense() const;
    void equipArmor(int defense);
    
private:
    int defense_;
};
```

### Система персонажей
```cpp
// Базовый класс персонажа с композицией
class Character {
public:
    Character(const std::string& name) : name_(name) {}
    
    // Добавление компонентов
    void addComponent(std::unique_ptr<Component> component);
    
    // Получение компонентов
    template<typename T>
    T* getComponent() const;
    
    // Проверка наличия компонента
    bool hasComponent(const std::string& componentType) const;
    
    // Основные методы
    void takeDamage(int damage);
    void heal(int amount);
    int attack() const;
    bool castSpell(const std::string& spellName);
    
    // Информация о персонаже
    std::string getName() const;
    std::string getInfo() const;
    void printStatus() const;
    
private:
    std::string name_;
    std::vector<std::unique_ptr<Component>> components_;
    
    // Вспомогательные методы
    Component* findComponent(const std::string& type) const;
};

// Фабрика персонажей
class CharacterFactory {
public:
    static std::unique_ptr<Character> createWarrior(const std::string& name);
    static std::unique_ptr<Character> createMage(const std::string& name);
    static std::unique_ptr<Character> createArcher(const std::string& name);
    static std::unique_ptr<Character> createPaladin(const std::string& name);
    static std::unique_ptr<Character> createRogue(const std::string& name);
};
```

### Тестовый случай
```cpp
int main() {
    // Создаем персонажей с помощью фабрики
    auto warrior = CharacterFactory::createWarrior("Александр");
    auto mage = CharacterFactory::createMage("Гандальф");
    auto archer = CharacterFactory::createArcher("Леголас");
    
    // Выводим информацию о персонажах
    warrior->printStatus();
    mage->printStatus();
    archer->printStatus();
    
    // Демонстрируем бой
    std::cout << "\n=== БОЙ ===" << std::endl;
    
    // Воин атакует мага
    int damage = warrior->attack();
    mage->takeDamage(damage);
    
    // Маг кастует заклинание
    mage->castSpell("Огненный шар");
    
    // Лучник атакует воина
    damage = archer->attack();
    warrior->takeDamage(damage);
    
    // Выводим статус после боя
    std::cout << "\n=== СТАТУС ПОСЛЕ БОЯ ===" << std::endl;
    warrior->printStatus();
    mage->printStatus();
    archer->printStatus();
    
    // Создаем кастомного персонажа
    auto customCharacter = std::make_unique<Character>("Кастомный герой");
    customCharacter->addComponent(std::make_unique<HealthComponent>(150));
    customCharacter->addComponent(std::make_unique<ManaComponent>(100));
    customCharacter->addComponent(std::make_unique<WeaponComponent>("Меч", 25));
    customCharacter->addComponent(std::make_unique<ArmorComponent>(15));
    
    std::cout << "\n=== КАСТОМНЫЙ ПЕРСОНАЖ ===" << std::endl;
    customCharacter->printStatus();
    
    return 0;
}
```

---

## 📋 Задание 3: Система управления файлами с композицией

### Описание
Создайте систему управления файлами, используя композицию для различных атрибутов файлов.

### Требования
1. **Компоненты**: Создайте компоненты для различных атрибутов файлов
2. **Композиция**: Файлы состоят из компонентов
3. **Гибкость**: Легко добавлять новые типы файлов
4. **Расширяемость**: Новые атрибуты можно добавлять без изменения существующего кода

### Компоненты файлов
```cpp
// Базовый интерфейс для компонентов файлов
class FileComponent {
public:
    virtual ~FileComponent() = default;
    virtual std::string getType() const = 0;
    virtual std::string getInfo() const = 0;
};

// Компонент размера файла
class SizeComponent : public FileComponent {
public:
    SizeComponent(size_t size) : size_(size) {}
    
    std::string getType() const override { return "Size"; }
    std::string getInfo() const override { return std::to_string(size_) + " bytes"; }
    
    size_t getSize() const { return size_; }
    void setSize(size_t size) { size_ = size; }
    
private:
    size_t size_;
};

// Компонент даты создания
class DateComponent : public FileComponent {
public:
    DateComponent(const std::string& creationDate) : creationDate_(creationDate) {}
    
    std::string getType() const override { return "Date"; }
    std::string getInfo() const override { return "Created: " + creationDate_; }
    
    std::string getCreationDate() const { return creationDate_; }
    void setCreationDate(const std::string& date) { creationDate_ = date; }
    
private:
    std::string creationDate_;
};

// Компонент прав доступа
class PermissionsComponent : public FileComponent {
public:
    PermissionsComponent(const std::string& permissions) : permissions_(permissions) {}
    
    std::string getType() const override { return "Permissions"; }
    std::string getInfo() const override { return "Permissions: " + permissions_; }
    
    std::string getPermissions() const { return permissions_; }
    void setPermissions(const std::string& permissions) { permissions_ = permissions; }
    
    bool canRead() const;
    bool canWrite() const;
    bool canExecute() const;
    
private:
    std::string permissions_;
};

// Компонент владельца
class OwnerComponent : public FileComponent {
public:
    OwnerComponent(const std::string& owner) : owner_(owner) {}
    
    std::string getType() const override { return "Owner"; }
    std::string getInfo() const override { return "Owner: " + owner_; }
    
    std::string getOwner() const { return owner_; }
    void setOwner(const std::string& owner) { owner_ = owner; }
    
private:
    std::string owner_;
};

// Компонент типа файла
class TypeComponent : public FileComponent {
public:
    TypeComponent(const std::string& mimeType) : mimeType_(mimeType) {}
    
    std::string getType() const override { return "Type"; }
    std::string getInfo() const override { return "Type: " + mimeType_; }
    
    std::string getMimeType() const { return mimeType_; }
    void setMimeType(const std::string& mimeType) { mimeType_ = mimeType; }
    
    bool isTextFile() const;
    bool isImageFile() const;
    bool isVideoFile() const;
    
private:
    std::string mimeType_;
};

// Компонент содержимого
class ContentComponent : public FileComponent {
public:
    ContentComponent(const std::string& content) : content_(content) {}
    
    std::string getType() const override { return "Content"; }
    std::string getInfo() const override { return "Content length: " + std::to_string(content_.length()); }
    
    std::string getContent() const { return content_; }
    void setContent(const std::string& content) { content_ = content; }
    
    size_t getContentLength() const { return content_.length(); }
    
private:
    std::string content_;
};
```

### Система файлов
```cpp
// Базовый класс файла с композицией
class File {
public:
    File(const std::string& name) : name_(name) {}
    
    // Добавление компонентов
    void addComponent(std::unique_ptr<FileComponent> component);
    
    // Получение компонентов
    template<typename T>
    T* getComponent() const;
    
    // Проверка наличия компонента
    bool hasComponent(const std::string& componentType) const;
    
    // Основные методы
    std::string getName() const;
    std::string getInfo() const;
    void printDetails() const;
    
    // Операции с файлом
    bool canRead() const;
    bool canWrite() const;
    bool canExecute() const;
    
private:
    std::string name_;
    std::vector<std::unique_ptr<FileComponent>> components_;
    
    // Вспомогательные методы
    FileComponent* findComponent(const std::string& type) const;
};

// Фабрика файлов
class FileFactory {
public:
    static std::unique_ptr<File> createTextFile(const std::string& name, const std::string& content);
    static std::unique_ptr<File> createImageFile(const std::string& name, size_t size);
    static std::unique_ptr<File> createExecutableFile(const std::string& name, size_t size);
    static std::unique_ptr<File> createConfigFile(const std::string& name, const std::string& content);
};
```

### Тестовый случай
```cpp
int main() {
    // Создаем файлы с помощью фабрики
    auto textFile = FileFactory::createTextFile("readme.txt", "Это файл README");
    auto imageFile = FileFactory::createImageFile("photo.jpg", 1024000);
    auto executableFile = FileFactory::createExecutableFile("program.exe", 2048000);
    
    // Выводим информацию о файлах
    textFile->printDetails();
    imageFile->printDetails();
    executableFile->printDetails();
    
    // Создаем кастомный файл
    auto customFile = std::make_unique<File>("custom.dat");
    customFile->addComponent(std::make_unique<SizeComponent>(512));
    customFile->addComponent(std::make_unique<DateComponent>("2024-01-15"));
    customFile->addComponent(std::make_unique<PermissionsComponent>("rw-r--r--"));
    customFile->addComponent(std::make_unique<OwnerComponent>("user"));
    customFile->addComponent(std::make_unique<TypeComponent>("application/octet-stream"));
    
    std::cout << "\n=== КАСТОМНЫЙ ФАЙЛ ===" << std::endl;
    customFile->printDetails();
    
    // Проверяем права доступа
    std::cout << "\n=== ПРОВЕРКА ПРАВ ДОСТУПА ===" << std::endl;
    std::cout << "textFile can read: " << textFile->canRead() << std::endl;
    std::cout << "textFile can write: " << textFile->canWrite() << std::endl;
    std::cout << "executableFile can execute: " << executableFile->canExecute() << std::endl;
    
    return 0;
}
```

---

## 📋 Задание 4: Система управления заказами с композицией

### Описание
Создайте систему управления заказами, используя композицию для различных аспектов заказов.

### Требования
1. **Компоненты**: Создайте компоненты для различных аспектов заказов
2. **Композиция**: Заказы состоят из компонентов
3. **Гибкость**: Легко добавлять новые типы заказов
4. **Расширяемость**: Новые атрибуты можно добавлять без изменения существующего кода

### Компоненты заказов
```cpp
// Базовый интерфейс для компонентов заказов
class OrderComponent {
public:
    virtual ~OrderComponent() = default;
    virtual std::string getType() const = 0;
    virtual std::string getInfo() const = 0;
};

// Компонент клиента
class CustomerComponent : public OrderComponent {
public:
    CustomerComponent(const std::string& customerId, const std::string& customerName) 
        : customerId_(customerId), customerName_(customerName) {}
    
    std::string getType() const override { return "Customer"; }
    std::string getInfo() const override { return "Customer: " + customerName_ + " (" + customerId_ + ")"; }
    
    std::string getCustomerId() const { return customerId_; }
    std::string getCustomerName() const { return customerName_; }
    
private:
    std::string customerId_;
    std::string customerName_;
};

// Компонент товаров
class ItemsComponent : public OrderComponent {
public:
    ItemsComponent() = default;
    
    std::string getType() const override { return "Items"; }
    std::string getInfo() const override { return "Items: " + std::to_string(items_.size()); }
    
    void addItem(const std::string& itemId, const std::string& itemName, int quantity, double price);
    void removeItem(const std::string& itemId);
    std::vector<std::tuple<std::string, std::string, int, double>> getItems() const;
    double getTotalPrice() const;
    int getTotalQuantity() const;
    
private:
    std::vector<std::tuple<std::string, std::string, int, double>> items_;
};

// Компонент доставки
class ShippingComponent : public OrderComponent {
public:
    ShippingComponent(const std::string& address, const std::string& method) 
        : address_(address), method_(method) {}
    
    std::string getType() const override { return "Shipping"; }
    std::string getInfo() const override { return "Shipping: " + method_ + " to " + address_; }
    
    std::string getAddress() const { return address_; }
    std::string getMethod() const { return method_; }
    void setAddress(const std::string& address) { address_ = address; }
    void setMethod(const std::string& method) { method_ = method; }
    
private:
    std::string address_;
    std::string method_;
};

// Компонент оплаты
class PaymentComponent : public OrderComponent {
public:
    PaymentComponent(const std::string& method, const std::string& status) 
        : method_(method), status_(status) {}
    
    std::string getType() const override { return "Payment"; }
    std::string getInfo() const override { return "Payment: " + method_ + " (" + status_ + ")"; }
    
    std::string getMethod() const { return method_; }
    std::string getStatus() const { return status_; }
    void setStatus(const std::string& status) { status_ = status; }
    
    bool isPaid() const { return status_ == "paid"; }
    
private:
    std::string method_;
    std::string status_;
};

// Компонент статуса заказа
class StatusComponent : public OrderComponent {
public:
    StatusComponent(const std::string& status) : status_(status) {}
    
    std::string getType() const override { return "Status"; }
    std::string getInfo() const override { return "Status: " + status_; }
    
    std::string getStatus() const { return status_; }
    void setStatus(const std::string& status) { status_ = status; }
    
    bool isPending() const { return status_ == "pending"; }
    bool isProcessing() const { return status_ == "processing"; }
    bool isShipped() const { return status_ == "shipped"; }
    bool isDelivered() const { return status_ == "delivered"; }
    bool isCancelled() const { return status_ == "cancelled"; }
    
private:
    std::string status_;
};
```

### Система заказов
```cpp
// Базовый класс заказа с композицией
class Order {
public:
    Order(const std::string& orderId) : orderId_(orderId) {}
    
    // Добавление компонентов
    void addComponent(std::unique_ptr<OrderComponent> component);
    
    // Получение компонентов
    template<typename T>
    T* getComponent() const;
    
    // Проверка наличия компонента
    bool hasComponent(const std::string& componentType) const;
    
    // Основные методы
    std::string getOrderId() const;
    std::string getInfo() const;
    void printDetails() const;
    
    // Операции с заказом
    bool canShip() const;
    bool canCancel() const;
    bool isComplete() const;
    
private:
    std::string orderId_;
    std::vector<std::unique_ptr<OrderComponent>> components_;
    
    // Вспомогательные методы
    OrderComponent* findComponent(const std::string& type) const;
};

// Фабрика заказов
class OrderFactory {
public:
    static std::unique_ptr<Order> createOnlineOrder(const std::string& orderId, 
                                                   const std::string& customerId,
                                                   const std::string& customerName);
    static std::unique_ptr<Order> createStoreOrder(const std::string& orderId,
                                                  const std::string& customerId,
                                                  const std::string& customerName);
    static std::unique_ptr<Order> createWholesaleOrder(const std::string& orderId,
                                                      const std::string& customerId,
                                                      const std::string& customerName);
};
```

### Тестовый случай
```cpp
int main() {
    // Создаем заказы с помощью фабрики
    auto onlineOrder = OrderFactory::createOnlineOrder("ORD001", "CUST001", "Иван Иванов");
    auto storeOrder = OrderFactory::createStoreOrder("ORD002", "CUST002", "Петр Петров");
    
    // Выводим информацию о заказах
    onlineOrder->printDetails();
    storeOrder->printDetails();
    
    // Создаем кастомный заказ
    auto customOrder = std::make_unique<Order>("ORD003");
    customOrder->addComponent(std::make_unique<CustomerComponent>("CUST003", "Мария Сидорова"));
    
    auto itemsComponent = std::make_unique<ItemsComponent>();
    itemsComponent->addItem("ITEM001", "Ноутбук", 1, 50000.0);
    itemsComponent->addItem("ITEM002", "Мышь", 2, 1500.0);
    customOrder->addComponent(std::move(itemsComponent));
    
    customOrder->addComponent(std::make_unique<ShippingComponent>("Москва, ул. Ленина, 1", "Курьер"));
    customOrder->addComponent(std::make_unique<PaymentComponent>("Карта", "paid"));
    customOrder->addComponent(std::make_unique<StatusComponent>("processing"));
    
    std::cout << "\n=== КАСТОМНЫЙ ЗАКАЗ ===" << std::endl;
    customOrder->printDetails();
    
    // Проверяем возможности заказа
    std::cout << "\n=== ПРОВЕРКА ВОЗМОЖНОСТЕЙ ===" << std::endl;
    std::cout << "onlineOrder can ship: " << onlineOrder->canShip() << std::endl;
    std::cout << "customOrder can cancel: " << customOrder->canCancel() << std::endl;
    std::cout << "customOrder is complete: " << customOrder->isComplete() << std::endl;
    
    return 0;
}
```

---

## 📋 Задание 5: Комплексная система с композицией

### Описание
Создайте систему управления игрой, используя композицию для различных игровых объектов.

### Требования
1. **Компоненты**: Создайте компоненты для различных аспектов игровых объектов
2. **Композиция**: Игровые объекты состоят из компонентов
3. **Гибкость**: Легко добавлять новые типы объектов
4. **Расширяемость**: Новые компоненты можно добавлять без изменения существующего кода

### Компоненты игровых объектов
```cpp
// Базовый интерфейс для компонентов игровых объектов
class GameObjectComponent {
public:
    virtual ~GameObjectComponent() = default;
    virtual std::string getType() const = 0;
    virtual void update(float deltaTime) = 0;
    virtual std::string getInfo() const = 0;
};

// Компонент позиции
class PositionComponent : public GameObjectComponent {
public:
    PositionComponent(float x, float y, float z) : x_(x), y_(y), z_(z) {}
    
    std::string getType() const override { return "Position"; }
    void update(float deltaTime) override {}
    std::string getInfo() const override { 
        return "Position: (" + std::to_string(x_) + ", " + std::to_string(y_) + ", " + std::to_string(z_) + ")"; 
    }
    
    void setPosition(float x, float y, float z) { x_ = x; y_ = y; z_ = z; }
    void move(float dx, float dy, float dz) { x_ += dx; y_ += dy; z_ += dz; }
    
    float getX() const { return x_; }
    float getY() const { return y_; }
    float getZ() const { return z_; }
    
private:
    float x_, y_, z_;
};

// Компонент здоровья
class HealthComponent : public GameObjectComponent {
public:
    HealthComponent(int maxHealth) : maxHealth_(maxHealth), currentHealth_(maxHealth) {}
    
    std::string getType() const override { return "Health"; }
    void update(float deltaTime) override {}
    std::string getInfo() const override { 
        return "Health: " + std::to_string(currentHealth_) + "/" + std::to_string(maxHealth_); 
    }
    
    void takeDamage(int damage) { currentHealth_ = std::max(0, currentHealth_ - damage); }
    void heal(int amount) { currentHealth_ = std::min(maxHealth_, currentHealth_ + amount); }
    bool isAlive() const { return currentHealth_ > 0; }
    
    int getCurrentHealth() const { return currentHealth_; }
    int getMaxHealth() const { return maxHealth_; }
    
private:
    int maxHealth_;
    int currentHealth_;
};

// Компонент движения
class MovementComponent : public GameObjectComponent {
public:
    MovementComponent(float speed) : speed_(speed), velocityX_(0), velocityY_(0), velocityZ_(0) {}
    
    std::string getType() const override { return "Movement"; }
    void update(float deltaTime) override {}
    std::string getInfo() const override { 
        return "Speed: " + std::to_string(speed_) + ", Velocity: (" + 
               std::to_string(velocityX_) + ", " + std::to_string(velocityY_) + ", " + std::to_string(velocityZ_) + ")"; 
    }
    
    void setVelocity(float vx, float vy, float vz) { velocityX_ = vx; velocityY_ = vy; velocityZ_ = vz; }
    void setSpeed(float speed) { speed_ = speed; }
    
    float getSpeed() const { return speed_; }
    float getVelocityX() const { return velocityX_; }
    float getVelocityY() const { return velocityY_; }
    float getVelocityZ() const { return velocityZ_; }
    
private:
    float speed_;
    float velocityX_, velocityY_, velocityZ_;
};

// Компонент рендеринга
class RenderComponent : public GameObjectComponent {
public:
    RenderComponent(const std::string& model) : model_(model), visible_(true) {}
    
    std::string getType() const override { return "Render"; }
    void update(float deltaTime) override {}
    std::string getInfo() const override { 
        return "Model: " + model_ + ", Visible: " + (visible_ ? "Yes" : "No"); 
    }
    
    void setModel(const std::string& model) { model_ = model; }
    void setVisible(bool visible) { visible_ = visible; }
    
    std::string getModel() const { return model_; }
    bool isVisible() const { return visible_; }
    
private:
    std::string model_;
    bool visible_;
};

// Компонент коллизии
class CollisionComponent : public GameObjectComponent {
public:
    CollisionComponent(float width, float height, float depth) 
        : width_(width), height_(height), depth_(depth), enabled_(true) {}
    
    std::string getType() const override { return "Collision"; }
    void update(float deltaTime) override {}
    std::string getInfo() const override { 
        return "Size: " + std::to_string(width_) + "x" + std::to_string(height_) + "x" + std::to_string(depth_) + 
               ", Enabled: " + (enabled_ ? "Yes" : "No"); 
    }
    
    void setSize(float width, float height, float depth) { width_ = width; height_ = height; depth_ = depth; }
    void setEnabled(bool enabled) { enabled_ = enabled; }
    
    bool checkCollision(const CollisionComponent& other) const;
    
    float getWidth() const { return width_; }
    float getHeight() const { return height_; }
    float getDepth() const { return depth_; }
    bool isEnabled() const { return enabled_; }
    
private:
    float width_, height_, depth_;
    bool enabled_;
};
```

### Система игровых объектов
```cpp
// Базовый класс игрового объекта с композицией
class GameObject {
public:
    GameObject(const std::string& name) : name_(name) {}
    
    // Добавление компонентов
    void addComponent(std::unique_ptr<GameObjectComponent> component);
    
    // Получение компонентов
    template<typename T>
    T* getComponent() const;
    
    // Проверка наличия компонента
    bool hasComponent(const std::string& componentType) const;
    
    // Основные методы
    std::string getName() const;
    void update(float deltaTime);
    std::string getInfo() const;
    void printDetails() const;
    
private:
    std::string name_;
    std::vector<std::unique_ptr<GameObjectComponent>> components_;
    
    // Вспомогательные методы
    GameObjectComponent* findComponent(const std::string& type) const;
};

// Фабрика игровых объектов
class GameObjectFactory {
public:
    static std::unique_ptr<GameObject> createPlayer(const std::string& name);
    static std::unique_ptr<GameObject> createEnemy(const std::string& name);
    static std::unique_ptr<GameObject> createItem(const std::string& name);
    static std::unique_ptr<GameObject> createObstacle(const std::string& name);
};
```

### Тестовый случай
```cpp
int main() {
    // Создаем игровые объекты с помощью фабрики
    auto player = GameObjectFactory::createPlayer("Игрок");
    auto enemy = GameObjectFactory::createEnemy("Враг");
    auto item = GameObjectFactory::createItem("Зелье здоровья");
    auto obstacle = GameObjectFactory::createObstacle("Стена");
    
    // Выводим информацию об объектах
    player->printDetails();
    enemy->printDetails();
    item->printDetails();
    obstacle->printDetails();
    
    // Создаем кастомный объект
    auto customObject = std::make_unique<GameObject>("Кастомный объект");
    customObject->addComponent(std::make_unique<PositionComponent>(10.0f, 5.0f, 0.0f));
    customObject->addComponent(std::make_unique<HealthComponent>(100));
    customObject->addComponent(std::make_unique<MovementComponent>(5.0f));
    customObject->addComponent(std::make_unique<RenderComponent>("custom_model.obj"));
    customObject->addComponent(std::make_unique<CollisionComponent>(2.0f, 2.0f, 2.0f));
    
    std::cout << "\n=== КАСТОМНЫЙ ОБЪЕКТ ===" << std::endl;
    customObject->printDetails();
    
    // Обновляем объекты
    std::cout << "\n=== ОБНОВЛЕНИЕ ОБЪЕКТОВ ===" << std::endl;
    player->update(0.016f);
    enemy->update(0.016f);
    customObject->update(0.016f);
    
    return 0;
}
```

---

## 🎯 Критерии оценки

### Отлично (5 баллов)
- ✅ Композиция используется вместо наследования где это уместно
- ✅ Компоненты хорошо разделены по ответственности
- ✅ Код гибкий и расширяемый
- ✅ Легко добавлять новые типы объектов
- ✅ Код следует современным стандартам C++

### Хорошо (4 балла)
- ✅ Композиция используется в большинстве случаев
- ✅ Компоненты в целом хорошо разделены
- ✅ Есть небольшие проблемы с архитектурой
- ✅ Код работает корректно

### Удовлетворительно (3 балла)
- ✅ Базовая композиция реализована
- ✅ Код компилируется и работает
- ❌ Есть проблемы с разделением ответственности
- ❌ Код сложно расширять

### Неудовлетворительно (2 балла)
- ❌ Композиция не используется или используется неправильно
- ❌ Код плохо структурирован
- ❌ Код не компилируется или работает неправильно

---

## 💡 Подсказки

1. **Композиция vs Наследование**: Используйте композицию для "has-a" отношений
2. **Компоненты**: Разделяйте функциональность на независимые компоненты
3. **Гибкость**: Композиция делает код более гибким
4. **Расширяемость**: Новые компоненты можно добавлять без изменения существующего кода
5. **Тестирование**: Компоненты легче тестировать по отдельности

---

*Композиция vs Наследование - это важный выбор в дизайне. Правильное использование композиции делает код более гибким и поддерживаемым!*
