#include "serverthread.h"

#include "fileapi.h"
#include "osapi.h"
#include "sensorapi.h"

#define READ_SENSOR_CMD_CODE 0x00000002     // Код команды чтения данных с датчика
#define READ_FILE_CMD_CODE 0x00000003       // Код команды чтения данных из файла

#define READ_FILE_DATA_COUNT 64             // Количество элементов данных, которые требуется прочитать
                                            // при соответствующем запросе клиента

// Выполнить работу сервера
static void execServer(ServerParams *serverParams);

// Обрабатывать входящее соединение
static int handleConnection(SOCKET listenSocket, ServerParams *serverParams);

// Процедура потока сервера
void serverMain(void *param)
{
    // assert (param)

    ServerParams *serverParams = (ServerParams *) param;

    // Задать состояние потока "Работает"
    serverParams->state = Thread_Running_State;

    execServer(serverParams);       // Выполнить программу сервера

    // Задать состояние потока "Завершено"
    serverParams->state = Thread_Finished_State;
}


// Выполнить работу сервера
void execServer(ServerParams *serverParams)
{
    // 1. Инициализировать TCP/IP сервер

    WSADATA ws;     // Объект инициализации

    WORD wVersionRequested = MAKEWORD(2, 2);
    int stat = WSAStartup(wVersionRequested, &ws);
    if (stat) {
        serverParams->ret.error = Server_Wsa_Startup_Error;
        serverParams->ret.errorCode = stat;
        return;
    }

    // Создать сокет
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        serverParams->ret.error = Server_Create_Socket_Error;
        serverParams->ret.errorCode = WSAGetLastError();
        WSACleanup();
        return;
    }

    // Связать порт
    stat = bind(sock,
                &(serverParams->args.serverAddr),
                sizeof(serverParams->args.serverAddr));
    if (stat == SOCKET_ERROR) {
        serverParams->ret.error = Server_Bind_Error;
        serverParams->ret.errorCode = WSAGetLastError();
        closesocket(sock);
        WSACleanup();
        return;
    }

    // Слушать входящие соединения
    stat = listen(sock, SOMAXCONN);
    if (stat == SOCKET_ERROR) {
        serverParams->ret.error = Server_Listen_Error;
        serverParams->ret.errorCode = WSAGetLastError();
        closesocket(sock);
        WSACleanup();
        return;
    }

    // Обрабатывать подключения
    while (!serverParams->control.exitFlag) {
        stat = handleConnection(sock, serverParams);
    }

    // 3. Деинициализировать сервер
    closesocket(sock);
    WSACleanup();
}


// Обрабатывать входящее соединение
int handleConnection(SOCKET listenSocket, ServerParams *serverParams)
{
    sockaddr clientAddr;     // Адрес клиента, который подключился
    int clientAddrLen = 0;

    // Принять входящее соединение
    SOCKET clientSocket = accept(listenSocket, (sockaddr *) &clientAddr, &clientAddrLen);
    if (clientSocket == INVALID_SOCKET) {
        return WSAGetLastError();
    }

    // Сохранить адрес подключившегося клиента
    if (os_lockGet(serverParams->sync.lastClientLockHnd, LOCK_TIMEOUT)) {
        memcpy(serverParams->args.lastClientAddr, &clientAddr, sizeof(clientAddr));
        os_lockRelease(serverParams->sync.lastClientLockHnd);
    }

    // Получить данные от клиента
    uint32 cmdRecv = 0;     // Принятая команда
    int stat = recv(clientSocket, (char *) &cmdRecv, sizeof(cmdRecv), 0);
    if (stat > 0) {            // Данные приняты
        if ((uint32) stat < sizeof(cmdRecv)) {       // Если данных недостаточно
            stat = WSAGetLastError();
            closesocket(clientSocket);
            return stat;
        }

    }
    else if (stat == 0) {      // Соединение закрыто клиентом
        closesocket(clientSocket);
        return 0;
    }
    else {
        stat = WSAGetLastError();
        closesocket(clientSocket);
        return stat;
    }

    // Выполнить команду
    if (cmdRecv == READ_SENSOR_CMD_CODE) {      // Если нужно прочитать датчик

        if (os_lockGet(serverParams->sync.sensorLockHnd, LOCK_TIMEOUT)) {
            uint32 sensorData = io_sensorRead();       // Прочитать датчик
            os_lockRelease(serverParams->sync.sensorLockHnd);

            // Отправить прочитанное значение клиенту
            stat = send(clientSocket, (char *) &sensorData, sizeof(uint32), 0);
            if (stat == SOCKET_ERROR) {
                stat = WSAGetLastError();
                closesocket(clientSocket);
                return stat;
            }
        }
    }
    else if (cmdRecv == READ_FILE_CMD_CODE) {   // Если нужно прочитать файл

        if (os_lockGet(serverParams->sync.fileLockHnd, LOCK_TIMEOUT)) {
            DataBlock readedData[READ_FILE_DATA_COUNT];     // Массив данных
            uint32 readedCount = 0;                         // Кол-во прочитанных элементов
            stat = fileApi_readDataFromFile(serverParams->args.filePath,
                                     readedData,
                                     READ_FILE_DATA_COUNT,
                                     0,
                                     &readedCount);
            os_lockRelease(serverParams->sync.fileLockHnd);

            // Если чтение файла прошло успешно
            if (!stat) {
                // Отправить количество прочитанных данных
                stat = send(clientSocket, (char *) &readedCount, sizeof(readedCount), 0);
                if (stat == SOCKET_ERROR) {
                    stat = WSAGetLastError();
                    closesocket(clientSocket);
                    return stat;
                }

                // Отправить прочитанные данные клиенту
                stat = send(clientSocket, (char *) readedData, sizeof(DataBlock) * readedCount, 0);
                if (stat == SOCKET_ERROR) {
                    stat = WSAGetLastError();
                    closesocket(clientSocket);
                    return stat;
                }
            }

        }
    }

    // Закрыть соединение
    stat = shutdown(clientSocket, 1);   // 1 - SD_SEND - Shutdown send operations;
    if (stat == SOCKET_ERROR) {
        stat = WSAGetLastError();
        closesocket(clientSocket);
        return stat;
    }

    closesocket(clientSocket);
    return 0;
}
