#include "hello_w.h"

HelloWindow::HelloWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::HelloWindow)
{
    ui->setupUi(this);

    connect(ui->registrationButton, &QPushButton::clicked, this, &HelloWindow::RegistrationButtonClicked);
    connect(ui->loginButton, &QPushButton::clicked, this, &HelloWindow::LoginButtonClicked);
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
    qDebug() << login << ' ' << password;
    // LoginDialog loginDialog(this);
    // loginDialog.exec();
}