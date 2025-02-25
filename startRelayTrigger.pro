QT -= gui
QT += websockets sql
CONFIG += c++11 console
CONFIG -= app_bundle

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        ChatServer.cpp \
        Database.cpp \
        Functions.cpp \
        GPIOKeyEvent.cpp \
        NetworkConnman.cpp \
        OutputTrigger.cpp \
        SocketClient.cpp \
        StartRelayTrigger.cpp \
        main.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    ChatServer.h \
    Database.h \
    GPIOKeyEvent.h \
    NetworkConnman.h \
    OutputTrigger.h \
    SocketClient.h \
    StartRelayTrigger.h

LIBS += -lgpiod
