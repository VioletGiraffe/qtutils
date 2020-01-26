#pragma once

struct SortingOptions
{
	inline explicit SortingOptions(bool digitsAfterLetters = false) :
		_digitsAfterLetters(digitsAfterLetters)
	{}

	bool _digitsAfterLetters;
};

