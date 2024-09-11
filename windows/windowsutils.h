#pragma once

class QString;

namespace detail {

size_t toUncWcharArray(const QString& path, wchar_t* array, size_t arraySize);

} // namespace detail

template <size_t N>
size_t toUncWcharArray(const QString& path, wchar_t(&array)[N]) {
	return detail::toUncWcharArray(path, array, N);
}
