#include "server.h"
#include <QFile>

Config LoadConfig(const QString& fileName) {
    Config config;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        std::cerr << "Couldn't open config file!" << std::endl;
        return config;
    }

    QByteArray data = file.readAll();

    QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
    if (jsonDoc.isNull()) {
        std::cerr << "Invalid JSON format!" << std::endl;
        return config;
    }

    QJsonObject rootObj = jsonDoc.object();

    QJsonObject dbObj = rootObj["database"].toObject();
    config.PORT = dbObj["PORT"].toInt();
    config.DB_HOST = dbObj["DB_HOST"].toString();
    config.DB_PORT = dbObj["DB_PORT"].toString();
    config.DB_NAME = dbObj["DB_NAME"].toString();
    config.DB_USER = dbObj["DB_USER"].toString();
    config.DB_PASSWORD = dbObj["DB_PASSWORD"].toString();
    config.PORT = dbObj["PORT"].toInt();

    QJsonObject appObj = rootObj["app"].toObject();
    qDebug() << config.PORT;
    return config;
}

int main() {
    Config conf = LoadConfig("../configs/conf.json");
    try {

        boost::asio::io_context io_context;

        Server server(io_context, conf);
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