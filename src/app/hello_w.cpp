#include "hello_w.h"


HelloWindow::HelloWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::HelloWindow)
{
    ui->setupUi(this);

    connect(ui->registrationButton, &QPushButton::clicked, this, &HelloWindow::RegistrationButtonClicked);
    connect(ui->loginButton, &QPushButton::clicked, this, &HelloWindow::LoginButtonClicked);

    boost::asio::io_context io_context;
    _handler = std::make_shared<CommandHandler>(io_context);
    _handler->Connect(SERVER_ADDRESS, PORT);
}

void HelloWindow::RegistrationButtonClicked()
{
    RegistrationDialog regDialog(this);
    regDialog.exec();
}

void HelloWindow::LoginButtonClicked()
{
    QString login = ui->login->text();
    QString password = ui->password->text();
    _handler->SendCommand(Operation::Login, login.toStdString() + ' ' + password.toStdString() + '\0');
}