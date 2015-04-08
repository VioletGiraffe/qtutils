#pragma once

struct SortingOptions
{
	explicit SortingOptions(bool digitsAfterLetters = false) :
		_digitsAfterLetters(digitsAfterLetters)
	{}

	bool _digitsAfterLetters;
};

