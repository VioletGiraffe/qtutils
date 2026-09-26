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

		// Shared by every key sortKey() makes of an empty string: compare() recognizes it by its data's address.
		// Not thread_local: a key made on one thread must be recognized on another.
		[[nodiscard]] inline const QCollatorSortKey& emptyStringKey() {
			static const QCollatorSortKey key = numericCollator(Qt::CaseSensitive).sortKey(QString{});
			return key;
		}

		// A derived class may form a pointer to QCollatorSortKey's protected data member and apply it to any key
		struct SortKeyAccess : QCollatorSortKey {
			[[nodiscard]] static const void* dataAddress(const QCollatorSortKey& key) noexcept {
				return (key.*(&SortKeyAccess::d)).data();
			}
		};
	}

	// Negative, zero or positive as l sorts before, together with or after r
	[[nodiscard]] inline int compare(QStringView l, QStringView r, bool caseSensitive = true) noexcept {
		return detail::collator(caseSensitive).compare(l, r);
	}

	// Comparing two keys is much cheaper than comparing their strings; compare them with the overload below
	[[nodiscard]] inline QCollatorSortKey sortKey(const QString& s, bool caseSensitive = true) {
		if (s.isEmpty())
			return detail::emptyStringKey();

		return detail::collator(caseSensitive).sortKey(s);
	}

	// Orders as compare() orders the keys' strings; requires keys from sortKey().
	// QCollatorSortKey::compare() alone does not: compare() sorts an empty string first, Qt's macOS keys sort it last.
	[[nodiscard]] inline int compare(const QCollatorSortKey& l, const QCollatorSortKey& r) {
		static const void* const emptyStringKeyData = detail::SortKeyAccess::dataAddress(detail::emptyStringKey());
		const bool lEmpty = detail::SortKeyAccess::dataAddress(l) == emptyStringKeyData;
		const bool rEmpty = detail::SortKeyAccess::dataAddress(r) == emptyStringKeyData;
		if (lEmpty || rEmpty)
			return (int)!lEmpty - (int)!rEmpty;

		return l.compare(r);
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
