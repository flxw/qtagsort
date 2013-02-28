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
    musicdatamodel.cpp

HEADERS  += mainwindow.h \
    patternvalidator.h \
    filehandler.h \
    authordialog.h \
    resultdialog.h \
    musicdatamodel.h \
    versioninfo.h

FORMS    += mainwindow.ui \
    authordialog.ui \
    resultdialog.ui

INCLUDEPATH += -I/usr/include/neon
LIBS         += -ltag -lmusicbrainz5
