#ifndef TYPES_H
#define TYPES_H

#define LOCK_TIMEOUT 1000               // Общий таймаут захвата мьютекса (для непродолжительных операций)
#define WAIT_DEINIT_THREAD 1000         // Таймаут ожидания завершения работы потока
#define WAIT_FINISHED_THREAD 20         // Таймаут ожидания потока,
                                        // который находится в состоянии Finished,
                                        // чтобы дать ему время на деиницализацию.

#define FIFO_SIZE 32        // Размер буфера FIFO

#define LOCAL_SERVER_PORT   8081    // Порт локального сервера (для приёма соединений)
#define REMOTE_SERVER_PORT  8080    // Порт удалённого сервера (для установления соединения)

#define MAX_FILE_DATA_COUNT 20479   // Максимальное количество элементов данных в файле

typedef unsigned int uint32;
typedef unsigned char uint8;

// Состояние работы потока
typedef enum {
    Thread_Unknown_State = 0,    // Поток не выполнял ранее никаких действий
    Thread_Started_State,        // Поток запущен, но ещё не выполняется
    Thread_Running_State,        // Поток выполняется
    Thread_Finished_State        // Поток завершил своё выполнение
} ThreadState;

#endif // TYPES_H
