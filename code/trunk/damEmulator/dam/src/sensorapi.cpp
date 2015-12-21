#include "sensorapi.h"

#define DELAY_LOOP_COUNT 10000  // Количество циклов задержки
#define DELAY(count) {for(int i = 0; i < count; ++i) {}}

static uint32 sensorCounter = 0;        // Счётчик кол-ва обращений к датчику

// Прочитать данные с датчика
uint32 io_sensorRead(void)
{
    DELAY(DELAY_LOOP_COUNT);        // Вставить искусственную задержку
    return sensorCounter++;
}
