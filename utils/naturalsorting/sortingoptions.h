#pragma once

#include <initializer_list>

struct SortingOptions
{
	enum Options {
		NoSpecialBehavior = 0,
		DigitsAfterLetters = 1
	};

	inline SortingOptions(std::initializer_list<Options>&& options)
	{
		for (auto option : options)
			_options = static_cast<Options>(_options | option);
	}

	[[nodiscard]] inline Options options() const
	{
		return _options;
	}

	[[nodiscard]] inline bool optionEnabled(const Options option) const
	{
		return (_options & option) != 0;
	}

private:
	Options _options = NoSpecialBehavior;
};

