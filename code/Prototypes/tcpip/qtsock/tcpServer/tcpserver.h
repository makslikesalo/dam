#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QObject>
#include <QPointer>
#include <QtNetwork/QTcpServer>

class TcpServer : public QObject
{
    Q_OBJECT
public:
    explicit TcpServer(QObject *parent = 0);

signals:

private slots:
    void onNewConnection(void);

    void onReadyRead(void);

private:
    QPointer<QTcpServer> server;
    QPointer<QTcpSocket> client;
};


#endif // TCPSERVER_H
