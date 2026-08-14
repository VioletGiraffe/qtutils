#include "clabelelided.h"

#include <QEvent>
#include <QFontMetrics>
#include <QHelpEvent>
#include <QPainter>
#include <QToolTip>

void CLabelElided::setElideMode(Qt::TextElideMode mode)
{
	_elideMode = mode;
	update();
}

Qt::TextElideMode CLabelElided::elideMode() const
{
	return _elideMode;
}

bool CLabelElided::event(QEvent* e)
{
	// Show automatic tooltip if no custom tooltip is set
	if (e->type() == QEvent::ToolTip && toolTip().isEmpty())
	{
		QHelpEvent* helpEvent = static_cast<QHelpEvent*>(e);
		// Check if text is actually elided
		QFontMetrics fm(font());
		if (fm.horizontalAdvance(text()) > contentsRect().width())
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
	painter.setFont(font());

	const QRect cr = contentsRect();

	QFontMetrics fm(font());
	const QString elidedText = fm.elidedText(text(), _elideMode, cr.width());

	painter.setPen(palette().color(foregroundRole()));
	painter.drawText(cr, alignment(), elidedText);
}
