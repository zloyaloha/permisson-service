#pragma once
#include "ui_app_win.h"
#include "command_handler.h"
#include <QApplication>
#include <QWidget>
#include <QMainWindow>
#include <QDebug>
#include <QRegExp>
#include <QRegularExpression>
#include <QPixmap>

class MainWindow : public QMainWindow {
    Q_OBJECT
    public:
        MainWindow(std::shared_ptr<CommandHandler> commandor, QWidget *pwgt = nullptr);
        ~MainWindow();
        void SetupWindow(const QString& username, const QString& token);
    private slots:
        void onRoleMessageReceived(const QString& role);
    private:
        std::shared_ptr<CommandHandler> _commandHandler;
        Ui::MainWindow *ui;
        std::string _username;
        std::string _token;    
};