#include "cloggerinmemory.h"

void CLoggerInMemory::log(const QString& msg)
{
	std::lock_guard lock{ _mutex };
	_entries.push_back(msg);
}

QStringList CLoggerInMemory::contents() const
{
	std::lock_guard lock{ _mutex };
	return _entries;
}
