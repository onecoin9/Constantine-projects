//#include <QCoreApplication>
//#include <QDebug>
//#include <QDateTime>
//#include <QFileInfo>
//#include <QtWidgets/QWidget>
//#include "spdlog/spdlog.h"
//#include "spdlog/sinks/rotating_file_sink.h"
//#include "spdlog/pattern_formatter.h"
//
//QWidget* g_logViewerInstance; /// 我这里是一个编辑框,根据自己的实际情况进行更改。 我是全局变量
//std::shared_ptr<spdlog::logger> file_logger; /// 这里是文件输出，我也是一个全局变量
//
///// 下面代码主要用于在一些特殊日志进行输出到文件内，或者界面上的。
//#define G_Debug QMessageLogger(__FILE__, __LINE__, Q_FUNC_INFO, "SPECIAL").debug()
//#define G_Info QMessageLogger(__FILE__, __LINE__, Q_FUNC_INFO, "SPECIAL").info()
//#define G_Warning QMessageLogger(__FILE__, __LINE__, Q_FUNC_INFO, "SPECIAL").warning()
//#define G_Critical QMessageLogger(__FILE__, __LINE__, Q_FUNC_INFO, "SPECIAL").critical()
//
//#define G_Debug_N QMessageLogger(__FILE__, __LINE__, Q_FUNC_INFO, "SPECIAL").debug().noquote()
//#define G_Info_N QMessageLogger(__FILE__, __LINE__, Q_FUNC_INFO, "SPECIAL").info().noquote()
//#define G_Warning_N QMessageLogger(__FILE__, __LINE__, Q_FUNC_INFO, "SPECIAL").warning().noquote()
//#define G_Critical_N QMessageLogger(__FILE__, __LINE__, Q_FUNC_INFO, "SPECIAL").critical().noquote()
//
///// 下面代码主要是用于开发环境以及生产环境去除 日志
//#ifdef QT_DEBUG
//#define D_Warning_N qWarning().noquote()
//#define D_Debug_N qDebug().noquote()
//#define D_Info_N qInfo().noquote()
//#define D_Critical_N  qCritical().noquote()
//
//#else
//#define D_Warning_N QMessageLogger().noDebug()
//#define D_Debug_N QMessageLogger().noDebug()
//#define D_Info_N QMessageLogger().noDebug()
//#define D_Critical_N  QMessageLogger().noDebug()
//#endif
//
//void customMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& str) {
//    // ANSI color codes
//    const char* colorReset = "\033[0m";
//    const char* colorType;
//
//    QString typeStr;
//    switch (type) {
//    case QtDebugMsg:
//        typeStr = "Debug";
//        colorType = "\033[0;34m"; // Blue
//        break;
//    case QtWarningMsg:
//        typeStr = "Warning";
//        colorType = "\033[0;33m"; // Yellow
//        break;
//    case QtCriticalMsg:
//        typeStr = "Critical";
//        colorType = "\033[0;31m"; // Red
//        break;
//    case QtFatalMsg:
//        typeStr = "Fatal";
//        colorType = "\033[0;31m"; // Red
//        break;
//    case QtInfoMsg:
//        typeStr = "Info";
//        colorType = "\033[0;32m"; // Green
//        break;
//    }
//
//#ifdef QT_DEBUG
//    QString templateString = "%1 %2 [%3:%4] %5 %6";
//#else
//    QString templateString = "%1 %2 [%3:%4] %5 %6";
//#endif
//
//    auto file = QFileInfo(context.file).fileName();
//    auto time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
//    QString msg = templateString.arg(
//        time,
//        typeStr,
//        file,
//        QString::number(context.line),
//        context.function,
//        str
//    );
//
//
//    // 判断日志是否包含特殊标识
//    bool isSpecial = context.category && QString(context.category) == "SPECIAL";
//    if (isSpecial) {
//        if (type > QtMsgType::QtDebugMsg and g_logViewerInstance) {
//            auto gui_msg = QString("%1 %2").arg(time, str);
//            // 使用 GUI 线程更新日志窗口
//            QMetaObject::invokeMethod(g_logViewerInstance, "appendMessage", Qt::QueuedConnection,
//                Q_ARG(QString, gui_msg));
//        }
//        // 使用 spdlog 将日志消息记录到文件
//        file_logger->log(spdlog::level::from_str(typeStr.toStdString()), "{}", msg.toStdString());
//    }
//    // 添加颜色前缀
//    msg.prepend(colorType);
//    // 添加颜色后缀
//    msg.append(colorReset);
//    QTextStream(stdout) << msg << endl;
//}
//
///// 初始化spdlog日志
//void initLog() {
//    // 获取当前日期，并将其格式化为字符串
//    QString currentDate = QDateTime::currentDateTime().toString("yyyy-MM-dd");
//
//    // 创建日志文件名，将当前日期添加到文件名中
//    std::string logFileName = "logs/logfile-" + currentDate.toStdString() + ".log";
//    /// 初始化 spdlog 日志接收器
//    size_t max_file_size = 10 * 1024 * 1024; // 10MB
//    size_t max_files = 10;
//    // 创建文件日志记录器
//    file_logger = spdlog::rotating_logger_mt("file_logger", logFileName, max_file_size, max_files);
//    file_logger->set_level(spdlog::level::info);
//    file_logger->set_formatter(std::make_unique<spdlog::pattern_formatter>("%v"));
//}
//
//int main(int argc, char* argv[]) {
//    QCoreApplication a(argc, argv);
//    initLog();
//    ///> 日志初始化
//    qInstallMessageHandler(customMessageHandler);
//    qSetMessagePattern("[%{type}] %{function} %{file}:%{line}   %{message}");
//    D_Debug_N << "测试1";
//    D_Info_N << "测试1";
//    D_Warning_N << "测试1";
//    D_Critical_N << "测试1";
//
//    G_Debug_N << "测试1";
//    G_Info_N << "测试1";
//    G_Warning_N << "测试1";
//    G_Critical_N << "测试1";
//    exit(1);
//    return QCoreApplication::exec();
//}
