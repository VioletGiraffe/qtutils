#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QByteArrayView>
#include <QString>
RESTORE_COMPILER_WARNINGS

#include <optional>
#include <stddef.h>

[[nodiscard]] size_t lineNumberForPosition(const QString& text, int pos) noexcept;

// The text of `bytes`, empty where they are binary.
// A byte order mark decides the encoding and is dropped from the result.
// Without one the bytes are UTF-8, unless a NUL within the first `sniffBytes` rules that out.
// The default is the span git's own binary heuristic reads.
[[nodiscard]] std::optional<QString> decodedAsText(QByteArrayView bytes, qsizetype sniffBytes = 8000);
