#pragma once

#include "sortingoptions.h"
#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QCollator>
RESTORE_COMPILER_WARNINGS

class CNaturalSorterQCollator
{
public:
	CNaturalSorterQCollator() noexcept;
	[[nodiscard]] bool compare(const QString& l,const QString& r, SortingOptions options) const;

private:
	QCollator _collator;
};
