QT += core
QT -= gui

LIBS += -lws2_32

TARGET = serversocketapi
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp

