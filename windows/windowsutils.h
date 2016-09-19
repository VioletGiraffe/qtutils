#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QString>
RESTORE_COMPILER_WARNINGS

QString ErrorStringFromLastError();
QString toUncPath(const QString& somePath);
