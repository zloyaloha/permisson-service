#include <stdint.h>
#include <string>
#include <cstring>
#include <sstream>
#include <iostream>

namespace {
    const int BUFFER_SIZE = 65665;
}

enum Operation {
    Login,
    GetSalt,
};

class BaseCommand {
public:
    Operation _op;
    int32_t _pid;
    std::string _msg_data;
    BaseCommand(Operation op, int32_t pid, const std::string& msg_data) : 
        _op(op), _pid(pid), _msg_data(msg_data) {}

    BaseCommand(const std::string& msg) 
    {
        std::istringstream stream(msg);
        std::string line;
        _msg_data = "";
        std::getline(stream, line);
        _op = Operation(std::stoi(line));
        std::getline(stream, line);
        _pid = std::stoi(line);
        while (std::getline(stream, line)) {
            _msg_data += line;
        }
    }

    std::string toPacket() {
        std::ostringstream oss;
        oss << int(_op) << '\n' << _pid << '\n' << _msg_data << '\0';
        return oss.str();
    }
};