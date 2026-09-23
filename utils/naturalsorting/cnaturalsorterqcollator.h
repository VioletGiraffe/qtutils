#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QCollator>
#include <QLocale>
RESTORE_COMPILER_WARNINGS

namespace NaturalSort {
	namespace detail {
		// QCollator does not collate in the C locale, numeric mode included: English stands in for it
		[[nodiscard]] inline QCollator numericCollator(Qt::CaseSensitivity caseSensitivity) {
			const QLocale collationLocale = QLocale().collation();
			QCollator c{ collationLocale.language() == QLocale::C ? QLocale{ QLocale::English } : collationLocale };
			c.setCaseSensitivity(caseSensitivity);
			c.setNumericMode(true);
			return c;
		}
	}

	// Negative, zero or positive as l sorts before, together with or after r
	[[nodiscard]] inline int compare(const QString& l, const QString& r, bool caseSensitive = true) noexcept {
		thread_local static QCollator collatorCaseSensitive = detail::numericCollator(Qt::CaseSensitive);
		thread_local static QCollator collatorCaseInsensitive = detail::numericCollator(Qt::CaseInsensitive);

		// Fix for the new breaking changes in QCollator in Qt 5.14 - null strings are no longer a valid input
		return (caseSensitive ? collatorCaseSensitive : collatorCaseInsensitive).compare(qToStringViewIgnoringNull(l), qToStringViewIgnoringNull(r));
	}

	[[nodiscard]] inline bool lessThan(const QString& l, const QString& r, bool caseSensitive = true) noexcept {
		return compare(l, r, caseSensitive) < 0;
	}

	[[nodiscard]] inline bool lessCaseSensitive(const QString& l, const QString& r) noexcept {
		return lessThan(l, r, true);
	}

	[[nodiscard]] inline bool lessCaseInsensitive(const QString& l, const QString& r) noexcept {
		return lessThan(l, r, false);
	}

}
