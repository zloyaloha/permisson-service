#include <boost/asio.hpp>
#include "message.h"

namespace {
    const std::string SERVER_ADDRESS = "127.0.0.1";
    const std::string PORT = "12345";
}

using boost::asio::ip::tcp;

class CommandHandler {
    private:
        tcp::socket socket;
        boost::asio::io_context& io_context; 
    public:
        CommandHandler(boost::asio::io_context& io_context);
        void Connect(const std::string &host, const std::string& port);
        void SendCommand(const Operation op, const std::string& msg_data);
};