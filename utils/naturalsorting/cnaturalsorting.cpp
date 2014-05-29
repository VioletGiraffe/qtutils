#include "cnaturalsorting.h"
#include "naturalsorting_qt.h"
#include "../../QtIncludes"

CNaturalSorting::CNaturalSorting(NaturalSortingAlgorithm algorithm, SortingOptions options) : _options(options), _algorithm(algorithm)
{
}

void CNaturalSorting::setSortingOptions(SortingOptions options)
{
	_options = options;
}

void CNaturalSorting::setSortingAlgorithm(NaturalSortingAlgorithm algorithm)
{
	_algorithm = algorithm;
}

bool CNaturalSorting::lessThan(const QString &l, const QString &r) const
{
	return lessThan(l, r, _algorithm, _options);
}

bool CNaturalSorting::equal(const QString & l, const QString & r) const
{
	return compare(l, r) == 0;
}

int CNaturalSorting::compare(const QString & l, const QString & r) const
{
	if (lessThan(l, r))
		return -1;
	else if (lessThan(r, l))
		return 1;
	else
		return 0;
}

bool CNaturalSorting::lessThan(const QString &l, const QString &r, NaturalSortingAlgorithm algorithm, SortingOptions options)
{
	switch(algorithm)
	{
	case nsaQtForum:
		return naturalSortComparisonQt(l, r, options);
	default:
		Q_ASSERT(false);
		return false;
	}
}
