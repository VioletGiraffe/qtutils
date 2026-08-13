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
	// The native handle of widget's top-level window, for native APIs needing an owner window. Deliberately not
	// widget->winId(): on a child that call turns it (and by default its siblings) into native windows.
	void* nativeOwnerWinId(const QWidget* widget);
	bool widgetBelongsToHierarchy(QWidget * widget, QObject * hierarchy);

	// Geometry of the screen the widget is on, minus taskbars and other reserved areas
	QRect currentScreenGeometryForWidget(const QWidget* widget);
	QRect geometryAtCenter(const QRect& reference, qreal scale);
	QRect geometryAtCenter(const QRect& reference, const QSize& size);

	void centerWidgetInRect(QWidget* widget, const QRect& rect, const QSize& newWidgetSize = QSize(-1, -1));

	void centerWidgetOnScreen(QWidget* widget, const QSize& newWidgetSize = QSize(-1, -1));
	void centerWidgetOnScreen(QWidget* widget, qreal fractionOfScreenSize);

	void centerWidgetInParent(QWidget* widget, const QSize& newWidgetSize = QSize(-1, -1));
	void centerWidgetInParent(QWidget* widget, qreal fractionOfParentSize);

	// Placement for already-sized top-level widgets; showing them is the caller's. Unlike the centerWidget*
	// family, these confine the widget to the screen.

	// Moves the widget fully onto its screen if part of it is outside. Top-level only: only there are pos()
	// and move() in screen coordinates.
	void keepWidgetWithinScreen(QWidget* widget);
	// Directly under `anchor`, or above it when there is no room below
	void placeUnder(QWidget* widget, const QWidget* anchor);
	// In the middle of `window`, for a widget whose trigger sits in a corner too far from where the eye is
	void placeCenteredOn(QWidget* widget, const QWidget* window);
} // namespace WidgetUtils
