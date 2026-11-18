#pragma once

#include "cglowunderlay.h"
#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QColor>
#include <QLineEdit>
#include <QRect>
#include <QString>
RESTORE_COMPILER_WARNINGS

#include <memory>
#include <optional>

class QTimer;

class CLineEdit : public QLineEdit
{
public:
	using QLineEdit::QLineEdit;
	~CLineEdit() override;

	void setSelectAllOnFocus(bool select);
	[[nodiscard]] bool selectAllOnFocus() const;

	// A text-mode cursor: a cell filled with `fill` over the character after the cursor, the character redrawn in `text`.
	// Blinks at the platform's rate while the field has focus.
	// glowRadius: a halo in the fill colour, 0 for none. A CGlowUnderlay paints it: it shows through transparent backgrounds,
	// such as this line edit's in a combo box.
	struct BlockCursor
	{
		QColor fill, text;
		int glowRadius = 0;
	};
	// Empty: the native cursor. Requires CTextCursorHidingStyle as the application style, which hides the native cursor.
	void setBlockCursor(const std::optional<BlockCursor>& cursor);

protected:
	void focusInEvent (QFocusEvent * event) override;
	void focusOutEvent(QFocusEvent* event) override;
	void paintEvent(QPaintEvent* event) override;

private:
	struct BlockCell
	{
		QRect rect; // in this widget's coordinates
		QString character; // empty past the end of the text and on a tab
	};
	// Empty while the block is not shown
	[[nodiscard]] std::optional<BlockCell> shownBlockCursor() const;

	void restartBlockCursorBlinking();
	// Repaints the block, and the glow where it was and where it is now
	void updateBlockCursorArea();
	void paintBlockCursor();

private:
	bool _bSelectAllOnFocus = true;

	std::optional<BlockCursor> _blockCursor;
	QTimer* _blinkTimer = nullptr;
	bool _blockCursorShown = true; // the blink phase
	std::unique_ptr<CGlowUnderlay> _glowUnderlay; // created with the first block cursor
};
