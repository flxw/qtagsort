#-------------------------------------------------
#
# Project created by QtCreator 2012-12-27T19:55:14
#
#-------------------------------------------------

QT       += core gui network script

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qtagsort
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    patternvalidator.cpp \
    authordialog.cpp \
    resultdialog.cpp \
    musicdatamodel.cpp \
    fingerprinter.cpp \
    proposalselectiondialog.cpp

HEADERS  += mainwindow.h \
    patternvalidator.h \
    authordialog.h \
    resultdialog.h \
    musicdatamodel.h \
    versioninfo.h \
    fingerprinter.h \
    proposalselectiondialog.h

FORMS    += mainwindow.ui \
    authordialog.ui \
    resultdialog.ui \
    proposalselectiondialog.ui

unix {
    LIBS += -ltag -lchromaprint -lavformat -lavcodec -lavutil -lswresample
}

win32 {
    LIBS += -lc:\libs\tag
    LIBS += -lc:\libs\chromaprint
    LIBS += -lc:\libs\avformat
    LIBS += -lc:\libs\avcodec
    LIBS += -lc:\libs\avutil
    LIBS += -lc:\libs\swresample
}
