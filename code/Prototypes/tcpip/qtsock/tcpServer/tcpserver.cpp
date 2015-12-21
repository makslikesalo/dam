#include "tcpserver.h"

#include <QtNetwork/QHostAddress>
#include <QtNetwork/QTcpSocket>
#include <QTextStream>
#include <QDataStream>

#include <QDebug>

TcpServer::TcpServer(QObject *parent) : QObject(parent)
{
    server = new QTcpServer(this);
    connect(server, SIGNAL(newConnection()), this, SLOT(onNewConnection()));
    if (!server->listen(QHostAddress::LocalHost, 56789)) {
        qDebug() << "Fail to listen";
    }
    else
        qDebug() << "Start server";
}


void TcpServer::onNewConnection(void) {
    client = server->nextPendingConnection();
    if (client) {
        connect(client, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
        connect(client, SIGNAL(disconnected()), client, SLOT(deleteLater()));
    }
}


void TcpServer::onReadyRead(void) {
    // Принять данные

    // Принять данные
    int val = 0;
    {
        QByteArray ba = client->read(1024);
        QDataStream ds(&ba, QIODevice::ReadWrite);
        ds >> val;
        qDebug() << val;
    }

    QByteArray ba;
    QDataStream ds(&ba, QIODevice::ReadWrite);
    val++;
    ds << val;

    client->write(ba);
}




