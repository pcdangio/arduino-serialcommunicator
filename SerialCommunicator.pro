TEMPLATE = lib
CONFIG += c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += $(ProgramFiles)\Arduino\hardware\arduino\avr\cores\arduino
INCLUDEPATH += $$PWD\src

SOURCES += \
    src/Communicator.cpp \
    src/Message.cpp \
    src/utility/Outbound.cpp \
    src/utility/Inbound.cpp

HEADERS += \
    src/SerialCommunicator.h \
    src/Communicator.h \
    src/Message.h \
    src/utility/Outbound.h \
    src/utility/Inbound.h \
    src/utility/MessageStatus.h

RESOURCES +=

DISTFILES += \
    library.properties
