TEMPLATE = lib
TARGET   = qtutils
CONFIG += staticlib c++14

mac* | linux*{
	CONFIG(release, debug|release):CONFIG += Release
	CONFIG(debug, debug|release):CONFIG += Debug
}

android {
	Release:OUTPUT_DIR=android/release
	Debug:OUTPUT_DIR=android/debug

} else:ios {
	Release:OUTPUT_DIR=ios/release
	Debug:OUTPUT_DIR=ios/debug

} else {
	Release:OUTPUT_DIR=release
	Debug:OUTPUT_DIR=debug

}

DESTDIR  = ../bin/$${OUTPUT_DIR}
OBJECTS_DIR = ../build/$${OUTPUT_DIR}/qtutils
MOC_DIR     = ../build/$${OUTPUT_DIR}/qtutils
UI_DIR      = ../build/$${OUTPUT_DIR}/qtutils
RCC_DIR     = ../build/$${OUTPUT_DIR}/qtutils

QT = core gui widgets

# Required for qDebug() to log function name, file and line in release build
DEFINES += QT_MESSAGELOGCONTEXT

INCLUDEPATH += ../cpputils ../cpp-template-utils

win*{
	QMAKE_CXXFLAGS += /MP /wd4251
	DEFINES += WIN32_LEAN_AND_MEAN NOMINMAX
	QMAKE_CXXFLAGS_WARN_ON = /W4

	QMAKE_LFLAGS += /INCREMENTAL /DEBUG:FASTLINK
}

linux*{

}

linux*|mac*{
	QMAKE_CXXFLAGS += -pedantic-errors
	QMAKE_CFLAGS += -pedantic-errors
	QMAKE_CXXFLAGS_WARN_ON = -Wall -Wno-c++11-extensions -Wno-local-type-template-args -Wno-deprecated-register

	CONFIG(release, debug|release):CONFIG += Release
	CONFIG(debug, debug|release):CONFIG += Debug

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

win*{
	include(windows/windows.pri)
}

win32*:!*msvc2012:*msvc*:!*msvc2010:*msvc* {
	QMAKE_CXXFLAGS += /FS
}

HEADERS += \
	imageprocessing/resize/cimageinterpolationkernel.h

SOURCES += \
	utils/naturalsorting/naturalsorting_qt.cpp \
	imageprocessing/resize/cimageinterpolationkernel.cpp

