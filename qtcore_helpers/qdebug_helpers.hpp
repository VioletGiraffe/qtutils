#pragma once
#include "compiler/compiler_warnings_control.h"
#include "../std_helpers/qt_string_helpers.hpp"

DISABLE_COMPILER_WARNINGS
#include <QDebug>
RESTORE_COMPILER_WARNINGS

inline QDebug& operator<<(QDebug& debug, const std::string& std_str)
{
	return debug << qStrFromStdStrU8(std_str);
}

inline QDebug operator<<(QDebug&& debug, const std::string& std_str)
{
	return debug << qStrFromStdStrU8(std_str);
}
