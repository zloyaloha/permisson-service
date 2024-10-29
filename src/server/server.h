#include <iostream>
#include <vector>
#include <boost/asio.hpp>
#include <string>
#include <ctime>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <message.h>

using boost::asio::ip::tcp;

class IServerObserver;

namespace {
    const int PORT = 12345;
}

class Observable {
public:
    virtual void NotifyObservers(const std::string& str);
    virtual void AddObserver(std::shared_ptr<IServerObserver> obs);
protected:
    std::vector<std::shared_ptr<IServerObserver>> _observers;
};

class Server : public std::enable_shared_from_this<Server>, public Observable {
public:
    Server(boost::asio::io_context& io_context);
    void AcceptConnections();
private:
    tcp::acceptor _acceptor;
};

class Session : public std::enable_shared_from_this<Session>, public Observable {
public:
    explicit Session(tcp::socket socket, std::vector<std::shared_ptr<IServerObserver>>& obs);

    void Start() {
        ReadMessage();
    }

private:
    void ReadMessage();

    tcp::socket _socket;
    std::array<char, BUFFER_SIZE> _data;
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