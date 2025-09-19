QT += core widgets network serialport
CONFIG += c++17

TARGET = DDRIntegrationTest
TEMPLATE = app

# 定义源文件
SOURCES += \
    CustomMessageHandler.cpp \
    DDRPerformanceMonitor.cpp \
    DDRIntegrationTest.cpp

HEADERS += \
    CustomMessageHandler.h \
    DDRPerformanceMonitor.h

# 添加必要的包含路径
INCLUDEPATH += . \
               .. \
               ../.. \
               ../../..

# 添加预处理器定义
DEFINES += QT_GUI_LIB QT_CORE_LIB QT_NETWORK_LIB QT_SERIALPORT_LIB

# 优化设置
CONFIG(release, debug|release) {
    QMAKE_CXXFLAGS_RELEASE += -O3
}

# Windows特定设置
win32 {
    CONFIG += console
    DESTDIR = bin
}

# 输出目录
OBJECTS_DIR = build/obj
MOC_DIR = build/moc
