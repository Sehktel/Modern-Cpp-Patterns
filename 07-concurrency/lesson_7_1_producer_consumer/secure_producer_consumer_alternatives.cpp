#include <iostream>
#include <thread>
#include <mutex>
#include <memory>
#include <vector>
#include <chrono>
#include <cstring>
#include <atomic>

// ============================================================================
// Р‘Р•Р—РћРџРђРЎРќР«Р• РђР›Р¬РўР•Р РќРђРўРР’Р« producer_consumer
// ============================================================================

// TODO: Р”РѕР±Р°РІРёС‚СЊ РєРѕРјРјРµРЅС‚Р°СЂРёРё РЅР° СЂСѓСЃСЃРєРѕРј СЏР·С‹РєРµ
// TODO: РЎРѕР·РґР°С‚СЊ РґРѕРїРѕР»РЅРёС‚РµР»СЊРЅС‹Рµ Р±РµР·РѕРїР°СЃРЅС‹Рµ РІР°СЂРёР°РЅС‚С‹
// TODO: Р”РѕР±Р°РІРёС‚СЊ С‚РµСЃС‚С‹ Р±РµР·РѕРїР°СЃРЅРѕСЃС‚Рё

// ----------------------------------------------------------------------------
// Р‘Р•Р—РћРџРђРЎРќРђРЇ РђР›Р¬РўР•Р РќРђРўРР’Рђ 1: [РќРђР—Р’РђРќРР• РџРћР”РҐРћР”Рђ]
// ----------------------------------------------------------------------------

class Secureproducer_consumerV1 {
private:
    // TODO: Р”РѕР±Р°РІРёС‚СЊ Р±РµР·РѕРїР°СЃРЅС‹Рµ РїСЂРёРІР°С‚РЅС‹Рµ С‡Р»РµРЅС‹
    mutable std::mutex data_mutex;
    
    Secureproducer_consumerV1() {
        // TODO: Р”РѕР±Р°РІРёС‚СЊ Р±РµР·РѕРїР°СЃРЅС‹Р№ РєРѕРЅСЃС‚СЂСѓРєС‚РѕСЂ
        std::cout << "Secureproducer_consumerV1 СЃРѕР·РґР°РЅ РІ РїРѕС‚РѕРєРµ " << std::this_thread::get_id() << std::endl;
    }
    
public:
    // РЈРґР°Р»СЏРµРј РєРѕРїРёСЂРѕРІР°РЅРёРµ Рё РїСЂРёСЃРІР°РёРІР°РЅРёРµ
    Secureproducer_consumerV1(const Secureproducer_consumerV1&) = delete;
    Secureproducer_consumerV1& operator=(const Secureproducer_consumerV1&) = delete;
    
    // TODO: Р”РѕР±Р°РІРёС‚СЊ Р±РµР·РѕРїР°СЃРЅС‹Рµ РјРµС‚РѕРґС‹
    
    ~Secureproducer_consumerV1() {
        // TODO: Р”РѕР±Р°РІРёС‚СЊ Р±РµР·РѕРїР°СЃРЅС‹Р№ РґРµСЃС‚СЂСѓРєС‚РѕСЂ
        std::cout << "Secureproducer_consumerV1 СѓРЅРёС‡С‚РѕР¶РµРЅ" << std::endl;
    }
};

// ----------------------------------------------------------------------------
// Р‘Р•Р—РћРџРђРЎРќРђРЇ РђР›Р¬РўР•Р РќРђРўРР’Рђ 2: [РќРђР—Р’РђРќРР• РџРћР”РҐРћР”Рђ]
// ----------------------------------------------------------------------------

class Secureproducer_consumerV2 {
private:
    // TODO: Р”РѕР±Р°РІРёС‚СЊ Р±РµР·РѕРїР°СЃРЅС‹Рµ РїСЂРёРІР°С‚РЅС‹Рµ С‡Р»РµРЅС‹
    
    Secureproducer_consumerV2() {
        // TODO: Р”РѕР±Р°РІРёС‚СЊ Р±РµР·РѕРїР°СЃРЅС‹Р№ РєРѕРЅСЃС‚СЂСѓРєС‚РѕСЂ
        std::cout << "Secureproducer_consumerV2 СЃРѕР·РґР°РЅ РІ РїРѕС‚РѕРєРµ " << std::this_thread::get_id() << std::endl;
    }
    
public:
    // РЈРґР°Р»СЏРµРј РєРѕРїРёСЂРѕРІР°РЅРёРµ Рё РїСЂРёСЃРІР°РёРІР°РЅРёРµ
    Secureproducer_consumerV2(const Secureproducer_consumerV2&) = delete;
    Secureproducer_consumerV2& operator=(const Secureproducer_consumerV2&) = delete;
    
    // TODO: Р”РѕР±Р°РІРёС‚СЊ Р±РµР·РѕРїР°СЃРЅС‹Рµ РјРµС‚РѕРґС‹
    
    ~Secureproducer_consumerV2() {
        // TODO: Р”РѕР±Р°РІРёС‚СЊ Р±РµР·РѕРїР°СЃРЅС‹Р№ РґРµСЃС‚СЂСѓРєС‚РѕСЂ
        std::cout << "Secureproducer_consumerV2 СѓРЅРёС‡С‚РѕР¶РµРЅ" << std::endl;
    }
};

// ----------------------------------------------------------------------------
// Р‘Р•Р—РћРџРђРЎРќРђРЇ РђР›Р¬РўР•Р РќРђРўРР’Рђ 3: [РќРђР—Р’РђРќРР• РџРћР”РҐРћР”Рђ]
// ----------------------------------------------------------------------------

class Alternativeproducer_consumer {
private:
    // TODO: Р”РѕР±Р°РІРёС‚СЊ РїСЂРёРІР°С‚РЅС‹Рµ С‡Р»РµРЅС‹ Р°Р»СЊС‚РµСЂРЅР°С‚РёРІРЅРѕРіРѕ РїРѕРґС…РѕРґР°
    
public:
    Alternativeproducer_consumer() {
        // TODO: Р”РѕР±Р°РІРёС‚СЊ РєРѕРЅСЃС‚СЂСѓРєС‚РѕСЂ
        std::cout << "Alternativeproducer_consumer СЃРѕР·РґР°РЅ" << std::endl;
    }
    
    // РЈРґР°Р»СЏРµРј РєРѕРїРёСЂРѕРІР°РЅРёРµ Рё РїСЂРёСЃРІР°РёРІР°РЅРёРµ
    Alternativeproducer_consumer(const Alternativeproducer_consumer&) = delete;
    Alternativeproducer_consumer& operator=(const Alternativeproducer_consumer&) = delete;
    
    // TODO: Р”РѕР±Р°РІРёС‚СЊ РјРµС‚РѕРґС‹
    
    ~Alternativeproducer_consumer() {
        // TODO: Р”РѕР±Р°РІРёС‚СЊ РґРµСЃС‚СЂСѓРєС‚РѕСЂ
        std::cout << "Alternativeproducer_consumer СѓРЅРёС‡С‚РѕР¶РµРЅ" << std::endl;
    }
};

// ----------------------------------------------------------------------------
// Р”Р•РњРћРќРЎРўР РђР¦РРЇ Р‘Р•Р—РћРџРђРЎРќР«РҐ РђР›Р¬РўР•Р РќРђРўРР’
// ----------------------------------------------------------------------------

void demonstrateSecureproducer_consumerV1() {
    std::cout << "\n=== Р”Р•РњРћРќРЎРўР РђР¦РРЇ Secureproducer_consumerV1 ===" << std::endl;
    
    // TODO: Р”РѕР±Р°РІРёС‚СЊ РґРµРјРѕРЅСЃС‚СЂР°С†РёСЋ РїРµСЂРІРѕР№ Р±РµР·РѕРїР°СЃРЅРѕР№ Р°Р»СЊС‚РµСЂРЅР°С‚РёРІС‹
    
    std::cout << "Secureproducer_consumerV1 РїСЂРѕРґРµРјРѕРЅСЃС‚СЂРёСЂРѕРІР°РЅ" << std::endl;
}

void demonstrateSecureproducer_consumerV2() {
    std::cout << "\n=== Р”Р•РњРћРќРЎРўР РђР¦РРЇ Secureproducer_consumerV2 ===" << std::endl;
    
    // TODO: Р”РѕР±Р°РІРёС‚СЊ РґРµРјРѕРЅСЃС‚СЂР°С†РёСЋ РІС‚РѕСЂРѕР№ Р±РµР·РѕРїР°СЃРЅРѕР№ Р°Р»СЊС‚РµСЂРЅР°С‚РёРІС‹
    
    std::cout << "Secureproducer_consumerV2 РїСЂРѕРґРµРјРѕРЅСЃС‚СЂРёСЂРѕРІР°РЅ" << std::endl;
}

void demonstrateAlternativeproducer_consumer() {
    std::cout << "\n=== Р”Р•РњРћРќРЎРўР РђР¦РРЇ Alternativeproducer_consumer ===" << std::endl;
    
    // TODO: Р”РѕР±Р°РІРёС‚СЊ РґРµРјРѕРЅСЃС‚СЂР°С†РёСЋ Р°Р»СЊС‚РµСЂРЅР°С‚РёРІРЅРѕРіРѕ РїРѕРґС…РѕРґР°
    
    std::cout << "Alternativeproducer_consumer РїСЂРѕРґРµРјРѕРЅСЃС‚СЂРёСЂРѕРІР°РЅ" << std::endl;
}

// ----------------------------------------------------------------------------
// РћРЎРќРћР’РќРђРЇ Р¤РЈРќРљР¦РРЇ
// ----------------------------------------------------------------------------

int main() {
    std::cout << "=== Р”Р•РњРћРќРЎРўР РђР¦РРЇ Р‘Р•Р—РћРџРђРЎРќР«РҐ РђР›Р¬РўР•Р РќРђРўРР’ producer_consumer ===" << std::endl;
    
    // Р”РµРјРѕРЅСЃС‚СЂР°С†РёСЏ СЂР°Р·Р»РёС‡РЅС‹С… Р±РµР·РѕРїР°СЃРЅС‹С… СЂРµР°Р»РёР·Р°С†РёР№
    demonstrateSecureproducer_consumerV1();
    demonstrateSecureproducer_consumerV2();
    demonstrateAlternativeproducer_consumer();
    
    std::cout << "\n=== Р Р•РљРћРњР•РќР”РђР¦РР РџРћ Р‘Р•Р—РћРџРђРЎРќРћРЎРўР ===" << std::endl;
    std::cout << "1. TODO: Р”РѕР±Р°РІРёС‚СЊ РєРѕРЅРєСЂРµС‚РЅС‹Рµ СЂРµРєРѕРјРµРЅРґР°С†РёРё РґР»СЏ РґР°РЅРЅРѕРіРѕ РїР°С‚С‚РµСЂРЅР°" << std::endl;
    std::cout << "2. TODO: Р РµРєРѕРјРµРЅРґР°С†РёРё РїРѕ Р±РµР·РѕРїР°СЃРЅРѕРјСѓ РїСЂРѕРіСЂР°РјРјРёСЂРѕРІР°РЅРёСЋ" << std::endl;
    std::cout << "3. TODO: Р›СѓС‡С€РёРµ РїСЂР°РєС‚РёРєРё" << std::endl;
    std::cout << "4. TODO: РРЅСЃС‚СЂСѓРјРµРЅС‚С‹ Р°РЅР°Р»РёР·Р°" << std::endl;
    
    return 0;
}

