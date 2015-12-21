#ifndef ACQUISITIONTHREAD
#define ACQUISITIONTHREAD

#include "types.h"
#include "winsock.h"

// Модуль работы с потоком сбора данных

// Параметры потока сбора данных
typedef struct {
    // Общие параметры
    struct {
        const char *filePath;       // Путь к файлу, в который будет производиться запись данных
        uint32 maxFileCount;        // Максимальное кол-во элементов данных в файле
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

    ThreadState state;  // Состояние потока
} AcquisitionParams;

// Процедура потока сбора данных
void acquisitionMain(void *param);

#endif // ACQUISITIONTHREAD

