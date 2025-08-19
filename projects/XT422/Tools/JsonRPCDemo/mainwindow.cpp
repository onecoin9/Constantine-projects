#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QtEndian>
#include <QUuid>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <cstring>
#include <QTcpServer>
#include <QProcess>
#include <QTimer>
#include <QSettings>

#include <QDebug>

// Please ensure that HEADER_LENGTH, MAGIC_NUMBER, HEADER_VERSION, etc. are defined appropriately

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_socket(new QTcpSocket(this))
    , m_server(nullptr)
{
    ui->setupUi(this);

    // Load the external executable's default path from the INI file.
    QSettings settings("config.ini", QSettings::IniFormat);
    // The key "externalExe" stores the path; change the default as needed.
    QString serverPath = settings.value("externalExe", "D:\\svn\\AG06\\AIPE\\Build\\Aprog.exe").toString();
    
    // Assuming your UI has a widget called "EditServerPath"
    ui->EditServerPath->setText(serverPath);

    // When the client receives data, call the slot onSocketReadyRead()
    connect(m_socket, &QTcpSocket::readyRead, this, &MainWindow::onSocketReadyRead);

    connect(ui->BtnStartServer, &QPushButton::clicked, this, &MainWindow::on_btnStartServer_clicked);
    connect(ui->BtnDisConnect, &QPushButton::clicked, this, &MainWindow::on_btnDisConnect_clicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::sendCommand(const QString &cmd, const QJsonObject &data)
{
    // Construct JSON-RPC request data
    QJsonObject request;
    request["jsonrpc"] = "2.0";
    request["method"] = cmd;
    request["params"] = data;
    request["id"] = QUuid::createUuid().toString();

    QJsonDocument doc(request);
    QByteArray payload = doc.toJson(QJsonDocument::Compact);

    // Construct message header, a total of 32 bytes:
    // 4 bytes MagicNumber (big endian), 2 bytes version (big endian),
    // 4 bytes payload length (big endian), and the remaining 22 bytes reserved (filled with zero)
    QByteArray header;
    header.resize(HEADER_LENGTH);
    header.fill(0);

    // Write MagicNumber
    quint32 magic = qToBigEndian(MAGIC_NUMBER);
    memcpy(header.data(), &magic, 4);

    // Write version
    quint16 version = qToBigEndian(HEADER_VERSION);
    memcpy(header.data() + 4, &version, 2);

    // Write payload length
    quint32 length = qToBigEndian(static_cast<quint32>(payload.size()));
    memcpy(header.data() + 6, &length, 4);

    // Concatenate header and payload
    QByteArray packet = header + payload;

    // Send data; flush ensures the data is sent immediately
    m_socket->write(packet);
    m_socket->flush();
}

void MainWindow::on_btnDoCmd_clicked()
{
    QJsonObject params;
    params["param1"] = "value1";
    sendCommand("ClientDoCmd", params);
}

void MainWindow::on_btnGetSpecialBit_clicked()
{
    QJsonObject params;
    sendCommand("GetSpecialBit", params);
}

void MainWindow::on_btnNewOlderIDBurn_clicked()
{
    QJsonObject params;
    params["info"] = "Sample info";
    sendCommand("NewOlderIDBurn", params);
}

void MainWindow::on_btnStartServer_clicked()
{
    // Retrieve the external executable path from the UI (EditServerPath)
    QString externalExe = ui->EditServerPath->toPlainText();

    // 使用 QProcess 启动外部程序
    QProcess *process = new QProcess(this);
    process->start(externalExe);

    // 等待最多 3000 毫秒确保外部程序已成功启动
    if (!process->waitForStarted(3000)) {
        QMessageBox::critical(this, tr("External Application Launch Failed"),
                              tr("Failed to launch external application: %1").arg(process->errorString()));
        return;
    }

    // 禁用启动按钮，避免多次点击
    ui->BtnStartServer->setEnabled(false);

    // 使用 QTimer::singleShot 等待一段时间（例如 1000 毫秒）让外部程序初始化其 TCP 服务，
    // 然后通过 TCP 客户端连接到外部程序
    QTimer::singleShot(1000, this, [this]() {
        m_socket->connectToHost("127.0.0.1", 12345);
        if (!m_socket->waitForConnected(3000)) {
            QMessageBox::critical(this, tr("Connection Failed"),
                                  tr("Unable to connect to the external application's TCP server."));
        } else {
            QMessageBox::information(this, tr("Connected"),
                                     tr("Connected to external application successfully."));
        }
    });
}

// New: Slot for handling BtnDisConnect click event. Sends an exit command to the external application.
void MainWindow::on_btnDisConnect_clicked()
{
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        QJsonObject params; // Optionally include additional parameters
        sendCommand("ExitApp", params); // Command message to instruct the external executable to exit

        m_socket->disconnectFromHost();
        if (m_socket->state() != QAbstractSocket::UnconnectedState) {
            m_socket->waitForDisconnected(3000);
        }

        QMessageBox::information(this, tr("Disconnected"),
                                 tr("Sent exit command and disconnected from the external application."));
        // Re-enable the start button to allow restarting if needed.
        ui->BtnStartServer->setEnabled(true);
    } else {
        QMessageBox::information(this, tr("Not Connected"),
                                 tr("The socket is currently not connected."));
    }
}

void MainWindow::onNewConnection()
{
    // Process all pending new connections
    while (m_server->hasPendingConnections()) {
        QTcpSocket *clientSocket = m_server->nextPendingConnection();
        // Bind each new connection's readyRead signal to process the data received from the client
        connect(clientSocket, &QTcpSocket::readyRead, this, [=]() {
            QByteArray data = clientSocket->readAll();
            if (data.size() <= HEADER_LENGTH) {
                return; // Not enough data for a header, ignore the packet
            }
            QByteArray payload = data.mid(HEADER_LENGTH);
            QJsonDocument doc = QJsonDocument::fromJson(payload);
            if (doc.isObject()) {
                QJsonObject request = doc.object();
                qDebug() << "Server received request:" << request;
                // Based on request["method"], you can implement additional logic
            }
        });
    }
}

void MainWindow::onSocketReadyRead()
{
    QByteArray data = m_socket->readAll();
    if (data.size() <= HEADER_LENGTH) {
        return;  // Data length is insufficient for a header, ignore the packet
    }
    QByteArray payload = data.mid(HEADER_LENGTH);
    QJsonDocument doc = QJsonDocument::fromJson(payload);
    if (doc.isObject()) {
        QJsonObject respObj = doc.object();
        qDebug() << "Client received response:" << respObj;
    }
}
