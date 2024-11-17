#pragma once
#include "ui_app_win.h"
#include "command_handler.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
    public:
        MainWindow(std::shared_ptr<CommandHandler> commandor, QWidget *pwgt = nullptr);
        ~MainWindow();
        void SetupWindow(const QString& username, bool role);
    public slots:

private:
    std::shared_ptr<CommandHandler> _commandHandler;
    Ui::MainWindow *ui;
};