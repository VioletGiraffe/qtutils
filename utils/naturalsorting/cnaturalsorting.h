#ifndef CNATURALSORTING_H
#define CNATURALSORTING_H

struct SortingOptions
{
	SortingOptions(bool digitsAfterLetters = false) :
		_digitsAfterLetters(digitsAfterLetters)
	{}

	bool _digitsAfterLetters;
};

enum NaturalSortingAlgorithm
{
	nsaQtForum
};

class QString;
class CNaturalSorting
{
public:
	CNaturalSorting(NaturalSortingAlgorithm algorithm, SortingOptions options);

	void setSortingOptions(SortingOptions options);
	void setSortingAlgorithm(NaturalSortingAlgorithm algorithm);

	bool lessThan(const QString& l, const QString& r) const;
	static bool lessThan(const QString& l, const QString& r, NaturalSortingAlgorithm algorithm, SortingOptions options);

private:
	SortingOptions          _options;
	NaturalSortingAlgorithm _algorithm;
};

#endif // CNATURALSORTING_H
