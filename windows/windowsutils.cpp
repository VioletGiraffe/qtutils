#include "windowsutils.h"

#include "assert/advanced_assert.h"
#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QString>
RESTORE_COMPILER_WARNINGS

template <size_t N>
inline wchar_t* append(wchar_t* dst, const wchar_t (&src)[N])
{
	::memcpy(dst, src, N * sizeof(wchar_t));
	return dst + N - 1; // Null was already part of the input literal
}

QString toUncPath(QString somePath)
{
	somePath.replace('/', '\\');

	if (somePath.contains(':')) // Quick and dirty test for absolute path
		return somePath.prepend(R"(\\?\)");
	else
		return somePath;
}

void detail::toUncWcharArray(const QString& somePath, wchar_t* array, size_t arraySize)
{
	wchar_t* currentStrStart = array;
	if (somePath.contains(':')) // Quick and dirty test for absolute path
		currentStrStart = append(array, LR"(\\?\)");

	const size_t len = (size_t)somePath.toWCharArray(currentStrStart);
	assert_debug_only(len < arraySize - (array - currentStrStart));

	// Replace backslashes with forward slashes
	for (size_t i = 0; i < len; ++i)
	{
		if (currentStrStart[i] == L'\\')
			currentStrStart[i] = L'/';
	}

	currentStrStart[len] = L'\0';
}
