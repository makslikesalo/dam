#include "fileapi.h"
#include <errno.h>

// НАИМЕНОВАНИЕ
// Записать массив данных в файл
// ПАРАМЕТРЫ
// - Путь и имя файла (может существовать, а может и нет)
// - Массив данных для записи
// - Кол-во элементов массива данных (может быть нулевым)
// - Максимальное количество элементов данных в файле
// ВОЗВРАЩАЕТ
// - 0 - массив данных успешно записан в файл, !0 - ошибка записи
// ПРЕДУСЛОВИЯ
// - Файл может существовать, а может и не существовать
// - Наличие достаточного свободного места на диске
// - Кол-во элементов массива данных должно быть БОЛЬШЕ ИЛИ РАВНО 0
// ПОСТУСЛОВИЯ
// - Данные успешно записаны в существующий или новый файл
// - Какая-либо ошибка
// ОШИБКИ
// - Ошибка работы с файлом
//   - Ошибка работы с существующим файлом
//      - Ошибка открытия файла
//      - Ошибка чтения файла
//   - Ошибка работы с новым файлом
//      - Ошибка создания файла
//      - Ошибка записи в файл
//   - Ошибка наличия свободного места на диске
// ДОПОЛНИТЕЛЬНО
// - Если файл не существует, то создаётся новый файл.
// - Если файл существует и он некорректен, то файл пересоздаётся.
// - Если формат существующего файла некорректен, то файл пересоздаётся.
int fileApi_writeDataToFile(const char *filePath,
                             const DataBlock *db,
                             uint32 count,
                             uint32 maxCount)
{
    int stat = 0;       // Статус выполнения операции

    if (fileApi_isFileExists(filePath)) {           // Если Файл существует
        //Открыть существующий файл на чтение и запись (r+ чтение с записью)
        FILE *file = fopen(filePath, "rb+");

        if (!file)                      // Если произошла ошибка открытия файла
            return errno;

        if (fileApi_isValidFileFormat(file, maxCount))     // Если Формат файла корректен
            stat = fileApi_writeData(file, db, count, maxCount);        // Записать массив данных
        else {
            // Переоткрыть файл на запись и чтение (w+ создать пустой с записью и чтением)
            file = freopen(filePath, "wb+", file);
            if (!file)                  // Если произошла ошибка открытия файла
                return ferror(file);
            stat = fileApi_writeData(file, db, count, maxCount);        // Записать массив данных
        }

        if (stat) {                     // Если произошла ошибка записи данных
            fclose(file);
            return stat;
        }

        fclose(file);
    }
    else {
        // Создать новый файл на запись (w+ создать новый с записью)
        FILE *file = fopen(filePath, "wb+");

        if (!file)                  // Если произошла ошибка открытия файла
            return errno;

        // Записать массив данных
        stat = fileApi_writeData(file, db, count, maxCount);

        if (stat) {                // Если произошла ошибка записи данных
            fclose(file);
            return stat;
        }

        fclose(file);
    }

    return 0;
}


// НАИМЕНОВАНИЕ
// Записать массив данных
// ПАРАМЕТРЫ
// - Открытый файл на запись
// - Данные на запись
// - Кол-во элементов массива данных (может быть нулевым)
// - Макс. кол-во элементов данных в файле
// ВОЗВРАЩАЕТ
// - 0 - Данные успешно записаны, !0 – ошибка записи данных
// ПРЕДУСЛОВИЯ
// - Файл должен быть открыт на запись и чтение
// - Формат файла должен быть корректен
// - Наличие достаточного свободного места на диске
// - Данных может не быть, данные могут быть или данных может быть больше, чем допустимо макс. размером файла
// - Параметр макс. кол-во элементов данных в файле должен быть больше нуля
// - В файле как может существовать, так может и не существовать элемент позиции
// ПОСТУСЛОВИЯ
// - Данные успешно записаны в файл (если они есть), позиция указывает на последний элемент данных.
// ОШИБКИ
// - Ошибка записи в файл
// ДОПОЛНИТЕЛЬНО
// - Если данных для записи нет, то в файл ничего не пишется
// - Если данных больше, чем допустимо макс. размером файла, то они будут писаться по кругу
// - По окончании работы, элемент позиции в файле указывает на последний элемент массива данных
// ПРИМЕЧАНИЕ
//    Возможные состояния файла:
//        - Файл пуст
//        - Файл не пуст
//            - Файл заполнен не до предела
//                - В файле есть только элемент позиции = НЕ ОПРЕДЕЛЕНО
//                - В файле есть позиция, и элементы данных
//            - Файл заполнен до предела
//    Возможное значение позиции:
//        - Позиция = НЕ ОПРЕДЕЛЕНО
//        - Позиция указывает на элемент
int fileApi_writeData(FILE *f,
                      const DataBlock *db,
                      uint32 count,
                      uint32 maxCount)
{
    // Исходный размер файла, Байт
    const long Original_File_Size = fileApi_getFileSize(f);

    TPosition pos = 0;          // Элемент позиции текущего элемента
    int currentDataCount = 0;   // Текущее кол-во элементов данных в файле

    rewind(f);          // Сбросить позицию файла в начало и очистить последнюю ошибку

    if (Original_File_Size <= 0) {      // Если файл пуст?
        // Записать элемент позиции в начало файла.
        if (!fwrite(&pos, sizeof(TPosition), 1, f))
            return ferror(f);
    }
    else {
        // Вычислить текущее кол-во элементов данных в файле
        currentDataCount =
            calcDataCount(Original_File_Size, sizeof(DataBlock), sizeof(TPosition));

        // Прочитать элемент позиции
        if (!fread(&pos, sizeof(TPosition), 1, f))
            return ferror(f);
    }

    // ДЛЯ ВСЕХ элементов массива данных
    for (int i = 0; i < count; ++i) {
        // Шаг 1. Определить следующий элемент позиции, куда будет записан текущий элемент данных.
        // Следующий элемент позиции может указывать как на существующий элемент в файле,
        // так и на последний несуществующий, что будет означать необходимость
        // добавления элемента в конец файла.
        //
        // Шаг 2. Записать текущий элемент данных в позицию, которая была определена на шаге 1.
        // Если позиция указывает на элемент, следующий за самым последним, то
        // добавляем элемент в конец файла.
        // Если позиция указывает на существующий элемент, то перезаписываем его текущим.

        // Шаг 1.
        // Если имеются записанные элементы данных в файл
        if (currentDataCount > 0) {
            // Если Элемент позиции указывает на последний элемент в файле
            if (pos >= currentDataCount - 1) {
                // Если Добавлять элементы в файл ещё можно
                if (currentDataCount < maxCount)
                    // Элемент позиции - есть последний ещё несуществующий элемент в файле
                    pos = currentDataCount;
                else
                    //Элемент позиции - есть первый по счёту элемент в файле
                    pos = 0;
            }
            else // ИНАЧЕ [Элемент позиции указывает на не последний элемент в файле]
                // Элемент позиции - есть следующий по счёту элемент в файле
                ++pos;
        }
        else
            pos = 0;

        // Шаг 2.
        // Если Элемент позиции - есть существующий элемент данных в файле
        if (pos < currentDataCount) {
            // Перезаписать существующий элемент данных
            // Определить смещение
            long int offset = sizeof(TPosition) + pos * sizeof(DataBlock);
            if (fseek(f, offset, SEEK_SET))    // Установить позицию в файле
                return ferror(f);
            if (!fwrite(&(db[i]), sizeof(DataBlock), 1, f))
                return ferror(f);
        }
        else { // ИНАЧЕ [Элемент позиции - есть несуществующий последний элемент в файле]
            // Добавить в конец файла элемент данных
            if (fseek(f, 0L, SEEK_END))      // Установить позицию в файле
                return ferror(f);
            if (!fwrite(&(db[i]), sizeof(DataBlock), 1, f))
                return ferror(f);
            currentDataCount++; // Увелить кол-во счётчик кол-ва элементов данных в файле
        }
    }

    // Переписать элемент позиции
    rewind(f);          // Сбросить позицию файла в начало и очистить последнюю ошибку
    if (!fwrite(&pos, sizeof(TPosition), 1, f))
        return ferror(f);

    return 0;       // Успешное завершение работы процедуры
}


// НАИМЕНОВАНИЕ
// Прочитать массив данных из файла
// ПАРАМЕТРЫ
// - Путь и имя файла
// - Инициализированный массив для записи прочитанных данных
// - Кол-во запрашиваемых элементов массива данных (может быть нулевым)
// - Возвращаемое значение кол-ва фактически прочитанных элементов данных
// - Смещение от последнего записанного элемента
// ВОЗВРАЩАЕТ
// - 0 - успешно, !0 - Ошибка
// ПРЕДУСЛОВИЯ
// - Файл должен существовать
// - Формат файла должен быть корректным
// - Кол-во запрашиваемых элементов должно быть больше или равно нулю
// - Массив, куда будут записываться прочитанные данные, должен быть инициализирован
//   на всю величину Кол-ва запрашиваемых элементов массива данных
// ПОСТУСЛОВИЯ
// - Прочитано столько элементов данных, сколько указано в соответствующем параметре
// - Та или иная ошибка
// ОШИБКИ
// - Ошибка работы с файлом
//   - Ошибка существования файла
//   - Ошибка работы с существующим файлом
//      - Ошибка открытия файла
//      - Ошибка чтения файла
// - Ошибка формата файла
int fileApi_readDataFromFile(const char *filePath,
                             DataBlock *db,
                             uint32 count,
                             uint32 offset,
                             uint32 *readCount)
{
    // assert (filePath)
    // assert (db)
    // assert (readCount)

    // Иницилизировать возвращаемое значение прочитанных элементов данных
    *readCount = 0;

    int stat = 0;       // Статус выполнения операции

    // Если Кол-во запрашиваемых элементов РАВНО 0
    if (!count)
        return 0;

    // Открыть существующий файл только на чтение (r - чтение)
    FILE *file = fopen(filePath, "rb");
    if (!file)                      // Если произошла ошибка открытия файла
        return errno;

    // Если Формат файла не корректен
    if (!fileApi_isValidFileFormat(file, 0)) {
        fclose(file);
        return -1;
    }

    // Размер файла, Байт
    const long File_Size = fileApi_getFileSize(file);

    // Вычислить текущее кол-во элементов данных в файле
    const uint32 File_Data_Count =
        calcDataCount(File_Size, sizeof(DataBlock), sizeof(TPosition));

    // Прочитать из файла элемент позиции
    int originalPos = 0;
    rewind(file);          // Сбросить позицию файла в начало и очистить последнюю ошибку
    if (!fread(&originalPos, sizeof(TPosition), 1, file)) {
        stat = ferror(file);
        fclose(file);
        return stat;
    }

    // Позиция текущего элемента
    int pos = originalPos;

    // Если смещение превышает кол-во элементов данных в файле
    if (offset >= File_Data_Count) {
        // Возвратить пустой массив данных
        fclose(file);
        return 0;
    }

    // Вычилить позицию с учетом заданного смещения
    pos = pos - offset;

    // Если вычисленная позиция получилась несуществующей
    if (pos < 0)
        pos = File_Data_Count - (-pos);     // Отсчитать от конца файла

    // Кол-во прочитанных элементов данных
    int readDataCount = 0;

    do {
        // Прочитать из файла элемент под текущей позицией
        long int offset = sizeof(TPosition) + pos * sizeof(DataBlock);
        if (fseek(file, offset, SEEK_SET)) {    // Установить позицию в файле
            stat = ferror(file);
            fclose(file);
            return stat;
        }
        if (!fread(&(db[readDataCount]), sizeof(DataBlock), 1, file)) {
            stat = ferror(file);
            fclose(file);
            return stat;
        }

        ++readDataCount;        // Счётчик прочитанных элементов++
        if (--pos < 0)          // Если позиция предыдущего элемента МЕНЬШЕ 0
            // Индекс позиции текущего элемента = Индекс последнего элемента в файле
            pos = File_Data_Count - 1;
    }
    while (readDataCount < count &&
           pos != originalPos);
    // ПОКА Счётчик прочитанных элементов МЕНЬШЕ Кол-ва запрашиваемых элементов И
    //    Позиция текущего элемента НЕ РАВНА исходной позиции [предотвратить чтение по второму кругу]

    // Вернуть кол-во прочитанных данных
    *readCount = readDataCount;

    fclose(file);
    return 0;
}

// НАИМЕНОВАНИЕ
// Формат файла корректен
// ОПИСАНИЕ
// Процедура проверяет содержимое файла на предмет соответствия формату.
// Файл должен также отвечать требованиям кратности размера содержимого.
// Если параметр maxCount = 0, то процедура не проверяет файл
// на превышение максимально допустимого размера.
// Если maxCount > 0, то идёт проверка размера файла на превышение
// некоторого размера. И если размер превышен, то файл считается некорректным.
// ПАРАМЕТРЫ
// - Открытый файл на чтение
// - Макс. кол-во элементов данных в файле
// ВОЗВРАЩАЕТ
// - 1 - формат файла корректен, 0 - формат файла не корректен
// ПРЕДУСЛОВИЯ
// - Файл должен быть открыт на чтение
// - Файл может быть пуст, а может и нет.
// ПОСТУСЛОВИЯ
// - Вывод: Формат файла корректен или нет
// ОШИБКИ
// - Ошибка чтения файла
// ДОПОЛНИТЕЛЬНО
// - Максимальный размер файла определяется по формуле:
// Макс. кол-во элементов данных в файле * размер эл. данных + размер эл. позиции
int fileApi_isValidFileFormat(FILE *f, uint32 maxCount)
{
    long fileSize = fileApi_getFileSize(f);     // Размер файла, Байт

    // Если Файл пуст или размер не удалось определить
    if (fileSize <= 0)
        return 0;
    // Если размер файла МЕНЬШЕ или РАВЕН размеру элемента позиции
    else if (fileSize <= sizeof(TPosition))
        return 0;
    // Если Размер файла НЕ кратен размеру элементов данных
    // с учётом размера элемента позиции
    else if ( (fileSize - sizeof(TPosition)) % sizeof(DataBlock) )
        return 0;
    // Если Размер файла превышает максимальный размер
    // (игнорировать проверку, если maxCount = 0).
    else if (maxCount > 0 &&
             fileSize > calcFileSize(sizeof(DataBlock),
                                    maxCount,
                                    sizeof(TPosition)))
        return 0;

    // Прочитать элемент позиции
    TPosition pos = 0;      // Элемент позиции
    rewind(f);              // Сброcить указатель текущей позиции файла в начало
    if (!fread(&pos, sizeof(TPosition), 1, f))
        return 0;

    // Если Элемент позиции указывает на несуществующий элемент данных
    if ( pos >= calcDataCount(fileSize,
                             sizeof(DataBlock),
                             sizeof(TPosition)) )
        return false;

    return true;
}


// Получить размер файла, байт
// Файл должен быть открыт на чтение
long fileApi_getFileSize(FILE *f)
{
    if (!f)
        return -1;

    // Установить позицию в конец файла
    if ( fseek(f, 0L, SEEK_END) )
        return -1;

    // Получить текущее значение позиции файла
    return ftell(f);
}


// Проверить, существует ли файл c указанным путём и наименованием filePath?
// Возвращает 1 - если существует, 0 - если не существует
int fileApi_isFileExists(const char *filePath)
{
    FILE *f = fopen(filePath, "r");
    if (f) {
        fclose(f);
        return 1;
    }
    return 0;
}
