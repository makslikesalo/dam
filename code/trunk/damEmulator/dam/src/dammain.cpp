// Исходный файл, в котором опеределена главная процедура эмулятора - damMain()

#include "types.h"
#include "osapi.h"
#include "utils.h"

#include "string.h"

#include "acquisitionthread.h"
#include "serverthread.h"

// Флаг завершения выполнения программы (устанавливается эмулятором)
int exitDam = 0;

// Головная функция
void damMain(void) {

    // Параметры системы
    // Константы
    const char *Data_File_Path = "values.dat";  // Путь и наименование файла с данными
    const uint32 Max_Data_File_Count = 20479;   // Максимальное количество элементов данных в файле
    // Объекты сети
    sockaddr serverAddr;                                // Адрес локального сервера
    sockaddr_in *serv = (sockaddr_in *) &serverAddr;    // Объект для хранения адреса
    memset(serv, 0, sizeof(sockaddr_in));               // Обнулить объект
    serv->sin_family = AF_INET;                         // TCP/IP
    serv->sin_addr.s_addr = inet_addr("127.0.0.1");     // localhost
    serv->sin_port = htons(LOCAL_SERVER_PORT);

    // Объекты совместного использования
    // Мьютексы
    uint32 sensorLockHnd = os_lockCreate();     // Мьютекс совместного доступа к датчику
    uint32 fileLockHnd = os_lockCreate();       // Мьютекс совместного доступа к файлу
    uint32 lastClientLockHnd = os_lockCreate(); // Мьютекс совместного доступа к файлу
    // Объекты сети
    sockaddr lastClientAddr;                    // Адрес последнего обратившегося клиента
    memset(&lastClientAddr, 0, sizeof(lastClientAddr));

    // Инициализация потока сбора данных
    // Параметры
    AcquisitionParams acquisitionParams;
    memset(&acquisitionParams, 0, sizeof(acquisitionParams));
    acquisitionParams.args.filePath = Data_File_Path;
    acquisitionParams.args.maxFileCount = Max_Data_File_Count;
    acquisitionParams.args.lastClientAddr = &lastClientAddr;
    acquisitionParams.sync.fileLockHnd = fileLockHnd;
    acquisitionParams.sync.sensorLockHnd = sensorLockHnd;
    acquisitionParams.sync.lastClientLockHnd = lastClientLockHnd;
    // Создать поток
    uint32 acquisitionThreadHnd = os_threadCreate(acquisitionMain, &acquisitionParams, 0, NULL, 0);
    if (!acquisitionThreadHnd)
        return;

    // Инициализация потока сервера
    // Параметры
    ServerParams serverParams;
    memset(&serverParams, 0, sizeof(serverParams));
    serverParams.args.filePath = Data_File_Path;
    serverParams.args.maxFileCount = Max_Data_File_Count;
    serverParams.args.serverAddr = serverAddr;
    serverParams.args.lastClientAddr = &lastClientAddr;
    serverParams.sync.sensorLockHnd = sensorLockHnd;
    serverParams.sync.fileLockHnd = fileLockHnd;
    serverParams.sync.lastClientLockHnd = lastClientLockHnd;
    // Создать поток
    uint32 serverThreadHnd = os_threadCreate(serverMain, &serverParams, 0, NULL, 0);
    if (!serverThreadHnd)
        return;

    do {
        // Запустить поток сбора данных
        acquisitionParams.state = Thread_Started_State;
        if (!os_threadStart(acquisitionThreadHnd, 1))
            break;

        // Запустить поток сервера
        serverParams.state = Thread_Started_State;
        if (!os_threadStart(serverThreadHnd, 1))
            break;

        // Ждать пока не будет взведён флаг завершения
        while (!exitDam) {}

    } while (0);

    // Добровольно завершить потоки
    acquisitionParams.control.exitFlag = true;
    serverParams.control.exitFlag = true;

    // Подождать завершения потоков
    damWait(WAIT_DEINIT_THREAD, 0);

    // Принудительно завершить потоки
    os_threadStart(acquisitionThreadHnd, 0);
    os_threadStart(serverThreadHnd, 0);
}

