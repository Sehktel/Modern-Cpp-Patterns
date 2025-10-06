# Урок 2.1: SOLID принципы
# Демонстрация пяти фундаментальных принципов объектно-ориентированного проектирования

cmake_minimum_required(VERSION 3.10)

# Исполняемые файлы для демонстрации SOLID принципов
add_executable(solid_principles solid_principles.cpp)
target_link_libraries(solid_principles common_utils)

add_executable(srp_example srp_example.cpp)
target_link_libraries(srp_example common_utils)

add_executable(ocp_example ocp_example.cpp)
target_link_libraries(ocp_example common_utils)

add_executable(lsp_example lsp_example.cpp)
target_link_libraries(lsp_example common_utils)

add_executable(isp_example isp_example.cpp)
target_link_libraries(isp_example common_utils)

add_executable(dip_example dip_example.cpp)
target_link_libraries(dip_example common_utils)

# Добавляем в общий target модуля
add_dependencies(principles_all solid_principles srp_example ocp_example lsp_example isp_example dip_example)
