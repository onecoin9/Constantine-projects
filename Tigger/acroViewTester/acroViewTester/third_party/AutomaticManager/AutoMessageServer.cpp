#include "AutoMessageServer.h"
#include "AuthUtils.h"
//#include "AngkLogger.h"
#include "ExternalServer.h"

#include <QCoreApplication>
#include <QMessageBox>
#include <QDebug>
#include <QThread>

AutoMessageServer::AutoMessageServer(QObject* parent)
    : QObject(parent)
    , m_externalServer(nullptr)
{

}

AutoMessageServer::~AutoMessageServer()
{
    // 停止 ExternalServer
    if (m_externalServer) {
        m_externalServer->stopServer();
        m_externalServer.reset();
    }
}

void AutoMessageServer::startServer(const QString& serverIP, quint16 licPort)
{
    m_serverIP = serverIP;
    m_licPort = licPort;

    // 异步启动外部服务器
    startExternalServerAsync();
}


void AutoMessageServer::stopServer(int stopWhenExit)
{
    if (stopWhenExit == 1) {
        AuthUtils::Instance().StopExternalProcess();
    }
}

void AutoMessageServer::startExternalServerAsync()
{
    // 使用 QThread::create 创建新线程
    QThread* thread = QThread::create([this]() {
        bool processExists = AuthUtils::Instance().IsProcessRunning();
        bool processStarted = false;

        if (!processExists) {
            processStarted = AuthUtils::Instance().StartExternalProcess();
        }
        // 等待1秒让进程完全启动
        std::this_thread::sleep_for(std::chrono::seconds(1));
        // 再次检查进程是否运行
        processExists = AuthUtils::Instance().IsProcessRunning();

        // 如果外部进程存在或启动成功，则启动外部服务器端口
        bool success = false;
        if (processExists) {
            //ALOG_INFO("External LicServer process is running.", "CU", "--");
            // 使用 QMetaObject::invokeMethod 在主线程中安全地创建和设置 ExternalServer
            QMetaObject::invokeMethod(this, [this, &success]() {
                if (!m_externalServer) {
                    m_externalServer = std::make_unique<ExternalServer>();
                    connect(m_externalServer.get(), &ExternalServer::reportRequested,
                        this, [this](QTcpSocket* socket) {
                            emit reportRequested(socket);
                            //ALOG_INFO("GetReport requested from external server.", "CU", "--");
                        }, Qt::QueuedConnection);

                    success = m_externalServer->startServer(m_serverIP, m_licPort);
                    if (!success) {
                        //ALOG_ERROR("Failed to start external server on port %d.", "CU", "--", m_licPort);
                        m_externalServer.reset();
                    }
                }
            }, Qt::BlockingQueuedConnection);
        }
        else {
            //ALOG_ERROR("External LicServer process is not running.", "CU", "--");
        }

        // 在主线程中启动主服务器
        QMetaObject::invokeMethod(this, [this, success, processExists]() {
            if (!success) {
                if (!processExists) {
                    QString msg = QString("External auth process is not running. "
                               "Please check if the lic file is installed correctly.");
                    //ALOG_ERROR(msg.toStdString().c_str(), "CU", "--");
                    //QMessageBox::critical(nullptr, "Error", msg);
                } else {
                    QString msg = QString("Failed to start on port %1. "
                        "Please check if the port is already in use.").arg(m_licPort);
                    //ALOG_ERROR(msg.toStdString().c_str(), "CU", "--");
                    //QMessageBox::critical(nullptr, "Error", msg);
                }
            }else {
                //ALOG_INFO("Start listen on port %d.", "CU", "--", m_licPort);
            }
        }, Qt::QueuedConnection);
    });

    // 设置线程完成后自动删除
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    
    // 启动线程
    thread->start();
}
