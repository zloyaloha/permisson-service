#include "server.h"

void Server::AcceptConnections() {
    NotifyObservers("Server is waiting!");
    _acceptor.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket) {
            if (!ec) {
                NotifyObservers("Client connected!");
                std::make_shared<Session>(std::move(socket), _observers)->Start();
            } else {
                NotifyObservers("ERROR while connecting" + ec.message());
            }
            AcceptConnections();  // Принять следующее подключение
        });
}

Server::Server(boost::asio::io_context& io_context)
    : _acceptor(io_context, tcp::endpoint(tcp::v4(), PORT)) {}

Session::Session(tcp::socket socket, std::vector<std::shared_ptr<IServerObserver>>& observers) 
    : _socket(std::move(socket)) 
{
    for (auto obs: observers) {
        AddObserver(obs);
    }
}

void Session::ReadMessage() {
    auto self(shared_from_this());
    _socket.async_read_some(boost::asio::buffer(_data),
        [this, self](boost::system::error_code ec, std::size_t length) {
            if (!ec) {
                NotifyObservers("Сообщение получено");
                ReadMessage();
            } else {
                NotifyObservers("Ошибка при получении сообщения " + ec.message());
            }
        });
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