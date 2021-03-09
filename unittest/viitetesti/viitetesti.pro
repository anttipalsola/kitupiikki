QT += testlib gui
QT -=

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

INCLUDEPATH += $$PWD/../../kitsas
VPATH += $$PWD/../../kitsas

SOURCES +=  tst_viitetest.cpp \
    laskutus/viitenumero.cpp \
    validator/ibanvalidator.cpp

HEADERS += laskutus/viitenumero.h \
    validator/ibanvalidator.h
