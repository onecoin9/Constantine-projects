cmake_minimum_required(VERSION 3.16)
project(DDRPerformanceIntegration)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network SerialPort)

# 自动处理Qt的MOC
set(CMAKE_AUTOMOC ON)

# 添加包含目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
)

# 添加源文件
set(SOURCES
    CustomMessageHandler.cpp
    DDRPerformanceMonitor.cpp
    # 其他必要的源文件可以在这里添加
)

set(HEADERS
    CustomMessageHandler.h
    DDRPerformanceMonitor.h
    # 其他必要的头文件可以在这里添加
)

# 创建集成测试可执行文件
add_executable(DDRIntegrationTest
    ${SOURCES}
    ${HEADERS}
    DDRIntegrationTest.cpp
)

# 创建压力测试可执行文件
add_executable(DDRStressTest
    ${SOURCES}
    ${HEADERS}
    DDRStressTest.cpp
)

# 链接Qt库
target_link_libraries(DDRIntegrationTest
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::SerialPort
)

target_link_libraries(DDRStressTest
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::SerialPort
)

# 添加编译定义
target_compile_definitions(DDRIntegrationTest PRIVATE
    QT_GUI_LIB
    QT_CORE_LIB
    QT_NETWORK_LIB
    QT_SERIALPORT_LIB
)

target_compile_definitions(DDRStressTest PRIVATE
    QT_GUI_LIB
    QT_CORE_LIB
    QT_NETWORK_LIB
    QT_SERIALPORT_LIB
)

# 优化设置
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(DDRIntegrationTest PRIVATE -O3 -march=native)
    target_compile_options(DDRStressTest PRIVATE -O3 -march=native)
endif()

# 设置输出目录
set_target_properties(DDRIntegrationTest PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

set_target_properties(DDRStressTest PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 添加自定义目标来运行测试
add_custom_target(run_integration_test
    COMMAND DDRIntegrationTest
    DEPENDS DDRIntegrationTest
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    COMMENT "Running DDR Integration Test"
)

add_custom_target(run_stress_test
    COMMAND DDRStressTest
    DEPENDS DDRStressTest
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    COMMENT "Running DDR Stress Test"
)

# 添加帮助目标
add_custom_target(help_tests
    COMMAND ${CMAKE_COMMAND} -E echo "Available test targets:"
    COMMAND ${CMAKE_COMMAND} -E echo "  run_integration_test - Run DDR integration test"
    COMMAND ${CMAKE_COMMAND} -E echo "  run_stress_test      - Run DDR stress test"
    COMMENT "Displaying available test targets"
)
