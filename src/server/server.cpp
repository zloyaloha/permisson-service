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
                NotifyObservers("Сообщение получено\n" + data);
                _threadPool.EnqueueTask([this, data] { _worker->ProccessOperation(data); });
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

void Worker::SendResponse(const std::string& response) {
    auto self(shared_from_this());
    boost::asio::async_write(*_socket, boost::asio::buffer(response + '\0'),
        [this, self](boost::system::error_code ec, std::size_t) {
            if (ec) {
                NotifyObservers("Ошибка при отправке ответа " + ec.message());
                _socket->close();
            } else {
                NotifyObservers("Ответ отправлен");
            }
        });
}

void Worker::ProccessOperation(const std::string &msg) {
    BaseCommand command(msg);
    static std::string result;
    switch (command._op) {
        case Operation::Registrate:
            NotifyObservers("Registrate " + command._msg_data[0]);
            result = Registrate(command._msg_data[0], command._msg_data[1]); // login, password
            SendResponse(result);
            break;
    }
}

std::string Worker::GetSalt(const std::string& login) {
    pqxx::work work(*_connection);
    pqxx::result result = work.exec("SELECT permission_app.GetSalt(" + work.quote(login) + ");");
    std::string res = "";
    for (const auto &row : result) {
        res += row[0].as<std::string>();
    }
    work.commit();
    return res;
}


std::string Worker::Registrate(const std::string& login, const std::string& password) {
    if (CheckUserExist(login)) {
        return "Exists";
    }
    std::string salt = GenerateSalt();
    std::string hashedPassword = HashPassword(password, salt);
    CreateUser(login, hashedPassword, salt);
    return "Success";
}

bool Worker::CheckUserExist(const std::string& login) {
    pqxx::work work(*_connection);
    pqxx::result result = work.exec("SELECT permission_app.UserExists(" + work.quote(login) + ");");
    std::string res = "";
    for (const auto &row : result) {
        res += row[0].as<std::string>();
    }
    if (res == "t") {
        return true;
    }
    work.commit();
    return false;
}

void Worker::CreateUser(const std::string &username, const std::string& hashed_password, const std::string& salt) {
    pqxx::work work(*_connection);
    pqxx::result result = work.exec("CALL permission_app.CreateUser(" 
                                        + work.quote(username) + ", "
                                        + work.quote(hashed_password) + ", "
                                        + work.quote(salt) + ");");
    std::string res = "";
    for (const auto &row : result) {
        res += row[0].as<std::string>();
    }
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