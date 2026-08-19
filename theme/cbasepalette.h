#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QColor>
RESTORE_COMPILER_WARNINGS

class QPalette;

// The colour vocabulary every app using this framework shares. Apps keep their own tokens beside it;
// only these are understood here, and only these feed the QPalette mapping below.
//
// The nine core fields must be authored. The seven derived fields may be left default-constructed,
// and resolvedPalette() computes them from the core - author one only where a palette genuinely
// differs from what the derivation produces.
struct CBasePalette
{
	// Core - required.
	QColor windowBg;    // the window's own background behind the content surfaces: bars, gaps between panes
	QColor surface;     // content surfaces
	QColor surfaceAlt;  // offset surface: secondary bars, alternating rows
	QColor text;
	QColor textDim;     // secondary text, placeholders, disabled states
	QColor button;
	QColor accent;      // primary action and emphasis
	QColor selectionBg;
	QColor selectionFg;

	// Derived - optional.
	QColor border;        // default container outline
	QColor borderSubtle;  // separators within one surface
	QColor borderStrong;  // edges of interactive controls
	QColor accentFg;      // text and glyphs drawn on top of `accent`
	QColor accentText;    // accent-coloured text on an ordinary surface
	QColor accentBg;      // accent-tinted surface
	QColor buttonBorder;
};

// Fills in every derived field left default-constructed, leaving authored ones untouched. Idempotent,
// so resolving an already-resolved palette is harmless.
[[nodiscard]] CBasePalette resolvedPalette(CBasePalette authored);

// Maps the palette onto QPalette roles so stock controls and native fallbacks follow the theme.
// The palette must be resolved.
[[nodiscard]] QPalette qtPaletteFor(const CBasePalette& palette);
