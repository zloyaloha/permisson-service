#pragma once
#include <QApplication>
#include <QWidget>
#include <QMainWindow>
#include <QDebug>
#include <iostream>
#include "ui_hello_w.h"
#include "registration_w.h"
#include "command_handler.h"

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
    std::shared_ptr<CommandHandler> _handler;
};
