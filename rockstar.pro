TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
TARGET = brockstar

QMAKE_CXXFLAGS += -std=c++20

SOURCES += \
        evaluator.cpp \
        function.cpp \
        main.cpp \
        scanner.cpp \
        token.cpp \
        value.cpp

HEADERS += \
    evaluator.h \
    function.h \
    scanner.h \
    token.h \
    value.h
