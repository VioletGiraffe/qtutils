#pragma once

#include <QLabel>

class QPainter;

// Reports a minimum width of just the ellipsis: elided text fits into any width, so the text length must not drive the
// layout's minimum. sizeHint() is still QLabel's un-elided text width, so a caller that also needs the preferred width
// bounded must constrain it itself - QSizePolicy::Ignored, or an explicit maximum.
// wordWrap() is honored: the lines fill the available height, and everything that doesn't fit collapses into a single
// ellipsis line placed according to elideMode() - the head and the tail of the dropped text survive around it. Under
// ElideNone the overflow is clipped instead, as QLabel does. The wrapped label still asks for the full text height via
// QLabel::heightForWidth(), so the elision only engages once something bounds the height.
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
	void paintSingleLine(QPainter& painter, const QRect& textArea);
	void paintWrapped(QPainter& painter, const QRect& textArea);

	// The area the text is painted in: contentsRect() less margin() and indent(), following QLabel's own arithmetic.
	[[nodiscard]] QRect textRect() const;
	[[nodiscard]] Qt::Alignment visualAlignment() const;

	Qt::TextElideMode _elideMode = Qt::ElideMiddle;
	bool _textIsTruncated = false; // Determined while painting, read by the tooltip handler
};
