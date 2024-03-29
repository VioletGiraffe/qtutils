#pragma once
#include "assert/advanced_assert.h"
#include "container/set_operations.hpp"
#include "lang/type_traits_fast.hpp"

#include <algorithm>
#include <deque>

template <typename T>
class CHistoryList
{
public:
	using container_type = std::deque<T>;
	explicit CHistoryList(size_t maxSize = 2000) noexcept :
		_currentIndex{ size_t_max },
		_maxSize{ maxSize }
	{}

	// Access and status
	[[nodiscard]] inline bool empty() const { return _list.empty(); }
	[[nodiscard]] inline size_t size() const { return _list.size(); }
	inline void clear() { _list.clear(); }

	[[nodiscard]] inline typename container_type::const_iterator begin() const { return _list.begin(); }
	[[nodiscard]] inline typename container_type::const_iterator end() const { return _list.end(); }
	[[nodiscard]] inline typename container_type::const_reverse_iterator rbegin() const { return _list.rbegin(); }
	[[nodiscard]] inline typename container_type::const_reverse_iterator rend() const { return _list.rend(); }

	[[nodiscard]] inline const T& front() const { return _list.front(); }
	[[nodiscard]] inline const T& back() const { return _list.back(); }

	[[nodiscard]] inline const T& operator[](size_t index) const { return _list[index]; }

	[[nodiscard]] inline bool historyLocationSet() const { return _currentIndex <= _list.size(); }

	[[nodiscard]] inline const container_type& list() const { return _list; }

	[[nodiscard]] size_t currentIndex() const;
	[[nodiscard]] const T& currentItem() const;

	[[nodiscard]] bool isAtEnd() const;
	[[nodiscard]] bool isAtBeginning() const;

	// Actions
	void addLatest(const T& item);
	void addLatest(const std::vector<T>& items);
	const T& navigateBack();
	const T& navigateForward();

private:
	container_type _list;
	size_t         _currentIndex;
	const size_t   _maxSize;
};

template <typename T>
const T& CHistoryList<T>::navigateForward()
{
	if (_currentIndex < size() - 1)
		_currentIndex = currentIndex() + 1;
	return currentItem();
}

template <typename T>
const T& CHistoryList<T>::navigateBack()
{
	if (_currentIndex > 0)
		_currentIndex = currentIndex() - 1;
	return currentItem();
}

template <typename T>
const T& CHistoryList<T>::currentItem() const
{
	if (!empty())
		return _list[currentIndex()];
	else
	{
		static const T blank;
		return blank;
	}
}

template <typename T>
bool CHistoryList<T>::isAtEnd() const
{
	assert_r(empty() || _currentIndex < size());
	return !empty() && _currentIndex == size() - 1;
}

template <typename T>
bool CHistoryList<T>::isAtBeginning() const
{
	assert_r(empty() || _currentIndex < size());
	return !empty() && _currentIndex == 0;
}

template <typename T>
size_t CHistoryList<T>::currentIndex() const
{
	if (empty())
		return size_t_max;
	else if (_currentIndex < size())
		return _currentIndex;
	else
		return size() - 1;
}

template <typename T>
void CHistoryList<T>::addLatest(const T& item)
{
	if (currentItem() == item)
		return;

	const auto currentPosition = std::find(_list.begin(), _list.end(), item);
	if (currentPosition == _list.end())
		_list.push_back(item);
	else if (size() > 1 && currentPosition != _list.end() - 1)
		std::rotate(currentPosition, currentPosition + 1, _list.end());

	assert_r(!empty());
	if (size() > 1 && _currentIndex <= size() - 2) // If we already were at the end of the list, just move the index to the newest element
	{
		// If we were not in the end, move and insert [0; _currentIndex] just before end
		decltype(_list) newList;
		for (size_t i = _currentIndex + 1; i < size() - 1; ++i)
			newList.push_back(std::move(_list[i]));
		for (size_t i = 0; i <= _currentIndex; ++i)
			newList.push_back(std::move(_list[i]));
		_list = newList;
		_list.emplace_back(item);
	}

	if (_list.size() > _maxSize)
		_list.pop_front();

	_currentIndex = _list.size() - 1;
}

template <typename T>
void CHistoryList<T>::addLatest(const std::vector<T>& items)
{
	_list = container_type(items.begin(), items.end());
	_list = SetOperations::uniqueElements<SetOperations::ItemOrder::KeepLastOccurrence>(_list);
	// Trim to size - erase old (first) items
	if (const auto size = _list.size(); size > _maxSize)
		_list.erase(_list.begin(), _list.begin() + size - _maxSize);

	_currentIndex = _list.size() - 1;
}
