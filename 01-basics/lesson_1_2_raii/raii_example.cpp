#include <iostream>
#include <memory>
#include <fstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <mutex>

/**
 * @file raii_example.cpp
 * @brief Демонстрация принципа RAII в различных сценариях
 * 
 * Этот файл показывает, как RAII обеспечивает автоматическое управление
 * ресурсами и exception safety в различных ситуациях.
 */

// ============================================================================
// БАЗОВЫЙ ПРИМЕР RAII - УПРАВЛЕНИЕ ПАМЯТЬЮ
// ============================================================================

/**
 * @brief Классический пример RAII - управление динамической памятью
 * 
 * Демонстрирует основные принципы:
 * - Ресурс захватывается в конструкторе
 * - Ресурс освобождается в деструкторе
 * - Исключения не нарушают освобождение ресурсов
 */
class IntArray {
private:
    int* data_;
    size_t size_;
    
public:
    explicit IntArray(size_t size) : size_(size) {
        std::cout << "IntArray: Выделяем память для " << size << " элементов" << std::endl;
        data_ = new int[size];
        
        // Инициализируем массив
        for (size_t i = 0; i < size_; ++i) {
            data_[i] = 0;
        }
    }
    
    ~IntArray() {
        std::cout << "IntArray: Освобождаем память для " << size_ << " элементов" << std::endl;
        delete[] data_;
        data_ = nullptr;
    }
    
    // Запрещаем копирование для демонстрации единоличного владения
    IntArray(const IntArray&) = delete;
    IntArray& operator=(const IntArray&) = delete;
    
    // Разрешаем перемещение
    IntArray(IntArray&& other) noexcept 
        : data_(other.data_), size_(other.size_) {
        other.data_ = nullptr;
        other.size_ = 0;
        std::cout << "IntArray: Перемещение объекта" << std::endl;
    }
    
    IntArray& operator=(IntArray&& other) noexcept {
        if (this != &other) {
            delete[] data_;
            data_ = other.data_;
            size_ = other.size_;
            other.data_ = nullptr;
            other.size_ = 0;
            std::cout << "IntArray: Перемещение с присваиванием" << std::endl;
        }
        return *this;
    }
    
    // Методы для работы с данными
    int& operator[](size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Индекс вне диапазона");
        }
        return data_[index];
    }
    
    const int& operator[](size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Индекс вне диапазона");
        }
        return data_[index];
    }
    
    size_t size() const { return size_; }
    
    void fill(int value) {
        for (size_t i = 0; i < size_; ++i) {
            data_[i] = value;
        }
    }
};

// ============================================================================
// RAII ДЛЯ ФАЙЛОВЫХ ОПЕРАЦИЙ
// ============================================================================

/**
 * @brief RAII wrapper для файловых операций
 * 
 * Демонстрирует:
 * - Автоматическое открытие/закрытие файлов
 * - Exception safety при работе с файлами
 * - Move semantics для передачи владения файлом
 */
class FileManager {
private:
    std::unique_ptr<std::fstream> file_;
    std::string filename_;
    bool isOpen_;
    
public:
    explicit FileManager(const std::string& filename) 
        : filename_(filename), isOpen_(false) {
        std::cout << "FileManager: Открываем файл " << filename << std::endl;
        
        file_ = std::make_unique<std::fstream>(filename);
        if (!file_->is_open()) {
            throw std::runtime_error("Не удалось открыть файл: " + filename);
        }
        isOpen_ = true;
    }
    
    ~FileManager() {
        if (isOpen_) {
            std::cout << "FileManager: Закрываем файл " << filename_ << std::endl;
            file_->close();
            isOpen_ = false;
        }
    }
    
    // Запрещаем копирование
    FileManager(const FileManager&) = delete;
    FileManager& operator=(const FileManager&) = delete;
    
    // Разрешаем перемещение
    FileManager(FileManager&& other) noexcept 
        : file_(std::move(other.file_)), filename_(std::move(other.filename_)), isOpen_(other.isOpen_) {
        other.isOpen_ = false;
        std::cout << "FileManager: Перемещение файла" << std::endl;
    }
    
    FileManager& operator=(FileManager&& other) noexcept {
        if (this != &other) {
            if (isOpen_) {
                file_->close();
            }
            file_ = std::move(other.file_);
            filename_ = std::move(other.filename_);
            isOpen_ = other.isOpen_;
            other.isOpen_ = false;
            std::cout << "FileManager: Перемещение файла с присваиванием" << std::endl;
        }
        return *this;
    }
    
    // Методы для работы с файлом
    void write(const std::string& data) {
        if (!isOpen_) {
            throw std::runtime_error("Файл не открыт");
        }
        *file_ << data;
        file_->flush();
    }
    
    std::string read() {
        if (!isOpen_) {
            throw std::runtime_error("Файл не открыт");
        }
        
        file_->seekg(0, std::ios::end);
        size_t size = file_->tellg();
        file_->seekg(0, std::ios::beg);
        
        std::string content(size, ' ');
        file_->read(&content[0], size);
        return content;
    }
    
    bool isOpen() const { return isOpen_; }
    const std::string& getFilename() const { return filename_; }
};

// ============================================================================
// RAII ДЛЯ СИНХРОНИЗАЦИИ
// ============================================================================

/**
 * @brief RAII wrapper для мьютекса
 * 
 * Демонстрирует:
 * - Автоматическое блокирование/разблокирование мьютекса
 * - Exception safety в многопоточном коде
 * - Блокировка в конструкторе, разблокировка в деструкторе
 */
class MutexGuard {
private:
    std::mutex& mutex_;
    bool locked_;
    
public:
    explicit MutexGuard(std::mutex& mutex) : mutex_(mutex), locked_(false) {
        std::cout << "MutexGuard: Блокируем мьютекс" << std::endl;
        mutex_.lock();
        locked_ = true;
    }
    
    ~MutexGuard() {
        if (locked_) {
            std::cout << "MutexGuard: Разблокируем мьютекс" << std::endl;
            mutex_.unlock();
            locked_ = false;
        }
    }
    
    // Запрещаем копирование и перемещение
    MutexGuard(const MutexGuard&) = delete;
    MutexGuard& operator=(const MutexGuard&) = delete;
    MutexGuard(MutexGuard&&) = delete;
    MutexGuard& operator=(MutexGuard&&) = delete;
    
    void unlock() {
        if (locked_) {
            mutex_.unlock();
            locked_ = false;
            std::cout << "MutexGuard: Ручная разблокировка мьютекса" << std::endl;
        }
    }
};

// ============================================================================
// ДЕМОНСТРАЦИЯ EXCEPTION SAFETY
// ============================================================================

/**
 * @brief Демонстрация различных уровней exception safety
 */
class ExceptionSafetyDemo {
private:
    std::vector<int> data_;
    std::unique_ptr<int> resource_;
    
public:
    ExceptionSafetyDemo() : resource_(std::make_unique<int>(42)) {
        std::cout << "ExceptionSafetyDemo: Создание объекта" << std::endl;
    }
    
    ~ExceptionSafetyDemo() {
        std::cout << "ExceptionSafetyDemo: Уничтожение объекта" << std::endl;
    }
    
    // 1. Базовые гарантии - ресурсы освобождаются, состояние может быть изменено
    void basicGuarantee() {
        std::cout << "Выполняем операцию с базовыми гарантиями" << std::endl;
        
        // Если исключение произойдет здесь, ресурс_ автоматически освободится
        // Но data_ может остаться в частично измененном состоянии
        data_.push_back(1);
        data_.push_back(2);
        // Симуляция исключения
        throw std::runtime_error("Исключение в basicGuarantee");
    }
    
    // 2. Строгие гарантии - состояние откатывается при исключении
    void strongGuarantee() {
        std::cout << "Выполняем операцию со строгими гарантиями" << std::endl;
        
        auto backup = data_;  // Создаем резервную копию
        
        try {
            data_.push_back(3);
            data_.push_back(4);
            // Симуляция исключения
            throw std::runtime_error("Исключение в strongGuarantee");
        } catch (...) {
            data_ = std::move(backup);  // Откатываем изменения
            std::cout << "Откат изменений при исключении" << std::endl;
            throw;
        }
    }
    
    // 3. Noexcept гарантии - функция не выбрасывает исключений
    void noexceptGuarantee() noexcept {
        std::cout << "Выполняем noexcept операцию" << std::endl;
        
        // Эта функция гарантированно не выбросит исключение
        if (!data_.empty()) {
            data_.clear();
        }
    }
    
    void printState() const {
        std::cout << "Состояние объекта: data_.size() = " << data_.size();
        if (resource_) {
            std::cout << ", resource_ = " << *resource_;
        }
        std::cout << std::endl;
    }
};

// ============================================================================
// ДЕМОНСТРАЦИОННЫЕ ФУНКЦИИ
// ============================================================================

/**
 * @brief Демонстрация автоматического управления памятью
 */
void demonstrateMemoryManagement() {
    std::cout << "\n=== Демонстрация управления памятью ===" << std::endl;
    
    try {
        IntArray array(5);
        
        // Заполняем массив
        for (size_t i = 0; i < array.size(); ++i) {
            array[i] = static_cast<int>(i * 10);
        }
        
        // Выводим содержимое
        std::cout << "Содержимое массива: ";
        for (size_t i = 0; i < array.size(); ++i) {
            std::cout << array[i] << " ";
        }
        std::cout << std::endl;
        
        // При выходе из области видимости память автоматически освободится
        // даже если произойдет исключение
        
    } catch (const std::exception& e) {
        std::cout << "Исключение: " << e.what() << std::endl;
        // Память все равно освободится благодаря RAII
    }
}

/**
 * @brief Демонстрация управления файлами
 */
void demonstrateFileManagement() {
    std::cout << "\n=== Демонстрация управления файлами ===" << std::endl;
    
    try {
        // Создаем временный файл для демонстрации
        const std::string filename = "raii_demo.txt";
        
        {
            FileManager file(filename);
            
            // Записываем данные в файл
            file.write("Привет, RAII!\n");
            file.write("Это демонстрация автоматического управления файлами.\n");
            
            std::cout << "Данные записаны в файл: " << file.getFilename() << std::endl;
            
            // Файл автоматически закроется при выходе из области видимости
        }
        
        // Читаем файл
        {
            FileManager file(filename);
            std::string content = file.read();
            std::cout << "Содержимое файла:\n" << content << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cout << "Ошибка работы с файлом: " << e.what() << std::endl;
    }
}

/**
 * @brief Демонстрация синхронизации
 */
void demonstrateSynchronization() {
    std::cout << "\n=== Демонстрация синхронизации ===" << std::endl;
    
    std::mutex demoMutex;
    
    try {
        MutexGuard guard(demoMutex);
        
        std::cout << "Критическая секция: выполняем важную операцию" << std::endl;
        
        // Имитация работы в критической секции
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // Мьютекс автоматически разблокируется при выходе из области видимости
        // даже если произойдет исключение
        
    } catch (const std::exception& e) {
        std::cout << "Исключение в критической секции: " << e.what() << std::endl;
        // Мьютекс все равно разблокируется
    }
}

/**
 * @brief Демонстрация exception safety
 */
void demonstrateExceptionSafety() {
    std::cout << "\n=== Демонстрация Exception Safety ===" << std::endl;
    
    // 1. Базовые гарантии
    try {
        ExceptionSafetyDemo demo;
        demo.printState();
        demo.basicGuarantee();
    } catch (const std::exception& e) {
        std::cout << "Поймали исключение: " << e.what() << std::endl;
        // Ресурсы автоматически освободились
    }
    
    // 2. Строгие гарантии
    try {
        ExceptionSafetyDemo demo;
        demo.printState();
        demo.strongGuarantee();
        demo.printState(); // Состояние не изменилось
    } catch (const std::exception& e) {
        std::cout << "Поймали исключение: " << e.what() << std::endl;
    }
    
    // 3. Noexcept гарантии
    ExceptionSafetyDemo demo;
    demo.noexceptGuarantee();
    demo.printState();
}

/**
 * @brief Демонстрация move semantics в RAII
 */
void demonstrateMoveSemantics() {
    std::cout << "\n=== Демонстрация Move Semantics в RAII ===" << std::endl;
    
    // Создаем объект
    IntArray array1(3);
    array1.fill(42);
    
    std::cout << "array1 создан, размер: " << array1.size() << std::endl;
    
    // Перемещаем объект
    IntArray array2 = std::move(array1);
    
    std::cout << "array1 перемещен в array2" << std::endl;
    std::cout << "array2 размер: " << array2.size() << std::endl;
    
    // array1 теперь пустой (но безопасно уничтожается)
    // array2 владеет ресурсами
}

// ============================================================================
// ОСНОВНАЯ ФУНКЦИЯ
// ============================================================================

int main() {
    std::cout << "🏗️ Демонстрация принципа RAII (Resource Acquisition Is Initialization)" << std::endl;
    std::cout << std::string(70, '=') << std::endl;
    
    demonstrateMemoryManagement();
    demonstrateFileManagement();
    demonstrateSynchronization();
    demonstrateExceptionSafety();
    demonstrateMoveSemantics();
    
    std::cout << "\n✅ Демонстрация RAII завершена!" << std::endl;
    std::cout << "Ключевые принципы:" << std::endl;
    std::cout << "• Ресурсы захватываются в конструкторе" << std::endl;
    std::cout << "• Ресурсы освобождаются в деструкторе" << std::endl;
    std::cout << "• Исключения не нарушают освобождение ресурсов" << std::endl;
    std::cout << "• Move semantics для эффективной передачи владения" << std::endl;
    
    return 0;
}
