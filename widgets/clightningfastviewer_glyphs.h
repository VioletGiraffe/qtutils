#pragma once

// UTF-8 source: MSVC requires /utf-8, set in qtutils.pro and global.pri.

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QChar>
#include <QFontMetrics>
RESTORE_COMPILER_WARNINGS

#include <array>

// Stand-in glyphs for the characters that show nothing of their own, resolved against one font.
namespace GlyphSubstitution {

inline constexpr bool isPrintableAscii(char16_t code)
{
	return code >= 0x20 && code < 0x7F;
}

// The CP437 glyphs for the C0 controls, as a DOS-era dump showed them.
// Slot 0 is ours, not CP437's: CP437 draws NUL as a blank, which would be indistinguishable from a space.
inline constexpr char16_t cp437ControlGlyphs[0x20] = {
	0x2205, 0x263A, 0x263B, 0x2665, 0x2666, 0x2663, 0x2660, 0x2022, // empty set, smilies, card suits, bullet
	0x25D8, 0x25CB, 0x25D9, 0x2642, 0x2640, 0x266A, 0x266B, 0x263C, // circles, gender signs, notes, sun
	0x25BA, 0x25C4, 0x2195, 0x203C, 0x00B6, 0x00A7, 0x25AC, 0x21A8, // triangles, arrows, pilcrow, section, bar
	0x2191, 0x2193, 0x2192, 0x2190, 0x221F, 0x2194, 0x25B2, 0x25BC  // arrows, right angle, triangles
};

inline constexpr char16_t cp437DeleteGlyph = 0x2302;   // house

// Stand-ins for 0x80-0xFF, 16 per row: the high nibble selects the shape family, so a run of related bytes reads as one texture.
inline constexpr char16_t highByteGlyphs[0x80] = {
	u'▖', u'▗', u'▘', u'▝', u'▚', u'▞', u'▙', u'▛', u'▜', u'▟', u'▀', u'▄', u'▌', u'▐', u'█', u'░', // 0x80, quadrant and half blocks
	u'▒', u'▓', u'◐', u'◑', u'◒', u'◓', u'◔', u'◕', u'◖', u'◗', u'⯊', u'⯋', u'◧', u'◨', u'⬒', u'⬓', // 0x90, shades, then circles and squares half filled
	u'◩', u'◪', u'⬔', u'⬕', u'⬖', u'⬗', u'⬘', u'⬙', u'◭', u'◮', u'◫', u'▣', u'△', u'▽', u'▷', u'◁', // 0xA0, triangles and diamonds half filled
	u'◢', u'◣', u'◤', u'◥', u'◸', u'◹', u'◺', u'◿', u'▴', u'▵', u'▸', u'▹', u'▾', u'▿', u'◂', u'◃', // 0xB0, corners and points: orientation
	u'●', u'◉', u'◎', u'◌', u'◆', u'◇', u'◈', u'⬢', u'⬡', u'⬣', u'⬟', u'⬠', u'■', u'□', u'⬚', u'▢', // 0xC0, closed silhouettes
	u'◜', u'◝', u'◞', u'◟', u'◠', u'◡', u'◚', u'◛', u'▰', u'▱', u'▭', u'▯', u'▤', u'▥', u'▧', u'▨', // 0xD0, arcs, outlines, hatching
	u'▁', u'▂', u'▃', u'▅', u'▆', u'▇', u'▔', u'▏', u'▎', u'▍', u'▋', u'▊', u'▉', u'▕', u'▦', u'▩', // 0xE0, one-axis mass
	u'▪', u'▫', u'◾', u'◽', u'◼', u'◻', u'⬛', u'⬜', u'⬥', u'⬦', u'⬧', u'⬨', u'⬩', u'⬪', u'⬫', u'◊', // 0xF0, size variants
};
inline constexpr char16_t openBoxGlyph = 0x2423;         // marks a no-break space
inline constexpr char16_t invisibleMarkerGlyph = 0x25AF; // white vertical rectangle
inline constexpr char16_t fallbackGlyph = '.';

// Every entry exists in the font the tables were built for and fits one column of it.
struct Tables
{
	std::array<QChar, 256> text; // Text mode, indexed by code point
	std::array<QChar, 256> hex;  // Hex mode ASCII column, indexed by byte
	QChar invisibleMarker;       // Text mode stand-in for the non-printables above U+00FF

	[[nodiscard]] QChar substituteFor(QChar ch) const
	{
		const char16_t code = ch.unicode();
		return code < 0x100 ? text[code] : invisibleMarker;
	}
};

// charWidth is the width of the grid cell both modes paint on
[[nodiscard]] inline Tables buildTables(const QFontMetrics& fontMetrics, int charWidth)
{
	// A glyph the font lacks arrives from a fallback font at its own advance. Text mode draws each stand-in on its own column origin, so anything that
	// fits the cell is safe there; the hex columns batch glyphs into one drawText, where each advances the next, so only an exact fit stays on the grid.
	const auto usable = [&](char16_t code, bool exactWidthRequired) {
		const QChar glyph(code);
		const bool inFont = fontMetrics.inFont(glyph);
		const int advance = fontMetrics.horizontalAdvance(glyph);
		const bool fits = exactWidthRequired ? advance == charWidth : (advance > 0 && advance <= charWidth);
		return inFont && fits ? glyph : QChar(fallbackGlyph);
	};

	Tables tables;
	tables.invisibleMarker = usable(invisibleMarkerGlyph, false);

	tables.text.fill(tables.invisibleMarker); // Covers the C1 controls, the soft hyphen, and every printable slot, which is never read
	for (char16_t code = 0; code < 0x20; ++code)
		tables.text[code] = usable(cp437ControlGlyphs[code], false);
	tables.text[0x7F] = usable(cp437DeleteGlyph, false);
	tables.text[0xA0] = usable(openBoxGlyph, false);

	for (char16_t byte = 0; byte < 256; ++byte)
	{
		if (isPrintableAscii(byte))
			tables.hex[byte] = QChar(byte);
		else if (byte < 0x20)
			tables.hex[byte] = usable(cp437ControlGlyphs[byte], true);
		else if (byte == 0x7F)
			tables.hex[byte] = usable(cp437DeleteGlyph, true);
		else
			tables.hex[byte] = usable(highByteGlyphs[byte - 0x80], true);
	}

	return tables;
}

} // namespace GlyphSubstitution
