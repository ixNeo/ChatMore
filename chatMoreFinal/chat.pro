#-------------------------------------------------
#
# Project created by QtCreator 2016-7-1T19:53:56
#
#-------------------------------------------------

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT+=widgets
QT += network script
TARGET = chat
TEMPLATE = app
QT += multimedia multimediawidgets

SOURCES += main.cpp\
    chatroom.cpp \
    login.cpp \
    privatechat.cpp \
    tcpclientfile.cpp \
    tcpserverfile.cpp

HEADERS  += \
    chatroom.h \
    login.h \
    privatechat.h \
    tcpclientfile.h \
    tcpserverfile.h

FORMS    += \
    ChatRoom.ui \
    PrivateChat.ui \
    Login.ui \
    TcpClientFile.ui \
    TcpServerFile.ui
QT += multimedia

win32:INCLUDEPATH += $$PWD
RESOURCES += \
    images.qrc
CONFIG += testlib\
#CONFIG += console
CONFIG += resources_big
