#include <iostream>
#include <vector>
#include <boost/asio.hpp>
#include <string>
#include <ctime>
#include <chrono>
#include <fstream>
#include <iomanip>

using boost::asio::ip::tcp;

class IServerObserver;

namespace {
    const int PORT = 12345;
}

class Server : public std::enable_shared_from_this<Server>{
public:
    Server(boost::asio::io_context& io_context);
    void NotifyObservers(const std::string& str);
    void AddObserver(std::shared_ptr<IServerObserver> obs);
    void AcceptConnections();
private:
    tcp::acceptor _acceptor;
private:
    std::vector<std::shared_ptr<IServerObserver>> _observers;
};

class Session : public std::enable_shared_from_this<Session> {
public:
    explicit Session(tcp::socket socket, std::shared_ptr<Server> server);

    void Start() {
        ReadMessage();
    }

private:
    void ReadMessage();

    tcp::socket socket_;
    std::shared_ptr<Server> _server;
    std::array<char, 128> _data;
};

class IServerObserver {
    public:
        virtual void Notify(const std::tm* time, const std::string& str) = 0;
};

class ServerTextObserver : public IServerObserver {
    public:
        ServerTextObserver(const std::string& file);
        void Notify(const std::tm* time, const std::string& str) override;
    private:
        std::ofstream of;
};

class ServerTerminalObserver : public IServerObserver {
    public:
        void Notify(const std::tm* time, const std::string& str) override;
};