#ifndef WRITEFILETHREAD
#define WRITEFILETHREAD

#include "types.h"
#include "fifo.h"

// Модуль работы с потоком записи данных в файл

// Ошибки работы модуля
typedef enum {
    WriteFile_No_Error = 0,
    WriteFile_Write_Error
} WriteFileError;

// Параметры потока оповещения
typedef struct {
    // Общие параметры
    struct {
        Fifo *fifo;             // Указатель на объект буфера FIFO
        const char *filePath;   // Путь к файлу для записи данных
        uint32 maxFileCount;    // Максимальное кол-во элементов данных в файле
    } args;

    // Системные параметры
    struct {
        int exitFlag;               // Управляющий флаг для завершения работы потока, 1 - завершить
    } control;

    // Примитивы синхронизации
    struct {
        uint32 fifoLockHnd;         // Хендлер мьютекса обращения к буферу FIFO
        uint32 fileLockHnd;         // Хендлер мьютекса обращения к файлу
    } sync;

    // Данные, которые возвращает поток по завершении выполнения
    struct {
        WriteFileError error;      // Тип ошибки
        int errorCode;          // Код ошибки для указанного типа ошибки
    } ret;

    ThreadState state;          // Состояние потока
} WriteFileParams;

// Обработчик потока записи в файл
void writeFileMain(void *param);


#endif // WRITEFILETHREAD

