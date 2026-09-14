#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QColor>
#include <QPalette>
RESTORE_COMPILER_WARNINGS

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <span>

namespace ColorUtils
{
	namespace detail {
		inline double linearized(float channel)
		{
			const double value = static_cast<double>(channel);
			return value <= 0.03928 ? value / 12.92 : std::pow((value + 0.055) / 1.055, 2.4);
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

	// 'preferred' while it keeps 'minContrast' on 'fill', otherwise black or white, whichever contrasts more
	[[nodiscard]] inline QColor readableTextOn(const QColor& fill, const QColor& preferred, double minContrast = 4.5)
	{
		if (contrastRatio(preferred, fill) >= minContrast)
			return preferred;

		return contrastRatio(Qt::black, fill) >= contrastRatio(Qt::white, fill) ? QColor{ Qt::black } : QColor{ Qt::white };
	}

	// Hues spread around the colour wheel, in order of preference: yellow, magenta, green, cyan
	inline constexpr std::array<int, 4> spreadHues{ 60, 300, 120, 180 };

	// The first of 'candidateHues' at least 'minDistance' degrees from every saturated palette accent (Highlight, Accent, Link),
	// or else the candidate furthest from its nearest accent. For marks meant to stand out from the theme. Requires a candidate.
	[[nodiscard]] inline int hueDistinctFromPalette(const QPalette& palette, std::span<const int> candidateHues = spreadHues, int minDistance = 60)
	{
		const auto distanceToNearestAccent = [&palette](int hue) {
			int nearest = 180;
			for (const QPalette::ColorRole role : { QPalette::Highlight, QPalette::Accent, QPalette::Link })
			{
				const QColor color = palette.color(role);
				if (color.hsvSaturation() < 64) // A grey has no hue to clash with
					continue;

				const int distance = std::abs(hue - color.hsvHue());
				nearest = std::min({ nearest, distance, 360 - distance });
			}
			return nearest;
		};

		for (const int hue : candidateHues)
		{
			if (distanceToNearestAccent(hue) >= minDistance)
				return hue;
		}

		return *std::ranges::max_element(candidateHues, {}, distanceToNearestAccent);
	}

	// 'hue' at full saturation, its lightness stepped away from 'background' until it reaches 'targetContrast' against it.
	// Ends at white or black when the contrast is out of reach.
	[[nodiscard]] inline QColor saturatedColorWithContrast(int hue, const QColor& background, double targetContrast)
	{
		const double backgroundLuminance = relativeLuminance(background);
		const bool lighterThanBackground = contrastRatio(background, Qt::white) > contrastRatio(background, Qt::black);

		constexpr int steps = 50;
		QColor color;
		for (int step = 0; step <= steps; ++step)
		{
			const float lightness = static_cast<float>(lighterThanBackground ? step : steps - step) / steps;
			color = QColor::fromHslF(static_cast<float>(hue) / 360.0f, 1.0f, lightness);

			// A colour on the near side of the background's luminance reaches the contrast in the wrong direction
			const double luminance = relativeLuminance(color);
			const bool pastBackground = lighterThanBackground ? luminance > backgroundLuminance : luminance < backgroundLuminance;
			if (pastBackground && contrastRatio(color, background) >= targetContrast)
				break;
		}

		return color;
	}

	enum class SearchMatch { Current, Other };

	// Fill marking a search match: a hue distinct from the palette, the current match contrasting more with Base than the others
	[[nodiscard]] inline QColor searchMatchFill(const QPalette& palette, SearchMatch match)
	{
		return saturatedColorWithContrast(hueDistinctFromPalette(palette), palette.color(QPalette::Base), match == SearchMatch::Current ? 3.0 : 1.8);
	}
}
