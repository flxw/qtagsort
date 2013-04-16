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
    proposalselectiondialog.cpp \
    sametargetchoicedialog.cpp

HEADERS  += mainwindow.h \
    patternvalidator.h \
    authordialog.h \
    resultdialog.h \
    musicdatamodel.h \
    versioninfo.h \
    fingerprinter.h \
    proposalselectiondialog.h \
    sametargetchoicedialog.h

FORMS    += mainwindow.ui \
    authordialog.ui \
    resultdialog.ui \
    proposalselectiondialog.ui \
    sametargetchoicedialog.ui

unix {
    LIBS += -ltag -lchromaprint -lavformat -lavcodec -lavutil -lswresample
}

win32:win32-msvc2010 {
    LIBS += -lc:/libs/tag
    LIBS += -lc:/libs/chromaprint
    LIBS += -lc:/libs/avformat-55
    LIBS += -lc:/libs/avcodec-55
    LIBS += -lc:/libs/avutil-52
    LIBS += -lc:/libs/swresample-0

    INCLUDEPATH += c:/libs/taglib/taglib/toolkit
    INCLUDEPATH += c:/libs/taglib
    INCLUDEPATH += c:/libs/chromaprint/src
    INCLUDEPATH += c:/libs/ffmpeg/include
}
