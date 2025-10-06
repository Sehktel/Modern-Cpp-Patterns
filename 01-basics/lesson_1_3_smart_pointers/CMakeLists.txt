# CMakeLists.txt РґР»СЏ РјРѕРґСѓР»СЏ smart_pointers СЃ Р°РЅР°Р»РёР·РѕРј Р±РµР·РѕРїР°СЃРЅРѕСЃС‚Рё
cmake_minimum_required(VERSION 3.16)

# РќР°СЃС‚СЂРѕР№РєР° СЃС‚Р°РЅРґР°СЂС‚Р° C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# РџРѕРґРєР»СЋС‡РµРЅРёРµ РѕР±С‰РёС… СѓС‚РёР»РёС‚
add_subdirectory(../../common common)

# ----------------------------------------------------------------------------
# РћРЎРќРћР’РќР«Р• Р¦Р•Р›Р Р”Р›РЇ РР—РЈР§Р•РќРРЇ РџРђРўРўР•Р РќРђ
# ----------------------------------------------------------------------------

# РљР»Р°СЃСЃРёС‡РµСЃРєРёР№ smart_pointers
add_executable(smart_pointers_pattern smart_pointers_pattern.cpp)
target_link_libraries(smart_pointers_pattern common)

# РЎРѕРІСЂРµРјРµРЅРЅС‹Р№ smart_pointers
add_executable(modern_smart_pointers modern_smart_pointers.cpp)
target_link_libraries(modern_smart_pointers common)

# ----------------------------------------------------------------------------
# Р¦Р•Р›Р Р”Р›РЇ РђРќРђР›РР—Рђ Р‘Р•Р—РћРџРђРЎРќРћРЎРўР
# ----------------------------------------------------------------------------

# РЈСЏР·РІРёРјС‹Рµ СЂРµР°Р»РёР·Р°С†РёРё smart_pointers
add_executable(smart_pointers_vulnerabilities smart_pointers_vulnerabilities.cpp)
target_link_libraries(smart_pointers_vulnerabilities common)

# Р’РµСЂСЃРёСЏ СЃ ThreadSanitizer РґР»СЏ РѕР±РЅР°СЂСѓР¶РµРЅРёСЏ race conditions
add_executable(smart_pointers_vulnerabilities_tsan smart_pointers_vulnerabilities.cpp)
target_compile_options(smart_pointers_vulnerabilities_tsan PRIVATE -fsanitize=thread -fno-omit-frame-pointer -g)
target_link_options(smart_pointers_vulnerabilities_tsan PRIVATE -fsanitize=thread)
target_link_libraries(smart_pointers_vulnerabilities_tsan common)

# Р’РµСЂСЃРёСЏ СЃ AddressSanitizer РґР»СЏ РѕР±РЅР°СЂСѓР¶РµРЅРёСЏ use-after-free
add_executable(smart_pointers_vulnerabilities_asan smart_pointers_vulnerabilities.cpp)
target_compile_options(smart_pointers_vulnerabilities_asan PRIVATE -fsanitize=address -fno-omit-frame-pointer -g)
target_link_options(smart_pointers_vulnerabilities_asan PRIVATE -fsanitize=address)
target_link_libraries(smart_pointers_vulnerabilities_asan common)

# Р’РµСЂСЃРёСЏ СЃ UndefinedBehaviorSanitizer РґР»СЏ РѕР±РЅР°СЂСѓР¶РµРЅРёСЏ undefined behavior
add_executable(smart_pointers_vulnerabilities_ubsan smart_pointers_vulnerabilities.cpp)
target_compile_options(smart_pointers_vulnerabilities_ubsan PRIVATE -fsanitize=undefined -fno-omit-frame-pointer -g)
target_link_options(smart_pointers_vulnerabilities_ubsan PRIVATE -fsanitize=undefined)
target_link_libraries(smart_pointers_vulnerabilities_ubsan common)

# Р‘РµР·РѕРїР°СЃРЅС‹Рµ Р°Р»СЊС‚РµСЂРЅР°С‚РёРІС‹
add_executable(secure_smart_pointers_alternatives secure_smart_pointers_alternatives.cpp)
target_link_libraries(secure_smart_pointers_alternatives common)

# Р­РєСЃРїР»РѕРёС‚С‹
add_executable(smart_pointers_exploits exploits/smart_pointers_exploits.cpp)
target_link_libraries(smart_pointers_exploits common)

# ----------------------------------------------------------------------------
# РРќРЎРўР РЈРљР¦РР РџРћ РРЎРџРћР›Р¬Р—РћР’РђРќРР®
# ----------------------------------------------------------------------------

message(STATUS "=== РњРћР”РЈР›Р¬ smart_pointers РЎ РђРќРђР›РР—РћРњ Р‘Р•Р—РћРџРђРЎРќРћРЎРўР ===")
message(STATUS "")
message(STATUS "РћСЃРЅРѕРІРЅС‹Рµ С†РµР»Рё:")
message(STATUS "  make smart_pointers_pattern          - РљР»Р°СЃСЃРёС‡РµСЃРєРёР№ smart_pointers")
message(STATUS "  make modern_smart_pointers           - РЎРѕРІСЂРµРјРµРЅРЅС‹Р№ smart_pointers")
message(STATUS "")
message(STATUS "РђРЅР°Р»РёР· Р±РµР·РѕРїР°СЃРЅРѕСЃС‚Рё:")
message(STATUS "  make smart_pointers_vulnerabilities  - РЈСЏР·РІРёРјС‹Рµ СЂРµР°Р»РёР·Р°С†РёРё")
message(STATUS "  make smart_pointers_vulnerabilities_tsan  - РЎ ThreadSanitizer")
message(STATUS "  make smart_pointers_vulnerabilities_asan  - РЎ AddressSanitizer")
message(STATUS "  make smart_pointers_vulnerabilities_ubsan  - РЎ UndefinedBehaviorSanitizer")
message(STATUS "")
message(STATUS "Р‘РµР·РѕРїР°СЃРЅС‹Рµ Р°Р»СЊС‚РµСЂРЅР°С‚РёРІС‹:")
message(STATUS "  make secure_smart_pointers_alternatives  - Р‘РµР·РѕРїР°СЃРЅС‹Рµ СЂРµР°Р»РёР·Р°С†РёРё")
message(STATUS "")
message(STATUS "Р­РєСЃРїР»РѕРёС‚С‹:")
message(STATUS "  make smart_pointers_exploits         - РџСЂР°РєС‚РёС‡РµСЃРєРёРµ СЌРєСЃРїР»РѕРёС‚С‹")
message(STATUS "")
message(STATUS "Р—Р°РїСѓСЃРє Р°РЅР°Р»РёР·Р°:")
message(STATUS "  ./smart_pointers_vulnerabilities_tsan    # РћР±РЅР°СЂСѓР¶РµРЅРёРµ race conditions")
message(STATUS "  ./smart_pointers_vulnerabilities_asan     # РћР±РЅР°СЂСѓР¶РµРЅРёРµ use-after-free")
message(STATUS "  ./smart_pointers_vulnerabilities_ubsan    # РћР±РЅР°СЂСѓР¶РµРЅРёРµ undefined behavior")
message(STATUS "")
message(STATUS "РЎС‚Р°С‚РёС‡РµСЃРєРёР№ Р°РЅР°Р»РёР·:")
message(STATUS "  clang --analyze smart_pointers_vulnerabilities.cpp")
message(STATUS "  cppcheck --enable=all smart_pointers_vulnerabilities.cpp")
message(STATUS "")
message(STATUS "Р”РёРЅР°РјРёС‡РµСЃРєРёР№ Р°РЅР°Р»РёР· СЃ Valgrind:")
message(STATUS "  valgrind --tool=helgrind ./smart_pointers_vulnerabilities")
message(STATUS "  valgrind --tool=memcheck ./smart_pointers_vulnerabilities")

