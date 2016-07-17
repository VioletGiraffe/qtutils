#include "cloggerinterface.h"
#include "assert/advanced_assert.h"

CLoggerInterface::~CLoggerInterface()
{
}

QStringList CLoggerInterface::contents() const
{
	assert_and_return_unconditional_r("Not implemented", QStringList());
}
