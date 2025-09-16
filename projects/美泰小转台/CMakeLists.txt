cmake_minimum_required(VERSION 3.16)
project(TesterFramework VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 对 MSVC 使用 UTF-8 源码编码
if (MSVC)
    add_compile_options("/utf-8")
endif()

# 设置CMake模块路径
#list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# 引入第三方库管理
#include(ThirdParty)

# 找到Qt5包
find_package(Qt5 5.15 REQUIRED COMPONENTS Core Widgets SerialPort Network Concurrent)

# 使用 file(GLOB_RECURSE) 自动发现源文件、头文件和UI文件
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.h")
file(GLOB_RECURSE UI_FILES "src/*.ui")

# 排除演示用 jsonRpcClientDemoSRC 目录，避免与新引入的类重复定义
file(GLOB_RECURSE DEMO_SOURCES "src/domain/jsonRpcClientDemoSRC/*.cpp")
file(GLOB_RECURSE DEMO_HEADERS "src/domain/jsonRpcClientDemoSRC/*.h")
if (DEMO_SOURCES)
    list(REMOVE_ITEM SOURCES ${DEMO_SOURCES})
endif()
if (DEMO_HEADERS)
    list(REMOVE_ITEM HEADERS ${DEMO_HEADERS})
endif()

# 资源文件
set(RESOURCE_FILES
    resources.qrc
)

# 创建可执行文件
add_executable(${PROJECT_NAME} WIN32
    ${SOURCES}
    ${HEADERS}
    ${UI_FILES}
    ${RESOURCE_FILES}
)

# 设置 Visual Studio 的启动项目为可执行目标，而不是 ALL_BUILD
if (MSVC)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
endif()

# 为目标添加包含目录
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/core
    ${CMAKE_CURRENT_SOURCE_DIR}/include/infrastructure
    ${CMAKE_CURRENT_SOURCE_DIR}/include/services
    ${CMAKE_CURRENT_SOURCE_DIR}/include/domain
    ${CMAKE_CURRENT_SOURCE_DIR}/include/domain/protocols
    ${CMAKE_CURRENT_SOURCE_DIR}/include/application
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/spdlog/include
)

# 链接Qt5库和第三方库
target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Widgets
    Qt5::SerialPort
    Qt5::Network
    Qt5::Concurrent
    #third_party_libs  # 第三方库接口
)

# 设置输出目录
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 复制配置文件到输出目录
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/config/
     DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
