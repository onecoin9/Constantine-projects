cmake_minimum_required(VERSION 3.16)

project(JsonRpcClient 
    VERSION 1.0.0
    DESCRIPTION "JSON-RPC 2.0 Client for Qt"
    LANGUAGES CXX
)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找Qt5组件
find_package(Qt5 REQUIRED COMPONENTS Core Network Widgets)

# 启用Qt的MOC、UIC、RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 源文件
set(CLIENT_SOURCES
    JsonRpcClient.h
    JsonRpcClient.cpp
    Ag06DoCustomProtocol.h
    Ag06DoCustomProtocol.cpp
    xt_trim_param.h
    Ag06TrimDialog.h
    Ag06TrimDialog.cpp
)

set(GUI_SOURCES
    JsonRpcTestWidget.h
    JsonRpcTestWidget.cpp
    gui_main.cpp
)

set(CONSOLE_SOURCES
    example_main.cpp
)

# 创建静态库（可选）
add_library(JsonRpcClientLib STATIC ${CLIENT_SOURCES})
target_link_libraries(JsonRpcClientLib Qt5::Core Qt5::Network)
target_include_directories(JsonRpcClientLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# 创建GUI可执行文件
add_executable(JsonRpcClientGUI ${CLIENT_SOURCES} ${GUI_SOURCES})
target_link_libraries(JsonRpcClientGUI Qt5::Core Qt5::Network Qt5::Widgets)

# 创建控制台可执行文件
add_executable(JsonRpcClientConsole ${CLIENT_SOURCES} ${CONSOLE_SOURCES})
target_link_libraries(JsonRpcClientConsole Qt5::Core Qt5::Network)

# 设置输出目录
set_target_properties(JsonRpcClientGUI JsonRpcClientConsole PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

set_target_properties(JsonRpcClientLib PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

# Windows平台特定设置
if(WIN32)
    # 设置GUI应用程序
    set_target_properties(JsonRpcClientGUI PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
    
    # 设置控制台应用程序
    set_target_properties(JsonRpcClientConsole PROPERTIES
        WIN32_EXECUTABLE FALSE
    )
    
    # UTF-8编码支持
    if(MSVC)
        target_compile_options(JsonRpcClientGUI PRIVATE /utf-8)
        target_compile_options(JsonRpcClientConsole PRIVATE /utf-8)
        target_compile_options(JsonRpcClientLib PRIVATE /utf-8)
    elseif(MINGW)
        target_compile_options(JsonRpcClientGUI PRIVATE -finput-charset=utf-8 -fexec-charset=utf-8)
        target_compile_options(JsonRpcClientConsole PRIVATE -finput-charset=utf-8 -fexec-charset=utf-8)
        target_compile_options(JsonRpcClientLib PRIVATE -finput-charset=utf-8 -fexec-charset=utf-8)
    endif()
endif()

# 安装规则（可选）
install(TARGETS JsonRpcClientGUI JsonRpcClientConsole
    RUNTIME DESTINATION bin
)

install(TARGETS JsonRpcClientLib
    ARCHIVE DESTINATION lib
)

install(FILES JsonRpcClient.h
    DESTINATION include
)

# 打印配置信息
message(STATUS "JsonRpcClient配置完成")
message(STATUS "Qt5版本: ${Qt5_VERSION}")
message(STATUS "编译器: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "C++标准: ${CMAKE_CXX_STANDARD}")
message(STATUS "构建类型: ${CMAKE_BUILD_TYPE}")

# 设置默认构建类型
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# 编译选项
target_compile_definitions(JsonRpcClientGUI PRIVATE
    QT_MESSAGELOGCONTEXT
    $<$<CONFIG:Debug>:DEBUG>
)

target_compile_definitions(JsonRpcClientConsole PRIVATE
    QT_MESSAGELOGCONTEXT
    $<$<CONFIG:Debug>:DEBUG>
)

target_compile_definitions(JsonRpcClientLib PRIVATE
    QT_MESSAGELOGCONTEXT
    $<$<CONFIG:Debug>:DEBUG>
) 