#pragma once
#include <QApplication>
#include <QWidget>
#include <QMainWindow>
#include <QDebug>
#include <QRegExp>
#include <QRegularExpression>
#include <iostream>
#include "ui_hello_w.h"
#include "registration_w.h"
#include "command_handler.h"
// #include <openssl/evp.h>
// #include <openssl/sha.h>

namespace {
    const int MIN_LOGIN_SIZE = 3;
    const int MAX_LOGIN_SIZE = 20;
    const int MIN_PASSWORD_SIZE = 4;
    enum LoginAndPasswordValid {
        InvalidLen = -2,
        InvalidSymbol = -1,
        Valid = 1
    };
}

class StringHandler;

class HelloWindow : public QMainWindow {
    Q_OBJECT
    public:
        HelloWindow(QWidget *pwgt = nullptr);
        ~HelloWindow() {}
    public slots:
        void RegistrationButtonClicked();
        void LoginButtonClicked();
private:
    Ui::HelloWindow *ui;
    std::shared_ptr<CommandHandler> _commandHandler;
    std::shared_ptr<StringHandler> _stringHandler;
};

class StringHandler {
    public:
        StringHandler() = default;
        LoginAndPasswordValid IsValidLogin(const QString& password);
        LoginAndPasswordValid IsValidPassword(const QString& password);
        std::string HashPassword(const QString& password);
};
