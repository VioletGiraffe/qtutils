#pragma once

#include <stddef.h>

class QString;

[[nodiscard]] size_t lineNumberForPosition(const QString& text, int pos) noexcept;
