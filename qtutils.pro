TEMPLATE = lib
TARGET   = qtutils
CONFIG += staticlib

CONFIG += strict_c++ c++14

mac* | linux*{
	CONFIG(release, debug|release):CONFIG *= Release optimize_full
	CONFIG(debug, debug|release):CONFIG *= Debug
}

contains(QT_ARCH, x86_64) {
	ARCHITECTURE = x64
} else {
	ARCHITECTURE = x86
}

android {
	Release:OUTPUT_DIR=android/release
	Debug:OUTPUT_DIR=android/debug

} else:ios {
	Release:OUTPUT_DIR=ios/release
	Debug:OUTPUT_DIR=ios/debug

} else {
	Release:OUTPUT_DIR=release/$${ARCHITECTURE}
	Debug:OUTPUT_DIR=debug/$${ARCHITECTURE}
}

DESTDIR  = ../bin/$${OUTPUT_DIR}
OBJECTS_DIR = ../build/$${OUTPUT_DIR}/$${TARGET}
MOC_DIR     = ../build/$${OUTPUT_DIR}/$${TARGET}
UI_DIR      = ../build/$${OUTPUT_DIR}/$${TARGET}
RCC_DIR     = ../build/$${OUTPUT_DIR}/$${TARGET}

QT = core gui widgets

# Required for qInfo() to log function name, file and line in release build
DEFINES += QT_MESSAGELOGCONTEXT

INCLUDEPATH += ../cpputils ../cpp-template-utils

win*{
	QMAKE_CXXFLAGS += /MP /Zi /wd4251
	DEFINES += WIN32_LEAN_AND_MEAN NOMINMAX
	QMAKE_CXXFLAGS_WARN_ON = /W4

	!*msvc2013*:QMAKE_LFLAGS += /DEBUG:FASTLINK

	Debug:QMAKE_LFLAGS += /INCREMENTAL
	Release:QMAKE_LFLAGS += /OPT:REF /OPT:ICF

	*msvc*{
		QMAKE_CXXFLAGS += /FS
	}

}

linux*{

}

linux*|mac*{
	QMAKE_CXXFLAGS += -pedantic-errors
	QMAKE_CFLAGS += -pedantic-errors
	QMAKE_CXXFLAGS_WARN_ON = -Wall -Wno-c++11-extensions -Wno-local-type-template-args -Wno-deprecated-register

	Release:DEFINES += NDEBUG=1
	Debug:DEFINES += _DEBUG

	PRE_TARGETDEPS += $${DESTDIR}/libcpputils.a
}

include(logger/logger.pri)
include(aboutdialog/aboutdialog.pri)
include(settings/settings.pri)
include(taskbarprogress/taskbarprogress.pri)
include(utils/utils.pri)
include(settingsui/settingsui.pri)
include(mouseclickdetector/mouseclickdetector.pri)
include(historylist/historylist.pri)
include(imageprocessing/imageprocessing.pri)
include(widgets/widgets.pri)
include(dialogs/dialogs.pri)
include(ui/ui.pri)
include(std_helpers/std_helpers.pri)
include(qtcore_helpers/qtcore_helpers.pri)

win*{
	include(windows/windows.pri)
}

HEADERS += \
	imageprocessing/resize/cimageinterpolationkernel.h

SOURCES += \
	utils/naturalsorting/naturalsorting_qt.cpp \
	imageprocessing/resize/cimageinterpolationkernel.cpp
