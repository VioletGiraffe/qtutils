#pragma once

#include "compiler/compiler_warnings_control.h"
#include "hash/wheathash.hpp" // cpp-template-utils

DISABLE_COMPILER_WARNINGS
#include <QString>
#include <QStringView>
RESTORE_COMPILER_WARNINGS

#include <stddef.h>
#include <type_traits>

// Transparent: with std::equal_to<> as the map's equality, a QString key can be found by a QStringView
struct QStringHash {
	using is_avalanching = std::true_type;
	using is_transparent = void;
	[[nodiscard]] inline size_t operator()(QStringView s) const noexcept {
		return ::wheathash64(s.constData(), s.size() * sizeof(QChar));
	}
};
