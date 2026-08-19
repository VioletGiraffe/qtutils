#include "cbasepalette.h"

#include "assert/advanced_assert.h"

DISABLE_COMPILER_WARNINGS
#include <QPalette>
RESTORE_COMPILER_WARNINGS

#include <algorithm>
#include <cmath>

namespace {

// t = 0 yields `a`, t = 1 yields `b`. Blended in sRGB rather than linear space, unlike the luminance
// maths below: these are colours picked by eye, and linear mixing shifts them off what was chosen.
QColor mix(const QColor& a, const QColor& b, float t)
{
	return QColor::fromRgbF(
		std::lerp(a.redF(), b.redF(), t),
		std::lerp(a.greenF(), b.greenF(), t),
		std::lerp(a.blueF(), b.blueF(), t));
}

double linearized(double channel)
{
	return channel <= 0.03928 ? channel / 12.92 : std::pow((channel + 0.055) / 1.055, 2.4);
}

double relativeLuminance(const QColor& c)
{
	return 0.2126 * linearized(c.redF()) + 0.7152 * linearized(c.greenF()) + 0.0722 * linearized(c.blueF());
}

double contrastRatio(const QColor& a, const QColor& b)
{
	const double la = relativeLuminance(a), lb = relativeLuminance(b);
	return (std::max(la, lb) + 0.05) / (std::min(la, lb) + 0.05);
}

} // namespace

CBasePalette resolvedPalette(CBasePalette p)
{
	// Ordered: the border variants and buttonBorder read `border`, so it settles first.
	if (!p.border.isValid())
		p.border = mix(p.surface, p.text, 0.16f);
	if (!p.borderSubtle.isValid())
		p.borderSubtle = mix(p.surface, p.border, 0.55f);
	if (!p.borderStrong.isValid())
		p.borderStrong = mix(p.border, p.text, 0.35f);
	if (!p.buttonBorder.isValid())
		p.buttonBorder = p.border;

	if (!p.accentText.isValid())
		p.accentText = p.accent;
	if (!p.accentBg.isValid())
		p.accentBg = mix(p.surface, p.accent, 0.15f);
	if (!p.accentFg.isValid())
	{
		// Luminance, not lightness: a saturated blue reads as mid-lightness in HSL while carrying
		// little luminance, and picking on lightness puts dark glyphs on it.
		p.accentFg = contrastRatio(p.accent, p.surface) >= contrastRatio(p.accent, p.text) ? p.surface : p.text;
	}

	return p;
}

QPalette qtPaletteFor(const CBasePalette& p)
{
	assert_debug_only(p.border.isValid() && p.accentFg.isValid()); // resolvedPalette() was not called

	// Seeding from button and window lets Qt derive the bevel family (Light, Midlight, Dark, Shadow)
	// from our colours rather than leaving them on the platform palette, where they would not follow
	// the theme.
	QPalette palette{ p.button, p.windowBg };

	palette.setColor(QPalette::Window, p.windowBg);
	palette.setColor(QPalette::WindowText, p.text);
	palette.setColor(QPalette::Base, p.surface);
	palette.setColor(QPalette::AlternateBase, p.surfaceAlt);
	palette.setColor(QPalette::Text, p.text);
	palette.setColor(QPalette::Button, p.button);
	palette.setColor(QPalette::ButtonText, p.text);
	palette.setColor(QPalette::Mid, p.textDim);
	palette.setColor(QPalette::Highlight, p.selectionBg);
	palette.setColor(QPalette::HighlightedText, p.selectionFg);
	palette.setColor(QPalette::PlaceholderText, p.textDim);
	palette.setColor(QPalette::Link, p.accentText);
	palette.setColor(QPalette::LinkVisited, p.accentText);
	palette.setColor(QPalette::ToolTipBase, p.surface);
	palette.setColor(QPalette::ToolTipText, p.text);

	palette.setColor(QPalette::Disabled, QPalette::WindowText, p.textDim);
	palette.setColor(QPalette::Disabled, QPalette::Text, p.textDim);
	palette.setColor(QPalette::Disabled, QPalette::ButtonText, p.textDim);

	return palette;
}
