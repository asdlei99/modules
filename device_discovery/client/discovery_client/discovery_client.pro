QT += core network
QT -= gui

CONFIG += c++11

TARGET = discovery_client
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    discovery.cpp

HEADERS += \
    discovery.h
