#ifndef LOGIN_H
#define LOGIN_H
#pragma once
#include <QDialog>
#include "ui_login.h"

class Login : public QDialog
{
    Q_OBJECT

public:
    Login(QWidget* parent = nullptr);
    ~Login();

private slots:
    void onLoginButtonClicked();
    void onCancelButtonClicked();
    void onPasswordVisibilityToggled(bool checked);
    bool validateCredentials(const QString& username, const QString& password);

private:
    void showErrorMessage(const QString& message);
    void setupConnections();

private:
    Ui::LoginDialog* ui;
    bool isPasswordVisible;
};
#endif