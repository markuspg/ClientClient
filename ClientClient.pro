#-------------------------------------------------
#
# Project created by QtCreator 2015-07-09T14:56:03
#
#-------------------------------------------------

QT       += core network

QT       -= gui

TARGET = ClientClient
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += src/main.cpp \
    src/serverconnector.cpp

HEADERS += \
    src/serverconnector.h

QMAKE_CXXFLAGS += -std=c++11
