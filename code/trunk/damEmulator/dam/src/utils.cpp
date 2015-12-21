#include "utils.h"

#include "osapi.h"

// Блокировать текущий поток на заданный интервал времени
void damWait(uint32 ms, int *br)
{
    uint32 currentTime = os_timeGet();      // Текущее время
    uint32 timeoutTime = currentTime + ms;
    while (currentTime < timeoutTime &&
           (br == 0 || (br != 0 && !(*br))) ) {
        currentTime = os_timeGet();
    }
}
