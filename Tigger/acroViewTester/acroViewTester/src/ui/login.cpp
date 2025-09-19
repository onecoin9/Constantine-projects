#include "login.h"
#include <QMessageBox>
#include <QSettings>

Login::Login(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
    , isPasswordVisible(false)
{
    ui->setupUi(this);
    setWindowTitle(tr("用户登录"));

    // 设置样式表
    QString styleSheet = R"(
        QLineEdit {
            border: 1px solid #A0A0A0;
            border-radius: 3px;
            padding: 2px 5px;
            background: white;
            selection-background-color: #0078D4;
            font-size: 14px;
            min-height: 25px;
        }
        
        QLineEdit:focus {
            border: 2px solid #0078D4;
        }

        QPushButton#pushButtonShowPassword {
            border: none;
            background: transparent;
            padding: 0px;
            qproperty-iconSize: QSize(20, 20);
        }

        QPushButton#pushButtonShowPassword:hover {
            background-color: #E5E5E5;
            border-radius: 3px;
        }

        QPushButton#pushButtonLogin, QPushButton#pushButtonCancel {
            background-color: #0078D4;
            color: white;
            border: none;
            border-radius: 3px;
            padding: 5px 15px;
            min-width: 80px;
            min-height: 30px;
            font-size: 14px;
        }

        QPushButton#pushButtonLogin:hover, QPushButton#pushButtonCancel:hover {
            background-color: #006CBD;
        }

        QPushButton#pushButtonLogin:pressed, QPushButton#pushButtonCancel:pressed {
            background-color: #005BA1;
        }

        QLabel {
            font-size: 14px;
            color: #333333;
        }
    )";

    setStyleSheet(styleSheet);

    // 初始化为显示密码状态
    ui->lineEditPassword->setEchoMode(QLineEdit::Normal);
    ui->pushButtonShowPassword->setIcon(QIcon(":/acroViewTester/qrc/pics/eyeOpen.png"));
    ui->pushButtonShowPassword->setChecked(true);

    ui->lineEditPassword->setEchoMode(QLineEdit::Password);

    setupConnections();
}

Login::~Login()
{
    delete ui;
}

void Login::setupConnections()
{
    connect(ui->pushButtonLogin, &QPushButton::clicked,
        this, &Login::onLoginButtonClicked);

    connect(ui->pushButtonCancel, &QPushButton::clicked,
        this, &Login::onCancelButtonClicked);

    connect(ui->pushButtonShowPassword, &QPushButton::toggled,
        this, &Login::onPasswordVisibilityToggled);
}

void Login::onLoginButtonClicked()
{
    QString username = ui->lineEditUsername->text().trimmed();
    QString password = ui->lineEditPassword->text();

    if (username.isEmpty() || password.isEmpty()) {
        showErrorMessage(tr("用户名或密码不能为空！"));
        return;
    }

    if (validateCredentials(username, password)) {
        accept(); // 关闭对话框并返回QDialog::Accepted
    }
    else {
        showErrorMessage(tr("用户名或密码错误！"));
        ui->lineEditPassword->clear();
        ui->lineEditPassword->setFocus();
    }
}

void Login::onCancelButtonClicked()
{
    reject(); 
}

void Login::onPasswordVisibilityToggled(bool checked)
{
    if (checked) {
        ui->lineEditPassword->setEchoMode(QLineEdit::Normal);
        ui->pushButtonShowPassword->setIcon(QIcon(":/acroViewTester/qrc/pics/eyeOpen.png"));
    }
    else {
        ui->lineEditPassword->setEchoMode(QLineEdit::Password);
        ui->pushButtonShowPassword->setIcon(QIcon(":/acroViewTester/qrc/pics/eyeClose.png"));
    }
}

bool Login::validateCredentials(const QString& username, const QString& password)
{
    QSettings settings("AcroView", "AcroViewTester");

    // 验证管理员账户
    if (username.toLower() == "admin") {
        QString adminPassword = settings.value("654321", "admin").toString();
        return password == adminPassword;
    }

    // 验证操作员账户
    if (username.toLower() == "operator") {
        QString operatorPassword = settings.value("000000", "operator").toString();
        return password == operatorPassword;
    }

    return false;
}

void Login::showErrorMessage(const QString& message)
{
    QMessageBox::warning(this, tr("错误"), message);
}