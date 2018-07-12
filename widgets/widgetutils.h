#pragma once

class QLayout;
class QMainWindow;
class QWidget;

namespace WidgetUtils
{
	void setLayoutVisible(QLayout* layout, bool visible);
	QMainWindow* findParentMainWindow(QWidget* child);
	QMainWindow* findTopLevelWindow();
};

