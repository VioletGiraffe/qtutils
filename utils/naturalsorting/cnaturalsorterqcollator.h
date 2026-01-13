#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QCollator>
RESTORE_COMPILER_WARNINGS

class CNaturalSorterQCollator
{
public:
	inline CNaturalSorterQCollator() noexcept {
		_collator.setCaseSensitivity(Qt::CaseSensitive);
		_collator.setNumericMode(true);
	}

	inline [[nodiscard]] bool lessThan(const QString& l, const QString& r) const noexcept {
		// Fix for the new breaking changes in QCollator in Qt 5.14 - null strings are no longer a valid input
		return _collator.compare(qToStringViewIgnoringNull(l), qToStringViewIgnoringNull(r)) < 0;
	}

private:
	QCollator _collator;
};
