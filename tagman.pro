#-------------------------------------------------
#
# Project created by QtCreator 2012-12-27T19:55:14
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = tagman
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    patternvalidator.cpp \
    filehandler.cpp

HEADERS  += mainwindow.h \
    patternvalidator.h \
    filehandler.h

FORMS    += mainwindow.ui

LIBS     += -ltag
