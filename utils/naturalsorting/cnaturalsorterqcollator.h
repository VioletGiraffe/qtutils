#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QCollator>
RESTORE_COMPILER_WARNINGS

namespace NaturalSort {
	inline bool lessThan(const QString& l, const QString& r, bool caseSensitive = true) noexcept {
		thread_local static QCollator collator = [caseSensitive]() {
			QCollator c;
			c.setCaseSensitivity(caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);
			c.setNumericMode(true);
			return c;
		}();

		// Fix for the new breaking changes in QCollator in Qt 5.14 - null strings are no longer a valid input
		return collator.compare(qToStringViewIgnoringNull(l), qToStringViewIgnoringNull(r)) < 0;
	}

	inline bool lessCaseSensitive(const QString& l, const QString& r) noexcept {
		return lessThan(l, r, true);
	}
	inline bool lessCaseInsensitive(const QString& l, const QString& r) noexcept {
		return lessThan(l, r, false);
	}

}
