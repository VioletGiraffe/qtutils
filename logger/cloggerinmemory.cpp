#include "cloggerinmemory.h"

void CLoggerInMemory::log(const QString& msg)
{
	std::lock_guard lock{ _mutex };
	_entries.push_back(msg);
	trimToMaxEntriesLocked();
}

QStringList CLoggerInMemory::contents() const
{
	std::lock_guard lock{ _mutex };
	return QStringList(_entries.cbegin(), _entries.cend());
}

void CLoggerInMemory::setMaxEntries(size_t maxEntries)
{
	std::lock_guard lock{ _mutex };
	_maxEntries = maxEntries;
	trimToMaxEntriesLocked();
}

void CLoggerInMemory::trimToMaxEntriesLocked()
{
	while (_entries.size() > _maxEntries)
		_entries.pop_front();
}
