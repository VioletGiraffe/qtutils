#include "clabelelided.h"

#include <QEvent>
#include <QFontMetrics>
#include <QHelpEvent>
#include <QPainter>
#include <QStyle>
#include <QToolTip>

void CLabelElided::setElideMode(Qt::TextElideMode mode)
{
	if (mode == _elideMode)
		return;

	_elideMode = mode;
	updateGeometry(); // minimumSizeHint() depends on the mode
	update();
}

Qt::TextElideMode CLabelElided::elideMode() const
{
	return _elideMode;
}

QSize CLabelElided::minimumSizeHint() const
{
	if (_elideMode == Qt::ElideNone)
		return QLabel::minimumSizeHint();

	constexpr QChar ellipsis{0x2026}; // What QFontMetrics::elidedText substitutes
	const QFontMetrics fm(font());
	const int widthUnavailableForText = width() - textRect().width();
	return { fm.horizontalAdvance(ellipsis) + widthUnavailableForText, QLabel::minimumSizeHint().height() };
}

bool CLabelElided::event(QEvent* e)
{
	// Show automatic tooltip if no custom tooltip is set
	if (e->type() == QEvent::ToolTip && toolTip().isEmpty())
	{
		QHelpEvent* helpEvent = static_cast<QHelpEvent*>(e);
		// Check if text is actually elided
		QFontMetrics fm(font());
		if (fm.horizontalAdvance(text()) > textRect().width())
		{
			QToolTip::showText(helpEvent->globalPos(), text());
		}
		else
		{
			QToolTip::hideText();
		}

		return true;
	}

	return QLabel::event(e);
}

void CLabelElided::paintEvent(QPaintEvent*)
{
	QPainter painter(this);
	drawFrame(&painter);
	painter.setFont(font());

	const QRect textArea = textRect();

	QFontMetrics fm(font());
	const QString elidedText = fm.elidedText(text(), _elideMode, textArea.width());

	painter.setPen(palette().color(foregroundRole()));
	painter.drawText(textArea, visualAlignment(), elidedText);
}

QRect CLabelElided::textRect() const
{
	const int m = margin();
	QRect r = contentsRect().adjusted(m, m, -m, -m);

	int textIndent = indent();
	if (textIndent < 0 && frameWidth() != 0) // The implicit indent QLabel applies to framed labels
		textIndent = fontMetrics().horizontalAdvance(QChar{u'x'}) / 2 - m;

	if (textIndent > 0)
	{
		// indent() only applies to the edge the text is aligned to
		const Qt::Alignment align = visualAlignment();
		if (align & Qt::AlignLeft)
			r.setLeft(r.left() + textIndent);
		if (align & Qt::AlignRight)
			r.setRight(r.right() - textIndent);
		if (align & Qt::AlignTop)
			r.setTop(r.top() + textIndent);
		if (align & Qt::AlignBottom)
			r.setBottom(r.bottom() - textIndent);
	}

	return r;
}

Qt::Alignment CLabelElided::visualAlignment() const
{
	return QStyle::visualAlignment(layoutDirection(), alignment());
}
