#include "hello_w.h"


HelloWindow::HelloWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::HelloWindow)
{
    ui->setupUi(this);

    connect(ui->registrationButton, &QPushButton::clicked, this, &HelloWindow::RegistrationButtonClicked);
    connect(ui->loginButton, &QPushButton::clicked, this, &HelloWindow::LoginButtonClicked);

    boost::asio::io_context io_context;
    _commandHandler = std::make_shared<CommandHandler>(io_context);
    _stringHandler = std::make_shared<StringHandler>();

    _commandHandler->Connect(SERVER_ADDRESS, PORT);
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
    switch (_stringHandler->IsValidLogin(login)) {
        case (LoginAndPasswordValid::InvalidLen):
            std::cout << "Invalid len" << std::endl;
            return;
        case (LoginAndPasswordValid::InvalidSymbol):
            std::cout << "Invalid symb" << std::endl;
            return;
        case (LoginAndPasswordValid::Valid):
            break;
    }
    switch (_stringHandler->IsValidPassword(password)) {
        case (LoginAndPasswordValid::InvalidLen):
            return;
        case (LoginAndPasswordValid::InvalidSymbol):
            return;
        case (LoginAndPasswordValid::Valid):
            break;
    }
    std::cout << "Checks out" << std::endl;
    _commandHandler->SendCommand(Operation::GetSalt, login.toStdString());
    std::string response = _commandHandler->ReadResponse();
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

std::string StringHandler::HashPassword(const QString& password) {
    // return BCrypt::generateHash(password);
}