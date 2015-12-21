QT += core network
QT -= gui

TARGET = tcpClient
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    tcpclient.cpp

HEADERS += \
    tcpclient.h

