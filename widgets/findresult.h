#pragma once

#include <cstdint>

enum class FindResult { NotFound, Found, FoundAfterWrapAround };

// Matches counted from the start of the content, as a forward search steps through them
struct MatchCount
{
	int64_t number = 0; // 1-based place of the selected match; 0 when it is not among the counted ones
	int64_t total = 0;
	bool complete = false; // False when counting stopped at its deadline: 'total' is a lower bound
};
