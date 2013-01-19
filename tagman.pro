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
    filehandler.cpp \
    authordialog.cpp \
    resultdialog.cpp \
    sourcefileeditdialog.cpp

HEADERS  += mainwindow.h \
    patternvalidator.h \
    filehandler.h \
    authordialog.h \
    resultdialog.h \
    sourcefileeditdialog.h

FORMS    += mainwindow.ui \
    authordialog.ui \
    resultdialog.ui \
    sourcefileeditdialog.ui

LIBS     += -ltag
