#include "hello_window.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QFile file(":/styles/hello.qss");
    if (file.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(file.readAll());
        app.setStyleSheet(styleSheet);
    } else {
        qDebug() << "Failed to open style file";
    }
    boost::asio::io_context io_context;
    HelloWindow w;
    w.show();
    return app.exec();
}