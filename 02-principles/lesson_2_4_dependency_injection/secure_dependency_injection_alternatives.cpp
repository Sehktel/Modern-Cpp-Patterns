#include <iostream>
#include <thread>
#include <mutex>
#include <memory>
#include <vector>
#include <chrono>
#include <cstring>
#include <atomic>

// ============================================================================
// Р‘Р•Р—РћРџРђРЎРќР«Р• РђР›Р¬РўР•Р РќРђРўРР’Р« dependency_injection
// ============================================================================

// TODO: Р”РѕР±Р°РІРёС‚СЊ РєРѕРјРјРµРЅС‚Р°СЂРёРё РЅР° СЂСѓСЃСЃРєРѕРј СЏР·С‹РєРµ
// TODO: РЎРѕР·РґР°С‚СЊ РґРѕРїРѕР»РЅРёС‚РµР»СЊРЅС‹Рµ Р±РµР·РѕРїР°СЃРЅС‹Рµ РІР°СЂРёР°РЅС‚С‹
// TODO: Р”РѕР±Р°РІРёС‚СЊ С‚РµСЃС‚С‹ Р±РµР·РѕРїР°СЃРЅРѕСЃС‚Рё

// ----------------------------------------------------------------------------
// Р‘Р•Р—РћРџРђРЎРќРђРЇ РђР›Р¬РўР•Р РќРђРўРР’Рђ 1: [РќРђР—Р’РђРќРР• РџРћР”РҐРћР”Рђ]
// ----------------------------------------------------------------------------

class Securedependency_injectionV1 {
private:
    // TODO: Р”РѕР±Р°РІРёС‚СЊ Р±РµР·РѕРїР°СЃРЅС‹Рµ РїСЂРёРІР°С‚РЅС‹Рµ С‡Р»РµРЅС‹
    mutable std::mutex data_mutex;
    
    Securedependency_injectionV1() {
        // TODO: Р”РѕР±Р°РІРёС‚СЊ Р±РµР·РѕРїР°СЃРЅС‹Р№ РєРѕРЅСЃС‚СЂСѓРєС‚РѕСЂ
        std::cout << "Securedependency_injectionV1 СЃРѕР·РґР°РЅ РІ РїРѕС‚РѕРєРµ " << std::this_thread::get_id() << std::endl;
    }
    
public:
    // РЈРґР°Р»СЏРµРј РєРѕРїРёСЂРѕРІР°РЅРёРµ Рё РїСЂРёСЃРІР°РёРІР°РЅРёРµ
    Securedependency_injectionV1(const Securedependency_injectionV1&) = delete;
    Securedependency_injectionV1& operator=(const Securedependency_injectionV1&) = delete;
    
    // TODO: Р”РѕР±Р°РІРёС‚СЊ Р±РµР·РѕРїР°СЃРЅС‹Рµ РјРµС‚РѕРґС‹
    
    ~Securedependency_injectionV1() {
        // TODO: Р”РѕР±Р°РІРёС‚СЊ Р±РµР·РѕРїР°СЃРЅС‹Р№ РґРµСЃС‚СЂСѓРєС‚РѕСЂ
        std::cout << "Securedependency_injectionV1 СѓРЅРёС‡С‚РѕР¶РµРЅ" << std::endl;
    }
};

// ----------------------------------------------------------------------------
// Р‘Р•Р—РћРџРђРЎРќРђРЇ РђР›Р¬РўР•Р РќРђРўРР’Рђ 2: [РќРђР—Р’РђРќРР• РџРћР”РҐРћР”Рђ]
// ----------------------------------------------------------------------------

class Securedependency_injectionV2 {
private:
    // TODO: Р”РѕР±Р°РІРёС‚СЊ Р±РµР·РѕРїР°СЃРЅС‹Рµ РїСЂРёРІР°С‚РЅС‹Рµ С‡Р»РµРЅС‹
    
    Securedependency_injectionV2() {
        // TODO: Р”РѕР±Р°РІРёС‚СЊ Р±РµР·РѕРїР°СЃРЅС‹Р№ РєРѕРЅСЃС‚СЂСѓРєС‚РѕСЂ
        std::cout << "Securedependency_injectionV2 СЃРѕР·РґР°РЅ РІ РїРѕС‚РѕРєРµ " << std::this_thread::get_id() << std::endl;
    }
    
public:
    // РЈРґР°Р»СЏРµРј РєРѕРїРёСЂРѕРІР°РЅРёРµ Рё РїСЂРёСЃРІР°РёРІР°РЅРёРµ
    Securedependency_injectionV2(const Securedependency_injectionV2&) = delete;
    Securedependency_injectionV2& operator=(const Securedependency_injectionV2&) = delete;
    
    // TODO: Р”РѕР±Р°РІРёС‚СЊ Р±РµР·РѕРїР°СЃРЅС‹Рµ РјРµС‚РѕРґС‹
    
    ~Securedependency_injectionV2() {
        // TODO: Р”РѕР±Р°РІРёС‚СЊ Р±РµР·РѕРїР°СЃРЅС‹Р№ РґРµСЃС‚СЂСѓРєС‚РѕСЂ
        std::cout << "Securedependency_injectionV2 СѓРЅРёС‡С‚РѕР¶РµРЅ" << std::endl;
    }
};

// ----------------------------------------------------------------------------
// Р‘Р•Р—РћРџРђРЎРќРђРЇ РђР›Р¬РўР•Р РќРђРўРР’Рђ 3: [РќРђР—Р’РђРќРР• РџРћР”РҐРћР”Рђ]
// ----------------------------------------------------------------------------

class Alternativedependency_injection {
private:
    // TODO: Р”РѕР±Р°РІРёС‚СЊ РїСЂРёРІР°С‚РЅС‹Рµ С‡Р»РµРЅС‹ Р°Р»СЊС‚РµСЂРЅР°С‚РёРІРЅРѕРіРѕ РїРѕРґС…РѕРґР°
    
public:
    Alternativedependency_injection() {
        // TODO: Р”РѕР±Р°РІРёС‚СЊ РєРѕРЅСЃС‚СЂСѓРєС‚РѕСЂ
        std::cout << "Alternativedependency_injection СЃРѕР·РґР°РЅ" << std::endl;
    }
    
    // РЈРґР°Р»СЏРµРј РєРѕРїРёСЂРѕРІР°РЅРёРµ Рё РїСЂРёСЃРІР°РёРІР°РЅРёРµ
    Alternativedependency_injection(const Alternativedependency_injection&) = delete;
    Alternativedependency_injection& operator=(const Alternativedependency_injection&) = delete;
    
    // TODO: Р”РѕР±Р°РІРёС‚СЊ РјРµС‚РѕРґС‹
    
    ~Alternativedependency_injection() {
        // TODO: Р”РѕР±Р°РІРёС‚СЊ РґРµСЃС‚СЂСѓРєС‚РѕСЂ
        std::cout << "Alternativedependency_injection СѓРЅРёС‡С‚РѕР¶РµРЅ" << std::endl;
    }
};

// ----------------------------------------------------------------------------
// Р”Р•РњРћРќРЎРўР РђР¦РРЇ Р‘Р•Р—РћРџРђРЎРќР«РҐ РђР›Р¬РўР•Р РќРђРўРР’
// ----------------------------------------------------------------------------

void demonstrateSecuredependency_injectionV1() {
    std::cout << "\n=== Р”Р•РњРћРќРЎРўР РђР¦РРЇ Securedependency_injectionV1 ===" << std::endl;
    
    // TODO: Р”РѕР±Р°РІРёС‚СЊ РґРµРјРѕРЅСЃС‚СЂР°С†РёСЋ РїРµСЂРІРѕР№ Р±РµР·РѕРїР°СЃРЅРѕР№ Р°Р»СЊС‚РµСЂРЅР°С‚РёРІС‹
    
    std::cout << "Securedependency_injectionV1 РїСЂРѕРґРµРјРѕРЅСЃС‚СЂРёСЂРѕРІР°РЅ" << std::endl;
}

void demonstrateSecuredependency_injectionV2() {
    std::cout << "\n=== Р”Р•РњРћРќРЎРўР РђР¦РРЇ Securedependency_injectionV2 ===" << std::endl;
    
    // TODO: Р”РѕР±Р°РІРёС‚СЊ РґРµРјРѕРЅСЃС‚СЂР°С†РёСЋ РІС‚РѕСЂРѕР№ Р±РµР·РѕРїР°СЃРЅРѕР№ Р°Р»СЊС‚РµСЂРЅР°С‚РёРІС‹
    
    std::cout << "Securedependency_injectionV2 РїСЂРѕРґРµРјРѕРЅСЃС‚СЂРёСЂРѕРІР°РЅ" << std::endl;
}

void demonstrateAlternativedependency_injection() {
    std::cout << "\n=== Р”Р•РњРћРќРЎРўР РђР¦РРЇ Alternativedependency_injection ===" << std::endl;
    
    // TODO: Р”РѕР±Р°РІРёС‚СЊ РґРµРјРѕРЅСЃС‚СЂР°С†РёСЋ Р°Р»СЊС‚РµСЂРЅР°С‚РёРІРЅРѕРіРѕ РїРѕРґС…РѕРґР°
    
    std::cout << "Alternativedependency_injection РїСЂРѕРґРµРјРѕРЅСЃС‚СЂРёСЂРѕРІР°РЅ" << std::endl;
}

// ----------------------------------------------------------------------------
// РћРЎРќРћР’РќРђРЇ Р¤РЈРќРљР¦РРЇ
// ----------------------------------------------------------------------------

int main() {
    std::cout << "=== Р”Р•РњРћРќРЎРўР РђР¦РРЇ Р‘Р•Р—РћРџРђРЎРќР«РҐ РђР›Р¬РўР•Р РќРђРўРР’ dependency_injection ===" << std::endl;
    
    // Р”РµРјРѕРЅСЃС‚СЂР°С†РёСЏ СЂР°Р·Р»РёС‡РЅС‹С… Р±РµР·РѕРїР°СЃРЅС‹С… СЂРµР°Р»РёР·Р°С†РёР№
    demonstrateSecuredependency_injectionV1();
    demonstrateSecuredependency_injectionV2();
    demonstrateAlternativedependency_injection();
    
    std::cout << "\n=== Р Р•РљРћРњР•РќР”РђР¦РР РџРћ Р‘Р•Р—РћРџРђРЎРќРћРЎРўР ===" << std::endl;
    std::cout << "1. TODO: Р”РѕР±Р°РІРёС‚СЊ РєРѕРЅРєСЂРµС‚РЅС‹Рµ СЂРµРєРѕРјРµРЅРґР°С†РёРё РґР»СЏ РґР°РЅРЅРѕРіРѕ РїР°С‚С‚РµСЂРЅР°" << std::endl;
    std::cout << "2. TODO: Р РµРєРѕРјРµРЅРґР°С†РёРё РїРѕ Р±РµР·РѕРїР°СЃРЅРѕРјСѓ РїСЂРѕРіСЂР°РјРјРёСЂРѕРІР°РЅРёСЋ" << std::endl;
    std::cout << "3. TODO: Р›СѓС‡С€РёРµ РїСЂР°РєС‚РёРєРё" << std::endl;
    std::cout << "4. TODO: РРЅСЃС‚СЂСѓРјРµРЅС‚С‹ Р°РЅР°Р»РёР·Р°" << std::endl;
    
    return 0;
}

