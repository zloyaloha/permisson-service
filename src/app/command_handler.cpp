#include "command_handler.h"


CommandHandler::CommandHandler(boost::asio::io_context& io_context, const std::string& host, const std::string& port) : 
    _io_context(io_context), _socket(io_context), _host(host), _port(port), _resolver(io_context) {}

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
    boost::asio::async_read_until(_socket, responseBuffer, '\0',
        [this, self](boost::system::error_code ec, std::size_t bytes_transferred) {
            if (!ec) {
                std::istream is(&responseBuffer);
                std::string data;
                std::getline(is, data, '\0');
                BaseCommand command(data);
                HandleMessage(command);
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

        boost::asio::read_until(_socket, responseBuffer, '\0');

        std::string response(boost::asio::buffer_cast<const char*>(responseBuffer.data()), responseBuffer.size());

        if (!response.empty() && response.back() == '\0') {
            response.pop_back();
        }

        BaseCommand command(response);

        std::cout << "Ответ получен: " << response << std::endl;

        responseBuffer.consume(responseBuffer.size());
        
        return command;
    } catch (std::exception& e) {
        std::cerr << "Error while reading response: " << e.what() << std::endl;
    }
}