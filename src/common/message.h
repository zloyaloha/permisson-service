#include <stdint.h>
#include <string>
#include <cstring>
#include <sstream>
#include <iostream>

namespace {
    const int BUFFER_SIZE = 65665;
    const char TERMINATING_STRING = '\0';
}

enum Operation {
    Login,
    Registrate,
    Quit, 
    GetRole,
    CreateFile,
    GetFileList,
    CreateDir
};

class BaseCommand {
public:
    Operation _op;
    int32_t _packetSize;
    std::vector<std::string> _msg_data;
    BaseCommand(Operation op, int32_t pid, const std::initializer_list<std::string>& msg_data) : 
        _op(op), _packetSize(pid), _msg_data(msg_data) {}

    BaseCommand(const std::string& msg) 
    {
        std::istringstream stream(msg);
        std::string line;
        std::getline(stream, line);
        _op = Operation(std::stoi(line));
        std::getline(stream, line);
        _packetSize = std::stoi(line);
        while (std::getline(stream, line)) {
            _msg_data.push_back(line);
        }
        // _msg_data[_msg_data.size() - 1] = _msg_data[_msg_data.size() - 1].substr(0, _msg_data[_msg_data.size() - 1].size() - 1);
    }

    std::string toPacket() {
        std::ostringstream oss;
        if (_msg_data.size() == 0) {
            return "" + TERMINATING_STRING;
        }
        for (int i = 0; i < _msg_data.size() - 1; i++) {
            oss << _msg_data[i] << '\n';
        }
        oss << _msg_data[_msg_data.size() - 1] << TERMINATING_STRING;
        std::string dataStr = oss.str();
        return std::to_string(_op) + "\n" + std::to_string(dataStr.size() + 2 + 2) + "\n" + dataStr;
    }

    std::string MakeJson() const {
        std::string json = "";
        for (const std::string& row: _msg_data) {
            json += row;
        }
        return json;
    }
};