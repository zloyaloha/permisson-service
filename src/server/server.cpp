#include "server.h"

void Server::AcceptConnections() {
    NotifyObservers("Server is waiting!");
    _acceptor.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket) {
            if (!ec) {
                NotifyObservers("Client connected!");
                std::make_shared<Session>(std::move(socket), shared_from_this())->Start();
            } else {
                NotifyObservers("ERROR while connecting" + ec.message());
            }
            AcceptConnections();  // Принять следующее подключение
        });
}

Server::Server(boost::asio::io_context& io_context)
    : _acceptor(io_context, tcp::endpoint(tcp::v4(), PORT)) {}

void Server::AddObserver(std::shared_ptr<IServerObserver> obs) {
    _observers.push_back(obs);
}

Session::Session(tcp::socket socket, std::shared_ptr<Server> server) : socket_(std::move(socket)), _server(server) {}

void Session::ReadMessage() {
    auto self(shared_from_this());
    socket_.async_read_some(boost::asio::buffer(_data),
        [this, self](boost::system::error_code ec, std::size_t length) {
            if (!ec) {
                _server->NotifyObservers("Сообщение получено");
                ReadMessage();
            } else {
                _server->NotifyObservers("Ошибка при получении сообщения " + ec.message());
            }
        });
}

void Server::NotifyObservers(const std::string& event) {
    auto now = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
    std::tm* localTime = std::localtime(&currentTime);
    std::cout << _observers.size() << std::endl;
    for (auto obs: _observers) {
        obs->Notify(localTime, event);
    }
}

ServerTextObserver::ServerTextObserver(const std::string& file) : of(file) {}

void ServerTextObserver::Notify(const std::tm* time, const std::string& str) {
    of << std::put_time(time, "%d-%m-%Y %H:%M:%S") << '\t' << str << '\n';
}

void ServerTerminalObserver::Notify(const std::tm* time, const std::string& str) {
    std::cout << std::put_time(time, "%d-%m-%Y %H:%M:%S") << '\t' << str << std::endl;
}