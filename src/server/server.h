#include <iostream>
#include <vector>
#include <boost/asio.hpp>
#include <string>
#include <ctime>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <openssl/sha.h>
#include <openssl/rand.h>

#include "thread_pool.h"
#include "message.h"

using boost::asio::ip::tcp;

class IServerObserver;

namespace {
    const int PORT = 12345;
    const int NUMBER_OF_THREADS = 4;
    const std::string DB_HOST = "localhost";
    const std::string DB_PORT = "5432";
    const std::string DB_NAME = "permission-db";
    const std::string DB_USER = "app_user";
    const std::string DB_PASSWORD = "sup1234";
    const std::string PARAM_STRING = 
        "host=" + DB_HOST + 
        " port=" + DB_PORT + 
        " dbname=" + DB_NAME + 
        " user=" + DB_USER + 
        " password=" + DB_PASSWORD; 
}

class Observable {
public:
    Observable(const std::vector<std::shared_ptr<IServerObserver>>& other) : _observers(other) {}
    Observable() = default;
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
    ThreadPool _threadPool;
};

class Worker : public std::enable_shared_from_this<Worker>, public Observable {
    public:
        Worker(ThreadPool& threadPool, std::shared_ptr<tcp::socket> socket, std::vector<std::shared_ptr<IServerObserver>>& observer);
        ~Worker();
        void SendResponse(const std::string& response);
        void ProccessOperation(const std::string& message);
        std::string Registrate(const std::string& login, const std::string& password);
    private:
        std::string GenerateSalt(std::size_t size = 16) const; // bytes
        std::string HashPassword(const std::string& password, const std::string& salt) const;
        std::string ToHexString(const unsigned char* data, std::size_t length) const;
        std::string GetSalt(const std::string& login);
        void CreateUser(const std::string& login, const std::string& hashed_password, const std::string& salt);
        bool CheckUserExist(const std::string& login);

    private:
    
        ThreadPool& _threadPool;
        std::unique_ptr<pqxx::connection> _connection;
        std::shared_ptr<tcp::socket> _socket;
};

class Session : public std::enable_shared_from_this<Session>, public Observable {
public:
    explicit Session(std::shared_ptr<tcp::socket> socket, std::vector<std::shared_ptr<IServerObserver>>& observers, ThreadPool& threadPool);
    void Start();
private:
    void ReadMessage();
    
    std::shared_ptr<tcp::socket> _socket;
    boost::asio::streambuf _buffer;
    ThreadPool& _threadPool;
    std::shared_ptr<Worker> _worker;
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