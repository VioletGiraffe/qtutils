#include "cnaturalsorting.h"
#include "naturalsorting_qt.h"
#include "../../QtIncludes"

CNaturalSorting::CNaturalSorting(NaturalSortingAlgorithm algorithm, SortingOptions options) : _algorithm(algorithm), _options(options)
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
