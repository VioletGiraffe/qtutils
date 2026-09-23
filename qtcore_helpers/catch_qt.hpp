#pragma once

// Catch2 with printers for the Qt Core types assertions compare. Containers of them, such as QStringList, print as ranges.
// A test binary that uses Qt includes this instead of catch.hpp in every source: a StringMaker specialization must be
// visible wherever its type is printed.
// A source defining main() includes test_main.hpp (cpp-template-utils) first, so that catch.hpp is compiled with the runner.

// Submodule includes
#include "compiler/compiler_warnings_control.h"


DISABLE_COMPILER_WARNINGS
#include "3rdparty/catch2/catch.hpp" // cpp-template-utils

#include <QByteArray>
#include <QChar>
#include <QString>
#include <QStringView>
RESTORE_COMPILER_WARNINGS

#include <string>

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
