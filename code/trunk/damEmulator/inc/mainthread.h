#ifndef MAINTHREAD_H
#define MAINTHREAD_H

#include <QThread>
#include <QDebug>

extern void damMain(void);      // Главная процедура DAM
extern int exitDam;             // Флаг для завершения работы DAM

// MainThread - класс главного исполняемого потока DAM.
// Предназначен для запуска в отдельном потоке главной процедуры damMain().
class MainThread : public QThread
{
    Q_OBJECT

public:
    MainThread(QObject *parent = 0) : QThread(parent) {}
    virtual ~MainThread() {
        exitDam = 1;            // Оповестить DAM, чтобы он деинициализировался
        if(!wait(5000))         // Подождать, чтобы поток завершил деинициализацию (макс. 5 сек)
        {
            // Если поток не деинициализировался в установленное время
            qWarning("Thread deadlock detected");
            terminate();        // Принудительно завершим поток
            wait();             // Ожидать принудительного завершения потока
        }
    }

protected:
    void run(void) {
        damMain();
    }
};

#endif // MAINTHREAD_H
