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

void CommandHandler::SendCommand(const Operation op, const std::initializer_list<std::string>& data) {
    try {
        if (socket.is_open()) {
            BaseCommand msg(op, getpid(), data);
            boost::asio::write(socket, boost::asio::buffer(msg.toPacket()));
            std::cout << "Command sent:\n" << msg.toPacket() << std::endl;
        } else {
            std::cerr << "Socket is not open." << std::endl;
        }
    } catch (std::exception& e) {
        std::cerr << "Error while sending command: " << e.what() << std::endl;
    }
}

std::string CommandHandler::ReadResponse() {
 try {
        if (!socket.is_open()) {
            std::cerr << "Socket is not open." << std::endl;
            return "";
        }

        boost::asio::streambuf responseBuffer;

        boost::asio::read_until(socket, responseBuffer, '\0');

        std::string response(boost::asio::buffer_cast<const char*>(responseBuffer.data()), responseBuffer.size());

        if (!response.empty() && response.back() == '\0') {
            response.pop_back();
        }

        std::cout << "Ответ получен: " << response << std::endl;

        responseBuffer.consume(responseBuffer.size());
        
        return response;
    } catch (std::exception& e) {
        std::cerr << "Error while reading response: " << e.what() << std::endl;
        return "";
    }
}