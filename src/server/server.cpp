#include "server.h"

void Server::AcceptConnections() {
    NotifyObservers("Server is waiting!");
    _acceptor.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket) {
            if (!ec) {
                NotifyObservers("Client connected!");
                auto socketPtr = std::make_shared<tcp::socket>(std::move(socket)); 
                std::make_shared<Session>(socketPtr, _observers, _threadPool)->Start();
            } else {
                NotifyObservers("ERROR while connecting" + ec.message());
            }
            AcceptConnections();
        });
}

Server::Server(boost::asio::io_context& io_context)
    : _acceptor(io_context, tcp::endpoint(tcp::v4(), PORT)), _threadPool(NUMBER_OF_THREADS) {}

Session::Session(std::shared_ptr<tcp::socket> socket, std::vector<std::shared_ptr<IServerObserver>>& observers, ThreadPool& threadPool) 
    : _socket(std::move(socket)), _threadPool(threadPool), _worker(std::make_shared<Worker>(threadPool, _socket, _observers)), Observable(observers) {}

void Session::Start() {
    // _data.resize(BUFFER_SIZE);
    ReadMessage();
}

void Session::ReadMessage() {
    auto self(shared_from_this());
    boost::asio::async_read_until(*_socket, _buffer, '\0',
        [this, self](boost::system::error_code ec, std::size_t bytes_transferred) {
            if (!ec) {
                std::istream is(&_buffer);
                std::string data;
                std::getline(is, data, '\0');
                BaseCommand command(data);
                if (command._op == Operation::Quit) {
                    if (command._msg_data[0] != "") { // logged connection
                        _worker->ProccessOperation(command);
                    }
                    _socket->close();
                    return;
                }
                NotifyObservers("Сообщение получено\n" + data);
                _threadPool.EnqueueTask([this, command] { _worker->ProccessOperation(command); });
                ReadMessage();
            } else {
                NotifyObservers("Ошибка при получении сообщения " + ec.message());
                boost::system::error_code ignored_ec;
                _socket->close(ignored_ec);
                NotifyObservers("Соединение закрыто");
            }
        });
}

Worker::Worker(ThreadPool& threadPool, std::shared_ptr<tcp::socket> socket, std::vector<std::shared_ptr<IServerObserver>>& observers) 
    : _threadPool(threadPool), _socket(socket), Observable(observers), _connection(std::make_unique<pqxx::connection>(PARAM_STRING)) {}

void Worker::SendResponse(const Operation op, const std::initializer_list<std::string>& data) {
    auto self(shared_from_this());
    BaseCommand msg(op, getpid(), data);
    boost::asio::async_write(*_socket, boost::asio::buffer(msg.toPacket()),
        [this, self](boost::system::error_code ec, std::size_t writed) {
            if (ec) {
                NotifyObservers("Ошибка при отправке ответа " + ec.message());
                _socket->close();
            } else {
                NotifyObservers("Ответ отправлен " + std::to_string(writed));
            }
        });
}

bool Worker::ValidateRequest(const std::string& username, const std::string& token) {
    std::string userToken = GetToken(username);
    std::cout << userToken << '\n' << token << std::endl;
    if (userToken == token) {
        return true;
    }
    return false;
}

std::string Worker::GetToken(const std::string& username) {
    pqxx::work work(*_connection);
    pqxx::result result = work.exec("SELECT permission_app.GetToken(" + work.quote(username) + ");");
    work.commit();
    return GetStringQueryResult(result);
}


void Worker::ProccessOperation(const BaseCommand &command) {
    static std::string result;
    switch (command._op) {
        case Operation::Registrate:
            NotifyObservers("Registrate " + command._msg_data[0]);
            result = Registrate(command._msg_data[0], command._msg_data[1]); // login, password
            SendResponse(command._op, {result}); // 
            break;
        case Operation::Login:
            NotifyObservers("Log in " + command._msg_data[0]);
            result = Login(command._msg_data[0], command._msg_data[1]); // login, password
            SendResponse(command._op, {result});
            break;
        case Operation::Quit:
            NotifyObservers("Quit " + command._msg_data[0]);
            if (!ValidateRequest(command._msg_data[0], command._msg_data[1])) { // username, token
                NotifyObservers("Security warning");
                break;
            }
            Quit(command._msg_data[1]); // user_id
            _connection->disconnect();
            break;
        case Operation::GetRole:
            NotifyObservers("Get role " + command._msg_data[0]); // username
            if (!ValidateRequest(command._msg_data[0], command._msg_data[1])) { // username, token
                NotifyObservers("Security warning");
                break;
            }
            result = GetRole(command._msg_data[0]); // username
            SendResponse(command._op, {result});
            break;
        case Operation::CreateFile:
            NotifyObservers("Create file " + command._msg_data[0]); // username
            if (!ValidateRequest(command._msg_data[0], command._msg_data[1])) { // username, token
                NotifyObservers("Security warning");
                break;
            }
            result = CreateFile(command._msg_data[0], command._msg_data[2], command._msg_data[3]); // username, path, filename
            SendResponse(command._op, {result});
            break;
    }
}

std::string Worker::CreateFile(const std::string& username, const std::string& path, const std::string& filename) {
    pqxx::work work(*_connection);
    pqxx::result result = work.exec("SELECT permission_app.AddFileToPath(" + work.quote(path) + ", " + work.quote(filename) + ", " + work.quote(username) + ");");
    work.commit();
    return GetStringQueryResult(result);
}

std::string Worker::GetRole(const std::string& role) {
    pqxx::work work(*_connection);
    pqxx::result result = work.exec("SELECT permission_app.UserRights(" + work.quote(role) + ");");
    work.commit();
    std::string admin = GetStringQueryResult(result);
    if (admin == "t") {
        return "admin";
    } else {
        return "common";
    }
}

void Worker::Quit(const std::string& token) {
    pqxx::work work(*_connection);
    pqxx::result result = work.exec("CALL permission_app.UpdateExitTime(" + work.quote(token) + ");");
    work.commit();
}

std::string Worker::Login(const std::string& login, const std::string& password) {
    int user_id = UserID(login);
    if (user_id == 0) {
        return "Invalid username";
    }
    std::pair<std::string, std::string> passwordAndSalt = GetSaltAndPassword(login);
    std::string hashedPassword = HashPassword(password, passwordAndSalt.second);
    if (hashedPassword != passwordAndSalt.first) {
        return "Invalid password";
    }
    std::string token = CreateSession(user_id);
    return token;
}

std::string Worker::Registrate(const std::string& login, const std::string& password) {
    if (UserID(login) != 0) {
        return "Exists";
    }
    std::string salt = GenerateSalt();
    std::string hashedPassword = HashPassword(password, salt);
    CreateUser(login, hashedPassword, salt);
    return "Success";
}

std::pair<std::string, std::string> Worker::GetSaltAndPassword(const std::string& login) {
    pqxx::work work(*_connection);
    pqxx::result result = work.exec("SELECT permission_app.GetSaltAndPassword(" + work.quote(login) + ");");
    work.commit();
    return GetPairQueryResult(result);  
}

std::string Worker::CreateSession(const int user_id) {
    pqxx::work work(*_connection);
    pqxx::result result = work.exec("SELECT permission_app.AddSession(" + work.quote(user_id) + ");");
    std::string response = GetStringQueryResult(result);
    work.commit();
    return response;
}


int Worker::UserID(const std::string& login) {
    pqxx::work work(*_connection);
    pqxx::result result = work.exec("SELECT permission_app.UserID(" + work.quote(login) + ");");
    std::string response = GetStringQueryResult(result);
    work.commit();
    return std::stoi(response);
}

void Worker::CreateUser(const std::string &username, const std::string& hashed_password, const std::string& salt) {
    pqxx::work work(*_connection);
    pqxx::result result = work.exec("CALL permission_app.CreateUser(" 
                                        + work.quote(username) + ", "
                                        + work.quote(hashed_password) + ", "
                                        + work.quote(salt) + ");");
    work.commit();
}

std::string Worker::GenerateSalt(std::size_t size) const {
    unsigned char salt[size];
    if (!RAND_bytes(salt, sizeof(salt))) {
        throw std::runtime_error("Error generating random bytes.");
    }
    return ToHexString(salt, size);
}

std::string Worker::HashPassword(const std::string& password, const std::string& salt) const {
    std::string saltedPassword = password + salt;
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(saltedPassword.c_str()), saltedPassword.length(), hash);

    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    return ss.str();
}

std::string Worker::ToHexString(const unsigned char* data, std::size_t length) const {
    std::stringstream ss;
    for (std::size_t i = 0; i < length; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]);
    }
    return ss.str();
}

std::string Worker::GetStringQueryResult(const pqxx::result& result) const {
    std::string res = "";
    for (const auto &row : result) {
        res += row[0].as<std::string>();
    }
    return res;
}

std::pair<std::string, std::string> Worker::GetPairQueryResult(const pqxx::result& result) const {
    std::string res = "";
    for (const auto &row : result) {
        res += row[0].as<std::string>();
    }
    res = res.substr(1, res.size() - 2); 
    size_t comma = res.find(',');
    std::string password = res.substr(0, comma);
    std::string salt = res.substr(comma + 1);
    return std::make_pair(password, salt);  
}

Worker::~Worker() {
    _connection->disconnect();
}

void Observable::NotifyObservers(const std::string& event) {
    auto now = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
    std::tm* localTime = std::localtime(&currentTime);
    for (auto obs: _observers) {
        obs->Notify(localTime, event);
    }
}

void Observable::AddObserver(std::shared_ptr<IServerObserver> obs) {
    _observers.push_back(obs);
}

ServerTextObserver::ServerTextObserver(const std::string& file) : of(file) {}

void ServerTextObserver::Notify(const std::tm* time, const std::string& str) {
    of << std::put_time(time, "%d-%m-%Y %H:%M:%S") << '\t' << str << '\n';
}

void ServerTerminalObserver::Notify(const std::tm* time, const std::string& str) {
    std::cout << std::put_time(time, "%d-%m-%Y %H:%M:%S") << '\t' << str << std::endl;
}