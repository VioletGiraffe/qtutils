#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QString>
RESTORE_COMPILER_WARNINGS

#include <stdint.h>

// IEC units. maxUnit caps the unit by its first letter ('B', 'K', 'M', ...); spacer separates thousands in the whole part.
// significantPlaces: the smallest unit whose whole part fits this many digits is used; 0 selects the largest unit that fits.
[[nodiscard]] QString fileSizeToString(uint64_t size, char maxUnit = '\0', const QString& spacer = {}, int significantPlaces = 4);

[[nodiscard]] QString secondsToTimeIntervalString(uint32_t secondsTotal);
