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
    sockaddr_in clientService;                          // Объект для хранения адреса
    memset(&clientService, 0, sizeof(sockaddr_in));     // Обнулить объект
    clientService.sin_family = AF_INET;                 // TCP/IP
    clientService.sin_addr.s_addr = inet_addr("127.0.0.1");
    clientService.sin_port = htons(56789);

    // Установить соединение
    stat = connect(sock, (SOCKADDR  *) &clientService, sizeof(clientService));
    if (stat == SOCKET_ERROR) {
        closesocket(sock);
        qDebug() << "Error to connect socket:" << WSAGetLastError();
        WSACleanup();
        return 0;
    }

    // Отослать данные на сервер
    int sendX = 987654;
    stat = send(sock, (char *) &sendX, sizeof(int), 0);
    if (stat == SOCKET_ERROR) {
        closesocket(sock);
        qDebug() << "Error to send:" << WSAGetLastError();
        WSACleanup();
        return 0;
    }

    // Разорвать соединение на передачу (оно больше не требуется)
    stat = shutdown(sock, 1);   // 1 - SD_SEND - Shutdown send operations
    if (stat == SOCKET_ERROR) {
        qDebug() << "Error to shutdown:" << WSAGetLastError();
        closesocket(sock);
        WSACleanup();
        return 0;
    }

    // Получить данные от сервера
    int recvX = 0;
    stat = recv(sock, (char *) &recvX, sizeof(int), 0);
    if (stat > 0)             // Данные приняты
        qDebug() << "In data:" << recvX;
    else if (stat == 0)       // Соединение закрыто сервером
        qDebug() << "Connection closed";
    else
        qDebug() << "Error to receive:" << WSAGetLastError();

    closesocket(sock);
    WSACleanup();

    return a.exec();
}

