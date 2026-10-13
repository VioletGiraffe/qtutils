#include "displayformatting.h"

#include "compiler/compiler_warnings_control.h"
#include "lang/type_traits_fast.hpp"

DISABLE_COMPILER_WARNINGS
#include <QObject>
#include <QString>
#include <QStringBuilder>
RESTORE_COMPILER_WARNINGS

#include <cctype>
#include <iterator>

QString fileSizeToString(uint64_t size, const char maxUnit, const QString& spacer, int significantPlaces)
{
	static constexpr uint64_t KB = 1024ULL;
	static constexpr uint64_t MB = 1024ULL * KB;
	static constexpr uint64_t GB = 1024ULL * MB;
	static constexpr uint64_t TB = 1024ULL * GB;
	static constexpr uint64_t PB = 1024ULL * TB;
	static constexpr uint64_t EB = 1024ULL * PB;

	struct Unit { uint64_t threshold; const char* label; };
	static constexpr Unit units[]{
		{ EB, "EiB" },
		{ PB, "PiB" },
		{ TB, "TiB" },
		{ GB, "GiB" },
		{ MB, "MiB" },
		{ KB, "KiB" },
		{ 0ULL, "B" }
	};

	const char maxUnitUpper = static_cast<char>(std::toupper(static_cast<unsigned char>(maxUnit)));

	const uint64_t maxUnitSize = [maxUnitUpper]() -> uint64_t {
		if (maxUnitUpper == 'B')
			return 0ULL; // bytes-only sentinel
		for (const auto& u : units) {
			if (u.label[0] == maxUnitUpper)
				return u.threshold;
		}
		return uint64_max;
	}();

	// First pass: try to satisfy significantPlaces (if non-zero)
	const Unit* chosen = nullptr;
	if (significantPlaces > 0)
	{
		// iterate from smallest to largest unit
		for (int i = std::size(units) - 1; i >= 0; --i)
		{
			const auto& u = units[i];

			if (maxUnitSize < u.threshold)
				continue;

			if (u.threshold != 0ULL)
			{
				if (size < u.threshold)
					continue;
			}

			const uint64_t whole = (u.threshold == 0ULL) ? size : (size / u.threshold);

			int digits = 1;
			for (uint64_t tmp = whole; tmp >= 10; tmp /= 10, ++digits) {}

			if (digits <= significantPlaces)
			{
				chosen = &u;
				break; // smallest unit that fits
			}
		}
	}

	// Fallback: only use the unit threshold if none was chosen above (or significantPlaces == 0)
	if (!chosen)
	{
		for (const auto& u : units)
		{
			if (maxUnitSize < u.threshold)
				continue;
			if (u.threshold == 0ULL || size >= u.threshold)
			{
				chosen = &u;
				break;
			}
		}
	}

	// Safety: if still not chosen (shouldn't happen), use bytes
	if (!chosen)
	{
		chosen = &units[6]; // bytes entry
	}

	// Build numeric string separately so we can insert the spacer into the whole part only
	QString numeric;
	if (chosen->threshold == 0ULL)
	{
		numeric = QString::number(size);
	}
	else
	{
		double n = static_cast<double>(size) / static_cast<double>(chosen->threshold);
		numeric = QString::number(n, 'f', 1);
	}

	// Insert thousands spacer into whole part only
	if (!spacer.isEmpty())
	{
		const qsizetype dotPos = numeric.indexOf('.');
		const qsizetype wholeLen = (dotPos == -1) ? numeric.size() : dotPos;
		for (qsizetype pos = wholeLen - 3; pos > 0; pos -= 3)
			numeric.insert(pos, spacer);
	}

	QString result;
	if (chosen->threshold == 0ULL)
	{
		result = QStringLiteral("%1 B").arg(numeric);
	}
	else
	{
		result = QStringLiteral("%1 %2").arg(numeric, QString::fromLatin1(chosen->label));
	}

	return result;
}

QString secondsToTimeIntervalString(uint32_t secondsTotal)
{
	const auto hours = secondsTotal / 3600;
	const auto minutes = (secondsTotal - (hours * 3600)) / 60;
	const auto seconds = secondsTotal % 60;

	QString result;
	if (hours > 0)
		result = QString::number(hours) % ' ' % QObject::tr("hours");

	if (minutes > 0)
	{
		if (hours > 0)
			result += ' ';
		result += QString::number(minutes) % ' ' % QObject::tr("minutes");
	}

	if (minutes > 0 || hours > 0)
		result += ' ';

	result += QString::number(seconds) % ' ' % QObject::tr("seconds");

	return result;
}
