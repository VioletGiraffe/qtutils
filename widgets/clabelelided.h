#pragma once

#include <QLabel>

// Reports a minimum width of just the ellipsis: elided text fits into any width, so the text length must not drive the
// layout's minimum. sizeHint() is still QLabel's un-elided text width, so a caller that also needs the preferred width
// bounded must constrain it itself - QSizePolicy::Ignored, or an explicit maximum.
// Only plain single-line text is painted: rich text, word wrap and pixmaps are ignored. Fix these gaps when a live
// potential consumer wants them.
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
	// The area the text is painted in: contentsRect() less margin() and indent(), following QLabel's own arithmetic.
	[[nodiscard]] QRect textRect() const;
	[[nodiscard]] Qt::Alignment visualAlignment() const;

	Qt::TextElideMode _elideMode = Qt::ElideMiddle;
};
