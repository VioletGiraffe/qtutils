#include "cbasepalette.h"
#include "colorutils.h"

#include "assert/advanced_assert.h"

DISABLE_COMPILER_WARNINGS
#include <QPalette>
RESTORE_COMPILER_WARNINGS

CBasePalette resolvedPalette(CBasePalette p)
{
	// Ordered: the border variants and buttonBorder read `border`, so it settles first.
	if (!p.border.isValid())
		p.border = ColorUtils::mix(p.surface, p.text, 0.16f);
	if (!p.borderSubtle.isValid())
		p.borderSubtle = ColorUtils::mix(p.surface, p.border, 0.55f);
	if (!p.borderStrong.isValid())
		p.borderStrong = ColorUtils::mix(p.border, p.text, 0.35f);
	if (!p.buttonBorder.isValid())
		p.buttonBorder = p.border;

	if (!p.accentText.isValid())
		p.accentText = p.accent;
	if (!p.accentBg.isValid())
		p.accentBg = ColorUtils::mix(p.surface, p.accent, 0.15f);
	if (!p.accentFg.isValid())
	{
		// Luminance, not lightness: a saturated blue reads as mid-lightness in HSL while carrying
		// little luminance, and picking on lightness puts dark glyphs on it.
		p.accentFg = ColorUtils::contrastRatio(p.accent, p.surface) >= ColorUtils::contrastRatio(p.accent, p.text) ? p.surface : p.text;
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
