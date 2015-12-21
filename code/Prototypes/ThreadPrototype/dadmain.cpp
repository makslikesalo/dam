#include <QMutex>
#include <QString>
#include <QDebug>

#include "types.h"
#include "osapi.h"

struct TaskParam {
    char sym;
    int count;
    QString *str;
    QMutex *stringMutex;
    QMutex *barrierMutex;
};

void task(void *param)
{
    if (!param)
        return;

    TaskParam *p = (TaskParam *) param;

    char sym = p->sym;
    int count = p->count;
    QString *str = p->str;

    for (int i = count; i > 0; --i) {
        //p->stringMutex->lock();
        (*str) += sym;
        for (int j = 0; j < 10000; ++j)
            noop;
        //p->stringMutex->unlock();
    }

    p->barrierMutex->unlock();
}

struct OutputTaskParam {
    QMutex *barrierMutexesArray;
    int barrierMutexesArraySize;
    QString *outputString;
};

void outputTask(void *param)
{
    if (!param)
        return;

    OutputTaskParam *p = (OutputTaskParam *) param;

    // Дождаться завершения задач
    for (int i = 0; i < p->barrierMutexesArraySize; ++i)
        (p->barrierMutexesArray)[i].lock();

    qDebug() << *(p->outputString);

    // Дождаться завершения задач
    for (int i = 0; i < p->barrierMutexesArraySize; ++i)
        (p->barrierMutexesArray)[i].unlock();
}


#define Tasks_Count 3

QString sharedString;
QMutex barrierMutexArray[Tasks_Count];
QMutex stringMutex;

TaskParam tasksArray[Tasks_Count] = {
    {'-', 1000, &sharedString, &stringMutex, &(barrierMutexArray[0])},
    {'I', 1000, &sharedString, &stringMutex, &(barrierMutexArray[1])},
    {'X', 1000, &sharedString, &stringMutex, &(barrierMutexArray[2])}
};

OutputTaskParam otp = {barrierMutexArray, Tasks_Count, &sharedString};

// Головная функция
void dadMain(void) {
    uint32 hnd1 = os_threadCreate(task, &tasksArray[0], 0, NULL, 0);
    uint32 hnd2 = os_threadCreate(task, &tasksArray[1], 0, NULL, 0);
    uint32 hnd3 = os_threadCreate(task, &tasksArray[2], 0, NULL, 0);
    uint32 hnd4 = os_threadCreate(outputTask, &otp, 0, NULL, 0);

    // Установить барьер
    for (int i = 0; i < Tasks_Count; ++i)
        barrierMutexArray[i].lock();

    os_threadStart(hnd1, 1);
    os_threadStart(hnd2, 1);
    os_threadStart(hnd3, 1);
    os_threadStart(hnd4, 1);
}

