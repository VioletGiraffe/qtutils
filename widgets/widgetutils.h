#pragma once

class QLayout;
class QMainWindow;
class QObject;
class QWidget;

namespace WidgetUtils
{
	void setLayoutVisible(QLayout* layout, bool visible);
	QMainWindow* findParentMainWindow(QWidget* child);
	QMainWindow* findTopLevelWindow();
	bool widgetBelongsToHierarchy(QWidget * const widget, QObject * const hierarchy);
}
