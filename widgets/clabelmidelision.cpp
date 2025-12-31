#include "clabelmidelision.h"

#include <QEvent>
#include <QFontMetrics>
#include <QHelpEvent>
#include <QPainter>
#include <QToolTip>

bool CLabelMidElision::event(QEvent* e)
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

void CLabelMidElision::paintEvent(QPaintEvent*)
{
	QPainter painter(this);
	painter.setFont(font());

	const QRect cr = contentsRect();

	QFontMetrics fm(font());
	const QString elidedText = fm.elidedText(text(), Qt::ElideMiddle, cr.width());

	painter.setPen(palette().color(foregroundRole()));
	painter.drawText(cr, alignment(), elidedText);
}
