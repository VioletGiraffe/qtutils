#include "cnaturalsorterqcollator.h"

CNaturalSorterQCollator::CNaturalSorterQCollator()
{
	_collator.setCaseSensitivity(Qt::CaseSensitive);
	_collator.setNumericMode(true);
}

bool CNaturalSorterQCollator::compare(const QString & l, const QString & r, SortingOptions /*options*/) const
{
	// Fix for the new breaking changes in QCollator in Qt 5.14 - null strings are no longer a valid input
	const auto leftIsEmpty = l.isEmpty();
	if (leftIsEmpty != r.isEmpty())
		return leftIsEmpty;
	else
		return leftIsEmpty ? false : _collator.compare(l, r) < 0;
}
