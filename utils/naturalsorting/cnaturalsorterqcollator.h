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

		[[nodiscard]] inline const QCollator& collator(bool caseSensitive) noexcept {
			thread_local static const QCollator collatorCaseSensitive = numericCollator(Qt::CaseSensitive);
			thread_local static const QCollator collatorCaseInsensitive = numericCollator(Qt::CaseInsensitive);
			return caseSensitive ? collatorCaseSensitive : collatorCaseInsensitive;
		}
	}

	// Negative, zero or positive as l sorts before, together with or after r
	[[nodiscard]] inline int compare(QStringView l, QStringView r, bool caseSensitive = true) noexcept {
		return detail::collator(caseSensitive).compare(l, r);
	}

	// Keys order as compare() orders their strings; comparing two keys is much cheaper than comparing the strings
	[[nodiscard]] inline QCollatorSortKey sortKey(const QString& s, bool caseSensitive = true) {
		return detail::collator(caseSensitive).sortKey(s);
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
