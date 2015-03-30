#pragma once

#include "../QtIncludes"
#include <functional>
#include <set>
#include <algorithm>

// Qt utils

inline void qtimerSingleShot(const std::function<void()>& lambda, int delay)
{
	QTimer * timer = new QTimer;
	timer->setSingleShot(true);
	QObject::connect(timer, &QTimer::timeout, [=]() {
		(lambda)();
		timer->deleteLater();
	});
	timer->start(delay);
}

// STL utils

namespace detail {
	template <typename T>
	struct NotDuplicatePredicate
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
	detail::NotDuplicatePredicate<typename Container::value_type> predicate;
	std::copy_if(c.begin(), c.end(), std::back_inserter(result), std::ref(predicate));
	return result;
}

