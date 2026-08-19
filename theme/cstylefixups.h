#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QColor>
#include <QObject>
#include <QProxyStyle>
RESTORE_COMPILER_WARNINGS

#include <functional>

// Fixups for Qt styling behaviors that QSS cannot express or repair on its own; the why of each is
// in docs/qt-styling-quirks.md. All are installed once at startup and resolve their parameters
// through a provider at each use, so they follow a live theme change with nothing to re-install.

// The private combo popup's opaque viewport defeats QSS border-radius, so this paints the popup's
// rounded surface itself. Install as an application-wide event filter.
class CComboPopupRounder final : public QObject
{
public:
	struct Frame
	{
		QColor borderColor;
		qreal radius = 0;
	};
	using FrameProvider = std::function<Frame ()>;

	explicit CComboPopupRounder(FrameProvider frame, QObject* parent = nullptr);

	bool eventFilter(QObject* watched, QEvent* event) override;

private:
	const FrameProvider _frame;
};

// QSplitterHandle does not enable WA_Hover itself, so QSS `QSplitter::handle:hover` would never
// match. Install as an application-wide event filter.
class CSplitterHandleHoverEnabler final : public QObject
{
public:
	using QObject::QObject;

	bool eventFilter(QObject* watched, QEvent* event) override;
};

// Widens the base style's text-hugging QPushButton focus frame by outset() pixels a side.
// Install via QApplication::setStyle().
class CFocusFrameStyle final : public QProxyStyle
{
public:
	using OutsetProvider = std::function<int ()>;

	// The base-style argument follows QProxyStyle: null proxies the application's default style.
	explicit CFocusFrameStyle(OutsetProvider outset, QStyle* baseStyle = nullptr);

	void drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter,
	                   const QWidget* widget) const override;

private:
	const OutsetProvider _outset;
};
