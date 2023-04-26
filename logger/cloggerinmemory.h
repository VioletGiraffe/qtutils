#pragma once

#include "cloggerinterface.h"

class CLoggerInMemory : public CLoggerInterface
{
public:
	void log(const QString& msg) override;
	[[nodiscard]] QStringList contents() const override;

private:
	QStringList _entries;
};
