#include "CProxyStyle.h"

#include <QApplication>

void CProxyStyle::drawComplexControl(ComplexControl control, const QStyleOptionComplex * option, QPainter * painter, const QWidget * widget) const
{
	QApplication::style()->drawComplexControl(control, option, painter, widget);
}

void CProxyStyle::drawControl(ControlElement element, const QStyleOption * option, QPainter * painter, const QWidget * widget) const
{
	QApplication::style()->drawControl(element, option, painter, widget);
}

void CProxyStyle::drawItemPixmap(QPainter * painter, const QRect & rectangle, int alignment, const QPixmap & pixmap) const
{
	QApplication::style()->drawItemPixmap(painter, rectangle, alignment, pixmap);
}

void CProxyStyle::drawItemText(QPainter * painter, const QRect & rectangle, int alignment, const QPalette & palette, bool enabled, const QString & text, QPalette::ColorRole textRole) const
{
	QApplication::style()->drawItemText(painter, rectangle, alignment, palette, enabled, text, textRole);
}

void CProxyStyle::drawPrimitive(PrimitiveElement element, const QStyleOption * option, QPainter * painter, const QWidget * widget) const
{
	QApplication::style()->drawPrimitive(element, option, painter, widget);
}

QPixmap CProxyStyle::generatedIconPixmap(QIcon::Mode iconMode, const QPixmap & pixmap, const QStyleOption * option) const
{
	return QApplication::style()->generatedIconPixmap(iconMode, pixmap, option);
}

QStyle::SubControl CProxyStyle::hitTestComplexControl(ComplexControl control, const QStyleOptionComplex * option, const QPoint & position, const QWidget * widget) const
{
	return QApplication::style()->hitTestComplexControl(control, option, position, widget);
}

QRect CProxyStyle::itemPixmapRect(const QRect & rectangle, int alignment, const QPixmap & pixmap) const
{
	return QApplication::style()->itemPixmapRect(rectangle, alignment, pixmap);
}

QRect CProxyStyle::itemTextRect(const QFontMetrics & metrics, const QRect & rectangle, int alignment, bool enabled, const QString & text) const
{
	return QApplication::style()->itemTextRect(metrics, rectangle, alignment, enabled, text);
}

int CProxyStyle::pixelMetric(PixelMetric metric, const QStyleOption * option, const QWidget * widget) const
{
	return QApplication::style()->pixelMetric(metric, option, widget);
}

void CProxyStyle::polish(QWidget * widget)
{
	QApplication::style()->polish(widget);
}

void CProxyStyle::polish(QApplication * application)
{
	QApplication::style()->polish(application);
}

void CProxyStyle::polish(QPalette & palette)
{
	QApplication::style()->polish(palette);
}

QSize CProxyStyle::sizeFromContents(ContentsType type, const QStyleOption * option, const QSize & contentsSize, const QWidget * widget) const
{
	return QApplication::style()->sizeFromContents(type, option, contentsSize, widget);
}

QPalette CProxyStyle::standardPalette() const
{
	return QApplication::style()->standardPalette();
}

int CProxyStyle::styleHint(StyleHint hint, const QStyleOption * option, const QWidget * widget, QStyleHintReturn * returnData) const
{
	return QApplication::style()->styleHint(hint, option, widget, returnData);
}

QRect CProxyStyle::subControlRect(ComplexControl control, const QStyleOptionComplex * option, SubControl subControl, const QWidget * widget) const
{
	return QApplication::style()->subControlRect(control, option, subControl, widget);
}

QRect CProxyStyle::subElementRect(SubElement element, const QStyleOption * option, const QWidget * widget) const
{
	return QApplication::style()->subElementRect(element, option, widget);
}

QPixmap CProxyStyle::standardPixmap(QStyle::StandardPixmap pixmap, const QStyleOption* option, const QWidget*widget) const
{
	return QApplication::style()->standardPixmap(pixmap, option, widget);
}

void CProxyStyle::unpolish(QWidget * widget)
{
	QApplication::style()->unpolish(widget);
}

void CProxyStyle::unpolish(QApplication * application)
{
	QApplication::style()->unpolish(application);
}

QIcon CProxyStyle::standardIcon(QStyle::StandardPixmap standardIcon, const QStyleOption *option, const QWidget *widget) const
{
	return QApplication::style()->standardIcon(standardIcon, option, widget);
}

int CProxyStyle::layoutSpacing(QSizePolicy::ControlType control1, QSizePolicy::ControlType control2, Qt::Orientation orientation, const QStyleOption *option, const QWidget *widget) const
{
	return QApplication::style()->layoutSpacing(control1, control2, orientation, option, widget);
}

