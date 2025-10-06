/**
 * @file isp_example.cpp
 * @brief Детальная демонстрация Interface Segregation Principle (ISP)
 * @author Senior C++ Developer
 * @date 2024
 * 
 * Этот файл содержит углубленный анализ ISP с математическим обоснованием
 * и практическими примерами из реальной разработки.
 */

#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <map>
#include <set>

/**
 * @brief Математическая формализация ISP
 * 
 * ISP можно формализовать следующим образом:
 * ∀I ∈ Interfaces, ∀C ∈ Clients: 
 * |UsedMethods(I,C)| / |AllMethods(I)| → 1
 * 
 * Другими словами: клиенты не должны зависеть от интерфейсов,
 * которые они не используют.
 */

// ============================================================================
// ПРИМЕР 1: СИСТЕМА УСТРОЙСТВ (КЛАССИЧЕСКИЙ ПРИМЕР)
// ============================================================================

/**
 * @brief Нарушение ISP: Слишком большой интерфейс устройства
 * 
 * Этот интерфейс нарушает ISP, так как содержит методы, которые
 * не все устройства должны поддерживать.
 */
class BadIDevice {
public:
    virtual ~BadIDevice() = default;
    virtual void read() = 0;
    virtual void write() = 0;
    virtual void scan() = 0;
    virtual void print() = 0;
    virtual void fax() = 0;
    virtual void copy() = 0;
    virtual void networkConnect() = 0;
    virtual void bluetoothConnect() = 0;
    virtual void wifiConnect() = 0;
    virtual void usbConnect() = 0;
};

/**
 * @brief Нарушение ISP: Принтер вынужден реализовывать ненужные методы
 */
class BadPrinter : public BadIDevice {
public:
    void read() override {
        throw std::runtime_error("Принтер не может читать файлы!");
    }
    
    void write() override {
        throw std::runtime_error("Принтер не может записывать файлы!");
    }
    
    void scan() override {
        throw std::runtime_error("Принтер не может сканировать!");
    }
    
    void print() override {
        std::cout << "🖨️ Печать документа\n";
    }
    
    void fax() override {
        throw std::runtime_error("Принтер не может отправлять факсы!");
    }
    
    void copy() override {
        std::cout << "📋 Копирование документа\n";
    }
    
    void networkConnect() override {
        std::cout << "🌐 Подключение к сети\n";
    }
    
    void bluetoothConnect() override {
        throw std::runtime_error("Принтер не поддерживает Bluetooth!");
    }
    
    void wifiConnect() override {
        std::cout << "📶 Подключение к Wi-Fi\n";
    }
    
    void usbConnect() override {
        std::cout << "🔌 Подключение через USB\n";
    }
};

/**
 * @brief Нарушение ISP: Сканер вынужден реализовывать ненужные методы
 */
class BadScanner : public BadIDevice {
public:
    void read() override {
        std::cout << "📖 Чтение документа\n";
    }
    
    void write() override {
        throw std::runtime_error("Сканер не может записывать файлы!");
    }
    
    void scan() override {
        std::cout << "📄 Сканирование документа\n";
    }
    
    void print() override {
        throw std::runtime_error("Сканер не может печатать!");
    }
    
    void fax() override {
        throw std::runtime_error("Сканер не может отправлять факсы!");
    }
    
    void copy() override {
        throw std::runtime_error("Сканер не может копировать!");
    }
    
    void networkConnect() override {
        throw std::runtime_error("Сканер не поддерживает сеть!");
    }
    
    void bluetoothConnect() override {
        std::cout << "🔵 Подключение через Bluetooth\n";
    }
    
    void wifiConnect() override {
        throw std::runtime_error("Сканер не поддерживает Wi-Fi!");
    }
    
    void usbConnect() override {
        std::cout << "🔌 Подключение через USB\n";
    }
};

/**
 * @brief Функция, которая демонстрирует нарушение ISP
 */
void demonstrateBadISP() {
    std::cout << "❌ НАРУШЕНИЕ ISP - Слишком большой интерфейс:\n";
    std::cout << std::string(50, '-') << "\n";
    
    std::unique_ptr<BadIDevice> printer = std::make_unique<BadPrinter>();
    std::unique_ptr<BadIDevice> scanner = std::make_unique<BadScanner>();
    
    std::cout << "Тестирование принтера:\n";
    try {
        printer->print();
        printer->copy();
        printer->networkConnect();
        printer->read(); // Выбросит исключение!
    } catch (const std::exception& e) {
        std::cout << "Ошибка: " << e.what() << "\n";
    }
    
    std::cout << "\nТестирование сканера:\n";
    try {
        scanner->read();
        scanner->scan();
        scanner->bluetoothConnect();
        scanner->print(); // Выбросит исключение!
    } catch (const std::exception& e) {
        std::cout << "Ошибка: " << e.what() << "\n";
    }
    
    std::cout << "\n🚨 ПРОБЛЕМА: Устройства вынуждены реализовывать ненужные методы!\n";
}

// ============================================================================
// ПРИМЕР 2: ПРАВИЛЬНАЯ РЕАЛИЗАЦИЯ ISP
// ============================================================================

/**
 * @brief Соблюдение ISP: Разделение интерфейсов по функциональности
 */

// Интерфейсы для основных функций устройств
class IReadable {
public:
    virtual ~IReadable() = default;
    virtual void read() = 0;
};

class IWritable {
public:
    virtual ~IWritable() = default;
    virtual void write() = 0;
};

class IScannable {
public:
    virtual ~IScannable() = default;
    virtual void scan() = 0;
};

class IPrintable {
public:
    virtual ~IPrintable() = default;
    virtual void print() = 0;
};

class IFaxable {
public:
    virtual ~IFaxable() = default;
    virtual void fax() = 0;
};

class ICopyable {
public:
    virtual ~ICopyable() = default;
    virtual void copy() = 0;
};

// Интерфейсы для подключений
class INetworkConnectable {
public:
    virtual ~INetworkConnectable() = default;
    virtual void networkConnect() = 0;
    virtual void networkDisconnect() = 0;
    virtual bool isNetworkConnected() const = 0;
};

class IBluetoothConnectable {
public:
    virtual ~IBluetoothConnectable() = default;
    virtual void bluetoothConnect() = 0;
    virtual void bluetoothDisconnect() = 0;
    virtual bool isBluetoothConnected() const = 0;
};

class IWiFiConnectable {
public:
    virtual ~IWiFiConnectable() = default;
    virtual void wifiConnect() = 0;
    virtual void wifiDisconnect() = 0;
    virtual bool isWiFiConnected() const = 0;
};

class IUSBConnectable {
public:
    virtual ~IUSBConnectable() = default;
    virtual void usbConnect() = 0;
    virtual void usbDisconnect() = 0;
    virtual bool isUSBConnected() const = 0;
};

// Конкретные устройства реализуют только нужные им интерфейсы
class Printer : public IPrintable, public ICopyable, public INetworkConnectable, public IUSBConnectable {
private:
    bool networkConnected = false;
    bool usbConnected = false;
    
public:
    void print() override {
        std::cout << "🖨️ Печать документа\n";
    }
    
    void copy() override {
        std::cout << "📋 Копирование документа\n";
    }
    
    void networkConnect() override {
        networkConnected = true;
        std::cout << "🌐 Принтер подключен к сети\n";
    }
    
    void networkDisconnect() override {
        networkConnected = false;
        std::cout << "🌐 Принтер отключен от сети\n";
    }
    
    bool isNetworkConnected() const override {
        return networkConnected;
    }
    
    void usbConnect() override {
        usbConnected = true;
        std::cout << "🔌 Принтер подключен через USB\n";
    }
    
    void usbDisconnect() override {
        usbConnected = false;
        std::cout << "🔌 Принтер отключен от USB\n";
    }
    
    bool isUSBConnected() const override {
        return usbConnected;
    }
};

class Scanner : public IReadable, public IScannable, public IBluetoothConnectable, public IUSBConnectable {
private:
    bool bluetoothConnected = false;
    bool usbConnected = false;
    
public:
    void read() override {
        std::cout << "📖 Чтение документа\n";
    }
    
    void scan() override {
        std::cout << "📄 Сканирование документа\n";
    }
    
    void bluetoothConnect() override {
        bluetoothConnected = true;
        std::cout << "🔵 Сканер подключен через Bluetooth\n";
    }
    
    void bluetoothDisconnect() override {
        bluetoothConnected = false;
        std::cout << "🔵 Сканер отключен от Bluetooth\n";
    }
    
    bool isBluetoothConnected() const override {
        return bluetoothConnected;
    }
    
    void usbConnect() override {
        usbConnected = true;
        std::cout << "🔌 Сканер подключен через USB\n";
    }
    
    void usbDisconnect() override {
        usbConnected = false;
        std::cout << "🔌 Сканер отключен от USB\n";
    }
    
    bool isUSBConnected() const override {
        return usbConnected;
    }
};

class MultiFunctionDevice : public IPrintable, public IScannable, public IFaxable, 
                           public ICopyable, public INetworkConnectable, public IWiFiConnectable {
private:
    bool networkConnected = false;
    bool wifiConnected = false;
    
public:
    void print() override {
        std::cout << "🖨️ МФУ: Печать документа\n";
    }
    
    void scan() override {
        std::cout << "📄 МФУ: Сканирование документа\n";
    }
    
    void fax() override {
        std::cout << "📠 МФУ: Отправка факса\n";
    }
    
    void copy() override {
        std::cout << "📋 МФУ: Копирование документа\n";
    }
    
    void networkConnect() override {
        networkConnected = true;
        std::cout << "🌐 МФУ: Подключен к сети\n";
    }
    
    void networkDisconnect() override {
        networkConnected = false;
        std::cout << "🌐 МФУ: Отключен от сети\n";
    }
    
    bool isNetworkConnected() const override {
        return networkConnected;
    }
    
    void wifiConnect() override {
        wifiConnected = true;
        std::cout << "📶 МФУ: Подключен к Wi-Fi\n";
    }
    
    void wifiDisconnect() override {
        wifiConnected = false;
        std::cout << "📶 МФУ: Отключен от Wi-Fi\n";
    }
    
    bool isWiFiConnected() const override {
        return wifiConnected;
    }
};

class SmartPhone : public IReadable, public IWritable, public IPrintable, 
                   public IBluetoothConnectable, public IWiFiConnectable, public IUSBConnectable {
private:
    bool bluetoothConnected = false;
    bool wifiConnected = false;
    bool usbConnected = false;
    
public:
    void read() override {
        std::cout << "📱 Смартфон: Чтение файла\n";
    }
    
    void write() override {
        std::cout << "📱 Смартфон: Запись файла\n";
    }
    
    void print() override {
        std::cout << "📱 Смартфон: Печать через AirPrint\n";
    }
    
    void bluetoothConnect() override {
        bluetoothConnected = true;
        std::cout << "🔵 Смартфон: Подключен через Bluetooth\n";
    }
    
    void bluetoothDisconnect() override {
        bluetoothConnected = false;
        std::cout << "🔵 Смартфон: Отключен от Bluetooth\n";
    }
    
    bool isBluetoothConnected() const override {
        return bluetoothConnected;
    }
    
    void wifiConnect() override {
        wifiConnected = true;
        std::cout << "📶 Смартфон: Подключен к Wi-Fi\n";
    }
    
    void wifiDisconnect() override {
        wifiConnected = false;
        std::cout << "📶 Смартфон: Отключен от Wi-Fi\n";
    }
    
    bool isWiFiConnected() const override {
        return wifiConnected;
    }
    
    void usbConnect() override {
        usbConnected = true;
        std::cout << "🔌 Смартфон: Подключен через USB\n";
    }
    
    void usbDisconnect() override {
        usbConnected = false;
        std::cout << "🔌 Смартфон: Отключен от USB\n";
    }
    
    bool isUSBConnected() const override {
        return usbConnected;
    }
};

// ============================================================================
// ПРИМЕР 3: СИСТЕМА УПРАВЛЕНИЯ ПОЛЬЗОВАТЕЛЯМИ
// ============================================================================

/**
 * @brief Нарушение ISP: Слишком большой интерфейс пользователя
 */
class BadIUser {
public:
    virtual ~BadIUser() = default;
    virtual void login() = 0;
    virtual void logout() = 0;
    virtual void createPost() = 0;
    virtual void deletePost() = 0;
    virtual void likePost() = 0;
    virtual void commentPost() = 0;
    virtual void followUser() = 0;
    virtual void unfollowUser() = 0;
    virtual void sendMessage() = 0;
    virtual void receiveMessage() = 0;
    virtual void uploadFile() = 0;
    virtual void downloadFile() = 0;
    virtual void createGroup() = 0;
    virtual void joinGroup() = 0;
    virtual void leaveGroup() = 0;
    virtual void moderateContent() = 0;
    virtual void banUser() = 0;
    virtual void unbanUser() = 0;
};

/**
 * @brief Соблюдение ISP: Разделение интерфейсов пользователей
 */

// Базовые интерфейсы
class IAuthenticatable {
public:
    virtual ~IAuthenticatable() = default;
    virtual void login() = 0;
    virtual void logout() = 0;
    virtual bool isLoggedIn() const = 0;
};

class IPostManager {
public:
    virtual ~IPostManager() = default;
    virtual void createPost() = 0;
    virtual void deletePost() = 0;
    virtual void editPost() = 0;
};

class ISocialInteraction {
public:
    virtual ~ISocialInteraction() = default;
    virtual void likePost() = 0;
    virtual void commentPost() = 0;
    virtual void followUser() = 0;
    virtual void unfollowUser() = 0;
};

class IMessageManager {
public:
    virtual ~IMessageManager() = default;
    virtual void sendMessage() = 0;
    virtual void receiveMessage() = 0;
    virtual void deleteMessage() = 0;
};

class IFileManager {
public:
    virtual ~IFileManager() = default;
    virtual void uploadFile() = 0;
    virtual void downloadFile() = 0;
    virtual void deleteFile() = 0;
};

class IGroupManager {
public:
    virtual ~IGroupManager() = default;
    virtual void createGroup() = 0;
    virtual void joinGroup() = 0;
    virtual void leaveGroup() = 0;
    virtual void inviteToGroup() = 0;
};

class IModerator {
public:
    virtual ~IModerator() = default;
    virtual void moderateContent() = 0;
    virtual void banUser() = 0;
    virtual void unbanUser() = 0;
    virtual void deleteInappropriateContent() = 0;
};

// Конкретные типы пользователей
class RegularUser : public IAuthenticatable, public IPostManager, public ISocialInteraction, public IMessageManager {
private:
    bool loggedIn = false;
    
public:
    void login() override {
        loggedIn = true;
        std::cout << "👤 Обычный пользователь вошел в систему\n";
    }
    
    void logout() override {
        loggedIn = false;
        std::cout << "👤 Обычный пользователь вышел из системы\n";
    }
    
    bool isLoggedIn() const override {
        return loggedIn;
    }
    
    void createPost() override {
        std::cout << "📝 Обычный пользователь создал пост\n";
    }
    
    void deletePost() override {
        std::cout << "🗑️ Обычный пользователь удалил свой пост\n";
    }
    
    void editPost() override {
        std::cout << "✏️ Обычный пользователь отредактировал пост\n";
    }
    
    void likePost() override {
        std::cout << "❤️ Обычный пользователь поставил лайк\n";
    }
    
    void commentPost() override {
        std::cout << "💬 Обычный пользователь прокомментировал пост\n";
    }
    
    void followUser() override {
        std::cout << "➕ Обычный пользователь подписался на пользователя\n";
    }
    
    void unfollowUser() override {
        std::cout << "➖ Обычный пользователь отписался от пользователя\n";
    }
    
    void sendMessage() override {
        std::cout << "📤 Обычный пользователь отправил сообщение\n";
    }
    
    void receiveMessage() override {
        std::cout << "📥 Обычный пользователь получил сообщение\n";
    }
    
    void deleteMessage() override {
        std::cout << "🗑️ Обычный пользователь удалил сообщение\n";
    }
};

class ContentCreator : public IAuthenticatable, public IPostManager, public ISocialInteraction, 
                       public IMessageManager, public IFileManager {
private:
    bool loggedIn = false;
    
public:
    void login() override {
        loggedIn = true;
        std::cout << "🎨 Контент-креатор вошел в систему\n";
    }
    
    void logout() override {
        loggedIn = false;
        std::cout << "🎨 Контент-креатор вышел из системы\n";
    }
    
    bool isLoggedIn() const override {
        return loggedIn;
    }
    
    void createPost() override {
        std::cout << "🎨 Контент-креатор создал креативный пост\n";
    }
    
    void deletePost() override {
        std::cout << "🗑️ Контент-креатор удалил пост\n";
    }
    
    void editPost() override {
        std::cout << "✏️ Контент-креатор отредактировал пост\n";
    }
    
    void likePost() override {
        std::cout << "❤️ Контент-креатор поставил лайк\n";
    }
    
    void commentPost() override {
        std::cout << "💬 Контент-креатор прокомментировал пост\n";
    }
    
    void followUser() override {
        std::cout << "➕ Контент-креатор подписался на пользователя\n";
    }
    
    void unfollowUser() override {
        std::cout << "➖ Контент-креатор отписался от пользователя\n";
    }
    
    void sendMessage() override {
        std::cout << "📤 Контент-креатор отправил сообщение\n";
    }
    
    void receiveMessage() override {
        std::cout << "📥 Контент-креатор получил сообщение\n";
    }
    
    void deleteMessage() override {
        std::cout << "🗑️ Контент-креатор удалил сообщение\n";
    }
    
    void uploadFile() override {
        std::cout << "📤 Контент-креатор загрузил файл (изображение/видео)\n";
    }
    
    void downloadFile() override {
        std::cout << "📥 Контент-креатор скачал файл\n";
    }
    
    void deleteFile() override {
        std::cout << "🗑️ Контент-креатор удалил файл\n";
    }
};

class Moderator : public IAuthenticatable, public IPostManager, public ISocialInteraction, 
                  public IMessageManager, public IModerator {
private:
    bool loggedIn = false;
    
public:
    void login() override {
        loggedIn = true;
        std::cout << "🛡️ Модератор вошел в систему\n";
    }
    
    void logout() override {
        loggedIn = false;
        std::cout << "🛡️ Модератор вышел из системы\n";
    }
    
    bool isLoggedIn() const override {
        return loggedIn;
    }
    
    void createPost() override {
        std::cout << "📝 Модератор создал официальный пост\n";
    }
    
    void deletePost() override {
        std::cout << "🗑️ Модератор удалил пост\n";
    }
    
    void editPost() override {
        std::cout << "✏️ Модератор отредактировал пост\n";
    }
    
    void likePost() override {
        std::cout << "❤️ Модератор поставил лайк\n";
    }
    
    void commentPost() override {
        std::cout << "💬 Модератор прокомментировал пост\n";
    }
    
    void followUser() override {
        std::cout << "➕ Модератор подписался на пользователя\n";
    }
    
    void unfollowUser() override {
        std::cout << "➖ Модератор отписался от пользователя\n";
    }
    
    void sendMessage() override {
        std::cout << "📤 Модератор отправил сообщение\n";
    }
    
    void receiveMessage() override {
        std::cout << "📥 Модератор получил сообщение\n";
    }
    
    void deleteMessage() override {
        std::cout << "🗑️ Модератор удалил сообщение\n";
    }
    
    void moderateContent() override {
        std::cout << "🔍 Модератор модерирует контент\n";
    }
    
    void banUser() override {
        std::cout << "🚫 Модератор заблокировал пользователя\n";
    }
    
    void unbanUser() override {
        std::cout << "✅ Модератор разблокировал пользователя\n";
    }
    
    void deleteInappropriateContent() override {
        std::cout << "🗑️ Модератор удалил неподходящий контент\n";
    }
};

class Administrator : public IAuthenticatable, public IPostManager, public ISocialInteraction, 
                      public IMessageManager, public IFileManager, public IGroupManager, public IModerator {
private:
    bool loggedIn = false;
    
public:
    void login() override {
        loggedIn = true;
        std::cout << "👑 Администратор вошел в систему\n";
    }
    
    void logout() override {
        loggedIn = false;
        std::cout << "👑 Администратор вышел из системы\n";
    }
    
    bool isLoggedIn() const override {
        return loggedIn;
    }
    
    void createPost() override {
        std::cout << "📝 Администратор создал системный пост\n";
    }
    
    void deletePost() override {
        std::cout << "🗑️ Администратор удалил пост\n";
    }
    
    void editPost() override {
        std::cout << "✏️ Администратор отредактировал пост\n";
    }
    
    void likePost() override {
        std::cout << "❤️ Администратор поставил лайк\n";
    }
    
    void commentPost() override {
        std::cout << "💬 Администратор прокомментировал пост\n";
    }
    
    void followUser() override {
        std::cout << "➕ Администратор подписался на пользователя\n";
    }
    
    void unfollowUser() override {
        std::cout << "➖ Администратор отписался от пользователя\n";
    }
    
    void sendMessage() override {
        std::cout << "📤 Администратор отправил сообщение\n";
    }
    
    void receiveMessage() override {
        std::cout << "📥 Администратор получил сообщение\n";
    }
    
    void deleteMessage() override {
        std::cout << "🗑️ Администратор удалил сообщение\n";
    }
    
    void uploadFile() override {
        std::cout << "📤 Администратор загрузил системный файл\n";
    }
    
    void downloadFile() override {
        std::cout << "📥 Администратор скачал файл\n";
    }
    
    void deleteFile() override {
        std::cout << "🗑️ Администратор удалил файл\n";
    }
    
    void createGroup() override {
        std::cout << "👥 Администратор создал группу\n";
    }
    
    void joinGroup() override {
        std::cout << "➕ Администратор присоединился к группе\n";
    }
    
    void leaveGroup() override {
        std::cout << "➖ Администратор покинул группу\n";
    }
    
    void inviteToGroup() override {
        std::cout << "📨 Администратор пригласил в группу\n";
    }
    
    void moderateContent() override {
        std::cout << "🔍 Администратор модерирует контент\n";
    }
    
    void banUser() override {
        std::cout << "🚫 Администратор заблокировал пользователя\n";
    }
    
    void unbanUser() override {
        std::cout << "✅ Администратор разблокировал пользователя\n";
    }
    
    void deleteInappropriateContent() override {
        std::cout << "🗑️ Администратор удалил неподходящий контент\n";
    }
};

// ============================================================================
// ДЕМОНСТРАЦИЯ ПРИНЦИПА
// ============================================================================

void demonstrateGoodISP() {
    std::cout << "\n✅ СОБЛЮДЕНИЕ ISP - Разделение интерфейсов:\n";
    std::cout << std::string(50, '-') << "\n";
    
    // Создание устройств
    std::unique_ptr<Printer> printer = std::make_unique<Printer>();
    std::unique_ptr<Scanner> scanner = std::make_unique<Scanner>();
    std::unique_ptr<MultiFunctionDevice> mfd = std::make_unique<MultiFunctionDevice>();
    std::unique_ptr<SmartPhone> phone = std::make_unique<SmartPhone>();
    
    std::cout << "Тестирование принтера:\n";
    printer->print();
    printer->copy();
    printer->networkConnect();
    printer->usbConnect();
    
    std::cout << "\nТестирование сканера:\n";
    scanner->read();
    scanner->scan();
    scanner->bluetoothConnect();
    scanner->usbConnect();
    
    std::cout << "\nТестирование МФУ:\n";
    mfd->print();
    mfd->scan();
    mfd->fax();
    mfd->copy();
    mfd->networkConnect();
    mfd->wifiConnect();
    
    std::cout << "\nТестирование смартфона:\n";
    phone->read();
    phone->write();
    phone->print();
    phone->bluetoothConnect();
    phone->wifiConnect();
    phone->usbConnect();
}

void demonstrateUserISP() {
    std::cout << "\n✅ СОБЛЮДЕНИЕ ISP - Система управления пользователями:\n";
    std::cout << std::string(50, '-') << "\n";
    
    // Создание пользователей разных типов
    std::unique_ptr<RegularUser> regularUser = std::make_unique<RegularUser>();
    std::unique_ptr<ContentCreator> creator = std::make_unique<ContentCreator>();
    std::unique_ptr<Moderator> moderator = std::make_unique<Moderator>();
    std::unique_ptr<Administrator> admin = std::make_unique<Administrator>();
    
    std::cout << "Тестирование обычного пользователя:\n";
    regularUser->login();
    regularUser->createPost();
    regularUser->likePost();
    regularUser->followUser();
    regularUser->sendMessage();
    
    std::cout << "\nТестирование контент-креатора:\n";
    creator->login();
    creator->createPost();
    creator->uploadFile();
    creator->followUser();
    creator->sendMessage();
    
    std::cout << "\nТестирование модератора:\n";
    moderator->login();
    moderator->createPost();
    moderator->moderateContent();
    moderator->banUser();
    moderator->sendMessage();
    
    std::cout << "\nТестирование администратора:\n";
    admin->login();
    admin->createPost();
    admin->uploadFile();
    admin->createGroup();
    admin->moderateContent();
    admin->banUser();
}

void analyzeTradeOffs() {
    std::cout << "\n🔬 АНАЛИЗ КОМПРОМИССОВ ISP:\n";
    std::cout << std::string(50, '-') << "\n";
    
    std::cout << "📊 ПРЕИМУЩЕСТВА:\n";
    std::cout << "• Клиенты зависят только от нужных им интерфейсов\n";
    std::cout << "• Снижение coupling между модулями\n";
    std::cout << "• Улучшенная переиспользуемость интерфейсов\n";
    std::cout << "• Более четкое разделение ответственностей\n";
    std::cout << "• Упрощение тестирования и мокинга\n\n";
    
    std::cout << "⚠️ НЕДОСТАТКИ:\n";
    std::cout << "• Увеличение количества интерфейсов\n";
    std::cout << "• Потенциальное усложнение архитектуры\n";
    std::cout << "• Необходимость тщательного проектирования\n";
    std::cout << "• Возможное дублирование кода между интерфейсами\n\n";
    
    std::cout << "🎯 РЕКОМЕНДАЦИИ:\n";
    std::cout << "• Применяйте ISP для больших интерфейсов\n";
    std::cout << "• Используйте композицию интерфейсов\n";
    std::cout << "• Избегайте создания интерфейсов с одним методом\n";
    std::cout << "• Рассмотрите использование Adapter Pattern для совместимости\n";
}

int main() {
    std::cout << "🎯 ДЕТАЛЬНАЯ ДЕМОНСТРАЦИЯ INTERFACE SEGREGATION PRINCIPLE (ISP)\n";
    std::cout << "Автор: Senior C++ Developer\n";
    std::cout << "Цель: Углубленное изучение принципа разделения интерфейсов\n\n";
    
    demonstrateBadISP();
    demonstrateGoodISP();
    demonstrateUserISP();
    analyzeTradeOffs();
    
    std::cout << "\n📚 МАТЕМАТИЧЕСКОЕ ОБОСНОВАНИЕ:\n";
    std::cout << "ISP: ∀I ∈ Interfaces, ∀C ∈ Clients: \n";
    std::cout << "|UsedMethods(I,C)| / |AllMethods(I)| → 1\n\n";
    
    std::cout << "💡 КЛЮЧЕВЫЕ ВЫВОДЫ:\n";
    std::cout << "1. Клиенты не должны зависеть от интерфейсов, которые они не используют\n";
    std::cout << "2. Разделение больших интерфейсов на специализированные\n";
    std::cout << "3. ISP улучшает модульность и переиспользуемость\n";
    std::cout << "4. Применение требует баланса между простотой и функциональностью\n\n";
    
    std::cout << "🔬 Принципы - это инструменты для мышления о проблемах!\n";
    
    return 0;
}
