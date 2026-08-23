#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QLabel>
RESTORE_COMPILER_WARNINGS

class QPainter;

// The layout is free to make this label any size: the minimum width is one ellipsis, the minimum height one line, and
// hasHeightForWidth() is false, so a wrapped label doesn't demand the height its text would need - it elides into
// whatever it is given. Both size hints are still QLabel's un-elided ones, so a caller that needs the preferred size
// bounded as well must constrain it itself - QSizePolicy::Ignored, or an explicit maximum.
// The text is split into lines at newlines, and at wrap points too when wordWrap() is set. Every line is elided to the
// available width, and the lines that don't fit in the height collapse into a single ellipsis line placed according to
// elideMode() - the head and the tail of the dropped text survive around it. Under ElideNone the overflow is clipped
// instead, as QLabel does.
// Rich text and pixmaps are ignored. Fix these gaps when a live potential consumer wants them.
class CLabelElided final : public QLabel
{
public:
	using QLabel::QLabel;

	void setElideMode(Qt::TextElideMode mode);
	[[nodiscard]] Qt::TextElideMode elideMode() const;

	[[nodiscard]] QSize minimumSizeHint() const override;
	[[nodiscard]] bool hasHeightForWidth() const override;
	[[nodiscard]] int heightForWidth(int width) const override;

protected:
	bool event(QEvent* e) override;
	void paintEvent(QPaintEvent* e) override;

private:
	void paintText(QPainter& painter, const QRect& textArea);

	// The area the text is painted in: contentsRect() less margin() and indent(), following QLabel's own arithmetic.
	[[nodiscard]] QRect textRect() const;
	[[nodiscard]] Qt::Alignment visualAlignment() const;
	[[nodiscard]] int textFlags() const; // visualAlignment() plus the mnemonic handling the style asks for

	Qt::TextElideMode _elideMode = Qt::ElideMiddle;
	bool _textIsTruncated = false; // Determined while painting, read by the tooltip handler
};
