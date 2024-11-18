#include "command_handler.h"


CommandHandler::CommandHandler(boost::asio::io_context& io_context) : io_context(io_context), socket(io_context), responseBuffer() {}

void CommandHandler::Connect(const std::string &host, const std::string& port) {
    try {
        tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve(host, port);
        boost::asio::connect(socket, endpoints);
    } catch (std::exception& e) {
        std::cerr << "Error while connecting: " << e.what() << "\n";
    }
}

void CommandHandler::StartAsyncReading() {
    asyncReadingEnabled = true;
    AsyncReadResponse();
}

void CommandHandler::StopAsyncReading() {
    asyncReadingEnabled = false;
}

void CommandHandler::AsyncReadResponse() {
    if (!asyncReadingEnabled) return; // Проверяем, включено ли чтение
    std::shared_ptr<CommandHandler> self = shared_from_this();
    boost::asio::async_read_until(socket, responseBuffer2, '\0',
        [this, self](boost::system::error_code ec, std::size_t bytes_transferred) {
            if (!ec && asyncReadingEnabled) {
                std::cout << "In async2" << std::endl;
                std::istream responseStream(&responseBuffer2);
                std::string response;
                std::getline(responseStream, response);
                BaseCommand command(response);
                HandleMessage(command);

                AsyncReadResponse();
            } else if (ec) {
                std::cerr << "Error reading response: " << ec.message() << std::endl;
            }
        });
}

void CommandHandler::HandleMessage(const BaseCommand& command) {
    switch (command._op)
        {
        case Operation::GetRole:
            emit GetRoleMessageReceived(QString::fromStdString(command._msg_data[0]));
            break;
        default:
            break;
        }
}

void CommandHandler::SendCommand(const Operation op, const std::initializer_list<std::string>& data) {
    try {
        if (socket.is_open()) {
            BaseCommand msg(op, getpid(), data);
            boost::asio::write(socket, boost::asio::buffer(msg.toPacket()));
            std::cout << "Command sent: " << msg._op << std::endl;
        } else {
            std::cerr << "Socket is not open." << std::endl;
        }
    } catch (std::exception& e) {
        std::cerr << "Error while sending command: " << e.what() << std::endl;
    }
}

BaseCommand CommandHandler::ReadResponse() {
    try {
        boost::asio::streambuf responseBuffer;

        boost::asio::read_until(socket, responseBuffer, '\0');

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