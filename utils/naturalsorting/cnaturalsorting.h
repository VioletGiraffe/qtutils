#pragma once

#include "sortingoptions.h"
#include "cnaturalsorterqcollator.h"

enum NaturalSortingAlgorithm
{
	nsaQtForum,
	nsaQCollator
};

class QString;
class CNaturalSorting
{
public:
	CNaturalSorting(NaturalSortingAlgorithm algorithm, SortingOptions options);

	void setSortingOptions(SortingOptions options);
	void setSortingAlgorithm(NaturalSortingAlgorithm algorithm);

	[[nodiscard]] bool lessThan(const QString& l, const QString& r) const;
	[[nodiscard]] bool equal(const QString& l, const QString& r) const;
	[[nodiscard]] int compare(const QString& l, const QString& r) const;

private:
	SortingOptions          _options;
	NaturalSortingAlgorithm _algorithm;

	CNaturalSorterQCollator _collatorSorter;
};
