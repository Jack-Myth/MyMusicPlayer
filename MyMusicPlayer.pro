#-------------------------------------------------
#
# Project created by QtCreator 2018-06-18T18:27:38
#
#-------------------------------------------------

QT       += core gui
QT	+=network
QT += multimedia

CONFIG += openssl-linked
LIBS += -luser32

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MyMusicPlayer
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    main.cpp \
    mainwindow.cpp \
    commentwindow.cpp \
    lrcparser.cpp \
    searchwindow.cpp \
    utilitytools.cpp \
    commentwindow.cpp \
    lrcparser.cpp \
    main.cpp \
    mainwindow.cpp \
    searchwindow.cpp \
    utilitytools.cpp \
    libQREncode/bitstream.c \
    libQREncode/mask.c \
    libQREncode/mmask.c \
    libQREncode/mqrspec.c \
    libQREncode/qrencode.c \
    libQREncode/qrinput.c \
    libQREncode/qrspec.c \
    libQREncode/rscode.c \
    libQREncode/split.c \
    downloadmanager.cpp \
    downloadwindow.cpp \
    globalsetting.cpp \
    settingdialog.cpp

HEADERS += \
        mainwindow.h \
    mainwindow.h \
    commentwindow.h \
    lrcparser.h \
    searchwindow.h \
    libQREncode/bitstream.h \
    libQREncode/config.h \
    libQREncode/mask.h \
    libQREncode/mmask.h \
    libQREncode/mqrspec.h \
    libQREncode/qrencode.h \
    libQREncode/qrencode_inner.h \
    libQREncode/qrinput.h \
    libQREncode/qrspec.h \
    libQREncode/rscode.h \
    libQREncode/split.h \
    utilitytools.h \
    mystruct.h \
    commentwindow.h \
    lrcparser.h \
    mainwindow.h \
    mystruct.h \
    searchwindow.h \
    utilitytools.h \
    downloadmanager.h \
    downloadwindow.h \
    globalsetting.h \
    settingdialog.h

FORMS += \
        mainwindow.ui \
    mainwindow.ui \
    commentwindow.ui \
    searchwindow.ui \
    downloadwindow.ui \
    settingdialog.ui

CONFIG += mobility
MOBILITY = 

RESOURCES += \
    Resources/imagereousrce.qrc \
    Resources/imagereousrce.qrc

DISTFILES += \
    Resources/doc_search.png \
    Resources/list_expand.png \
    Resources/Logo.png \
    Resources/pause.png \
    Resources/play.png \
    Resources/search.png \
    Resources/skip_backward.png \
    Resources/ward.png \
    Resources/arrow.ico \
    Resources/checkmark.ico \
    Resources/delete.ico \
    Resources/down_arrow.ico \
    Resources/eject.ico \
    Resources/forward.ico \
    Resources/play.ico \
    Resources/plus.ico \
    Resources/repeat.ico \
    Resources/rewind.ico \
    Resources/right.ico \
    Resources/rrow.ico \
    Resources/skip_backward.ico \
    Resources/stop.ico \
    Resources/ward.ico
DEFINES +=HAVE_CONFIG_H 1
