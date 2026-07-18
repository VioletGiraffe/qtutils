#pragma once

#include "cloggerinterface.h"

#include <mutex>

// Thread-safe: intended as a global sink fed from an installed Qt message handler, which fires on any thread.
class CLoggerInMemory : public CLoggerInterface
{
public:
	void log(const QString& msg) override;
	[[nodiscard]] QStringList contents() const override;

private:
	mutable std::mutex _mutex;
	QStringList _entries;
};
