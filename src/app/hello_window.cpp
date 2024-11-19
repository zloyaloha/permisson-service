#include "hello_window.h"


HelloWindow::HelloWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::HelloWindow), _username(), _token()
{
    ui->setupUi(this);
    QPixmap pic("../src/common/file.png");
    int w = ui->label_pic->width();
    int h = ui->label_pic->height();
    QPixmap scaledPic = pic.scaled(w, h, Qt::KeepAspectRatio);
    ui->label_pic->setPixmap(scaledPic);
    connect(ui->registrationButton, &QPushButton::clicked, this, &HelloWindow::ToRegistrationButtonClicked);
    connect(ui->loginButton, &QPushButton::clicked, this, &HelloWindow::LoginButtonClicked);
    connect(ui->registrationRegistrationButton, &QPushButton::clicked, this, &HelloWindow::RegistrationButtonClicked);
    connect(ui->backToLoginWindowButton, &QPushButton::clicked, this, &HelloWindow::BackToLoginWindowButtonClicked);

    static boost::asio::io_context io_context;
    _commandHandler = std::make_shared<CommandHandler>(io_context, SERVER_ADDRESS, PORT);
    _stringHandler = std::make_shared<StringHandler>();
    ui->stackedWidget->setCurrentIndex(0);
    _commandHandler->Connect();
    _mainWindow = std::make_shared<MainWindow>(_commandHandler);
    std::thread([&]() { io_context.run(); }).detach();
}

HelloWindow::~HelloWindow() {
    _commandHandler->SendCommand(Operation::Quit, {_username, _token});
}

void HelloWindow::ToRegistrationButtonClicked()
{
    ui->stackedWidget->setCurrentIndex(1);
    ui->loginRegistration->setText("");
    ui->passwordRegistration->setText("");
}

void HelloWindow::BackToLoginWindowButtonClicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void HelloWindow::RegistrationButtonClicked()
{
    QString login = ui->loginRegistration->text();
    QString password = ui->passwordRegistration->text();
    if (!_stringHandler->ValidInput(login, password)) {
        return;
    }
    _commandHandler->SendCommand(Operation::Registrate, {login.toStdString(), password.toStdString()});
    BaseCommand response(_commandHandler->ReadResponse());
    if (response._msg_data[0] == "Success") {
        ui->stackedWidget->setCurrentIndex(0);
    } else if (response._msg_data[0] == "Exists") {
        ui->loginRegistration->setReadOnly(true);
        ui->loginRegistration->setText("Пользователь с таким именем уже существует");
        ui->loginRegistration->setStyleSheet("QLineEdit { color: red; }");
        ui->loginRegistration->setText("");
        ui->passwordRegistration->setText("");
    }
}

void HelloWindow::LoginButtonClicked()
{
    QString login = ui->loginText->text();
    QString password = ui->passwordText->text();
    if (!_stringHandler->ValidInput(login, password)) {
        return;
    }
    _commandHandler->SendCommand(Operation::Login, {login.toStdString(), password.toStdString()});
    BaseCommand response(_commandHandler->ReadResponse());
    if (response._msg_data[0] == "Not exists") {
        std::cout << "Пользователя не существует" << std::endl;
    } else if (response._msg_data[0] == "Invalid Password") {
        std::cout << "Пароль неверный" << std::endl;
    } else {
        std::cout << "Удачно зашел" << std::endl;
        _token = response._msg_data[0];
        _username = login.toStdString();
        _mainWindow->SetupWindow(login, QString::fromStdString(_token));
    }
}

LoginAndPasswordValid StringHandler::IsValidLogin(const QString& login) {
    if (login.size() < MIN_LOGIN_SIZE || login.size() > MAX_LOGIN_SIZE) {
        std::cout << login.size() << std::endl;
        return LoginAndPasswordValid::InvalidLen;
    }
    static QRegularExpression regex("^[a-zA-Z0-9]+$");
    return regex.match(login).hasMatch() ? LoginAndPasswordValid::Valid : LoginAndPasswordValid::InvalidSymbol;
}

LoginAndPasswordValid StringHandler::IsValidPassword(const QString& password) {
    if (password.size() < MIN_PASSWORD_SIZE) {
        std::cout << "???" << std::endl;
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