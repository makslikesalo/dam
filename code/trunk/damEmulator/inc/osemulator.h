#ifndef OSEMULATOR_H
#define OSEMULATOR_H

#include <QThread>
#include <QRunnable>
#include <QMutex>
#include <QMap>
#include <QTime>

#include "types.h"

// Класс экземпляра потока ОС
class OsThread : public QThread
{
    Q_OBJECT
    QRunnable *runnable;        // Объект с процедурой, которую нужно запустить в отдельном потоке

public:
    OsThread(QRunnable *r, QObject *parent = 0) : QThread(parent), runnable(r) {}

    virtual ~OsThread() {
        if (runnable)
            delete runnable;
    }

protected:
    void run(void) {
        if (runnable)
            runnable->run();
    }
};


// Класс эмулятора ОС (реализует паттерн - singleton)
class OsEmulator
{
    QMap<uint32, QThread *> threadsMap;         // Ассоциативный массив потоков
    uint32 hndCounter;                          // Счётчик обработчиков потоков
    QMap<uint32, QMutex *> mutexesMap;          // Ассоциативный массив мьютексов
    uint32 mutexesHndCounter;                   // Счётчик обработчиков мьютексов

    QTime startOsTime;                          // Время запуска эмулятора

    // Запретить создание объекта класса
    OsEmulator() : hndCounter(0), mutexesHndCounter(0) { startOsTime.start(); }
    OsEmulator(const OsEmulator&);
    OsEmulator& operator= (OsEmulator&);

public:
    ~OsEmulator()
    {
        foreach (QThread *t, threadsMap) {  // Остановить запущенные потоки
            if (t && t->isRunning()) {
                t->terminate();
                t->wait();
            }
        }
        qDeleteAll(threadsMap);
        qDeleteAll(mutexesMap);
    }

    // Возвратить глобальный объект
    static OsEmulator& getInstance(void) {
        static OsEmulator instance;
        return instance;

    }

    // Добавить задачу и вернуть её handler, 0 - Failed
    uint32 append(QRunnable *r) {
        Q_ASSERT(r);
        hndCounter++;
        threadsMap.insert(hndCounter, new OsThread(r, NULL));
        return hndCounter;
    }

    // Запустить поток с указанным идентификатором hnd,
    // flagStart = 0 - приостановить поток (реализовано не приостановление, а уничтожение потока,
    //                 ввиду отсутствия инструмента в QT).
    // flagStart = 1 - запустить поток
    // Возвращает:
    // 0 - операция не выполнена,
    // не 0 - операция успешно выполнена
    //
    // Если поток hnd существует
    // То
    //   Если поток нужно запустить (flagStart = 1)
    //   То
    //     Если поток не запущен
    //       Запускает поток
    //       Если поток запустился
    //       То возвращает 1
    //       Иначе возвращает 0
    //     Иначе [поток запущен]
    //       То возвращает 0
    //   Иначе Если поток нужно приостановить (flagStart = 0)
    //   То
    //     Если поток не запущен
    //     То возвратить 0
    //     Иначе [Если поток запущен]
    //        Приостановить поток
    //        Возвратить 1
    // Иначе [поток hnd не существует]
    // То возвращает 0
    int start(uint32 hnd, int flagStart) {
        QThread *t = threadsMap.value(hnd, NULL);
        if (t) {
            if (flagStart == 1) {       // Если поток нужно запустить
                if (!t->isRunning()) {  // Если поток не запущен
                    t->start();         // Запустить поток
                    return 1;           // Поток успешно запустился (нет возможности проверить так ли это)
                }
                else                    // Если поток запущен
                    return 0;
            }
            else if (flagStart == 0) {  // Если поток нужно остановить
                if (t->isRunning()) {   // Если поток запущен
                    t->terminate();
                    return 1;           // Поток успешно остановлен
                }
                else                    // Если поток не запущен
                    return 0;
            }
        }
        return 0;       // 0 - Failed
    }


    // Создать мьютекс и вернуть его handler, 0 - Failed
    uint32 createMutex(void) {
        mutexesHndCounter++;
        mutexesMap.insert(mutexesHndCounter, new QMutex());
        return mutexesHndCounter;
    }

    // Попытаться заблокировать мьютекс
    // 1 - ok, 0 - timeout or error
    int tryLockMutex(uint32 hnd, uint32 timeoutMs) {
        QMutex *m = mutexesMap.value(hnd, NULL);
        if (m)
            return m->tryLock(timeoutMs) ? 1 : 0;
        return 0;
    }

    // Освободить мьютекс
    void unlockMutex(uint32 hnd) {
        QMutex *m = mutexesMap.value(hnd, NULL);
        if (m)
            m->unlock();
    }


    // Получить количество мс с момента запуска эмулятора
    int getElapsedTime(void) const {
        return startOsTime.elapsed();
    }
};


#endif // OSEMULATOR_H
