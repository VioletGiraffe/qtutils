#pragma once

class QString;

namespace windows_utils_detail {

size_t toUncWcharArray(const QString& path, wchar_t* array, size_t arraySize);

} // namespace windows_utils_detail

template <size_t N>
size_t toUncWcharArray(const QString& path, wchar_t(&array)[N]) {
	return windows_utils_detail::toUncWcharArray(path, array, N);
}
