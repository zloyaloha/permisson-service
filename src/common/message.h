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
    Registrate
};

class BaseCommand {
public:
    Operation _op;
    int32_t _pid;
    std::vector<std::string> _msg_data;
    BaseCommand(Operation op, int32_t pid, const std::initializer_list<std::string>& msg_data) : 
        _op(op), _pid(pid), _msg_data(msg_data) {}

    BaseCommand(const std::string& msg) 
    {
        std::istringstream stream(msg);
        std::string line;
        std::getline(stream, line);
        _op = Operation(std::stoi(line));
        std::getline(stream, line);
        _pid = std::stoi(line);
        while (std::getline(stream, line)) {
            _msg_data.push_back(line);
        }
    }

    std::string toPacket() {
        std::ostringstream oss;
        oss << int(_op) << '\n' << _pid << '\n';
        std::cout << _msg_data.size() << std::endl;
        for (int i = 0; i < _msg_data.size() - 1; i++) {
            oss << _msg_data[i] << '\n';
        }
        oss << _msg_data[_msg_data.size() - 1] << '\0';
        return oss.str();
    }
};