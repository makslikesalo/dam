#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QObject>
#include <QPointer>
#include <QtNetwork/QTcpSocket>

class TcpClient : public QObject
{
    Q_OBJECT
public:
    explicit TcpClient(QObject *parent = 0);

signals:

public slots:

private slots:
    void onConnected(void);
    void onDisconnected(void);
    void onError(QAbstractSocket::SocketError);

    void onReadyRead(void);

private:
    QPointer<QTcpSocket> socket;
    int id;
};

#endif // TCPCLIENT_H
