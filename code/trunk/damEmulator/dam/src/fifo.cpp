#include "fifo.h"

// Добавить значение в конец буфера FIFO
void fifoAppend(Fifo *fifo, uint32 v)
{
    // assert (fifo)
    // assert (fifo->buf)

    // Если размер буфера не определён
    if (!fifo->bufSize)
        return;     // Ничего не делаем

    // Если буфер пуст
    if (fifo->size == 0) {
        fifo->buf[fifo->head] = v;
        ++(fifo->size);
    }
    else if (fifo->size < fifo->bufSize) {
        ++(fifo->head);
        if (fifo->head > fifo->bufSize - 1)
            fifo->head = 0;
        fifo->buf[fifo->head] = v;
        ++(fifo->size);
    }
}

// Удалить первый добавленный элемент в буфере FIFO и вернуть его значение
uint32 fifoTakeFirst(Fifo *fifo)
{
    // assert (fifo)
    // assert (fifo->buf)

    // Если в буфере один элемент
    if (fifo->size == 1) {
        fifo->size = 0;
        return fifo->buf[fifo->tail];
    }
    else if (fifo->size > 0) {        // Если в буфере несколько элементов
        int v = fifo->buf[fifo->tail];
        ++(fifo->tail);
        if (fifo->tail > fifo->bufSize - 1)
            fifo->tail = 0;
        --(fifo->size);
        return v;
    }

    return 0;
}

// FIFO Буфер пуст?
// Возвращает 0 - не пуст, 1 - пуст
int fifoIsEmpty(Fifo *fifo)
{
    // assert (fifo)
    return fifo->size == 0;
}

// FIFO буфер полон?
// Возвращает 0 - не полон, 1 - полон
int fifoIsFull(Fifo *fifo)
{
    // assert (fifo)
    return fifo->size == fifo->bufSize;
}
