#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QMainWindow>
#include <QWidget>
RESTORE_COMPILER_WARNINGS

// Every commonly used stock control on one scrollable page, for eyeballing a QStyle or a stylesheet and finding
// what it does not cover.
// Nothing here sets a colour, a font or a palette: the page has to render in whatever style and QSS the
// application installed, or it shows the gallery's own look instead of the application's.
// Every control appears in the states a style draws separately - disabled, checked, tristate, read-only, editable,
// each popup mode - because that is where a custom style falls short, not on the default state.
// Samples are inert except where the movement is the point. The colour scheme strip is a real control, driving
// CThemeController so the whole application re-themes and the page can be read in both polarities.
//
// Embeddable anywhere, and a parentless instance is a window of its own.
class CWidgetGallery final : public QWidget
{
public:
	explicit CWidgetGallery(QWidget* parent = nullptr);
};

// The gallery under the window furniture a plain widget cannot host: menu bar, toolbar, dock widget, status bar.
// Each is a styled surface of its own, and a custom style that covers the controls still routinely misses them.
class CWidgetGalleryWindow final : public QMainWindow
{
public:
	explicit CWidgetGalleryWindow(QWidget* parent = nullptr);

	// Deletes itself on close, so there is nothing to return
	static void showNew();
};
