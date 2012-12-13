TEMPLATE = app
TARGET = hello
DEPENDPATH += .
QMAKE_CXXFLAGS += -g -Wall -Wextra -std=c++11 -O0

# Input
HEADERS += channel.hh
HEADERS += atomic_channel.hh
SOURCES += main.cc



