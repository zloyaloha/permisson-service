#include <stdint.h>
#include <string>
#include <cstring>
#include <sstream>

namespace {
    const int BUFFER_SIZE = 65665;
}

enum Operation {
    Login
};

class BaseCommand {
private:
    Operation _op;
    int32_t _pid;
    std::string _msg_data;
public:
    BaseCommand(Operation op, int32_t pid, const std::string& msg_data) : 
        _op(op), _pid(pid), _msg_data(msg_data) {}

    std::string toPacket() {
        std::ostringstream oss;
        oss << int(_op) << '\n' << _pid << '\n' << _msg_data << '\n';
        return oss.str();
    }
};