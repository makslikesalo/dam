#include <QCoreApplication>

#include <QDebug>

#include <winsock.h>
#include <string>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    WSADATA ws;     // Объект инициализации

    WORD wVersionRequested = MAKEWORD(2, 2);
    int stat = WSAStartup(wVersionRequested, &ws);
    if (stat) {
        qDebug() << "Error WSAStartup:" << stat;
        return 0;
    }

    // Создать сокет
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        qDebug() << "Error to create socket:" << WSAGetLastError();
        WSACleanup();
        return 0;
    }


    // Параметры соединения
    sockaddr_in server;                          // Объект для хранения адреса
    memset(&server, 0, sizeof(sockaddr_in));     // Обнулить объект
    server.sin_family = AF_INET;                 // TCP/IP
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons(56789);

    // Связать порт
    stat = bind(sock, (SOCKADDR  *) &server, sizeof(server));
    if (stat == SOCKET_ERROR) {
        closesocket(sock);
        qDebug() << "Error to bind socket:" << WSAGetLastError();
        WSACleanup();
        return 0;
    }

    // Слушать входящие соединения
    stat = listen(sock, SOMAXCONN);
    if (stat == SOCKET_ERROR) {
        closesocket(sock);
        qDebug() << "Error to listen socket:" << WSAGetLastError();
        WSACleanup();
        return 0;
    }

    // Создать сокет для входящих сооединений
    SOCKET acceptSocket = accept(sock, NULL, NULL);
    if (stat == INVALID_SOCKET) {
        closesocket(sock);
        qDebug() << "Error to accept socket:" << WSAGetLastError();
        WSACleanup();
        return 0;
    }

    // Получить данные от клиента
    int recvX = 0;
    stat = recv(acceptSocket, (char *) &recvX, sizeof(int), 0);
    if (stat > 0)             // Данные приняты
        qDebug() << "In data:" << recvX;
    else if (stat == 0)       // Соединение закрыто клиентом
        qDebug() << "Connection closed";
    else
        qDebug() << "Error to receive:" << WSAGetLastError();

    // Отослать данные клиенту
    int sendX = 123456;
    stat = send(acceptSocket, (char *) &sendX, sizeof(int), 0);
    if (stat == SOCKET_ERROR) {
        closesocket(sock);
        qDebug() << "Error to send:" << WSAGetLastError();
        WSACleanup();
        return 0;
    }

    // Разорвать соединение на передачу (оно больше не требуется)
    stat = shutdown(acceptSocket, 1);   // 1 - SD_SEND - Shutdown send operations
    if (stat == SOCKET_ERROR) {
        qDebug() << "Error to shutdown:" << WSAGetLastError();
        closesocket(sock);
        WSACleanup();
        return 0;
    }

    // Cleanup
    closesocket(sock);
    WSACleanup();

    return a.exec();
}

