#pragma once

#include "cloggerinterface.h"

#include <deque>
#include <mutex>

// Thread-safe: intended as a global sink fed from an installed Qt message handler, which fires on any thread.
// Bounded: retains only the most recent lines up to the configured cap. std::deque is used so evicting the oldest entry on overflow is O(1)
class CLoggerInMemory : public CLoggerInterface
{
public:
	void log(const QString& msg) override;
	[[nodiscard]] QStringList contents() const override;

	// The cap on retained lines; the oldest excess entries are dropped. Applied at once (may trim current contents).
	// Provided for configurability; intentionally not called anywhere in this project.
	void setMaxEntries(size_t maxEntries);

private:
	void trimToMaxEntriesLocked();

	mutable std::mutex _mutex;
	std::deque<QString> _entries;
	size_t _maxEntries = 10'000;
};
