# CMakeLists.txt РґР»СЏ РјРѕРґСѓР»СЏ reactor СЃ Р°РЅР°Р»РёР·РѕРј Р±РµР·РѕРїР°СЃРЅРѕСЃС‚Рё
cmake_minimum_required(VERSION 3.16)

# РќР°СЃС‚СЂРѕР№РєР° СЃС‚Р°РЅРґР°СЂС‚Р° C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# РџРѕРґРєР»СЋС‡РµРЅРёРµ РѕР±С‰РёС… СѓС‚РёР»РёС‚
add_subdirectory(../../common common)

# ----------------------------------------------------------------------------
# РћРЎРќРћР’РќР«Р• Р¦Р•Р›Р Р”Р›РЇ РР—РЈР§Р•РќРРЇ РџРђРўРўР•Р РќРђ
# ----------------------------------------------------------------------------

# РљР»Р°СЃСЃРёС‡РµСЃРєРёР№ reactor
add_executable(reactor_pattern reactor_pattern.cpp)
target_link_libraries(reactor_pattern common)

# РЎРѕРІСЂРµРјРµРЅРЅС‹Р№ reactor
add_executable(modern_reactor modern_reactor.cpp)
target_link_libraries(modern_reactor common)

# ----------------------------------------------------------------------------
# Р¦Р•Р›Р Р”Р›РЇ РђРќРђР›РР—Рђ Р‘Р•Р—РћРџРђРЎРќРћРЎРўР
# ----------------------------------------------------------------------------

# РЈСЏР·РІРёРјС‹Рµ СЂРµР°Р»РёР·Р°С†РёРё reactor
add_executable(reactor_vulnerabilities reactor_vulnerabilities.cpp)
target_link_libraries(reactor_vulnerabilities common)

# Р’РµСЂСЃРёСЏ СЃ ThreadSanitizer РґР»СЏ РѕР±РЅР°СЂСѓР¶РµРЅРёСЏ race conditions
add_executable(reactor_vulnerabilities_tsan reactor_vulnerabilities.cpp)
target_compile_options(reactor_vulnerabilities_tsan PRIVATE -fsanitize=thread -fno-omit-frame-pointer -g)
target_link_options(reactor_vulnerabilities_tsan PRIVATE -fsanitize=thread)
target_link_libraries(reactor_vulnerabilities_tsan common)

# Р’РµСЂСЃРёСЏ СЃ AddressSanitizer РґР»СЏ РѕР±РЅР°СЂСѓР¶РµРЅРёСЏ use-after-free
add_executable(reactor_vulnerabilities_asan reactor_vulnerabilities.cpp)
target_compile_options(reactor_vulnerabilities_asan PRIVATE -fsanitize=address -fno-omit-frame-pointer -g)
target_link_options(reactor_vulnerabilities_asan PRIVATE -fsanitize=address)
target_link_libraries(reactor_vulnerabilities_asan common)

# Р’РµСЂСЃРёСЏ СЃ UndefinedBehaviorSanitizer РґР»СЏ РѕР±РЅР°СЂСѓР¶РµРЅРёСЏ undefined behavior
add_executable(reactor_vulnerabilities_ubsan reactor_vulnerabilities.cpp)
target_compile_options(reactor_vulnerabilities_ubsan PRIVATE -fsanitize=undefined -fno-omit-frame-pointer -g)
target_link_options(reactor_vulnerabilities_ubsan PRIVATE -fsanitize=undefined)
target_link_libraries(reactor_vulnerabilities_ubsan common)

# Р‘РµР·РѕРїР°СЃРЅС‹Рµ Р°Р»СЊС‚РµСЂРЅР°С‚РёРІС‹
add_executable(secure_reactor_alternatives secure_reactor_alternatives.cpp)
target_link_libraries(secure_reactor_alternatives common)

# Р­РєСЃРїР»РѕРёС‚С‹
add_executable(reactor_exploits exploits/reactor_exploits.cpp)
target_link_libraries(reactor_exploits common)

# ----------------------------------------------------------------------------
# РРќРЎРўР РЈРљР¦РР РџРћ РРЎРџРћР›Р¬Р—РћР’РђРќРР®
# ----------------------------------------------------------------------------

message(STATUS "=== РњРћР”РЈР›Р¬ reactor РЎ РђРќРђР›РР—РћРњ Р‘Р•Р—РћРџРђРЎРќРћРЎРўР ===")
message(STATUS "")
message(STATUS "РћСЃРЅРѕРІРЅС‹Рµ С†РµР»Рё:")
message(STATUS "  make reactor_pattern          - РљР»Р°СЃСЃРёС‡РµСЃРєРёР№ reactor")
message(STATUS "  make modern_reactor           - РЎРѕРІСЂРµРјРµРЅРЅС‹Р№ reactor")
message(STATUS "")
message(STATUS "РђРЅР°Р»РёР· Р±РµР·РѕРїР°СЃРЅРѕСЃС‚Рё:")
message(STATUS "  make reactor_vulnerabilities  - РЈСЏР·РІРёРјС‹Рµ СЂРµР°Р»РёР·Р°С†РёРё")
message(STATUS "  make reactor_vulnerabilities_tsan  - РЎ ThreadSanitizer")
message(STATUS "  make reactor_vulnerabilities_asan  - РЎ AddressSanitizer")
message(STATUS "  make reactor_vulnerabilities_ubsan  - РЎ UndefinedBehaviorSanitizer")
message(STATUS "")
message(STATUS "Р‘РµР·РѕРїР°СЃРЅС‹Рµ Р°Р»СЊС‚РµСЂРЅР°С‚РёРІС‹:")
message(STATUS "  make secure_reactor_alternatives  - Р‘РµР·РѕРїР°СЃРЅС‹Рµ СЂРµР°Р»РёР·Р°С†РёРё")
message(STATUS "")
message(STATUS "Р­РєСЃРїР»РѕРёС‚С‹:")
message(STATUS "  make reactor_exploits         - РџСЂР°РєС‚РёС‡РµСЃРєРёРµ СЌРєСЃРїР»РѕРёС‚С‹")
message(STATUS "")
message(STATUS "Р—Р°РїСѓСЃРє Р°РЅР°Р»РёР·Р°:")
message(STATUS "  ./reactor_vulnerabilities_tsan    # РћР±РЅР°СЂСѓР¶РµРЅРёРµ race conditions")
message(STATUS "  ./reactor_vulnerabilities_asan     # РћР±РЅР°СЂСѓР¶РµРЅРёРµ use-after-free")
message(STATUS "  ./reactor_vulnerabilities_ubsan    # РћР±РЅР°СЂСѓР¶РµРЅРёРµ undefined behavior")
message(STATUS "")
message(STATUS "РЎС‚Р°С‚РёС‡РµСЃРєРёР№ Р°РЅР°Р»РёР·:")
message(STATUS "  clang --analyze reactor_vulnerabilities.cpp")
message(STATUS "  cppcheck --enable=all reactor_vulnerabilities.cpp")
message(STATUS "")
message(STATUS "Р”РёРЅР°РјРёС‡РµСЃРєРёР№ Р°РЅР°Р»РёР· СЃ Valgrind:")
message(STATUS "  valgrind --tool=helgrind ./reactor_vulnerabilities")
message(STATUS "  valgrind --tool=memcheck ./reactor_vulnerabilities")

