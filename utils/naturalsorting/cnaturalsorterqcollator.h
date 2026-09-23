#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QCollator>
#include <QLocale>
#include <QStringView>
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
	[[nodiscard]] inline int compare(QStringView l, QStringView r, bool caseSensitive = true) noexcept {
		thread_local static QCollator collatorCaseSensitive = detail::numericCollator(Qt::CaseSensitive);
		thread_local static QCollator collatorCaseInsensitive = detail::numericCollator(Qt::CaseInsensitive);

		return (caseSensitive ? collatorCaseSensitive : collatorCaseInsensitive).compare(l, r);
	}

	[[nodiscard]] inline bool lessThan(QStringView l, QStringView r, bool caseSensitive = true) noexcept {
		return compare(l, r, caseSensitive) < 0;
	}

	[[nodiscard]] inline bool lessCaseSensitive(QStringView l, QStringView r) noexcept {
		return lessThan(l, r, true);
	}

	[[nodiscard]] inline bool lessCaseInsensitive(QStringView l, QStringView r) noexcept {
		return lessThan(l, r, false);
	}

}
