#include <boost/asio.hpp>
#include "message.h"
#include <string>
#include "iostream"

namespace {
    const std::string SERVER_ADDRESS = "127.0.0.1";
    const std::string PORT = "12345";
}

using boost::asio::ip::tcp;

class CommandHandler: public std::enable_shared_from_this<CommandHandler> {
    private:
        tcp::socket socket;
        boost::asio::io_context& io_context;
        boost::asio::streambuf responseBuffer;
    public:
        CommandHandler(boost::asio::io_context& io_context);
        void Connect(const std::string &host, const std::string& port);
        void SendCommand(const Operation op, const std::initializer_list<std::string>& data);
        std::string ReadResponse();
};