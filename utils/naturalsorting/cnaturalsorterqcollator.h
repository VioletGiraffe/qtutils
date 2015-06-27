#pragma once

#include "sortingoptions.h"

#include <QCollator>

class CNaturalSorterQCollator
{
public:
	CNaturalSorterQCollator();
	bool compare(const QString& l,const QString& r, SortingOptions options) const;

private:
	QCollator _collator;
};
