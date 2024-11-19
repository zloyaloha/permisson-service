#include "main_window.h"

MainWindow::MainWindow(std::shared_ptr<CommandHandler> commandor, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), _commandHandler(commandor)
{
    ui->setupUi(this);
    connect(_commandHandler.get(), &CommandHandler::GetRoleMessageReceived, this, &MainWindow::onRoleMessageReceived);
    connect(ui->createFileButton, &QPushButton::clicked, this, &MainWindow::CreateFileButtonClicked);
}

MainWindow::~MainWindow() {}

void MainWindow::SetupWindow(const QString& username, const QString& token) {
    _token = token.toStdString();
    _username = username.toStdString();
    ui->usernameLine->setText(username);
    _commandHandler->StartAsyncReading();
    _commandHandler->SendCommand(Operation::GetRole, {_username, _token});
    this->show();
}

void MainWindow::CreateFileButtonClicked() {
    std::string pathToFile = ui->createFile->text().toStdString();
size_t lastSlashPos = pathToFile.find_last_of("/\\");

if (lastSlashPos == std::string::npos) {
    std::string path = "";
    std::string filename = pathToFile;
    std::cout << "Path: " << path << ", Filename: " << filename << std::endl;
    _commandHandler->SendCommand(Operation::CreateFile, {_username, _token, path, filename});
} else {
    std::string path = pathToFile.substr(0, lastSlashPos);
    std::string filename = pathToFile.substr(lastSlashPos + 1);
    std::cout << "Path: " << path << ", Filename: " << filename << std::endl;
    _commandHandler->SendCommand(Operation::CreateFile, {_username, _token, path, filename});
}

}

void MainWindow::onRoleMessageReceived(const QString& message) {
    ui->roleLine->setText(message);
}