#ifndef MAINTHREAD_H
#define MAINTHREAD_H

#include <QThread>

extern void dadMain(void);      // Главная процедура DAD

// MainThread - класс главного исполняемого потока DAD.
// Предназначен для запуска в отдельном потоке главной процедуры dadMain().
class MainThread : public QThread
{
    Q_OBJECT

public:
    MainThread(QObject *parent = 0) : QThread(parent) {}
    virtual ~MainThread() {
    }

protected:
    void run(void) {
        dadMain();
    }
};

#endif // MAINTHREAD_H
