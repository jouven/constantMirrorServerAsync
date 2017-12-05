QT += core network
QT -= gui

QMAKE_CXXFLAGS += -std=c++17

TARGET = constantMirrorServerAsync
CONFIG += console
CONFIG -= app_bundle
CONFIG += no_keywords

TEMPLATE = app

SOURCES += main.cpp \
    mirrorConfig.cpp \
    downloadServer.cpp \
    updateClient.cpp \
    fileListRequestServer.cpp \
    downloadServerSocket.cpp \
    downloadServerThread.cpp \
    fileListRequestServerThread.cpp \
    fileListRequestServerSocket.cpp

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

HEADERS += \
    mirrorConfig.hpp \
    downloadServer.hpp \
    updateClient.hpp \
    fileListRequestServer.hpp \
    downloadServerSocket.hpp \
    downloadServerThread.hpp \
    fileListRequestServerThread.hpp \
    fileListRequestServerSocket.hpp

!win32:MYPATH = "/"
win32:MYPATH = "H:/veryuseddata/portable/msys64/"

#mine
INCLUDEPATH += $${MYPATH}home/jouven/mylibs/include

LIBS += -lessentialQtso -lsignalso -lbaseClassQtso -lqmutexUMapQtso -lsslUtilsso -lthreadedFunctionQtso -lfileHashQtso -lssl -lcrypto
#don't new line the "{"
#release
CONFIG(release, debug|release){
    LIBS += -L$${MYPATH}home/jouven/mylibs/release/
    DEPENDPATH += $${MYPATH}home/jouven/mylibs/release
    QMAKE_RPATHDIR += $${MYPATH}home/jouven/mylibs/release
}
#debug
CONFIG(debug, debug|release){
    LIBS += -L$${MYPATH}home/jouven/mylibs/debug/ -lbackwardSTso -ltimeso -lboost_date_time
    DEPENDPATH += $${MYPATH}home/jouven/mylibs/debug
    QMAKE_RPATHDIR += $${MYPATH}home/jouven/mylibs/debug
    DEFINES += DEBUGJOUVEN
}

QMAKE_CXXFLAGS_DEBUG -= -g
QMAKE_CXXFLAGS_DEBUG += -pedantic -Wall -Wextra -g3

#if not win32, add flto, mingw (on msys2) can't handle lto
!win32:QMAKE_CXXFLAGS_RELEASE += -flto=jobserver
#qt QMAKE defaults strike again, adds -mtune=core2 just because in win32
win32:QMAKE_CXXFLAGS -= -mtune=core2
QMAKE_CXXFLAGS_RELEASE += -mtune=sandybridge

#for -flto=jobserver in the link step to work with -j4
!win32:QMAKE_LINK = +g++

unix:QMAKE_LFLAGS += -fuse-ld=gold
QMAKE_LFLAGS_RELEASE += -fvisibility=hidden
#if not win32, add flto, mingw (on msys2) can't handle lto
!win32:QMAKE_LFLAGS_RELEASE += -flto=jobserver

