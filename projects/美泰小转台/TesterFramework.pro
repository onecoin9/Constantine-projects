# TesterFramework.pro
# Qt项目配置文件，对应CMakeLists.txt

QT += core widgets serialport network concurrent

CONFIG += c++17
CONFIG += console

TARGET = TesterFramework
TEMPLATE = app

# 编译器特定配置
msvc {
    QMAKE_CXXFLAGS += /utf-8
}

# 包含目录
INCLUDEPATH += $$PWD/include \
               $$PWD/include/core \
               $$PWD/include/infrastructure \
               $$PWD/include/services \
               $$PWD/include/domain \
               $$PWD/include/domain/protocols \
               $$PWD/include/application \
               $$PWD/include/ui

# 第三方库配置
# spdlog库配置 - header-only版本
DEFINES += USE_SPDLOG
#DEFINES += SPDLOG_HEADER_ONLY
INCLUDEPATH += $$PWD/third_party/spdlog/include

# 如果使用预编译的spdlog库，取消注释以下行
# LIBS += -L$$PWD/third_party/spdlog/lib -lspdlog

# 源文件
SOURCES += $$PWD/src/main.cpp \
           $$PWD/src/core/CoreEngine.cpp \
           $$PWD/src/core/Logger.cpp \
           $$PWD/src/domain/BurnDevice.cpp \
           $$PWD/src/domain/Dut.cpp \
           $$PWD/src/domain/HandlerDevice.cpp \
           $$PWD/src/domain/ProcessDevice.cpp \
           $$PWD/src/domain/TestBoardDevice.cpp \
           $$PWD/src/infrastructure/BinLogger.cpp \
           $$PWD/src/infrastructure/TcpChannel.cpp \
           $$PWD/src/services/DeviceManager.cpp \
           $$PWD/src/services/DutManager.cpp \
           $$PWD/src/services/SiteWorkflowRouter.cpp \
           $$PWD/src/services/WorkflowManager.cpp \
           $$PWD/src/application/ConditionalStep.cpp \
           $$PWD/src/application/DelayStep.cpp \
           $$PWD/src/application/DeviceCommandStep.cpp \
           $$PWD/src/application/GlobalConfigStep.cpp \
           $$PWD/src/application/LoopStep.cpp \
           $$PWD/src/application/ParseJsonFileStep.cpp \
           $$PWD/src/application/PostProcessStep.cpp \
           $$PWD/src/application/ProcessBinFileStep.cpp \
           $$PWD/src/application/SubWorkflowStep.cpp \
           $$PWD/src/application/SwitchStep.cpp \
           $$PWD/src/application/WaitForSignalStep.cpp \
           $$PWD/src/application/WorkflowContext.cpp \
           $$PWD/src/application/WorkflowUtils.cpp \
           $$PWD/src/application/RunProcessStep.cpp \
           $$PWD/src/ui/BurnControlWidget.cpp \
           $$PWD/src/ui/DeviceManagerDialog.cpp \
           $$PWD/src/ui/DutMonitorWidget.cpp \
           $$PWD/src/ui/HandlerControlWidget.cpp \
           $$PWD/src/ui/LogConfigDialog.cpp \
           $$PWD/src/ui/LogDisplayWidget.cpp \
           $$PWD/src/ui/MainWindow.cpp \
           $$PWD/src/ui/RecipeConfigDialog.cpp \
           $$PWD/src/domain/AsyncProcessDevice.cpp \
           $$PWD/src/domain/protocols/SProtocol.cpp \
           $$PWD/src/domain/protocols/XTProtocol.cpp \
           $$PWD/src/domain/protocols/XTProtocolFramer.cpp \
           $$PWD/src/infrastructure/SerialChannel.cpp \
           $$PWD/src/infrastructure/SerialPortManager.cpp \
           $$PWD/src/infrastructure/SerialPortWorker.cpp \
           $$PWD/src/infrastructure/TcpServerChannel.cpp \
           $$PWD/src/ui/DeviceControlWidgetFactory.cpp \
           $$PWD/src/ui/TestBoardControlWidget.cpp \
           $$PWD/src/utils/CRC16.cpp \
           src/Ag06DoCustomProtocol.cpp \
           src/domain/JsonRpcClient.cpp \
           src/ui/Ag06TrimDialog.cpp

# 头文件
HEADERS += $$PWD/include/core/CoreEngine.h \
           $$PWD/include/core/Logger.h \
           $$PWD/include/domain/BurnDevice.h \
           $$PWD/include/domain/Dut.h \
           $$PWD/include/domain/HandlerDevice.h \
           $$PWD/include/domain/IDevice.h \
           $$PWD/include/domain/IProtocolFramer.h \
           $$PWD/include/domain/ProcessDevice.h \
           $$PWD/include/domain/TestBoardDevice.h \
           $$PWD/include/services/DeviceManager.h \
           $$PWD/include/services/DutManager.h \
           $$PWD/include/services/SiteWorkflowRouter.h \
           $$PWD/include/services/WorkflowManager.h \
           $$PWD/include/application/ConditionalStep.h \
           $$PWD/include/application/DelayStep.h \
           $$PWD/include/application/DeviceCommandStep.h \
           $$PWD/include/application/GlobalConfigStep.h \
           $$PWD/include/application/IWorkflowStep.h \
           $$PWD/include/application/LoopStep.h \
           $$PWD/include/application/ParseJsonFileStep.h \
           $$PWD/include/application/PostProcessStep.h \
           $$PWD/include/application/ProcessBinFileStep.h \
           $$PWD/include/application/SubWorkflowStep.h \
           $$PWD/include/application/SwitchStep.h \
           $$PWD/include/application/WaitForSignalStep.h \
           $$PWD/include/application/WorkflowContext.h \
           $$PWD/include/application/WorkflowUtils.h \
           $$PWD/include/application/RunProcessStep.h \
           $$PWD/include/ui/BurnControlWidget.h \
           $$PWD/include/ui/DeviceManagerDialog.h \
           $$PWD/include/ui/DutMonitorWidget.h \
           $$PWD/include/ui/LogConfigDialog.h \
           $$PWD/include/ui/LogDisplayWidget.h \
           $$PWD/include/ui/MainWindow.h \
           $$PWD/include/ui/RecipeConfigDialog.h \
           $$PWD/include/ui/IDeviceControlWidget.h \
           $$PWD/include/utils/ByteUtils.h \
           $$PWD/include/domain/AsyncProcessDevice.h \
           $$PWD/include/domain/IProtocolFramer.h \
           $$PWD/include/domain/protocols/ISProtocol.h \
           $$PWD/include/domain/protocols/IXTProtocol.h \
           $$PWD/include/domain/protocols/SProtocol.h \
           $$PWD/include/domain/protocols/XTProtocol.h \
           $$PWD/include/domain/protocols/XTProtocolFramer.h \
           $$PWD/include/infrastructure/BinLogger.h \
           $$PWD/include/infrastructure/ICommunicationChannel.h \
           $$PWD/include/infrastructure/SerialChannel.h \
           $$PWD/include/infrastructure/SerialPortManager.h \
           $$PWD/include/infrastructure/SerialPortWorker.h \
           $$PWD/include/infrastructure/TcpChannel.h \
           $$PWD/include/infrastructure/TcpServerChannel.h \
           $$PWD/include/ui/DeviceControlWidgetFactory.h \
           $$PWD/include/ui/HandlerControlWidget.h \
           $$PWD/include/ui/IDeviceControlWidget.h \
           $$PWD/include/ui/TestBoardControlWidget.h \
           $$PWD/include/utils/CRC16.h \
           include/Ag06DoCustomProtocol.h \
           include/Ag06TrimDialog.h \
           include/domain/JsonRpcClient.h \
           include/xt_trim_param.h

# UI文件
FORMS += $$PWD/src/ui/TestBoardControlWidget.ui

# 资源文件
RESOURCES += $$PWD/resources.qrc

# 其他文件（用于在IDE中显示）
OTHER_FILES += $$PWD/README.md \
               $$PWD/.gitignore \
               $$PWD/CMakeLists.txt \
               $$PWD/cmake/ThirdParty.cmake

# 输出目录配置
DESTDIR = $$PWD/bin
OBJECTS_DIR = $$PWD/build/obj
MOC_DIR = $$PWD/build/moc
RCC_DIR = $$PWD/build/rcc
UI_DIR = $$PWD/build/ui

# 配置文件复制（构建后处理）
# 注意：qmake没有直接等价于CMake的file(COPY)命令
# 您可能需要手动复制config目录到bin目录，或使用构建脚本

# 如果需要在构建时自动复制配置文件，可以添加以下自定义目标：
# copyconfig.commands = $(COPY_DIR) $$shell_path($$PWD/config) $$shell_path($$DESTDIR/config)
# copyconfig.depends = FORCE
# QMAKE_EXTRA_TARGETS += copyconfig
