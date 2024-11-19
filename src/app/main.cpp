#include "hello_window.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    boost::asio::io_context io_context;
    HelloWindow w;
    w.show();
    return a.exec();
}