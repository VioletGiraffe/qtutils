#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QString>
RESTORE_COMPILER_WARNINGS

#include <cstdint>
#include <vector>

enum class FindResult { NotFound, Found, FoundAfterWrapAround };

// Matches counted from the start of the content, as a forward search steps through them
struct MatchCount
{
	int64_t number = 0; // 1-based place of the selected match; 0 when it is not among the counted ones
	int64_t total = 0;
	bool complete = false; // False when counting stopped at its deadline: 'total' is a lower bound
};

// Positions in the searched content: characters in text, bytes in binary data
struct MatchRange
{
	int64_t start = 0;
	int64_t length = 0;
};

// Matches found so far, in the order a forward search steps through them
struct MatchList
{
	std::vector<MatchRange> matches;
	bool complete = false; // False when the list stops short of the content's end, at a deadline or a size cap
};

// One match as a list of matches shows it
struct MatchDescription
{
	QString location; // A line number, or an offset in binary data
	QString excerpt; // The match with some of its line around it
	int64_t matchStart = 0; // In 'excerpt'
	int64_t matchLength = 0;
};
