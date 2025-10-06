#include <iostream>
#include <thread>
#include <mutex>
#include <memory>
#include <vector>
#include <chrono>
#include <cstring>
#include <atomic>

// ============================================================================
// Р‘Р•Р—РћРџРђРЎРќР«Р• РђР›Р¬РўР•Р РќРђРўРР’Р« composition_inheritance
// ============================================================================

// TODO: Р”РѕР±Р°РІРёС‚СЊ РєРѕРјРјРµРЅС‚Р°СЂРёРё РЅР° СЂСѓСЃСЃРєРѕРј СЏР·С‹РєРµ
// TODO: РЎРѕР·РґР°С‚СЊ РґРѕРїРѕР»РЅРёС‚РµР»СЊРЅС‹Рµ Р±РµР·РѕРїР°СЃРЅС‹Рµ РІР°СЂРёР°РЅС‚С‹
// TODO: Р”РѕР±Р°РІРёС‚СЊ С‚РµСЃС‚С‹ Р±РµР·РѕРїР°СЃРЅРѕСЃС‚Рё

// ----------------------------------------------------------------------------
// Р‘Р•Р—РћРџРђРЎРќРђРЇ РђР›Р¬РўР•Р РќРђРўРР’Рђ 1: [РќРђР—Р’РђРќРР• РџРћР”РҐРћР”Рђ]
// ----------------------------------------------------------------------------

class Securecomposition_inheritanceV1 {
private:
    // TODO: Р”РѕР±Р°РІРёС‚СЊ Р±РµР·РѕРїР°СЃРЅС‹Рµ РїСЂРёРІР°С‚РЅС‹Рµ С‡Р»РµРЅС‹
    mutable std::mutex data_mutex;
    
    Securecomposition_inheritanceV1() {
        // TODO: Р”РѕР±Р°РІРёС‚СЊ Р±РµР·РѕРїР°СЃРЅС‹Р№ РєРѕРЅСЃС‚СЂСѓРєС‚РѕСЂ
        std::cout << "Securecomposition_inheritanceV1 СЃРѕР·РґР°РЅ РІ РїРѕС‚РѕРєРµ " << std::this_thread::get_id() << std::endl;
    }
    
public:
    // РЈРґР°Р»СЏРµРј РєРѕРїРёСЂРѕРІР°РЅРёРµ Рё РїСЂРёСЃРІР°РёРІР°РЅРёРµ
    Securecomposition_inheritanceV1(const Securecomposition_inheritanceV1&) = delete;
    Securecomposition_inheritanceV1& operator=(const Securecomposition_inheritanceV1&) = delete;
    
    // TODO: Р”РѕР±Р°РІРёС‚СЊ Р±РµР·РѕРїР°СЃРЅС‹Рµ РјРµС‚РѕРґС‹
    
    ~Securecomposition_inheritanceV1() {
        // TODO: Р”РѕР±Р°РІРёС‚СЊ Р±РµР·РѕРїР°СЃРЅС‹Р№ РґРµСЃС‚СЂСѓРєС‚РѕСЂ
        std::cout << "Securecomposition_inheritanceV1 СѓРЅРёС‡С‚РѕР¶РµРЅ" << std::endl;
    }
};

// ----------------------------------------------------------------------------
// Р‘Р•Р—РћРџРђРЎРќРђРЇ РђР›Р¬РўР•Р РќРђРўРР’Рђ 2: [РќРђР—Р’РђРќРР• РџРћР”РҐРћР”Рђ]
// ----------------------------------------------------------------------------

class Securecomposition_inheritanceV2 {
private:
    // TODO: Р”РѕР±Р°РІРёС‚СЊ Р±РµР·РѕРїР°СЃРЅС‹Рµ РїСЂРёРІР°С‚РЅС‹Рµ С‡Р»РµРЅС‹
    
    Securecomposition_inheritanceV2() {
        // TODO: Р”РѕР±Р°РІРёС‚СЊ Р±РµР·РѕРїР°СЃРЅС‹Р№ РєРѕРЅСЃС‚СЂСѓРєС‚РѕСЂ
        std::cout << "Securecomposition_inheritanceV2 СЃРѕР·РґР°РЅ РІ РїРѕС‚РѕРєРµ " << std::this_thread::get_id() << std::endl;
    }
    
public:
    // РЈРґР°Р»СЏРµРј РєРѕРїРёСЂРѕРІР°РЅРёРµ Рё РїСЂРёСЃРІР°РёРІР°РЅРёРµ
    Securecomposition_inheritanceV2(const Securecomposition_inheritanceV2&) = delete;
    Securecomposition_inheritanceV2& operator=(const Securecomposition_inheritanceV2&) = delete;
    
    // TODO: Р”РѕР±Р°РІРёС‚СЊ Р±РµР·РѕРїР°СЃРЅС‹Рµ РјРµС‚РѕРґС‹
    
    ~Securecomposition_inheritanceV2() {
        // TODO: Р”РѕР±Р°РІРёС‚СЊ Р±РµР·РѕРїР°СЃРЅС‹Р№ РґРµСЃС‚СЂСѓРєС‚РѕСЂ
        std::cout << "Securecomposition_inheritanceV2 СѓРЅРёС‡С‚РѕР¶РµРЅ" << std::endl;
    }
};

// ----------------------------------------------------------------------------
// Р‘Р•Р—РћРџРђРЎРќРђРЇ РђР›Р¬РўР•Р РќРђРўРР’Рђ 3: [РќРђР—Р’РђРќРР• РџРћР”РҐРћР”Рђ]
// ----------------------------------------------------------------------------

class Alternativecomposition_inheritance {
private:
    // TODO: Р”РѕР±Р°РІРёС‚СЊ РїСЂРёРІР°С‚РЅС‹Рµ С‡Р»РµРЅС‹ Р°Р»СЊС‚РµСЂРЅР°С‚РёРІРЅРѕРіРѕ РїРѕРґС…РѕРґР°
    
public:
    Alternativecomposition_inheritance() {
        // TODO: Р”РѕР±Р°РІРёС‚СЊ РєРѕРЅСЃС‚СЂСѓРєС‚РѕСЂ
        std::cout << "Alternativecomposition_inheritance СЃРѕР·РґР°РЅ" << std::endl;
    }
    
    // РЈРґР°Р»СЏРµРј РєРѕРїРёСЂРѕРІР°РЅРёРµ Рё РїСЂРёСЃРІР°РёРІР°РЅРёРµ
    Alternativecomposition_inheritance(const Alternativecomposition_inheritance&) = delete;
    Alternativecomposition_inheritance& operator=(const Alternativecomposition_inheritance&) = delete;
    
    // TODO: Р”РѕР±Р°РІРёС‚СЊ РјРµС‚РѕРґС‹
    
    ~Alternativecomposition_inheritance() {
        // TODO: Р”РѕР±Р°РІРёС‚СЊ РґРµСЃС‚СЂСѓРєС‚РѕСЂ
        std::cout << "Alternativecomposition_inheritance СѓРЅРёС‡С‚РѕР¶РµРЅ" << std::endl;
    }
};

// ----------------------------------------------------------------------------
// Р”Р•РњРћРќРЎРўР РђР¦РРЇ Р‘Р•Р—РћРџРђРЎРќР«РҐ РђР›Р¬РўР•Р РќРђРўРР’
// ----------------------------------------------------------------------------

void demonstrateSecurecomposition_inheritanceV1() {
    std::cout << "\n=== Р”Р•РњРћРќРЎРўР РђР¦РРЇ Securecomposition_inheritanceV1 ===" << std::endl;
    
    // TODO: Р”РѕР±Р°РІРёС‚СЊ РґРµРјРѕРЅСЃС‚СЂР°С†РёСЋ РїРµСЂРІРѕР№ Р±РµР·РѕРїР°СЃРЅРѕР№ Р°Р»СЊС‚РµСЂРЅР°С‚РёРІС‹
    
    std::cout << "Securecomposition_inheritanceV1 РїСЂРѕРґРµРјРѕРЅСЃС‚СЂРёСЂРѕРІР°РЅ" << std::endl;
}

void demonstrateSecurecomposition_inheritanceV2() {
    std::cout << "\n=== Р”Р•РњРћРќРЎРўР РђР¦РРЇ Securecomposition_inheritanceV2 ===" << std::endl;
    
    // TODO: Р”РѕР±Р°РІРёС‚СЊ РґРµРјРѕРЅСЃС‚СЂР°С†РёСЋ РІС‚РѕСЂРѕР№ Р±РµР·РѕРїР°СЃРЅРѕР№ Р°Р»СЊС‚РµСЂРЅР°С‚РёРІС‹
    
    std::cout << "Securecomposition_inheritanceV2 РїСЂРѕРґРµРјРѕРЅСЃС‚СЂРёСЂРѕРІР°РЅ" << std::endl;
}

void demonstrateAlternativecomposition_inheritance() {
    std::cout << "\n=== Р”Р•РњРћРќРЎРўР РђР¦РРЇ Alternativecomposition_inheritance ===" << std::endl;
    
    // TODO: Р”РѕР±Р°РІРёС‚СЊ РґРµРјРѕРЅСЃС‚СЂР°С†РёСЋ Р°Р»СЊС‚РµСЂРЅР°С‚РёРІРЅРѕРіРѕ РїРѕРґС…РѕРґР°
    
    std::cout << "Alternativecomposition_inheritance РїСЂРѕРґРµРјРѕРЅСЃС‚СЂРёСЂРѕРІР°РЅ" << std::endl;
}

// ----------------------------------------------------------------------------
// РћРЎРќРћР’РќРђРЇ Р¤РЈРќРљР¦РРЇ
// ----------------------------------------------------------------------------

int main() {
    std::cout << "=== Р”Р•РњРћРќРЎРўР РђР¦РРЇ Р‘Р•Р—РћРџРђРЎРќР«РҐ РђР›Р¬РўР•Р РќРђРўРР’ composition_inheritance ===" << std::endl;
    
    // Р”РµРјРѕРЅСЃС‚СЂР°С†РёСЏ СЂР°Р·Р»РёС‡РЅС‹С… Р±РµР·РѕРїР°СЃРЅС‹С… СЂРµР°Р»РёР·Р°С†РёР№
    demonstrateSecurecomposition_inheritanceV1();
    demonstrateSecurecomposition_inheritanceV2();
    demonstrateAlternativecomposition_inheritance();
    
    std::cout << "\n=== Р Р•РљРћРњР•РќР”РђР¦РР РџРћ Р‘Р•Р—РћРџРђРЎРќРћРЎРўР ===" << std::endl;
    std::cout << "1. TODO: Р”РѕР±Р°РІРёС‚СЊ РєРѕРЅРєСЂРµС‚РЅС‹Рµ СЂРµРєРѕРјРµРЅРґР°С†РёРё РґР»СЏ РґР°РЅРЅРѕРіРѕ РїР°С‚С‚РµСЂРЅР°" << std::endl;
    std::cout << "2. TODO: Р РµРєРѕРјРµРЅРґР°С†РёРё РїРѕ Р±РµР·РѕРїР°СЃРЅРѕРјСѓ РїСЂРѕРіСЂР°РјРјРёСЂРѕРІР°РЅРёСЋ" << std::endl;
    std::cout << "3. TODO: Р›СѓС‡С€РёРµ РїСЂР°РєС‚РёРєРё" << std::endl;
    std::cout << "4. TODO: РРЅСЃС‚СЂСѓРјРµРЅС‚С‹ Р°РЅР°Р»РёР·Р°" << std::endl;
    
    return 0;
}

