#include "command_handler.h"
#include "iostream"

CommandHandler::CommandHandler(boost::asio::io_context& io_context) : io_context(io_context), socket(io_context) {}

void CommandHandler::Connect(const std::string &host, const std::string& port) {
    try {
        tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve(host, port);
        boost::asio::connect(socket, endpoints);
    } catch (std::exception& e) {
        std::cerr << "Error while connecting: " << e.what() << "\n";
    }
}

void CommandHandler::SendCommand(const Operation op, const std::string& msg_data) {
    try {
        if (socket.is_open()) {
            BaseCommand msg(Operation::Login, getpid(), msg_data);
            boost::asio::write(socket, boost::asio::buffer(msg.toPacket()));
            std::cout << "Command sent:\n" << msg.toPacket() << std::endl;
        } else {
            std::cerr << "Socket is not open." << std::endl;
        }
    } catch (std::exception& e) {
        std::cerr << "Error while sending command: " << e.what() << std::endl;
    }
}