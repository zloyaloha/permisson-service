#include "main_window.h"

MainWindow::MainWindow(std::shared_ptr<CommandHandler> commandor, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), _commandHandler(commandor)
{
    ui->setupUi(this);
    connect(_commandHandler.get(), &CommandHandler::GetRoleMessageReceived, this, &MainWindow::onRoleMessageReceived);
}

MainWindow::~MainWindow() {}

void MainWindow::SetupWindow(const QString& username, const QString& token) {
    _commandHandler->StartAsyncReading();
    _token = token.toStdString();
    _username = username.toStdString();
    ui->usernameLine->setText(username);
    _commandHandler->SendCommand(Operation::GetRole, {_username, _token});
    this->show();
}

void MainWindow::onRoleMessageReceived(const QString& message) {
    std::cout << "triggered" << std::endl;
    ui->roleLine->setText(message);
}