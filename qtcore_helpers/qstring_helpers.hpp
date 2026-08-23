#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QtCore/qstringliteral.h>
RESTORE_COMPILER_WARNINGS

#define QSL QStringLiteral
#define QL1 QLatin1String
