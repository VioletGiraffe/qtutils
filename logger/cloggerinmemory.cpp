#include "cloggerinmemory.h"

void CLoggerInMemory::log(const QString& msg)
{
	_entries.push_back(msg);
}

QStringList CLoggerInMemory::contents() const
{
	return _entries;
}
