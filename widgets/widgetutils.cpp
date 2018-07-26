#include "widgetutils.h"
#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QApplication>
#include <QLayout>
#include <QMainWindow>
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
