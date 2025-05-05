#pragma once
#include <iostream>
#include <vector>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/bind/bind.hpp>
#include <boost/process.hpp>
#include <boost/filesystem.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <string>
#include <ctime>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <tuple>
#include <sstream>
#include <boost/foreach.hpp>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QString>
#include <QStringList>
#include <QDebug>

#include <sw/redis++/redis++.h>

#include "thread_pool.h"
#include "message.h"

using tcp = boost::asio::ip::tcp;
namespace beast = boost::beast;
namespace http = beast::http;
using namespace boost::placeholders;

class IServerObserver;

namespace {
    const int NUMBER_OF_THREADS = 4;
}

struct Config {
    int PORT;
    QString DB_HOST;
    QString DB_PORT;
    QString DB_NAME;
    QString DB_USER;
    QString DB_PASSWORD;
};

struct FileInfo {
    QString path;
    QString fileName;
    QString fileType;
    QString userName;
    QString groupName;
    QString permissions;
};

class JsonHandler {
    public:
        JsonHandler() = default;
        QJsonObject GenerateFileTree(const pqxx::result& result);
        QJsonObject GenerateUsersList(const pqxx::result& result);
        QJsonObject GenerateGroupsList(const pqxx::result& result);
    private:
        void AddFileToTree(QJsonArray& parentArray, const QStringList& pathComponents, const FileInfo& file);
};

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
    Server(boost::asio::io_context& io_context, const Config& conf);
    void AcceptConnections();
private:
    tcp::acceptor _acceptor;
    ThreadPool _threadPool;
    Config _config;
};

class Worker : public std::enable_shared_from_this<Worker>, public Observable {
    public:
        Worker(ThreadPool& threadPool, std::shared_ptr<tcp::socket> socket, std::vector<std::shared_ptr<IServerObserver>>& observers, const Config& conf);
        ~Worker();
        void SendResponse(const Operation op, const std::initializer_list<std::string>& data);
        void ProccessOperation(const BaseCommand &command);
        std::string Registrate(const std::string& login, const std::string& password);
        std::string Login(const std::string& login, const std::string& password);
        std::string CreateFile(const std::string& username, const std::string& path, const std::string& filename);
        std::string CreateDir(const std::string& username, const std::string& path, const std::string& filename);
        std::string DeleteFile(const std::string& username, const std::string& filename);
        std::string GetFileList();
        std::string GetUsersList();
        std::string GetGroupsList();
        std::string AddUserToGroup(const std::string& groupName, const std::string& userName);
        std::string CreateGroup(const std::string& groupName, const std::string& userName);
        std::string DeleteGroup(const std::string& groupName, const std::string& userName);
        std::string AddFileToGroup(const std::string& fileName, const std::string& groupName, const std::string& userName);
        std::string ChangeRights(const std::string& fileName, const std::string& permissions, const std::string& userName);
        std::string GenerateToken() const;
        bool IsRootOrOwner(const std::string& userName, const std::string& fileName);
        bool IsRoot(const std::string& userName);
        bool CheckUserRightsToWrite(const std::string& userName, const std::string& fileName);
        bool CheckUserRightsToRead(const std::string& userName, const std::string& fileName);
        bool CheckUserRightsToExec(const std::string& userName, const std::string& fileName);
        void AddWriteEvent(const std::string& userName, const std::string& fileName);
        void AddReadEvent(const std::string& userName, const std::string& fileName);
        void AddExecEvent(const std::string& userName, const std::string& fileName);
        void Quit(const std::string& token, const std::string& username);
        void SaveActiveSession(int user_id, const std::string& ip);
        bool IsUserActive(int user_id) const;
        void UpdateActivity(const std::string& username);
        void PrintActiveSessions() const;
        void LoginEvent(const std::string& username, int id) const;
        void QuitEvent(const std::string& username, int id) const;
        std::string CreateBackup(const std::string& filePath);
        std::string RecoverDB(const std::string& filePath);
    private:
        std::string SetUserRights(const std::string& fileName, int mask);
        std::string SetGroupRights(const std::string& fileName, int mask);
        std::string SetAllRights(const std::string& fileName, int mask);
        bool ValidateRequest(const std::string& username, const std::string& token);
        std::string GetToken(const std::string& username);
        void StopActiveSession();
        std::string GetRole(const std::string& username);
        std::string CreateSession(const int user_id);
        std::string GenerateSalt(std::size_t size = 16) const; // bytes
        std::string HashPassword(const std::string& password, const std::string& salt) const;
        std::string ToHexString(const unsigned char* data, std::size_t length) const;
        std::pair<std::string, std::string> GetSaltAndPassword(const std::string& login);
        void CreateUser(const std::string& login, const std::string& hashed_password, const std::string& salt);
        int UserID(const std::string& login);
        int SetMask(int start, const std::string& permissions);
        std::string GetStringQueryResult(const pqxx::result& result) const;
        std::pair<std::string, std::string> GetPairQueryResult(const pqxx::result& result) const;
        std::vector<int> authorized_users;
        pqxx::result MakeQuery(const std::string& query);
        void PrepareQueries();
    private:
        JsonHandler _jsonHandler;
        ThreadPool& _threadPool;
        std::unique_ptr<pqxx::connection> _connection;
        std::unique_ptr<sw::redis::Redis> _connectionRedis;
        std::shared_ptr<tcp::socket> _socket;
        Config _config;
};

class Session : public std::enable_shared_from_this<Session>, public Observable {
public:
    explicit Session(std::shared_ptr<tcp::socket> socket, std::vector<std::shared_ptr<IServerObserver>>& observers, ThreadPool& threadPool, const Config& conf);
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


