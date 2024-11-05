#include "hello_w.h"


HelloWindow::HelloWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::HelloWindow)
{
    ui->setupUi(this);

    connect(ui->registrationButton, &QPushButton::clicked, this, &HelloWindow::ToRegistrationButtonClicked);
    connect(ui->loginButton, &QPushButton::clicked, this, &HelloWindow::LoginButtonClicked);
    connect(ui->registrationRegistrationButton, &QPushButton::clicked, this, &HelloWindow::RegistrationButtonClicked);
    connect(ui->backToLoginWindowButton, &QPushButton::clicked, this, &HelloWindow::BackToLoginWindowButtonClicked);

    boost::asio::io_context io_context;
    _commandHandler = std::make_shared<CommandHandler>(io_context);
    _stringHandler = std::make_shared<StringHandler>();
    ui->stackedWidget->setCurrentIndex(0);
    _commandHandler->Connect(SERVER_ADDRESS, PORT);
}

void HelloWindow::ToRegistrationButtonClicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}

void HelloWindow::BackToLoginWindowButtonClicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void HelloWindow::RegistrationButtonClicked()
{
    QString login = ui->login->text();
    QString password = ui->password->text();
    if (!_stringHandler->ValidInput(login, password)) {
        return;
    }
    _commandHandler->SendCommand(Operation::Registrate, {login.toStdString(), password.toStdString()});
}

void HelloWindow::LoginButtonClicked()
{
    QString login = ui->login->text();
    QString password = ui->password->text();
    if (!_stringHandler->ValidInput(login, password)) {
        return;
    }
    _commandHandler->SendCommand(Operation::Login, {login.toStdString()});
    std::string response = _commandHandler->ReadResponse();
    if (response == "") {
        std::cout << "Пользователь не найден" << std::endl;
    }
    // std::cout << response << std::endl;
    // _commandHandler->SendCommand(Operation::Login, login.toStdString() + ' ' + password.toStdString() + '\0');
}

LoginAndPasswordValid StringHandler::IsValidLogin(const QString& login) {
    if (login.size() < MIN_LOGIN_SIZE || login.size() > MAX_LOGIN_SIZE) {
        return LoginAndPasswordValid::InvalidLen;
    }
    static QRegularExpression regex("^[a-zA-Z0-9]+$");
    return regex.match(login).hasMatch() ? LoginAndPasswordValid::Valid : LoginAndPasswordValid::InvalidSymbol;
}

LoginAndPasswordValid StringHandler::IsValidPassword(const QString& password) {
    if (password.size() < MIN_PASSWORD_SIZE) {
        return LoginAndPasswordValid::InvalidLen;
    }
    static QRegularExpression regex("^[a-zA-Z0-9@$!%*?&]+$");
    return regex.match(password).hasMatch() ? LoginAndPasswordValid::Valid : LoginAndPasswordValid::InvalidSymbol;
}

bool StringHandler::ValidInput(const QString& login, const QString& password) {
    switch (IsValidLogin(login)) {
        case (LoginAndPasswordValid::InvalidLen):
            std::cout << "Invalid login len" << std::endl;
            return false;
        case (LoginAndPasswordValid::InvalidSymbol):
            std::cout << "Invalid login symb" << std::endl;
            return false;
        case (LoginAndPasswordValid::Valid):
            break;
    }
    switch (IsValidPassword(password)) {
        case (LoginAndPasswordValid::InvalidLen):
            std::cout << "Invalid password len" << std::endl;
            return false;
        case (LoginAndPasswordValid::InvalidSymbol):
            std::cout << "Invalid password len" << std::endl;
            return false;
        case (LoginAndPasswordValid::Valid):
            break;
    }
    return true;
}