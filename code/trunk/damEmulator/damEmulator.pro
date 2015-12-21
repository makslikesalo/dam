QT += core
QT -= gui

TARGET = damEmulator
CONFIG += console
CONFIG -= app_bundle

LIBS += -lws2_32

TEMPLATE = app

SOURCES += ./src/main.cpp \
    dam/src/dammain.cpp \
    dam/src/sensorapi.cpp \
    dam/src/fifo.cpp \
    dam/src/notifythread.cpp \
    dam/src/acquisitionthread.cpp \
    src/osapi.cpp \
    dam/src/utils.cpp \
    dam/src/writefilethread.cpp \
    dam/src/fileapi.cpp \
    dam/src/serverthread.cpp

INCLUDEPATH += \
        ./inc \
        ./dam/inc

HEADERS += \
    inc/mainthread.h \
    inc/osemulator.h \
    inc/osapi.h \
    dam/inc/types.h \
    dam/inc/sensorapi.h \
    dam/inc/fifo.h \
    dam/inc/notifythread.h \
    dam/inc/acquisitionthread.h \
    dam/inc/utils.h \
    dam/inc/writefilethread.h \
    dam/inc/fileapi.h \
    dam/inc/serverthread.h

