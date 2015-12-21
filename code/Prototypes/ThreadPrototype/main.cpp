#include <QCoreApplication>

#include "mainthread.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    MainThread mt;
    mt.start();         // Запустить главную процедуру в отдельном потоке
    return app.exec();
}
