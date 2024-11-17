#include "hello_window.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    HelloWindow w;
    w.show();
    return a.exec();
}