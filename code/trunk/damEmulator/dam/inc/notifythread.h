#ifndef NOTIFYTHREAD
#define NOTIFYTHREAD

#include "types.h"

#include <winsock.h>

// Модуль работы с потоком оповещения

// Ошибки работы модуля
typedef enum {
    Notify_No_Error = 0,
    Notify_Wsa_Startup_Error,
    Notify_Create_Socket_Error,
    Notify_Connect_Socket_Error,
    Notify_Send_Data_Error,
    Notify_Joint_Access_Mutex_Error     // Ошибка совместного доступа
} NotifyError;

// Параметры потока оповещения
typedef struct {
    // Общие параметры
    struct {
        uint32 value;               // Значение, о котором следует сообщить клиенту
        sockaddr *lastClientAddr;   // Адрес последнего подключившегося клиента
    } args;

    // Системные параметры
    struct {
        int exitFlag;               // Управляющий флаг для завершения работы потока, 1 - завершить
    } control;

    // Примитивы синхронизации
    struct {
        uint32 lastClientLockHnd;   // Хендлер мьютекса обращения к адресу клиента
    } sync;

    // Данные, которые возвращает поток по завершении выполнения
    struct {
        NotifyError error;      // Тип ошибки
        int errorCode;          // Код ошибки для указанного типа ошибки
    } ret;

    ThreadState state;          // Состояние потока
} NotifyParams;

// Обработчик потока оповещения
void notifyMain(void *param);

#endif // NOTIFYTHREAD

