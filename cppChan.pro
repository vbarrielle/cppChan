TEMPLATE = app
TARGET = channelTest
DEPENDPATH += .
QMAKE_CXXFLAGS += -g -Wall -Wextra -std=c++11 -O0
LIBS += -lgtest
QT -= gui
QT -= core

# Input
HEADERS += channel.hh
SOURCES += tests/test_channel.cc



