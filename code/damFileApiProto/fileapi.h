#ifndef FILEAPI_H
#define FILEAPI_H

#include <stdio.h>

#include "types.h"

// Значение позиции - НЕ ОПРЕДЕЛЕНО
#define UNDEFINED_POSITION 0xFFFFFFFF

#define RET_OK 0
#define RET_ERROR -1

// Блок данных
struct DataBlock {
    uint32 value;       // Значение
};

// Записать значение value в файл с указанным путём и наименованием filePath
void fileApi_writeValueToFile(const char *filePath, uint32 value);

// Записать в файл начальный блок данных
// Возвращает кол-во успешно записанных блоков данных
int fileApi_writeInitialData(FILE *f, DataBlock *d, size_t count);

// Записать в открытый файл на запись значение позиции последнего блока данных
int fileApi_writePosition(FILE *f, uint32 pos);

// Получить размер файла.
// Файл должен быть открыт на чтение
// Возвращает размер в байтах.
// Если файл не открыт, или не удалось определить размер, то возвращает -1.
// Процедура сбивает текущую позицию файла.
long fileApi_getFileSize(FILE *f);

// Проверить, существует ли файл c указанным путём и наименованием filePath?
// Возвращает 1 - если существует, 0 - если не существует
int fileApi_isFileExists(const char *filePath);

#endif // FILEAPI_H
