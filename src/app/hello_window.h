#pragma once
#include <QApplication>
#include <QWidget>
#include <QMainWindow>
#include <QMessageBox>
#include <QDebug>
#include <QRegExp>
#include <QRegularExpression>
#include <QPixmap>
#include <iostream>
#include "ui_hello_w.h"
#include "command_handler.h"
#include "main_window.h"

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
        ~HelloWindow();
        bool SendCommand(const Operation op, const std::initializer_list<std::string>& data);
        BaseCommand ReadResponse();
    public slots:
        void ToRegistrationButtonClicked();
        void RegistrationButtonClicked();
        void LoginButtonClicked();
        void BackToLoginWindowButtonClicked();
private:
    Ui::HelloWindow *ui;
    std::shared_ptr<MainWindow> _mainWindow;
    std::shared_ptr<CommandHandler> _commandHandler;
    std::shared_ptr<StringHandler> _stringHandler;
    std::string _token;
    std::string _username;
};

class StringHandler {
    public:
        StringHandler() = default;
        LoginAndPasswordValid IsValidLogin(const QString& password);
        LoginAndPasswordValid IsValidPassword(const QString& password);
        bool ValidInput(const QString& login, const QString& password);
};
