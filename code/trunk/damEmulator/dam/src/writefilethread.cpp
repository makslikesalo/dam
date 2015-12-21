#include "writefilethread.h"

#include "osapi.h"
#include "fileapi.h"

#define LOCK_FILE_TIMEOUT 5000          // Таймаут ожидания блокировки мьютекса файла, мс
#define CACHE_BUFFER_SIZE 30            // Размер кэша буфера FIFO

// Обработчик потока записи в файл.
// Работает следующим образом:
// 1. Смотрит буфер FIFO, если он не пуст, то происходит
//   перенос данных буфера FIFO во временный буфер (кэш). Причём во временный
//   буфер переносятся не все данные, а только те, которые в него помещаются.
// 2. Если во временном буфере имеются данные, то они записываются в файл.
// 3. Если после записи данных в файл, в буфере FIFO есть ещё данные,
//   то повторяется запись данных до тех пор, пока в буфере FIFO не останется данных.
void writeFileMain(void *param)
{
    // assert (param)
    // assert (param->args.fifo)

    WriteFileParams *writeFileParams = (WriteFileParams *) param;

    // Задать состояние потока "Работает"
    writeFileParams->state = Thread_Running_State;

    // Сборосить коды ошибок
    writeFileParams->ret.error = WriteFile_No_Error;
    writeFileParams->ret.errorCode = 0;

    // Заблокировать файл
    if (os_lockGet(writeFileParams->sync.fileLockHnd, LOCK_FILE_TIMEOUT)) {

        int repeatWrite = 0;          // Повторять запись в файл, 1 - повторять

        do {
            DataBlock cachedData[CACHE_BUFFER_SIZE];       // Кэш FIFO
            uint32 cacheSize = 0;       // Кол-во элементов в кэше

            // 1. Перенести некоторое количество данных из FIFO
            // во временный буфер cachedData
            if (os_lockGet(writeFileParams->sync.fifoLockHnd, LOCK_TIMEOUT)) {
                // Если буфер FIFO не пуст
                if (!fifoIsEmpty(writeFileParams->args.fifo)) {

                    // Пока буфер FIFO не пуст N
                    // Кэш не заполнен полностью
                    while (!fifoIsEmpty(writeFileParams->args.fifo) &&
                           cacheSize < CACHE_BUFFER_SIZE) {
                        // Вычитать элементы данных из буфера FIFO
                        cachedData[cacheSize].value = fifoTakeFirst(writeFileParams->args.fifo);
                        ++cacheSize;
                    }
                }
                os_lockRelease(writeFileParams->sync.fifoLockHnd);
            }

            // 2. Если имеются данные для записи
            if (cacheSize) {
                // Записать закэшированные данные в файл
                int stat = fileApi_writeDataToFile(writeFileParams->args.filePath,
                                        cachedData,
                                        cacheSize,
                                        writeFileParams->args.maxFileCount);
                if (stat) {     // Если произошла ошибка записи в файл
                    writeFileParams->ret.error = WriteFile_Write_Error;
                    writeFileParams->ret.errorCode = stat;
                    break;  // Выйти из цикла повторения записи файла
                }
            }

            // 3. Проверить, имеются ли в буфере FIFO ещё данные
            if (os_lockGet(writeFileParams->sync.fifoLockHnd, LOCK_TIMEOUT)) {
                repeatWrite = !fifoIsEmpty(writeFileParams->args.fifo);
                os_lockRelease(writeFileParams->sync.fifoLockHnd);
            }
            else
                repeatWrite = 0;

        } while (repeatWrite && !writeFileParams->control.exitFlag);
        // То тех пор, пока нужно повторять запись

        os_lockRelease(writeFileParams->sync.fileLockHnd);
    }

    // Задать состояние потока "Завершено"
    writeFileParams->state = Thread_Finished_State;
}
