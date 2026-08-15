#pragma once

#include <QLabel>

class QPainter;

// Reports a minimum width of just the ellipsis: elided text fits into any width, so the text length must not drive the
// layout's minimum. sizeHint() is still QLabel's un-elided text width, so a caller that also needs the preferred width
// bounded must constrain it itself - QSizePolicy::Ignored, or an explicit maximum.
// The text is split into lines at newlines, and at wrap points too when wordWrap() is set. Every line is elided to the
// available width, and the lines that don't fit in the height collapse into a single ellipsis line placed according to
// elideMode() - the head and the tail of the dropped text survive around it. Under ElideNone the overflow is clipped
// instead, as QLabel does. A wrapped label still asks for the full text height via QLabel::heightForWidth(), so its
// vertical elision only engages once something bounds the height.
// Rich text and pixmaps are ignored. Fix these gaps when a live potential consumer wants them.
class CLabelElided final : public QLabel
{
public:
	using QLabel::QLabel;

	void setElideMode(Qt::TextElideMode mode);
	[[nodiscard]] Qt::TextElideMode elideMode() const;

	[[nodiscard]] QSize minimumSizeHint() const override;

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
