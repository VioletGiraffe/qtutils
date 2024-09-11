#pragma once

class QString;

namespace detail {

void toUncWcharArray(const QString& path, wchar_t* array, size_t arraySize);

} // namespace detail

QString toUncPath(QString somePath);

template <size_t N>
void toUncWcharArray(const QString& path, wchar_t(&array)[N]) {
	detail::toUncWcharArray(path, array, N);
}
