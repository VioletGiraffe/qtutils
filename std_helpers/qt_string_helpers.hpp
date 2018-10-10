#pragma once
#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QString>
RESTORE_COMPILER_WARNINGS

#include <string>

inline QString qStrFromStdStrU8(const std::string& str)
{
	return QString::fromUtf8(str.data(), (int)str.size());
}
