#include <QCoreApplication>

#include "mainthread.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // Чтобы эмулируемая процедура не блокировала главный поток приложения,
    // Запустим главную процедуру damMain в отдельном потоке.
    MainThread mt;
    mt.start(QThread::LowPriority);

    return app.exec();
}
