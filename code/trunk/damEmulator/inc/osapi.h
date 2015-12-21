// Процедуры операционной системы (OsApi)

#ifndef OSAPI_H
#define OSAPI_H

#include "types.h"

#define noop (void)0        // Холостая инструкция

// Псевдоним процедуры, являющейся точкой входа в отдельном потоке
typedef void (*OsThreadEntry)(void *param);

// Создать поток
// start = 0 - создать приостановленным, start = 1 - запустить после создания
// Возвращает 0 - Failed, !0 - идентификатор (handle) созданнного потока.
uint32 os_threadCreate(OsThreadEntry entry, void *param, int start, uint32 *stackMem, uint32 stackSize);

// Запустить или приостановить поток с указанным идентификатором hnd.
// hnd = 0 - приостановить текущий поток, из которого данная процедура вызвана.
// start = 0 - приостановить поток (не реализовано, не применять), start = 1 - запустить поток.
// Возвращает 0 - Failed, !0 - если поток успешно приостановлен или запущен
int os_threadStart(uint32 hnd, int start);


// Создать мьютекс
// Возвращает идентификатор мьютекса в случае успешного создания, или 0 - Failed
uint32 os_lockCreate(void);

// Захватить мьютекс с указанным идентификатором с таймаутом timeoutMs
// Возвращает 0 - Failed, 1 - ОК
int os_lockGet(uint32 hnd, uint32 timeoutMs);

// Освободить мьютекс с указанным идентификатором
void os_lockRelease(uint32 hnd);


// Получить время, в мс, с момента старта DAD
uint32 os_timeGet(void);

#endif // OSAPI_H
