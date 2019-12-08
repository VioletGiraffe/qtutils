#pragma once

#include <algorithm>
#include <iterator>
#include <type_traits>
#include <utility>
#include <vector>

template <class Container>
std::vector<typename Container::value_type> to_vector(const Container& qtContainer)
{
	using T = typename Container::value_type;
	std::vector<T> v;
	v.reserve((size_t)qtContainer.size());

	std::copy(std::cbegin(qtContainer), std::cend(qtContainer), std::back_inserter(v));
	return v;
}

template <class Container>
std::vector<typename Container::value_type> to_vector(Container&& qtContainer)
{
	static_assert (std::is_rvalue_reference_v<decltype(qtContainer)>);
	using T = typename Container::value_type;
	std::vector<T> v;
	const size_t size = static_cast<size_t>(qtContainer.size());
	v.reserve(size);
	for (int i = 0; i < static_cast<int>(size); ++i)
		v.emplace_back(std::move(qtContainer[i]));

	return v;
}

template <typename T, class Container>
std::vector<T> to_vector(const Container& qtContainer)
{
	std::vector<T> v;
	v.reserve((size_t)qtContainer.size());

	std::copy(std::cbegin(qtContainer), std::cend(qtContainer), std::back_inserter(v));
	return v;
}

template <template <typename> class StdContainer, typename T, template <typename> class QtContainer>
QtContainer<T> from_std_container(const StdContainer<T>& stdContainer)
{
	QtContainer<T> qtContainer;
	std::copy(std::cbegin(stdContainer), std::cend(stdContainer), std::back_inserter(qtContainer));

	return qtContainer;
}
