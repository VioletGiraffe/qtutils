#pragma once

// Catch2 with printers for the Qt Core types assertions compare. Containers of them, such as QStringList, print as ranges.
// INFO, WARN and FAIL messages stream these types too: they bypass the printers and use operator<<.
// A test binary that uses Qt includes this instead of catch.hpp in every source: a StringMaker specialization must be
// visible wherever its type is printed.
// Catch2's implementation is compiled elsewhere: in catch2_runner, or in the one source that includes test_main.hpp first (cpp-template-utils).

// Submodule includes
#include "compiler/compiler_warnings_control.h"


DISABLE_COMPILER_WARNINGS
#include "3rdparty/catch2/catch.hpp" // cpp-template-utils

#include <QByteArray>
#include <QChar>
#include <QString>
#include <QStringView>
RESTORE_COMPILER_WARNINGS

#include <ostream>
#include <string>

// In QString's namespace, for argument-dependent lookup: Catch's own operator<< overloads hide the global scope from ordinary lookup
QT_BEGIN_NAMESPACE

inline std::ostream& operator<<(std::ostream& stream, const QString& value)
{
	return stream << value.toStdString();
}

inline std::ostream& operator<<(std::ostream& stream, const QStringView value)
{
	return stream << value.toString().toStdString();
}

inline std::ostream& operator<<(std::ostream& stream, const QLatin1StringView value)
{
	return stream << value.toString().toStdString();
}

inline std::ostream& operator<<(std::ostream& stream, const QByteArray& value)
{
	return stream << value.toStdString();
}

inline std::ostream& operator<<(std::ostream& stream, const QChar value)
{
	return stream << QString{ value }.toStdString();
}

QT_END_NAMESPACE

namespace Catch {

template<>
struct StringMaker<QString> {
	static std::string convert(const QString& value)
	{
		return '"' + value.toStdString() + '"';
	}
};

template<>
struct StringMaker<QStringView> {
	static std::string convert(const QStringView value)
	{
		return '"' + value.toString().toStdString() + '"';
	}
};

template<>
struct StringMaker<QLatin1StringView> {
	static std::string convert(const QLatin1StringView value)
	{
		return '"' + value.toString().toStdString() + '"';
	}
};

template<>
struct StringMaker<QByteArray> {
	static std::string convert(const QByteArray& value)
	{
		return '"' + value.toStdString() + '"';
	}
};

template<>
struct StringMaker<QChar> {
	static std::string convert(const QChar value)
	{
		return '\'' + QString{ value }.toStdString() + '\'';
	}
};

} // namespace Catch
