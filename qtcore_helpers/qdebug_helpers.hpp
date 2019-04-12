#pragma once
#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QByteArray>
#include <QDebug>
RESTORE_COMPILER_WARNINGS

inline QDebug& operator<<(QDebug& debug, const std::string& std_str)
{
	return debug << QString::fromStdString(std_str);
}

inline QDebug operator<<(QDebug&& debug, const std::string& std_str)
{
	return debug << QString::fromStdString(std_str);
}

template <typename T, size_t N>
QDebug operator<<(QDebug&& debug, const std::array<T, N>& ar)
{
	return debug << QByteArray::fromRawData(ar.data(), ar.size()).toHex();
}

template <typename T, size_t N>
QDebug& operator<<(QDebug& debug, const std::array<T, N>& ar)
{
	return debug << QByteArray::fromRawData((const char*)ar.data(), (int)ar.size() * sizeof(T)).toHex();
}