# 🎭 Visitor Pattern - Плакат

## 📋 Обзор паттерна

**Visitor Pattern** - это поведенческий паттерн проектирования, который позволяет добавлять новые операции к объектам без изменения их классов. Паттерн определяет операции, которые могут быть выполнены над элементами объектной структуры.

## 🎯 Назначение

- **Инкапсуляция операций**: Выносит операции из классов элементов в отдельные классы посетителей
- **Расширяемость**: Позволяет легко добавлять новые операции без изменения существующих классов
- **Разделение ответственности**: Отделяет алгоритмы от структуры объектов

## 🏗️ Структура паттерна

```
┌─────────────────┐    ┌─────────────────┐
│   Element       │    │   Visitor        │
│   (интерфейс)   │    │   (интерфейс)    │
├─────────────────┤    ├─────────────────┤
│ + accept(visitor)│    │ + visitElementA │
└─────────────────┘    │ + visitElementB │
         ▲              └─────────────────┘
         │                       ▲
         │                       │
    ┌────┴────┐            ┌─────┴─────┐
    │         │            │           │
┌───▼───┐ ┌──▼───┐    ┌───▼───┐ ┌───▼───┐
│ElementA│ │ElementB│    │Visitor1│ │Visitor2│
├───────┤ ├───────┤    ├───────┤ ├───────┤
│+accept │ │+accept │    │+visitA│ │+visitA│
└───────┘ └───────┘    │+visitB│ │+visitB│
                        └───────┘ └───────┘
```

## 💡 Ключевые компоненты

### 1. Element (Элемент)
```cpp
// Базовый интерфейс для элементов, которые могут принимать посетителей
class Element {
public:
    virtual ~Element() = default;
    virtual void accept(Visitor& visitor) = 0;
};
```

### 2. ConcreteElement (Конкретный элемент)
```cpp
// Конкретные классы элементов
class ConcreteElementA : public Element {
public:
    void accept(Visitor& visitor) override {
        visitor.visitConcreteElementA(*this);
    }
    
    std::string operationA() const {
        return "Операция A";
    }
};

class ConcreteElementB : public Element {
public:
    void accept(Visitor& visitor) override {
        visitor.visitConcreteElementB(*this);
    }
    
    std::string operationB() const {
        return "Операция B";
    }
};
```

### 3. Visitor (Посетитель)
```cpp
// Интерфейс посетителя
class Visitor {
public:
    virtual ~Visitor() = default;
    virtual void visitConcreteElementA(ConcreteElementA& element) = 0;
    virtual void visitConcreteElementB(ConcreteElementB& element) = 0;
};
```

### 4. ConcreteVisitor (Конкретный посетитель)
```cpp
// Конкретные реализации посетителей
class ConcreteVisitor1 : public Visitor {
public:
    void visitConcreteElementA(ConcreteElementA& element) override {
        std::cout << "Посетитель 1 обрабатывает: " << element.operationA() << std::endl;
    }
    
    void visitConcreteElementB(ConcreteElementB& element) override {
        std::cout << "Посетитель 1 обрабатывает: " << element.operationB() << std::endl;
    }
};

class ConcreteVisitor2 : public Visitor {
public:
    void visitConcreteElementA(ConcreteElementA& element) override {
        std::cout << "Посетитель 2 анализирует: " << element.operationA() << std::endl;
    }
    
    void visitConcreteElementB(ConcreteElementB& element) override {
        std::cout << "Посетитель 2 анализирует: " << element.operationB() << std::endl;
    }
};
```

## 🔧 Практический пример: Система обработки документов

```cpp
#include <iostream>
#include <vector>
#include <memory>
#include <string>

// Базовый класс документа
class Document {
public:
    virtual ~Document() = default;
    virtual void accept(class DocumentVisitor& visitor) = 0;
    virtual std::string getContent() const = 0;
};

// Конкретные типы документов
class TextDocument : public Document {
private:
    std::string content;
    
public:
    TextDocument(const std::string& text) : content(text) {}
    
    void accept(DocumentVisitor& visitor) override;
    std::string getContent() const override { return content; }
    
    int getWordCount() const {
        // Простой подсчет слов
        int count = 0;
        bool inWord = false;
        for (char c : content) {
            if (std::isalpha(c)) {
                if (!inWord) {
                    count++;
                    inWord = true;
                }
            } else {
                inWord = false;
            }
        }
        return count;
    }
};

class PDFDocument : public Document {
private:
    std::string filename;
    int pageCount;
    
public:
    PDFDocument(const std::string& file, int pages) 
        : filename(file), pageCount(pages) {}
    
    void accept(DocumentVisitor& visitor) override;
    std::string getContent() const override { return filename; }
    int getPageCount() const { return pageCount; }
};

// Интерфейс посетителя документов
class DocumentVisitor {
public:
    virtual ~DocumentVisitor() = default;
    virtual void visit(TextDocument& doc) = 0;
    virtual void visit(PDFDocument& doc) = 0;
};

// Реализация accept для конкретных документов
void TextDocument::accept(DocumentVisitor& visitor) {
    visitor.visit(*this);
}

void PDFDocument::accept(DocumentVisitor& visitor) {
    visitor.visit(*this);
}

// Конкретные посетители
class WordCountVisitor : public DocumentVisitor {
private:
    int totalWords = 0;
    
public:
    void visit(TextDocument& doc) override {
        int words = doc.getWordCount();
        totalWords += words;
        std::cout << "Текстовый документ: " << words << " слов" << std::endl;
    }
    
    void visit(PDFDocument& doc) override {
        std::cout << "PDF документ '" << doc.getContent() 
                  << "' не поддерживает подсчет слов" << std::endl;
    }
    
    int getTotalWords() const { return totalWords; }
};

class PageCountVisitor : public DocumentVisitor {
private:
    int totalPages = 0;
    
public:
    void visit(TextDocument& doc) override {
        std::cout << "Текстовый документ не имеет страниц" << std::endl;
    }
    
    void visit(PDFDocument& doc) override {
        int pages = doc.getPageCount();
        totalPages += pages;
        std::cout << "PDF документ '" << doc.getContent() 
                  << "': " << pages << " страниц" << std::endl;
    }
    
    int getTotalPages() const { return totalPages; }
};

class ExportVisitor : public DocumentVisitor {
public:
    void visit(TextDocument& doc) override {
        std::cout << "Экспорт текстового документа в TXT: " 
                  << doc.getContent() << std::endl;
    }
    
    void visit(PDFDocument& doc) override {
        std::cout << "Экспорт PDF документа в HTML: " 
                  << doc.getContent() << std::endl;
    }
};

// Коллекция документов
class DocumentCollection {
private:
    std::vector<std::unique_ptr<Document>> documents;
    
public:
    void addDocument(std::unique_ptr<Document> doc) {
        documents.push_back(std::move(doc));
    }
    
    void acceptVisitor(DocumentVisitor& visitor) {
        for (auto& doc : documents) {
            doc->accept(visitor);
        }
    }
    
    size_t size() const { return documents.size(); }
};

// Демонстрация использования
int main() {
    std::cout << "=== Демонстрация Visitor Pattern ===" << std::endl;
    
    // Создание коллекции документов
    DocumentCollection collection;
    collection.addDocument(std::make_unique<TextDocument>("Привет мир! Это тестовый документ."));
    collection.addDocument(std::make_unique<TextDocument>("Еще один документ с несколькими словами."));
    collection.addDocument(std::make_unique<PDFDocument>("report.pdf", 15));
    collection.addDocument(std::make_unique<PDFDocument>("manual.pdf", 50));
    
    std::cout << "\n📊 Анализ документов:" << std::endl;
    std::cout << "Количество документов: " << collection.size() << std::endl;
    
    // Подсчет слов
    std::cout << "\n🔢 Подсчет слов:" << std::endl;
    WordCountVisitor wordCounter;
    collection.acceptVisitor(wordCounter);
    std::cout << "Общее количество слов: " << wordCounter.getTotalWords() << std::endl;
    
    // Подсчет страниц
    std::cout << "\n📄 Подсчет страниц:" << std::endl;
    PageCountVisitor pageCounter;
    collection.acceptVisitor(pageCounter);
    std::cout << "Общее количество страниц: " << pageCounter.getTotalPages() << std::endl;
    
    // Экспорт документов
    std::cout << "\n📤 Экспорт документов:" << std::endl;
    ExportVisitor exporter;
    collection.acceptVisitor(exporter);
    
    return 0;
}
```

## 🎯 Применение в реальных проектах

### 1. **Компиляторы и интерпретаторы**
```cpp
// AST (Abstract Syntax Tree) с различными операциями
class ASTNode {
public:
    virtual void accept(ASTVisitor& visitor) = 0;
};

class ASTVisitor {
public:
    virtual void visit(AssignmentNode& node) = 0;
    virtual void visit(IfNode& node) = 0;
    virtual void visit(WhileNode& node) = 0;
};

// Различные посетители для разных операций
class TypeChecker : public ASTVisitor { /* ... */ };
class CodeGenerator : public ASTVisitor { /* ... */ };
class Optimizer : public ASTVisitor { /* ... */ };
```

### 2. **Графические редакторы**
```cpp
// Различные фигуры с операциями
class Shape {
public:
    virtual void accept(ShapeVisitor& visitor) = 0;
};

class ShapeVisitor {
public:
    virtual void visit(Circle& circle) = 0;
    virtual void visit(Rectangle& rect) = 0;
    virtual void visit(Triangle& triangle) = 0;
};

// Операции над фигурами
class AreaCalculator : public ShapeVisitor { /* ... */ };
class Renderer : public ShapeVisitor { /* ... */ };
class Serializer : public ShapeVisitor { /* ... */ };
```

### 3. **Системы управления контентом**
```cpp
// Различные типы контента
class Content {
public:
    virtual void accept(ContentVisitor& visitor) = 0;
};

class ContentVisitor {
public:
    virtual void visit(Article& article) = 0;
    virtual void visit(Video& video) = 0;
    virtual void visit(Image& image) = 0;
};

// Операции с контентом
class SEOAnalyzer : public ContentVisitor { /* ... */ };
class CacheManager : public ContentVisitor { /* ... */ };
class SearchIndexer : public ContentVisitor { /* ... */ };
```

## ⚡ Преимущества паттерна

### ✅ **Плюсы:**
- **Легкое добавление операций**: Новые операции добавляются без изменения существующих классов
- **Разделение ответственности**: Алгоритмы отделены от структуры объектов
- **Группировка операций**: Связанные операции группируются в одном посетителе
- **Расширяемость**: Легко добавлять новые типы элементов и посетителей

### ❌ **Минусы:**
- **Сложность**: Увеличивает сложность кода, особенно для простых операций
- **Нарушение инкапсуляции**: Посетители должны иметь доступ к внутреннему состоянию элементов
- **Сложность добавления элементов**: Добавление нового типа элемента требует изменения всех посетителей
- **Производительность**: Дополнительные вызовы методов могут влиять на производительность

## 🔄 Альтернативы и связанные паттерны

### **Альтернативы:**
- **Strategy Pattern**: Для замены алгоритмов
- **Command Pattern**: Для инкапсуляции операций
- **Template Method**: Для определения скелета алгоритма

### **Связанные паттерны:**
- **Composite**: Visitor часто используется с Composite для обхода деревьев
- **Interpreter**: Visitor может использоваться для интерпретации выражений
- **Strategy**: Visitor может рассматриваться как стратегия для операций

## 🎨 Современные улучшения в C++

### 1. **Использование std::variant (C++17)**
```cpp
#include <variant>
#include <vector>

// Типы документов как variant
using Document = std::variant<TextDocument, PDFDocument, ImageDocument>;

// Посетитель с std::visit
class ModernVisitor {
public:
    void visit(const Document& doc) {
        std::visit([this](const auto& document) {
            this->process(document);
        }, doc);
    }
    
private:
    template<typename T>
    void process(const T& document) {
        // Обработка конкретного типа документа
    }
};
```

### 2. **Концепты и шаблоны (C++20)**
```cpp
#include <concepts>

// Концепт для элементов
template<typename T>
concept Element = requires(T t, Visitor v) {
    t.accept(v);
};

// Шаблонный посетитель
template<Element... Elements>
class TemplateVisitor {
public:
    template<typename T>
    void visit(T& element) {
        if constexpr (std::is_same_v<T, Elements> || ...) {
            process(element);
        }
    }
    
private:
    template<typename T>
    void process(T& element) {
        // Обработка элемента
    }
};
```

## 🚀 Лучшие практики

### 1. **Правильное использование**
```cpp
// ✅ Хорошо: Четкое разделение операций
class DocumentAnalyzer : public DocumentVisitor {
    // Анализ документов
};

class DocumentExporter : public DocumentVisitor {
    // Экспорт документов
};

// ❌ Плохо: Смешивание несвязанных операций
class DocumentProcessor : public DocumentVisitor {
    // Анализ + экспорт + валидация - слишком много ответственности
};
```

### 2. **Управление состоянием посетителя**
```cpp
// ✅ Хорошо: Четкое управление состоянием
class StatisticsVisitor : public DocumentVisitor {
private:
    int processedCount = 0;
    std::vector<std::string> errors;
    
public:
    void visit(TextDocument& doc) override {
        processedCount++;
        // обработка...
    }
    
    void reset() {
        processedCount = 0;
        errors.clear();
    }
    
    int getProcessedCount() const { return processedCount; }
};
```

### 3. **Обработка ошибок**
```cpp
// ✅ Хорошо: Обработка ошибок в посетителе
class SafeVisitor : public DocumentVisitor {
public:
    void visit(TextDocument& doc) override {
        try {
            processDocument(doc);
        } catch (const std::exception& e) {
            handleError(doc, e);
        }
    }
    
private:
    void handleError(const Document& doc, const std::exception& e) {
        // Логирование и обработка ошибки
    }
};
```

## 📊 Сравнение с другими паттернами

| Аспект | Visitor | Strategy | Command |
|--------|---------|----------|---------|
| **Назначение** | Операции над объектами | Замена алгоритмов | Инкапсуляция операций |
| **Сложность** | Высокая | Средняя | Низкая |
| **Расширяемость** | Легко добавлять операции | Легко добавлять алгоритмы | Легко добавлять команды |
| **Производительность** | Средняя | Высокая | Низкая |
| **Применение** | AST, графические редакторы | Сортировка, валидация | Undo/redo, очереди |

## 🎯 Заключение

**Visitor Pattern** - это мощный инструмент для добавления операций к объектам без изменения их структуры. Он особенно полезен в системах с фиксированной структурой объектов, но изменяющимися операциями.

### **Когда использовать:**
- ✅ Структура объектов стабильна, но операции часто изменяются
- ✅ Нужно выполнять различные операции над объектами
- ✅ Операции должны быть сгруппированы логически
- ✅ Необходимо избежать загрязнения классов операциями

### **Когда НЕ использовать:**
- ❌ Структура объектов часто изменяется
- ❌ Операции простые и редко изменяются
- ❌ Производительность критична
- ❌ Нарушение инкапсуляции недопустимо

**Visitor Pattern** идеально подходит для компиляторов, графических редакторов, систем анализа данных и других приложений, где нужно выполнять различные операции над фиксированной структурой объектов! 🎭✨
