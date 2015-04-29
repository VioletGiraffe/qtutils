#include "cnaturalsorterqcollator.h"

CNaturalSorterQCollator::CNaturalSorterQCollator()
{
	_collator.setCaseSensitivity(Qt::CaseSensitive);
	_collator.setNumericMode(true);
}

bool CNaturalSorterQCollator::compare(const QString & l, const QString & r, SortingOptions /*options*/) const
{
	return _collator.compare(l, r) == -1;
}
