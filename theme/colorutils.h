#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QColor>
RESTORE_COMPILER_WARNINGS

#include <algorithm>
#include <cmath>

namespace ColorUtils
{
	namespace detail {
		inline double linearized(double channel)
		{
			return channel <= 0.03928 ? channel / 12.92 : std::pow((channel + 0.055) / 1.055, 2.4);
		}
	}

	// t = 0 yields `a`, t = 1 yields `b`. Blended in sRGB rather than linear space, unlike the luminance
	// maths below: these are colours picked by eye, and linear mixing shifts them off what was chosen.
	[[nodiscard]] inline QColor mix(const QColor& a, const QColor& b, float t)
	{
		return QColor::fromRgbF(
			std::lerp(a.redF(), b.redF(), t),
			std::lerp(a.greenF(), b.greenF(), t),
			std::lerp(a.blueF(), b.blueF(), t));
	}

	[[nodiscard]] inline double relativeLuminance(const QColor& c)
	{
		return 0.2126 * detail::linearized(c.redF()) + 0.7152 * detail::linearized(c.greenF())
			+ 0.0722 * detail::linearized(c.blueF());
	}

	// The WCAG ratio, from 1 (identical) to 21 (black on white).
	[[nodiscard]] inline double contrastRatio(const QColor& a, const QColor& b)
	{
		const double la = relativeLuminance(a), lb = relativeLuminance(b);
		return (std::max(la, lb) + 0.05) / (std::min(la, lb) + 0.05);
	}
}
