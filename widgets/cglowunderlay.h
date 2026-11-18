#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QColor>
#include <QObject>
#include <QPainterPath>
#include <QPointer>
#include <QRect>
RESTORE_COMPILER_WARNINGS

#include <functional>
#include <optional>
#include <vector>

class QWidget;

// Paints paintGlow() around a shape of a widget, the owner, so that no widget's box cuts the glow off and the owner needs no margin for it.
// The nearest ancestor with room for all of the glow paints it, before painting itself: it shows through the transparent backgrounds
// above it, the owner's included. The window paints a glow too large for any ancestor, with the room it has.
class CGlowUnderlay final : public QObject
{
public:
	struct Glow
	{
		QPainterPath shape; // in the owner's coordinates
		QColor color;
		int radius = 0;
		qreal opacity = 0.36;
	};
	// Empty: no glow
	using GlowProvider = std::function<std::optional<Glow> ()>;

	// The provider is asked again whenever the owner or an ancestor moves within the window, resizes, shows, hides or changes parent,
	// and on refresh().
	// Must be destroyed before the owner's QWidget base, as a member: the provider must never run on a half-destroyed owner.
	CGlowUnderlay(QWidget& owner, GlowProvider provider);

	// Asks the provider again and repaints the glow where it was and where it is now
	void refresh();

	bool eventFilter(QObject* watched, QEvent* event) override;

private:
	struct Placement
	{
		QWidget* painter = nullptr;
		QPoint ownerOrigin; // in the painter's coordinates
		int radius = 0; // within the painter's rect: a glow cut off by its edge would end in a line
		QRect area; // what the glow covers, in the painter's coordinates
	};
	// Empty while there is no glow, the owner is hidden, or the window has no room for any
	[[nodiscard]] std::optional<Placement> placement() const;
	// Filters the events of the owner's ancestors up to the window
	void watchAncestors();

private:
	QWidget& _owner;
	const GlowProvider _provider;
	std::optional<Glow> _glow;
	std::vector<QPointer<QWidget>> _watchedAncestors;
	// Where the glow was last painted, for refresh() to repaint it away; null before the first paint
	QPointer<QWidget> _paintedIn;
	QRect _paintedArea;
};
