#include "server.h"

int main() {
    try {
        boost::asio::io_context io_context;

        Server server(io_context);
        std::shared_ptr<ServerTerminalObserver> terminal_obs = std::make_shared<ServerTerminalObserver>();
        server.AddObserver(terminal_obs);
        server.AcceptConnections();
        std::thread server_thread([&io_context]() { io_context.run(); });

        server_thread.join();
    } catch (std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << "\n";
    }

    return 0;
}