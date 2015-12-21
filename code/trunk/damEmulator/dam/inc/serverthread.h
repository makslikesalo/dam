#ifndef SERVERTHREAD
#define SERVERTHREAD

#include "types.h"
#include <winsock.h>

// Модуль работы с потоком сервера

// Ошибки работы модуля
typedef enum {
    Server_No_Error = 0,
    Server_Wsa_Startup_Error,
    Server_Create_Socket_Error,
    Server_Bind_Error,
    Server_Listen_Error,
    Server_Accept_Error,
    Server_Receive_Error,
    Server_Send_Error,
    Server_Shutdown_Error
} ServerError;

// Параметры потока сервера
typedef struct {
    // Общие параметры
    struct {
        const char *filePath;       // Путь к файлу, в который будет производиться запись данных
        uint32 maxFileCount;        // Максимальное кол-во элементов данных в файле
        sockaddr serverAddr;        // Параметры сервера
        sockaddr *lastClientAddr;   // Адрес последнего подключившегося клиента
    } args;

    // Системные параметры
    struct {
        int exitFlag;   // Управляющий флаг для завершения работы потока, 1 - завершить
    } control;

    // Примитивы синхронизации
    struct {
        uint32 sensorLockHnd;       // Хендлер мьютекса обращения к датчику
        uint32 fileLockHnd;         // Хендлер мьютекса обращения к файлу
        uint32 lastClientLockHnd;   // Хендлер мьютекса обращения к адресу клиента
    } sync;

    // Данные, которые возвращает поток по завершении выполнения
    struct {
        ServerError error;      // Тип ошибки
        int errorCode;          // Код ошибки для указанного типа ошибки
    } ret;

    ThreadState state;  // Состояние потока
} ServerParams;

// Процедура потока сервера
void serverMain(void *param);


#endif // SERVERTHREAD

