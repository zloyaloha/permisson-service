#include "command_handler.h"


CommandHandler::CommandHandler(boost::asio::io_context& io_context, const std::string& host, const std::string& port) : 
    _io_context(io_context), _socket(io_context), _host(host), _port(port), _resolver(io_context) {
        // responseBuffer.prepare(8192);
    }

void CommandHandler::Connect() {
    auto self = shared_from_this();
    tcp::resolver::query query(_host, _port);
    _resolver.async_resolve(query, 
        [this, self](const boost::system::error_code& ec, tcp::resolver::results_type results) {
            if (!ec) {
                ConnectToServer(results);
            } else {
                std::cerr << "Error resolving: " << ec.message() << std::endl;
            }
        });
}

void CommandHandler::ConnectToServer(tcp::resolver::results_type& endpoints) {
    auto self = shared_from_this();
    boost::asio::async_connect(_socket, endpoints,
        [this, self](const boost::system::error_code& ec, const tcp::endpoint& endpoint) {
            if (!ec) {
                std::cout << "Connected to " << endpoint << std::endl;
                AsyncReadResponse();
            } else {
                std::cerr << "Error connecting: " << ec.message() << std::endl;
            }
        });
}

void CommandHandler::StartAsyncReading() {
    asyncReadingEnabled = true;
    AsyncReadResponse();
}

void CommandHandler::StopAsyncReading() {
    asyncReadingEnabled = false;
}

void CommandHandler::AsyncReadResponse() {
    auto self(shared_from_this());
    boost::asio::async_read(_socket, responseBuffer, boost::asio::transfer_at_least(1),
        [this, self](boost::system::error_code ec, std::size_t bytes_transferred) {
            if (!ec) {
                std::istream is(&responseBuffer);
                std::string data;
                std::getline(is, data, '\0');
                _buffer += data;
                responseBuffer.consume(bytes_transferred);
                try {
                    BaseCommand command(_buffer);
                    if (command._packetSize <= _buffer.size()) {
                        HandleMessage(command);
                        _buffer = "";
                    }
                } catch (...) {}
                responseBuffer.consume(bytes_transferred);
                AsyncReadResponse();
            } else {
                boost::system::error_code ignored_ec;
                _socket.close(ignored_ec);;
            }
    });
}

void CommandHandler::HandleMessage(const BaseCommand& command) {
    switch (command._op) {
        case Operation::GetRole:
            emit GetRoleMessageReceived(QString::fromStdString(command._msg_data[0]));
            break;
        case Operation::CreateFile:
            break;
        case Operation::GetFileList:
            emit UpdateFileList(QString::fromStdString(command._msg_data[0])); // json
            break;
        case Operation::DeleteFile:
            emit FileDeleted(QString::fromStdString(command._msg_data[0]));
            break;
        case Operation::GetUsersList:
            emit GetUsersList(QString::fromStdString(command._msg_data[0]));
            break;
        case Operation::GetGroupsList:
            emit GetGroupsList(QString::fromStdString(command._msg_data[0]));
            break;
        case Operation::AddUserToGroup:
            emit AddUserToGroup(QString::fromStdString(command._msg_data[0]));
            break;
        case Operation::CreateGroup:
            emit CreateGroup(QString::fromStdString(command._msg_data[0]));
            break;
        case Operation::DeleteGroup:
            emit DeleteGroup(QString::fromStdString(command._msg_data[0]));
            break;
        default:
            break;
    }
}

void CommandHandler::SendCommand(const Operation op, const std::initializer_list<std::string>& data) {
    auto self(shared_from_this());
    BaseCommand msg(op, getpid(), data);
    boost::asio::async_write(_socket, boost::asio::buffer(msg.toPacket()),
        [this, self](boost::system::error_code ec, std::size_t) {
            if (ec) {
                _socket.close();
            }
        });
}

BaseCommand CommandHandler::ReadResponse() {
    try {
        boost::asio::streambuf responseBuffer;

        boost::asio::read_until(_socket, responseBuffer, TERMINATING_STRING);
        
        std::istream is(&responseBuffer);
        std::string data;
        std::getline(is, data, '\0');

        BaseCommand command(data);

        std::cout << "Ответ получен: " << data << std::endl;

        responseBuffer.consume(data.size());
        
        return command;
    } catch (std::exception& e) {
        std::cerr << "Error while reading response: " << e.what() << std::endl;
    }
}