#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QPainter>
#include <QPointF>
#include <QTransform>
RESTORE_COMPILER_WARNINGS

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
