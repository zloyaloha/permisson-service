#include "main_window.h"

MainWindow::MainWindow(std::shared_ptr<CommandHandler> commandor, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), _commandHandler(commandor)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow() {}

void MainWindow::SetupWindow(const QString& username, bool role) {
    ui->usernameLine->setText(username);
    if (role) {
        ui->roleLine->setText("Администратор");
    } else {
        ui->roleLine->setText("Шестерка");
    }
    this->show();
}