#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QStringList>
RESTORE_COMPILER_WARNINGS

class CLoggerInterface
{
public:
	virtual ~CLoggerInterface();

	virtual void log(const QString& message) = 0;

	virtual QStringList contents() const;
};

template <class LoggerType>
CLoggerInterface& loggerInstance()
{
	static LoggerType instance;
	return instance;
}

