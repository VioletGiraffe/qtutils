#pragma once

#include "compiler/compiler_warnings_control.h"

class QString;

QString ErrorStringFromLastError();
QString toUncPath(const QString& somePath);
