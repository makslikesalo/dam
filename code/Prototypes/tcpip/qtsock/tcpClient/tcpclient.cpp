#include "tcpclient.h"

#include <QtNetwork/QHostAddress>

#include <QDataStream>
#include <QTime>
#include <QDebug>

TcpClient::TcpClient(QObject *parent) : QObject(parent), id(0)
{
    socket = new QTcpSocket(this);
    connect(socket, SIGNAL(connected()), this, SLOT(onConnected()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(onError(QAbstractSocket::SocketError)));
    connect(socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    socket->connectToHost(QHostAddress::LocalHost, 56789);
    qDebug() << "Start client";
}


void TcpClient::onConnected(void) {

    // Отослать запрос
    id = QTime::currentTime().msec();

    QByteArray ba;
    QDataStream ds(&ba, QIODevice::ReadWrite);
    ds << id;
    socket->write(ba);
}

void TcpClient::onReadyRead(void) {
    // Принять данные
    int val = 0;
    {
        QByteArray ba = socket->read(1024);
        QDataStream ds(&ba, QIODevice::ReadWrite);
        ds >> val;
        qDebug() << val;
        if (val > (id + 10)) {
            socket->disconnectFromHost();
            return;
        }
    }

    QByteArray ba;
    QDataStream ds(&ba, QIODevice::ReadWrite);
    val++;
    ds << val;

    socket->write(ba);

}

void TcpClient::onDisconnected(void) {
    qDebug() << "Disconnected";
}

void TcpClient::onError(QAbstractSocket::SocketError socketError) {
    qDebug() << "Error" << socketError;
}

