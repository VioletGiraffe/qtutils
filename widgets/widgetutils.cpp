#include "widgetutils.h"

DISABLE_COMPILER_WARNINGS
#include <QApplication>
#include <QDesktopWidget>
#include <QLayout>
#include <QMainWindow>
#include <QScreen>
#include <QWidget>
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

bool WidgetUtils::widgetBelongsToHierarchy(QWidget* const widget, QObject* const hierarchy)
{
	if (widget == hierarchy)
			return true;

		const auto& children = hierarchy->children();
		if (children.contains(widget))
			return true;

		for (const auto& child : children)
			if (widgetBelongsToHierarchy(widget, child))
				return true;

		return false;
}

QRect WidgetUtils::currentScreenGeometryForWidget(QWidget *widget)
{
	const int screenIndex = QApplication::desktop()->screenNumber(widget);
	if (const auto* primaryScreen = QApplication::primaryScreen(); screenIndex < 0)
		return primaryScreen ? primaryScreen->availableGeometry() : QRect{};

	const auto screens = QApplication::screens();
	if (screenIndex >= screens.size())
	{
		Q_ASSERT(screenIndex < screens.size());
		const auto* primaryScreen = QApplication::primaryScreen();
		return primaryScreen ? primaryScreen->availableGeometry() : QRect{};
	}

	return screens[screenIndex]->availableGeometry();
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

