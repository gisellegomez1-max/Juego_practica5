TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        Obstaculo.cpp \
        ResolutorColisiones.cpp \
        main.cpp

HEADERS += \
    Obstaculo.h \
    ResolutorColisiones.h \
    Rival.h
