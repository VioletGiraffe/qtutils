#include "widgetutils.h"

#include "assert/advanced_assert.h"

DISABLE_COMPILER_WARNINGS
#include <QApplication>
#include <QLayout>
#include <QMainWindow>
#include <QScreen>
RESTORE_COMPILER_WARNINGS

void WidgetUtils::setLayoutVisible(QLayout* layout, bool visible)
{
	if (!layout)
		return;

	for (int i = 0, count =	layout->count(); i < count; ++i)
	{
		QWidget * widget = layout->itemAt(i)->widget();
		if (widget)
			widget->setVisible(visible);
		else
			setLayoutVisible(layout->itemAt(i)->layout(), visible);
	}
}

QMainWindow* WidgetUtils::findParentMainWindow(QWidget* child)
{
	for (QWidget* widget = child; widget != nullptr; widget = widget->parentWidget())
	{
		if (widget->inherits("QMainWindow"))
			return qobject_cast<QMainWindow*>(widget);
	}

	return nullptr;
}

QMainWindow* WidgetUtils::findTopLevelWindow()
{
	for (QWidget* topLevelWidget: QApplication::topLevelWidgets())
	{
		if (topLevelWidget->inherits("QMainWindow"))
			return qobject_cast<QMainWindow*>(topLevelWidget);
	}

	return nullptr;
}

void* WidgetUtils::nativeOwnerWinId(const QWidget* widget)
{
	assert_and_return_r(widget, nullptr);
	return reinterpret_cast<void*>(widget->window()->winId());
}

bool WidgetUtils::widgetBelongsToHierarchy(QWidget* const widget, QObject* const hierarchy)
{
	if (widget == hierarchy)
			return true;

		const auto& children = hierarchy->children();
		if (children.contains(widget))
			return true;

		for (const auto& child : children)
		{
			if (widgetBelongsToHierarchy(widget, child))
				return true;
		}

		return false;
}

QRect WidgetUtils::currentScreenGeometryForWidget(const QWidget *widget)
{
	return widget->screen()->availableGeometry(); // falls back to the primary screen on its own
}

QRect WidgetUtils::geometryAtCenter(const QRect &reference, qreal scale)
{
	return geometryAtCenter(reference, reference.size() * scale);
}

QRect WidgetUtils::geometryAtCenter(const QRect &reference, const QSize &size)
{
	QRect centeredRect(QPoint{0, 0}, size);
	centeredRect.moveCenter(reference.center());
	return centeredRect;
}

void WidgetUtils::centerWidgetInRect(QWidget *widget, const QRect &rect, const QSize &newWidgetSize)
{
	widget->setGeometry(geometryAtCenter(rect, newWidgetSize.isValid() ? newWidgetSize : widget->size()));
}

void WidgetUtils::centerWidgetOnScreen(QWidget *widget, const QSize &newWidgetSize)
{
	centerWidgetInRect(widget, currentScreenGeometryForWidget(widget), newWidgetSize);
}

void WidgetUtils::centerWidgetInParent(QWidget *widget, const QSize &newWidgetSize)
{
	if (widget->parentWidget())
		centerWidgetInRect(widget, widget->parentWidget()->geometry(), newWidgetSize);
	else
		centerWidgetOnScreen(widget, newWidgetSize);
}

void WidgetUtils::centerWidgetOnScreen(QWidget *widget, qreal fractionOfScreenSize)
{
	centerWidgetOnScreen(widget, currentScreenGeometryForWidget(widget).size() * fractionOfScreenSize);
}

void WidgetUtils::centerWidgetInParent(QWidget *widget, qreal fractionOfParentSize)
{
	centerWidgetInParent(widget, widget->parentWidget() ? (widget->parentWidget()->size() * fractionOfParentSize) : widget->size());
}

void WidgetUtils::keepWidgetWithinScreen(QWidget *widget)
{
	assert_r(widget->isWindow());

	const QRect available = currentScreenGeometryForWidget(widget);
	const QRect frame = widget->frameGeometry(); // move() positions the frame, so the frame is what has to fit

	// qMin then qMax, not qBound: a widget larger than the screen inverts the bounds
	widget->move(qMax(available.left(), qMin(frame.left(), available.right() - frame.width() + 1)),
		qMax(available.top(), qMin(frame.top(), available.bottom() - frame.height() + 1)));
}

void WidgetUtils::placeUnder(QWidget *widget, const QWidget *anchor)
{
	QPoint topLeft = anchor->mapToGlobal(QPoint{0, anchor->height()});
	if (topLeft.y() + widget->height() > currentScreenGeometryForWidget(anchor).bottom())
		topLeft.setY(anchor->mapToGlobal(QPoint{0, 0}).y() - widget->height()); // above instead, clear of the anchor

	widget->move(topLeft);
	keepWidgetWithinScreen(widget);
}

void WidgetUtils::placeCenteredOn(QWidget *widget, const QWidget *window)
{
	centerWidgetInRect(widget, window->geometry()); // a top-level window's geometry is already global
	keepWidgetWithinScreen(widget);
}

