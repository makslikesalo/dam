#include "notifythread.h"

#include "osapi.h"
#include "utils.h"

#define THRESHOLD_CMD_CODE 0x00000001       // Код команды события превышения порога

#define CONNECTION_TIMEOUT_PAUSE 500       // Таймаут между попытками соединения с сервером
#define CONNECTION_ATTEMPTS_COUNT 5         // Кол-во попыток соединения с сервером

// Выполнить обмен данными, 1 - успешно, 0 - провал
static int makeExchange(NotifyParams *notifyParams);

// Обработчик потока оповещения
void notifyMain(void *param)
{
    // assert (param)
    NotifyParams *notifyParams = (NotifyParams *) param;

    // assert (notifyParams->args.lastClientAddr != 0)

    // Задать состояние потока "Работает"
    notifyParams->state = Thread_Running_State;

    // Попробовать выполнить обмен данными некоторое количество
    // раз с паузами между попытками
    int attempts = CONNECTION_ATTEMPTS_COUNT;   // Кол-во попыток соединения с сервером

    while (attempts > 0 && !notifyParams->control.exitFlag) {
        if (makeExchange(notifyParams))         // Выполнить обмен
            break;

        if (--attempts > 0)                     // Если есть ещё попытки
            damWait(CONNECTION_TIMEOUT_PAUSE,
                    &(notifyParams->control.exitFlag));   // Выждать паузу
    }

    // Задать состояние потока "Завершено"
    notifyParams->state = Thread_Finished_State;
}


// Выполнить обмен данными, 1 - успешно, 0 - провал
int makeExchange(NotifyParams *notifyParams)
{   
    // Установить соединение с последним клиентом
    WSADATA ws;     // Объект инициализации

    WORD wVersionRequested = MAKEWORD(2, 2);
    int stat = WSAStartup(wVersionRequested, &ws);
    if (stat) {
        notifyParams->ret.error = Notify_Wsa_Startup_Error;
        notifyParams->ret.errorCode = stat;
        return 0;
    }

    // Создать сокет
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        notifyParams->ret.error = Notify_Create_Socket_Error;
        notifyParams->ret.errorCode = WSAGetLastError();
        WSACleanup();
        return 0;
    }

    // Установить соединение
    // Адрес сервера
    sockaddr_in serverAddr;                             // Объект для хранения адреса
    memset(&serverAddr, 0, sizeof(sockaddr_in));        // Обнулить объект

    if (os_lockGet(notifyParams->sync.lastClientLockHnd, LOCK_TIMEOUT)) {
        // Сформировать адрес и порт сервера
        memcpy(&serverAddr, notifyParams->args.lastClientAddr, sizeof(sockaddr));
        serverAddr.sin_port = htons(REMOTE_SERVER_PORT);

        stat = connect(sock,
                       (SOCKADDR *) &serverAddr,
                       sizeof(sockaddr));
        os_lockRelease(notifyParams->sync.lastClientLockHnd);
    }
    else {
        notifyParams->ret.error = Notify_Joint_Access_Mutex_Error;
        notifyParams->ret.errorCode = 0;
        closesocket(sock);
        WSACleanup();
        return 0;
    }

    if (stat == SOCKET_ERROR) {
        notifyParams->ret.error = Notify_Connect_Socket_Error;
        notifyParams->ret.errorCode = WSAGetLastError();
        closesocket(sock);
        WSACleanup();
        return 0;
    }

    #pragma pack(push, 1)
    struct {
        uint32 cmd;
        uint32 value;
    } sendData;
    #pragma pack(pop)

    sendData.cmd = THRESHOLD_CMD_CODE;
    sendData.value = notifyParams->args.value;

    // Отослать данные на сервер
    stat = send(sock, (char *) &sendData, sizeof(sendData), 0);
    if (stat == SOCKET_ERROR) {
        notifyParams->ret.error = Notify_Send_Data_Error;
        notifyParams->ret.errorCode = WSAGetLastError();
        closesocket(sock);
        WSACleanup();
        return 0;
    }

    // Закрыть соединение
    closesocket(sock);
    WSACleanup();

    notifyParams->ret.error = Notify_No_Error;
    notifyParams->ret.errorCode = 0;

    return 1;
}
