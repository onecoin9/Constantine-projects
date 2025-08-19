#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QTcpServer>
#include <QJsonDocument>
#include <QJsonObject>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btnDoCmd_clicked();
    void on_btnGetSpecialBit_clicked();
    void on_btnNewOlderIDBurn_clicked();
    void on_btnStartServer_clicked();
    void onNewConnection();
    void onSocketReadyRead();
    void on_btnDisConnect_clicked();
private:
    void sendCommand(const QString &cmd, const QJsonObject &data);

private:
    Ui::MainWindow *ui;
    QTcpSocket *m_socket;
    QTcpServer *m_server;

    static constexpr quint32 MAGIC_NUMBER = 0x4150524F; // "APRO"
    static constexpr quint16 HEADER_VERSION = 1;
    static const int HEADER_LENGTH = 32;
};

#endif // MAINWINDOW_H