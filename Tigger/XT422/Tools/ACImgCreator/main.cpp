
#include <QtCore/QCoreApplication>
#include <Windows.h>
#include "CACImg.h"
#include "CLILog.h"

#define VERSION ("V1.2.3")
#define DATE    ("20240918")

void PrintBanner(ILog*pILog)
{
	pILog->PrintLog(LOGLEVEL_N, "============Createor Information============\r\n");
	pILog->PrintLog(LOGLEVEL_N, "Version   : %s\r\n", VERSION);
	pILog->PrintLog(LOGLEVEL_N, "Date      : %s\r\n", DATE);
	pILog->PrintLog(LOGLEVEL_N, "Copyright : Acroview Technology\r\n");
	pILog->PrintLog(LOGLEVEL_N, "============================================\r\n");
}

void PrintUsage(ILog* pILog)
{
	pILog->PrintLog(LOGLEVEL_N, "======Usage=====\r\n");
	pILog->PrintLog(LOGLEVEL_N, "ACImgCreator PartitionConfig.json Version\r\n");
	pILog->PrintLog(LOGLEVEL_N, "    PartitionConfig.json : the file absolute path of the partition config file, json format\r\n");
	pILog->PrintLog(LOGLEVEL_N, "    Version              : the version of the created file\r\n");
}

int main(int argc, char *argv[])
{
	SetConsoleOutputCP(CP_UTF8);
	int Ret = 0;
	CACImg ACImg;
	QCoreApplication a(argc, argv);
	QString	strSPIPara;	//-bin 表示生成MP的img，-efwm表示生成UP的img
	QString strJsonFile;
	QString strVersion;
	CLILog Log;
	Log.Init();
	
	if (argc != 4) {
		printf("Error: Parameter Number Failed\r\n");
		PrintUsage(&Log);
		goto __end;
	}

	strSPIPara.sprintf("%s", argv[1]);
	strJsonFile.sprintf("%s", argv[2]);
	strVersion.sprintf("%s", argv[3]);

	PrintBanner(&Log);
	ACImg.AttachILog(&Log);
	Ret=ACImg.CreateACImg(strJsonFile, strVersion, strSPIPara);
	
	//Ret = a.exec();

__end:
	return Ret;
}

/*
#include <QTcpServer>  
#include <QTcpSocket>  
#include <QCoreApplication>  
#include <QDataStream>  
#include <QDebug>  

class TcpServer : public QObject {
    Q_OBJECT
public:
    TcpServer(quint16 port, QObject* parent = nullptr) : QObject(parent), serverPort(port) {
        connect(&server, &QTcpServer::newConnection, this, &TcpServer::onNewConnection);
        connect(&clientSocket, &QTcpSocket::connected, this, &TcpServer::onConnected);
        connect(&clientSocket, &QTcpSocket::readyRead, this, &TcpServer::onReadyRead);
    }

    void startServer() {
        if (server.listen(QHostAddress::Any, serverPort)) {
            qDebug() << "Server started on port" << serverPort;
        }
        else {
            qDebug() << "Server failed to start";
        }
    }

    void connectToServer(const QString& host, quint16 port) {
        clientSocket.connectToHost(host, port);
    }

private slots:
    void onNewConnection() {
        QTcpSocket* incomingSocket = server.nextPendingConnection();
        connect(incomingSocket, &QTcpSocket::readyRead, [incomingSocket, this]() {
            QDataStream in(incomingSocket);
            in.setVersion(QDataStream::Qt_5_15);

            while (incomingSocket->bytesAvailable() > 0) {
                if (messageSize == 0) {
                    if (incomingSocket->bytesAvailable() < sizeof(quint32))
                        return;
                    in >> messageSize;
                }

                if (incomingSocket->bytesAvailable() < messageSize)
                    return;

                QByteArray data;
                in >> data;
                qDebug() << "Received:" << data;
                messageSize = 0;

                // Optionally send a response  
                QByteArray response = "Response from server";
                QDataStream out(incomingSocket);
                out.setVersion(QDataStream::Qt_5_15);
                out << quint32(response.size()) << response;
                incomingSocket->disconnectFromHost();
            }
            });
    }

    void onConnected() {
        qDebug() << "Connected to server";
        QByteArray message = "Hello from client";
        QDataStream out(&clientSocket);
        out.setVersion(QDataStream::Qt_5_15);
        out << quint32(message.size()) << message;
    }

    void onReadyRead() {
        QDataStream in(&clientSocket);
        in.setVersion(QDataStream::Qt_5_15);

        while (clientSocket.bytesAvailable() > 0) {
            if (messageSize == 0) {
                if (clientSocket.bytesAvailable() < sizeof(quint32))
                    return;
                in >> messageSize;
            }

            if (clientSocket.bytesAvailable() < messageSize)
                return;

            QByteArray data;
            in >> data;
            qDebug() << "Received from server:" << data;
            messageSize = 0;
            clientSocket.disconnectFromHost();
        }
    }

private:
    QTcpServer server;
    QTcpSocket clientSocket;
    quint16 serverPort;
    quint32 messageSize = 0;
};

int main(int argc, char* argv[]) {
    QCoreApplication a(argc, argv);

    TcpServer serverA(1234);
    serverA.startServer();

    TcpServer serverB(5678);
    serverB.startServer();

    // Example of serverA connecting to serverB  
    serverA.connectToServer("127.0.0.1", 5678);

    return a.exec();
}

#include "main.moc"

*/