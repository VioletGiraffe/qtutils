#include "stringutils.h"

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QString>
RESTORE_COMPILER_WARNINGS

#include <algorithm>

size_t lineNumberForPosition(const QString &text, int pos) noexcept
{
	return static_cast<size_t>(std::count(text.begin(), text.begin() + pos, '\n'));
}
