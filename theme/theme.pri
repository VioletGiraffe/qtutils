# QAbstractFileEngineHandler (cthemeiconhandler.*) is private API; the include path is needed to
# compile this module and any app that includes the handler's header. No link dependency involved.
QT += core-private

# Consumer requirement, not propagated by the static lib: QT += svg in the app's .pro if it uses
# CTintedSvgIconEngine (the qsvg plugin also renders QSS-referenced SVGs).
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
