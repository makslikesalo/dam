#include "acquisitionthread.h"

#include "osapi.h"
#include "sensorapi.h"
#include "fifo.h"
#include "notifythread.h"
#include "writefilethread.h"
#include "utils.h"

#define POLL_SENSOR_INTERVAL 10000  // Интервал опроса, мс датчика
#define THRESHOLD_VALUE 10000       // Пороговое значение значения датчика


// Произвести сбор данных
static void execAcquisition(AcquisitionParams *acquisitionParams,
                            uint32 notifyThreadHnd,
                            NotifyParams *notifyParams,
                            uint32 writeFileThreadHnd,
                            WriteFileParams *writeFileParams,
                            uint32 fifoLockHnd,
                            Fifo *fifo);

// Поток сбора данных
void acquisitionMain(void *param)
{
    // assert (acquisitionParams)

    AcquisitionParams *acquisitionParams = (AcquisitionParams *) param;

    // Задать состояние потока "Работает"
    acquisitionParams->state = Thread_Running_State;

    // Буфер FIFO
    Fifo fifo;
    memset(&fifo, 0, sizeof(fifo));
    char fifoBuf[FIFO_SIZE];
    fifo.buf = fifoBuf;
    fifo.bufSize = FIFO_SIZE;
    // Создать мьютекс доступа к буферу FIFO
    uint32 fifoLockHnd = os_lockCreate();

    // Поток оповещения
    // Параметры
    NotifyParams notifyParams;
    memset(&notifyParams, 0, sizeof(notifyParams));
    notifyParams.args.lastClientAddr = acquisitionParams->args.lastClientAddr;
    notifyParams.sync.lastClientLockHnd = acquisitionParams->sync.lastClientLockHnd;
    // Создать поток
    uint32 notifyThreadHnd = os_threadCreate(notifyMain, &notifyParams, 0, NULL, 0);

    // Поток записи в файл
    // Параметры
    WriteFileParams writeFileParams;
    memset(&writeFileParams, 0, sizeof(writeFileParams));
    writeFileParams.args.fifo = &fifo;
    writeFileParams.args.filePath = acquisitionParams->args.filePath;
    writeFileParams.args.maxFileCount = acquisitionParams->args.maxFileCount;
    writeFileParams.sync.fifoLockHnd = fifoLockHnd;
    writeFileParams.sync.fileLockHnd = acquisitionParams->sync.fileLockHnd;
    // Создать поток
    uint32 writeFileThreadHnd = os_threadCreate(writeFileMain, &writeFileParams, 0, NULL, 0);

    // Зафиксировать время следующего измерения
    uint32 currentTime = os_timeGet();                          // Текущее время
    uint32 timeoutTime = currentTime + POLL_SENSOR_INTERVAL;    // Время таймаута

    // Произвести первоначальный сбор данных
    execAcquisition(acquisitionParams,
                    notifyThreadHnd,
                    &notifyParams,
                    writeFileThreadHnd,
                    &writeFileParams,
                    fifoLockHnd,
                    &fifo);

    // Цикл ожидания таймаута измерения
    while (!acquisitionParams->control.exitFlag) {
        currentTime = os_timeGet();
        if (currentTime >= timeoutTime) {
            timeoutTime = currentTime + POLL_SENSOR_INTERVAL;
            // Произвести сбор данных
            execAcquisition(acquisitionParams,
                            notifyThreadHnd,
                            &notifyParams,
                            writeFileThreadHnd,
                            &writeFileParams,
                            fifoLockHnd,
                            &fifo);
        }
    }

    // Деинициализация
    notifyParams.control.exitFlag = 1;          // Завершить поток оповещения
    writeFileParams.control.exitFlag = 1;       // Заврешить поток записи в файл

    // Подождать завершения потоков
    damWait(WAIT_DEINIT_THREAD, NULL);

    // Принудительно завершить потоки
    os_threadStart(notifyThreadHnd, 0);
    os_threadStart(writeFileThreadHnd, 0);

    // Задать состояние потока "Завершен"
    acquisitionParams->state = Thread_Finished_State;
}


// Произвести сбор данных
void execAcquisition(AcquisitionParams *acquisitionParams,
                     uint32 notifyThreadHnd,
                     NotifyParams *notifyParams,
                     uint32 writeFileThreadHnd,
                     WriteFileParams *writeFileParams,
                     uint32 fifoLockHnd,
                     Fifo *fifo)
{
    uint32 sensorData = 0;      // Измеренное с датчика значение

    if (os_lockGet(acquisitionParams->sync.sensorLockHnd, LOCK_TIMEOUT)) {
        sensorData = io_sensorRead();       // Прочитать датчик
        os_lockRelease(acquisitionParams->sync.sensorLockHnd);
    }
    else
        return;

    if (os_lockGet(fifoLockHnd, LOCK_TIMEOUT)) {
        // Если FIFO не полон
        if (!fifoIsFull(fifo))
            fifoAppend(fifo, sensorData);     // Вставить значение в буфер FIFO
        os_lockRelease(fifoLockHnd);

        // Поток записи не занят?
        ThreadState state = writeFileParams->state;
        if (state == Thread_Unknown_State ||
            state == Thread_Finished_State) {

            // Если поток был завершём, дадим ему ещё время
            // для деинициализации
            if (state == Thread_Finished_State)
                damWait(WAIT_FINISHED_THREAD, NULL);

            // Запустить поток
            writeFileParams->state = Thread_Started_State;
            os_threadStart(writeFileThreadHnd, 1);
        }
    }

    // Значение больше порога?
    if (sensorData > THRESHOLD_VALUE) {
        // Поток оповещения не занят?
        ThreadState state = notifyParams->state;
        if (state == Thread_Unknown_State ||
            state == Thread_Finished_State) {

            // Если поток оповещения был завершём, дадим ему ещё время
            // для деинициализации
            if (state == Thread_Finished_State)
                damWait(WAIT_FINISHED_THREAD, NULL);

            // Оповестить о превышении порога
            notifyParams->args.value = sensorData;

            // Запустить поток оповещения
            notifyParams->state = Thread_Started_State;
            os_threadStart(notifyThreadHnd, 1);
        }
    }
}
