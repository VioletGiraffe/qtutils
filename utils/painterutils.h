#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QColor>
#include <QFont>
#include <QFontMetricsF>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPointF>
#include <QString>
#include <QTransform>
RESTORE_COMPILER_WARNINGS

#include <algorithm>
#include <cmath>

// The nearest logical point on a whole device pixel of the painted surface.
// Rounded through the device transform: at a fractional pixel ratio a widget's own origin need not be on one.
// Tidiness only: as of Qt 6.11.2 the raster engine rounds a fractional image origin itself and this snapping does not affect the rendering, as long as SmoothPixmapTransform is off.
[[nodiscard]] inline QPointF snappedToDevicePixel(const QPainter& painter, QPointF logical)
{
	const QTransform& toDevice = painter.deviceTransform();
	const QPointF device = toDevice.map(logical);
	return toDevice.inverted().map(QPointF(std::round(device.x()), std::round(device.y())));
}

// The width of a rect that holds the text whole: see docs/qt-text-quirks.md for why not QFontMetrics::horizontalAdvance()
[[nodiscard]] inline int roundedUpTextWidth(const QFont& font, const QString& text)
{
	return static_cast<int>(std::ceil(QFontMetricsF{ font }.horizontalAdvance(text)));
}

// A soft halo along the outline of `shape`: `opacity` at the outline, fading as a blurred edge does, to nothing at `radius` px to
// either side. The radius must fit inside the painted area, or its edge cuts the halo off.
// Also paints inside a closed shape: clip that out where the inside is drawn over.
inline void paintGlow(QPainter& painter, const QPainterPath& shape, const QColor& color, const int radius, const qreal opacity = 0.36)
{
	if (radius <= 0)
		return;

	// At a distance d from the outline the halo's opacity is opacity * erfc(d / (sigma * sqrt(2))): a Gaussian-blurred edge.
	// sigma is a third of the radius, where the opacity is down to 0.3 % of the outline's.
	const double sigma = radius / 3.0;
	const auto opacityAt = [&](const double distance) { return opacity * qreal{ color.alphaF() } * std::erfc(distance / (sigma * std::sqrt(2.0))); };

	painter.save();
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setBrush(Qt::NoBrush);
	// One stroke per pixel of the radius, the widest first. The pixel band at distance d lies under every stroke reaching past it:
	// each stroke adds what the wider ones lack at the band where it ends.
	double coveredByWider = 0.0;
	for (int reach = radius; reach > 0; --reach)
	{
		const double wanted = opacityAt(reach - 0.5);
		QColor layerColor = color;
		layerColor.setAlphaF(static_cast<float>(std::max((wanted - coveredByWider) / (1.0 - coveredByWider), 0.0)));
		coveredByWider = wanted;

		painter.setPen(QPen{ layerColor, 2.0 * reach, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin });
		painter.drawPath(shape);
	}
	painter.restore();
}
