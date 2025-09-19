QT += core network widgets
CONFIG += c++11

# 项目信息
TARGET = JsonRpcClientGUI
TEMPLATE = app

# 源文件
SOURCES += \
    JsonRpcClient.cpp \
    JsonRpcTestWidget.cpp \
    gui_main.cpp \
    Ag06DoCustomProtocol.cpp \
    Ag06TrimDialog.cpp

# 头文件
HEADERS += \
    JsonRpcClient.h \
    JsonRpcTestWidget.h \
    Ag06DoCustomProtocol.h \
    xt_trim_param.h \
    Ag06TrimDialog.h

# 控制台版本（可选）
console {
    QT -= gui widgets
    CONFIG += console
    CONFIG -= app_bundle
    TARGET = JsonRpcClientConsole
    SOURCES -= JsonRpcTestWidget.cpp gui_main.cpp
    SOURCES += example_main.cpp
    HEADERS -= JsonRpcTestWidget.h
}

# 编译器选项
win32 {
    # MSVC编译器
    msvc {
        QMAKE_CXXFLAGS += /utf-8
    }
    # MinGW编译器  
    mingw {
        QMAKE_CXXFLAGS += -finput-charset=utf-8 -fexec-charset=utf-8
    }
}

# 输出目录
CONFIG(debug, debug|release) {
    DESTDIR = debug
    OBJECTS_DIR = debug/obj
    MOC_DIR = debug/moc
} else {
    DESTDIR = release
    OBJECTS_DIR = release/obj
    MOC_DIR = release/moc
}

# 版本信息
VERSION = 1.0.0
QMAKE_TARGET_COMPANY = "Your Company"
QMAKE_TARGET_PRODUCT = "JsonRpcClient"
QMAKE_TARGET_DESCRIPTION = "JSON-RPC 2.0 Client for Qt"
QMAKE_TARGET_COPYRIGHT = "Copyright 2024" 