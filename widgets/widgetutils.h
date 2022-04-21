#pragma once
#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QSize>
RESTORE_COMPILER_WARNINGS

class QLayout;
class QMainWindow;
class QObject;
class QRect;
class QWidget;

namespace WidgetUtils
{
	void setLayoutVisible(QLayout* layout, bool visible);
	QMainWindow* findParentMainWindow(QWidget* child);
	QMainWindow* findTopLevelWindow();
	bool widgetBelongsToHierarchy(QWidget * const widget, QObject * const hierarchy);

	QRect currentScreenGeometryForWidget(QWidget* widget);
	QRect geometryAtCenter(const QRect& reference, qreal scale);
	QRect geometryAtCenter(const QRect& reference, const QSize& size);

	void centerWidgetInRect(QWidget* widget, const QRect& rect, const QSize& newWidgetSize = QSize(-1, -1));

	void centerWidgetOnScreen(QWidget* widget, const QSize& newWidgetSize = QSize(-1, -1));
	void centerWidgetOnScreen(QWidget* widget, qreal fractionOfScreenSize);

	void centerWidgetInParent(QWidget* widget, const QSize& newWidgetSize = QSize(-1, -1));
	void centerWidgetInParent(QWidget* widget, qreal fractionOfParentSize);
}
