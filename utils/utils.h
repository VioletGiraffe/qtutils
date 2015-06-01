#pragma once

#include "../QtIncludes"
#include <functional>
#include <set>
#include <algorithm>
#include <iterator>

// STL utils

namespace detail {
	template <typename T>
	struct NoDuplicatePredicate
	{
		bool operator()(const T& element)
		{
			return _s.insert(element).second; // true if a new element was inserted
		}
	private:
		std::set<T> _s;
	};
}

template <class Container>
inline Container uniqueElements(const Container& c)
{
	Container result;
	detail::NoDuplicatePredicate<typename Container::value_type> predicate;
	std::copy_if(c.begin(), c.end(), std::back_inserter(result), std::ref(predicate));
	return result;
}

template <template<typename...> class OutputContainerType, class ContainerType1, class ContainerType2>
OutputContainerType<typename ContainerType1::value_type, std::allocator<typename ContainerType1::value_type>> setTheoreticDifference(ContainerType1 c1, ContainerType2 c2,
	const std::function<bool(const typename ContainerType1::value_type&, const typename ContainerType1::value_type&)>& comp = [](const typename ContainerType1::value_type& l, const typename ContainerType1::value_type& r) {return l < r;})
{
	OutputContainerType<typename ContainerType1::value_type, std::allocator<typename ContainerType1::value_type>> result;
	std::sort(c1.begin(), c1.end(), comp);
	std::sort(c2.begin(), c2.end(), comp);

	std::set_difference(c1.begin(), c1.end(), c2.begin(), c2.end(), std::back_inserter(result), comp);

	return result;
}

