#pragma once
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/asio/buffer.hpp>
#include <QObject>
#include "message.h"
#include <string>
#include "iostream"

namespace {
    const std::string SERVER_ADDRESS = "127.0.0.1";
    const std::string PORT = "12345";
}

using boost::asio::ip::tcp;

class CommandHandler: public QObject, public std::enable_shared_from_this<CommandHandler> {
    Q_OBJECT 
    private:
        tcp::socket _socket;
        boost::asio::io_context& _io_context;
        tcp::resolver _resolver;
        boost::asio::streambuf responseBuffer;
        std::string _buffer;
        bool asyncReadingEnabled{false};
        std::string _host, _port;
    public:
        CommandHandler(boost::asio::io_context& io_context, const std::string& host, const std::string& port);
        void Connect();
        void SendCommand(const Operation op, const std::initializer_list<std::string>& data);
        BaseCommand ReadResponse();
        void StartAsyncReading();
        void StopAsyncReading();
        void AsyncReadResponse();
    private:
        void handle_read(const boost::system::error_code& error);
        void ConnectToServer(tcp::resolver::results_type& endpoints);
        void HandleMessage(const BaseCommand& command);
    signals:
        void GetRoleMessageReceived(const QString& response);
        void UpdateFileList(const QString& response);
};