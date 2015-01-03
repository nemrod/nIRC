TEMPLATE = app
TARGET = nirc
INCLUDEPATH += .

QT += widgets network

HEADERS += ChannelTab.h nIRC.h ServerTab.h Tab.h User.h
SOURCES += ChannelTab.cpp main.cpp nIRC.cpp ServerTab.cpp Tab.cpp User.cpp
