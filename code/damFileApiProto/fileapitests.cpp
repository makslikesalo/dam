#include "fileapi.h"

#include <QtTest/QtTest>

class TestWriteValue: public QObject
{
    Q_OBJECT
private slots:
    // Записать в файл начальный блок данных
    void writeInitialDataBlock(void);

    // Тест режимов открытия файлов
    void openRead(void);
    void openWrite(void);
    void writeAndRead(void);
    void writeToReadOnlyFile(void);
    void writeToWritePlusFile(void);
    void writeTwoFiles(void);
    void writeAndClearFile(void);

private:
    static const char *File_Path;
};

const char *TestWriteValue::File_Path = "values.dat";

void TestWriteValue::writeInitialDataBlock(void)
{
    // Создать новый файл на запись

        //qCritical() << "Can't open file" << File_Path;
        //QVERIFY(false);

}


// Тест режимов открытия файлов
void TestWriteValue::openRead(void)
{
    // Проверка режимов доступа к файлу

    // Открыть файл для чтения
    FILE *rf = fopen("rfile.tmp", "r");
    if (!rf)
        QVERIFY(true);
    else {
        fclose(rf);
        QVERIFY(false);
    }
}


// Тест режимов открытия файлов
void TestWriteValue::openWrite(void)
{
    // Создать новый файл на запись, уничтожить старое содержимое
    FILE *wf = fopen("wfile.tmp", "wb");
    if (!wf) {
        qCritical() << "W open err:" << ferror(wf);
        QVERIFY(false);
        return;
    }

    // Записать значение
    char x = 0x17;
    size_t ws = fwrite(&x, 1, 1, wf);
    if (!ws) {
        qCritical() << "W write err:" << ferror(wf);
        fclose(wf);
        QVERIFY(false);
        return;
    }

    if (wf)
        fclose(wf);

    QVERIFY(true);
}

// Записать и прочитать
void TestWriteValue::writeAndRead(void)
{
    // Создать новый файл на запись, уничтожить старое содержимое
    FILE *wf = fopen("wfile.tmp", "wb");
    if (!wf) {
        qCritical() << "W open err:" << ferror(wf);
        QVERIFY(false);
        return;
    }

    // Записать значение
    char x = 0x17;
    size_t ws = fwrite(&x, 1, 1, wf);
    if (!ws) {
        qCritical() << "W write err:" << ferror(wf);
        fclose(wf);
        QVERIFY(false);
        return;
    }

    if (wf)
        fclose(wf);

    // Открыть файл для чтения
    FILE *rf = fopen("wfile.tmp", "r");
    if (!rf) {
        qCritical() << "R open err:" << ferror(rf);
        QVERIFY(false);
        return;
    }

    char buff = 0;
    size_t rbytes = fread(&buff, 1, 1, rf);

    if (!rbytes) {
        qCritical() << "R read err:" << ferror(rf);
        QVERIFY(false);
        fclose(rf);
        return;
    }

    if (buff != 0x17) {
        qCritical() << "Value is diff" << buff;
        QVERIFY(false);
        fclose(rf);
        return;
    }

    fclose(rf);
    QVERIFY(true);
}


void TestWriteValue::writeToReadOnlyFile(void)
{
    // Открыть файл на чтение
    FILE *rf = fopen("wfile.tmp", "rb");
    if (!rf) {
        qCritical() << "Read open err:" << ferror(rf);
        QVERIFY(false);
        return;
    }

    // Записать значение
    char x = 0x17;
    size_t ws = fwrite(&x, 1, 1, rf);
    if (ws) {
        qCritical() << "Write data to read only file:" << ferror(rf);
        fclose(rf);
        QVERIFY(false);
        return;
    }

    if (rf)
        fclose(rf);

    QVERIFY(true);
}

void TestWriteValue::writeToWritePlusFile(void)
{
    // Создать файл на чтение и запись
    FILE *wf = fopen("writeplusfile.tmp", "wb+");
    if (!wf) {
        qCritical() << "Write open err:" << ferror(wf);
        QVERIFY(false);
        return;
    }

    // Записать значение в конец файла
    char x = 0x17;
    size_t ws = fwrite(&x, 1, 1, wf);
    if (!ws) {
        qCritical() << "Write data error:" << ferror(wf);
        fclose(wf);
        QVERIFY(false);
        return;
    }

    if (wf)
        fclose(wf);


    // Открыть файл на чтение и запись
    FILE *rf = fopen("writeplusfile.tmp", "rb+");
    if (!rf) {
        qCritical() << "Read open err:" << ferror(rf);
        QVERIFY(false);
        return;
    }

    // Прочитать значение из начала файла
    char rx = 0;
    size_t rs = fread(&rx, 1, 1, rf);
    if (!rs) {
        qCritical() << "Read data error:" << ferror(rf);
        fclose(rf);
        QVERIFY(false);
        return;
    }

    if (rx != x) {
        qCritical() << "Compare error" << x << "and" << rx;
        fclose(rf);
        QVERIFY(false);
        return;
    }

    // Дописать в конец байт
    if (fseek(rf, 0, SEEK_END)) {
        qCritical() << "Can't seek to end of files";
        fclose(rf);
        QVERIFY(false);
        return;
    }

    // Записать значение в текущую позицию
    char x2 = 0x72;
    if (!fwrite(&x2, 1, 1, rf)) {
        qCritical() << "Write data to EOF error:" << ferror(rf);
        fclose(rf);
        QVERIFY(false);
        return;
    }


    // Проверить содержимое файла
    char readBuf[2] = {0x00, 0x00};
    // Сбросить позицию в начало файла
    rewind(rf);
    if (!fread(readBuf, 1, 2, rf)) {
        qCritical() << "Read result data error:" << ferror(rf);
        fclose(rf);
        QVERIFY(false);
        return;
    }

    char chBuf[] = {0x17, 0x72};        // Буфер для сравнения
    if (memcmp(readBuf, chBuf, 2)) {
        qCritical() << "Buffers compare error";
        fclose(rf);
        QVERIFY(false);
        return;
    }


    if (rf)
        fclose(rf);

    QVERIFY(true);
}


void TestWriteValue::writeTwoFiles(void)
{
    // Создать файл на запись
    FILE *wf = fopen("writefile.tmp", "wb");
    if (!wf) {
        qCritical() << "Write open err:" << ferror(wf);
        QVERIFY(false);
        return;
    }

    // Создать файл на чтение
    FILE *rf = fopen("writefile.tmp", "rb");
    if (!rf) {
        qCritical() << "Read open err:" << ferror(rf);
        QVERIFY(false);
        fclose(wf);
        return;
    }

    // Записать байт в файл
    char dw = 0x75;
    if (!fwrite(&dw, 1, 1, wf)) {
        qCritical() << "Write err:" << ferror(wf);
        QVERIFY(false);
        fclose(rf);
        fclose(wf);
        return;
    }

    // Прочитать байт из файла
    char dr = 0x00;
    rewind(rf);
    if (!fread(&dr, 1, 1, rf)) {
        qCritical() << "Read err:" << ferror(rf) << feof(rf);
        QVERIFY(true);      // Правильно, нельзя одновременно читать и одновременно записывать файл.
        fclose(rf);
        fclose(wf);
        return;
    }

    // Сравнить прочитанный и записанный файл
    if (dr != dw) {
        qCritical() << "Compare error";
        QVERIFY(false);
        fclose(rf);
        fclose(wf);
        return;
    }

    fclose(rf);
    fclose(wf);

    QVERIFY(true);
}


// Записать и стереть файл
void TestWriteValue::writeAndClearFile(void)
{
    // Создать файл на запись
    FILE *wf = fopen("writeAndClear.tmp", "wb");
    if (!wf) {
        qCritical() << "Write open err:" << ferror(wf);
        QVERIFY(false);
        return;
    }

    // Записать данные в файл
    char writeBuf[] = {0x12, 0x33, 0x75, 0x45};
    if (!fwrite(writeBuf, sizeof(writeBuf), 1, wf)) {
        qCritical() << "Write err:" << ferror(wf);
        QVERIFY(false);
        fclose(wf);
        return;
    }

    fclose(wf);


    // Прочитать записанный ранее файл
    FILE *rf = fopen("writeAndClear.tmp", "rb+");
    if (!rf) {
        qCritical() << "Read open err:" << ferror(rf);
        QVERIFY(false);
        return;
    }

    // Прочитать данные из файла
    char readBuf[] = {0x00, 0x00, 0x00, 0x00};
    if (!fread(readBuf, sizeof(readBuf), 1, rf)) {
        qCritical() << "Read err:" << ferror(rf) << feof(rf);
        QVERIFY(false);
        fclose(rf);
        return;
    }

    // Сравнить записанные и прочитанные данные из файла
    if (memcmp(writeBuf, readBuf, 4)) {
        qCritical() << "Compare error";
        QVERIFY(false);
        fclose(rf);
        return;
    }

    // Очистить файл


    fclose(rf);

    QVERIFY(true);
}


QTEST_MAIN(TestWriteValue)
#include "fileapitests.moc"
