#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QString>
#include <QtGlobal>
RESTORE_COMPILER_WARNINGS

#include <stdint.h>

// The argument for the platform built for; otherText covers every platform but Windows and macOS.
// All three deduce one T: mixing plain literals with QString takes textForPlatform<QString>(...).
template <typename T>
[[nodiscard]] T textForPlatform([[maybe_unused]] T windowsText, [[maybe_unused]] T macText, [[maybe_unused]] T otherText)
{
#ifdef Q_OS_WIN
	return windowsText;
#elif defined Q_OS_MACOS
	return macText;
#else
	return otherText;
#endif
}

// IEC units. maxUnit caps the unit by its first letter ('B', 'K', 'M', ...); spacer separates thousands in the whole part.
// significantPlaces: the smallest unit whose whole part fits this many digits is used; 0 selects the largest unit that fits.
[[nodiscard]] QString fileSizeToString(uint64_t size, char maxUnit = '\0', const QString& spacer = {}, int significantPlaces = 4);

[[nodiscard]] QString secondsToTimeIntervalString(uint32_t secondsTotal);
