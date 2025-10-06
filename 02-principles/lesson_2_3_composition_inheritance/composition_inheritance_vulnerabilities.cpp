#include <iostream>
#include <thread>
#include <mutex>
#include <memory>
#include <vector>
#include <chrono>
#include <cstring>

// ============================================================================
// РЈРЇР—Р’РРњР«Р• Р Р•РђР›РР—РђР¦РР composition_inheritance Р”Р›РЇ РђРќРђР›РР—Рђ Р‘Р•Р—РћРџРђРЎРќРћРЎРўР
// ============================================================================

// TODO: Р”РѕР±Р°РІРёС‚СЊ РєРѕРјРјРµРЅС‚Р°СЂРёРё РЅР° СЂСѓСЃСЃРєРѕРј СЏР·С‹РєРµ
// TODO: РЎРѕР·РґР°С‚СЊ СЌРєСЃРїР»РѕРёС‚С‹ РґР»СЏ РєР°Р¶РґРѕР№ СѓСЏР·РІРёРјРѕСЃС‚Рё
// TODO: Р”РѕР±Р°РІРёС‚СЊ РёРЅСЃС‚СЂСѓРјРµРЅС‚С‹ Р°РЅР°Р»РёР·Р°

// ----------------------------------------------------------------------------
// РЈРЇР—Р’РРњРћРЎРўР¬ 1: [РќРђР—Р’РђРќРР• РЈРЇР—Р’РРњРћРЎРўР]
// ----------------------------------------------------------------------------
class Vulnerablecomposition_inheritance {
private:
    // TODO: Р”РѕР±Р°РІРёС‚СЊ РїСЂРёРІР°С‚РЅС‹Рµ С‡Р»РµРЅС‹ РєР»Р°СЃСЃР°
    
public:
    Vulnerablecomposition_inheritance() {
        // TODO: Р”РѕР±Р°РІРёС‚СЊ РєРѕРЅСЃС‚СЂСѓРєС‚РѕСЂ СЃ СѓСЏР·РІРёРјРѕСЃС‚СЏРјРё
        std::cout << "Vulnerablecomposition_inheritance СЃРѕР·РґР°РЅ" << std::endl;
    }
    
    // TODO: Р”РѕР±Р°РІРёС‚СЊ РјРµС‚РѕРґС‹ СЃ СѓСЏР·РІРёРјРѕСЃС‚СЏРјРё
    
    ~Vulnerablecomposition_inheritance() {
        // TODO: Р”РѕР±Р°РІРёС‚СЊ РґРµСЃС‚СЂСѓРєС‚РѕСЂ
        std::cout << "Vulnerablecomposition_inheritance СѓРЅРёС‡С‚РѕР¶РµРЅ" << std::endl;
    }
};

// ----------------------------------------------------------------------------
// РЈРЇР—Р’РРњРћРЎРўР¬ 2: [РќРђР—Р’РђРќРР• РЈРЇР—Р’РРњРћРЎРўР]
// ----------------------------------------------------------------------------
class AnotherVulnerablecomposition_inheritance {
private:
    // TODO: Р”РѕР±Р°РІРёС‚СЊ РїСЂРёРІР°С‚РЅС‹Рµ С‡Р»РµРЅС‹ РєР»Р°СЃСЃР°
    
public:
    AnotherVulnerablecomposition_inheritance() {
        // TODO: Р”РѕР±Р°РІРёС‚СЊ РєРѕРЅСЃС‚СЂСѓРєС‚РѕСЂ СЃ СѓСЏР·РІРёРјРѕСЃС‚СЏРјРё
        std::cout << "AnotherVulnerablecomposition_inheritance СЃРѕР·РґР°РЅ" << std::endl;
    }
    
    // TODO: Р”РѕР±Р°РІРёС‚СЊ РјРµС‚РѕРґС‹ СЃ СѓСЏР·РІРёРјРѕСЃС‚СЏРјРё
    
    ~AnotherVulnerablecomposition_inheritance() {
        // TODO: Р”РѕР±Р°РІРёС‚СЊ РґРµСЃС‚СЂСѓРєС‚РѕСЂ
        std::cout << "AnotherVulnerablecomposition_inheritance СѓРЅРёС‡С‚РѕР¶РµРЅ" << std::endl;
    }
};

// ----------------------------------------------------------------------------
// Р”Р•РњРћРќРЎРўР РђР¦РРЇ РЈРЇР—Р’РРњРћРЎРўР•Р™
// ----------------------------------------------------------------------------

void demonstrateVulnerability1() {
    std::cout << "\n=== Р”РµРјРѕРЅСЃС‚СЂР°С†РёСЏ СѓСЏР·РІРёРјРѕСЃС‚Рё 1 ===" << std::endl;
    
    // TODO: Р”РѕР±Р°РІРёС‚СЊ РґРµРјРѕРЅСЃС‚СЂР°С†РёСЋ РїРµСЂРІРѕР№ СѓСЏР·РІРёРјРѕСЃС‚Рё
    
    std::cout << "РЈСЏР·РІРёРјРѕСЃС‚СЊ 1 РїСЂРѕРґРµРјРѕРЅСЃС‚СЂРёСЂРѕРІР°РЅР°" << std::endl;
}

void demonstrateVulnerability2() {
    std::cout << "\n=== Р”РµРјРѕРЅСЃС‚СЂР°С†РёСЏ СѓСЏР·РІРёРјРѕСЃС‚Рё 2 ===" << std::endl;
    
    // TODO: Р”РѕР±Р°РІРёС‚СЊ РґРµРјРѕРЅСЃС‚СЂР°С†РёСЋ РІС‚РѕСЂРѕР№ СѓСЏР·РІРёРјРѕСЃС‚Рё
    
    std::cout << "РЈСЏР·РІРёРјРѕСЃС‚СЊ 2 РїСЂРѕРґРµРјРѕРЅСЃС‚СЂРёСЂРѕРІР°РЅР°" << std::endl;
}

// ----------------------------------------------------------------------------
// РРќРЎРўР РЈРњР•РќРўР« РђРќРђР›РР—Рђ
// ----------------------------------------------------------------------------

void runStaticAnalysis() {
    std::cout << "\n=== РРЅСЃС‚СЂСѓРєС†РёРё РґР»СЏ СЃС‚Р°С‚РёС‡РµСЃРєРѕРіРѕ Р°РЅР°Р»РёР·Р° ===" << std::endl;
    std::cout << "1. Clang Static Analyzer:" << std::endl;
    std::cout << "   clang --analyze composition_inheritance_vulnerabilities.cpp" << std::endl;
    std::cout << "2. Cppcheck:" << std::endl;
    std::cout << "   cppcheck --enable=all composition_inheritance_vulnerabilities.cpp" << std::endl;
    std::cout << "3. PVS-Studio:" << std::endl;
    std::cout << "   pvs-studio-analyzer trace -- make" << std::endl;
}

void runDynamicAnalysis() {
    std::cout << "\n=== РРЅСЃС‚СЂСѓРєС†РёРё РґР»СЏ РґРёРЅР°РјРёС‡РµСЃРєРѕРіРѕ Р°РЅР°Р»РёР·Р° ===" << std::endl;
    std::cout << "1. AddressSanitizer:" << std::endl;
    std::cout << "   g++ -fsanitize=address -g composition_inheritance_vulnerabilities.cpp -o composition_inheritance_asan" << std::endl;
    std::cout << "   ./composition_inheritance_asan" << std::endl;
    std::cout << "2. ThreadSanitizer:" << std::endl;
    std::cout << "   g++ -fsanitize=thread -g composition_inheritance_vulnerabilities.cpp -o composition_inheritance_tsan" << std::endl;
    std::cout << "   ./composition_inheritance_tsan" << std::endl;
    std::cout << "3. Valgrind:" << std::endl;
    std::cout << "   valgrind --tool=memcheck ./composition_inheritance_vulnerabilities" << std::endl;
}

// ----------------------------------------------------------------------------
// РћРЎРќРћР’РќРђРЇ Р¤РЈРќРљР¦РРЇ
// ----------------------------------------------------------------------------

int main() {
    std::cout << "=== РђРќРђР›РР— РЈРЇР—Р’РРњРћРЎРўР•Р™ Р’ РџРђРўРўР•Р РќР• composition_inheritance ===" << std::endl;
    
    // Р”РµРјРѕРЅСЃС‚СЂР°С†РёСЏ СЂР°Р·Р»РёС‡РЅС‹С… СѓСЏР·РІРёРјРѕСЃС‚РµР№
    demonstrateVulnerability1();
    demonstrateVulnerability2();
    
    // РРЅСЃС‚СЂСѓРєС†РёРё РїРѕ Р°РЅР°Р»РёР·Сѓ
    runStaticAnalysis();
    runDynamicAnalysis();
    
    std::cout << "\n=== Р’РќРРњРђРќРР•: Р­С‚РѕС‚ РєРѕРґ СЃРѕРґРµСЂР¶РёС‚ СѓСЏР·РІРёРјРѕСЃС‚Рё! ===" << std::endl;
    std::cout << "РСЃРїРѕР»СЊР·СѓР№С‚Рµ С‚РѕР»СЊРєРѕ РґР»СЏ РѕР±СѓС‡РµРЅРёСЏ Рё Р°РЅР°Р»РёР·Р° Р±РµР·РѕРїР°СЃРЅРѕСЃС‚Рё." << std::endl;
    
    return 0;
}

