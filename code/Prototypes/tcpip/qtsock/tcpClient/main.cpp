#include <QCoreApplication>

#include "tcpclient.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    TcpClient tcpClient;

    return a.exec();
}

