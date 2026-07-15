#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QCollator>
RESTORE_COMPILER_WARNINGS

namespace NaturalSort {
	[[nodiscard]] inline bool lessThan(const QString& l, const QString& r, bool caseSensitive = true) noexcept {
		thread_local static QCollator collatorCaseSensitive = []() {
			QCollator c;
			c.setCaseSensitivity(Qt::CaseSensitive);
			c.setNumericMode(true);
			return c;
		}();

		thread_local static QCollator collatorCaseInsensitive = []() {
			QCollator c;
			c.setCaseSensitivity(Qt::CaseInsensitive);
			c.setNumericMode(true);
			return c;
		}();

		// Fix for the new breaking changes in QCollator in Qt 5.14 - null strings are no longer a valid input
		return (caseSensitive ? collatorCaseSensitive : collatorCaseInsensitive).compare(qToStringViewIgnoringNull(l), qToStringViewIgnoringNull(r)) < 0;
	}

	[[nodiscard]] inline bool lessCaseSensitive(const QString& l, const QString& r) noexcept {
		return lessThan(l, r, true);
	}

	[[nodiscard]] inline bool lessCaseInsensitive(const QString& l, const QString& r) noexcept {
		return lessThan(l, r, false);
	}

}
