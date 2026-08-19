# Consumer requirements, not propagated by the static lib - declare in the app's .pro:
#  - QT += svg           if the app uses CTintedSvgIconEngine (the qsvg plugin also renders QSS-referenced SVGs)
#  - QT += core-private   if the app includes cthemeiconhandler.h (QAbstractFileEngineHandler is private API)
HEADERS += \
    $$PWD/cbasepalette.h \
    $$PWD/cstylefixups.h \
    $$PWD/cthemecontroller.h \
    $$PWD/cthemeiconhandler.h \
    $$PWD/ctintedsvgiconengine.h

SOURCES += \
    $$PWD/cbasepalette.cpp \
    $$PWD/cstylefixups.cpp \
    $$PWD/cthemecontroller.cpp \
    $$PWD/cthemeiconhandler.cpp \
    $$PWD/ctintedsvgiconengine.cpp
