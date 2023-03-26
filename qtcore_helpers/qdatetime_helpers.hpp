#pragma once
#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QDateTime>
RESTORE_COMPILER_WARNINGS

[[nodiscard]] inline time_t toTime_t(const QDateTime& date) noexcept
{
	return static_cast<time_t>(date.toMSecsSinceEpoch() / 1000LL);
}

[[nodiscard]]inline QDateTime fromTime_t(const time_t time) noexcept
{
	QDateTime date;
	date.setMSecsSinceEpoch(static_cast<qint64>(time) * 1000LL);
	return date;
}
