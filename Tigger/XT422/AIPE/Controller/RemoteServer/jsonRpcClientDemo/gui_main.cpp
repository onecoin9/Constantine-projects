#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include <QMessageBox>
#include <QLoggingCategory>
#include <QDebug>
#include "JsonRpcTestWidget.h"

#ifdef _WIN32
#include <Windows.h>
#include <io.h>
#include <fcntl.h>
#include <iostream>
#endif

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
#ifdef _WIN32
    // 为Windows GUI应用程序分配控制台窗口
    if (AllocConsole()) {
        // 重定向标准输出到控制台
        freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
        freopen_s((FILE**)stderr, "CONOUT$", "w", stderr);
        freopen_s((FILE**)stdin, "CONIN$", "r", stdin);
        
        // 设置控制台窗口标题
        SetConsoleTitleA("JSON-RPC Client Debug Console");
        
        // 确保cout、wcout、cin、wcin、wcerr、cerr、wclog和clog正常工作
        std::ios::sync_with_stdio(true);
        std::wcout.clear();
        std::cout.clear();
        std::wcerr.clear();
        std::cerr.clear();
        std::wcin.clear();
        std::cin.clear();
        
        qDebug() << "=== JSON-RPC 客户端调试控制台已启用 ===";
    }
#endif
    
    // 设置应用程序信息
    app.setApplicationName("JSON-RPC客户端测试工具");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Your Company");
    app.setOrganizationDomain("yourcompany.com");
    
    // 设置日志过滤规则：只显示我们的应用程序日志，过滤Qt内部调试信息
    QString logRules;
    
    // 检查是否启用详细日志模式（通过环境变量JSONRPC_VERBOSE_LOG=1控制）
    bool verboseMode = qgetenv("JSONRPC_VERBOSE_LOG") == "1";
    
    if (verboseMode) {
        // 详细模式：显示更多调试信息
        logRules = "*.debug=true\n*.info=true\n*.warning=true\n*.critical=true";
        qDebug() << "=== 详细日志模式已启用 ===";
    } else {
        // 正常模式：只显示应用程序相关日志
        logRules = 
            "*.debug=false\n"                 // 默认关闭所有debug日志
            "*.info=true\n"                   // 保留info级别日志  
            "*.warning=true\n"                // 保留warning日志
            "*.critical=true\n"               // 保留critical日志
            "qt.widgets.gestures=false\n"     // 关闭手势识别日志
            "qt.qpa.windows=false\n"          // 关闭Windows平台相关日志
            "qt.qpa.events=false\n"           // 关闭事件处理日志
            "qt.text.drawing=false\n"         // 关闭文本绘制日志
            "qt.widgets.*=false\n"            // 关闭所有widgets相关日志
            "qt.qpa.*=false\n"                // 关闭所有平台抽象层日志
            "qt.network.*=false\n"            // 关闭网络相关日志
            "default.debug=true";             // 保留我们应用程序的debug日志
    }
    
    QLoggingCategory::setFilterRules(logRules);
    
    // 设置应用程序样式
    app.setStyle(QStyleFactory::create("Fusion"));
    
    // 设置暗色主题（可选）
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    
    // 注释掉暗色主题，使用默认主题更友好
    // app.setPalette(darkPalette);
    
    try {
        // 创建并显示主窗口
        JsonRpcTestWidget mainWidget;
        mainWidget.show();
        
        // 显示欢迎信息
        QMessageBox::information(&mainWidget, "欢迎", 
                                "欢迎使用 JSON-RPC 客户端测试工具！\n\n"
                                "使用说明：\n"
                                "1. 首先在左侧输入服务器地址和端口\n"
                                "2. 点击「连接」按钮连接到服务器\n"
                                "3. 连接成功后，可以点击各种RPC方法按钮进行测试\n"
                                "4. 查看右侧的响应结果、通知消息和详细日志\n\n"
                                "提示：确保服务器已启动并在指定端口监听！\n\n"
#ifdef _WIN32
                                "注意：已为您打开调试控制台窗口，可查看详细的请求日志！"
#endif
                                );
        
        int result = app.exec();
        
#ifdef _WIN32
        // 程序结束时释放控制台
        FreeConsole();
#endif
        
        return result;
        
    } catch (const std::exception& e) {
        QMessageBox::critical(nullptr, "严重错误", 
                            QString("应用程序启动失败：%1").arg(e.what()));
        return -1;
    } catch (...) {
        QMessageBox::critical(nullptr, "严重错误", 
                            "应用程序启动时发生未知错误！");
        return -1;
    }
} 