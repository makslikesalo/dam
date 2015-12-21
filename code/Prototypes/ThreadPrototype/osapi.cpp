#include "osapi.h"

#include "osemulator.h"

// Обёртка задания для вызова в отдельном потоке
class TaskWrap : public QRunnable
{
public:

    OsThreadEntry func;
    void *param;

    TaskWrap(OsThreadEntry f, void *p) : func(f), param(p) {
    }

    ~TaskWrap() {
    }

    void run(void) {
        if (func)
            func(param);
    }
};

uint32 os_threadCreate(OsThreadEntry entry, void *param, int start, uint32 */*stackMem*/, uint32 /*stackSize*/)
{
    uint32 hnd = OsEmulator::getInstance().append(new TaskWrap(entry, param));
    if (!hnd)
        return 0;
    if (start)
        OsEmulator::getInstance().start(hnd, 1);
    return hnd;
}

int os_threadStart(uint32 hnd, int start)
{
    return OsEmulator::getInstance().start(hnd, start);
}


// Получить время, в мс, с момента старта DAD
uint32 os_timeGet(void)
{
    return OsEmulator::getInstance().getElapsedTime();
}
