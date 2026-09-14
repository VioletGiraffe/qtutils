TEMPLATE = lib
TARGET = qtutils
CONFIG += staticlib

CONFIG += strict_c++

mac* | linux* | freebsd {
	CONFIG(release, debug|release):CONFIG *= Release optimize_full
	CONFIG(debug, debug|release):CONFIG *= Debug
}

exists(../global.pri){
	include(../global.pri)
} else {
	CONFIG += c++2b
}


Release:OUTPUT_DIR=release
Debug:OUTPUT_DIR=debug

DESTDIR = ../bin/$${OUTPUT_DIR}
OBJECTS_DIR = ../build/$${OUTPUT_DIR}/$${TARGET}
MOC_DIR     = ../build/$${OUTPUT_DIR}/$${TARGET}
UI_DIR      = ../build/$${OUTPUT_DIR}/$${TARGET}
RCC_DIR     = ../build/$${OUTPUT_DIR}/$${TARGET}

QT = core gui widgets

# Required for qInfo() to log function name, file and line in release build
DEFINES += QT_MESSAGELOGCONTEXT

INCLUDEPATH += \
	$$PWD \
	../cpputils \
	../cpp-template-utils \

win*{
	QMAKE_CXXFLAGS += /MP /Zi /wd4251
	Debug:QMAKE_CXXFLAGS += /JMC
	QMAKE_CXXFLAGS += /std:c++latest /permissive- /Zc:__cplusplus /utf-8
	DEFINES += WIN32_LEAN_AND_MEAN NOMINMAX
	QMAKE_CXXFLAGS_WARN_ON = /W4

	!*msvc2013*:QMAKE_LFLAGS += /DEBUG:FASTLINK

	Debug:QMAKE_LFLAGS += /INCREMENTAL
	Release:QMAKE_LFLAGS += /OPT:REF /OPT:ICF
	*msvc*{
		QMAKE_CXXFLAGS += /FS
	}
}

linux*|mac*|freebsd{
	QMAKE_CXXFLAGS += -pedantic-errors
	QMAKE_CFLAGS += -pedantic-errors

	QMAKE_CXXFLAGS_WARN_ON *= -Wall -Wextra -Wnon-virtual-dtor -Woverloaded-virtual -Wcast-qual -Wdouble-promotion -Wfloat-conversion -Wundef
	QMAKE_CXXFLAGS_WARN_ON *= -Wformat=2 -Wextra-semi -Wzero-as-null-pointer-constant -Wfloat-equal -Wredundant-decls -Wvla

	QMAKE_CXXFLAGS *= -Werror=return-type -Werror=uninitialized -Werror=delete-non-virtual-dtor -Werror=address
	QMAKE_CXXFLAGS *= -Werror=sizeof-pointer-div -Werror=sizeof-pointer-memaccess

	contains(QMAKE_COMPILER, clang) {
		QMAKE_CXXFLAGS_WARN_ON *= -Wshadow-all -Wcast-align -Wcomma -Wconditional-uninitialized -Wheader-hygiene -Wloop-analysis -Wextra-semi-stmt -Wunreachable-code-aggressive
		QMAKE_CXXFLAGS_WARN_ON *= -Wshorten-64-to-32 -Wmissing-prototypes -Wmissing-variable-declarations
		QMAKE_CXXFLAGS_WARN_ON *= -Wimplicit-fallthrough -Wsuggest-override
		QMAKE_CXXFLAGS *= -Werror=return-stack-address -Werror=infinite-recursion
	} else {
		QMAKE_CXXFLAGS_WARN_ON *= -Wshadow -Wcast-align=strict -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wnull-dereference
		QMAKE_CXXFLAGS_WARN_ON *= -Wsuggest-override -Wnoexcept -Wmissing-declarations -Wmismatched-tags -Wunused-const-variable=1
		QMAKE_CXXFLAGS *= -Werror=return-local-addr -Werror=memset-transposed-args -Werror=nonnull-compare -Werror=mismatched-new-delete -Werror=infinite-recursion
		QMAKE_CXXFLAGS *= -Wcatch-value=3 -Werror=catch-value # -Werror=catch-value on its own would only enable level 1
	}

	Release:DEFINES += NDEBUG=1
	Debug:DEFINES += _DEBUG
}

include(logger/logger.pri)
include(appdialogs/appdialogs.pri)
include(settings/settings.pri)
include(taskbarprogress/taskbarprogress.pri)
include(utils/utils.pri)
include(theme/theme.pri)
include(string/string.pri)
include(mouseclickdetector/mouseclickdetector.pri)
include(historylist/historylist.pri)
include(widgets/widgets.pri)
include(dialogs/dialogs.pri)
include(ui/ui.pri)
include(std_helpers/std_helpers.pri)
include(qtcore_helpers/qtcore_helpers.pri)
win*:include(windows/windows.pri)
