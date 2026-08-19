#include "ctintedsvgiconengine.h"

#include "assert/advanced_assert.h"

DISABLE_COMPILER_WARNINGS
#include <QGuiApplication>
#include <QPaintDevice>
#include <QPainter>
#include <QPalette>
#include <QtSvg/QSvgRenderer> // qualified: qtutils declares no svg module; the consumer links it (see theme.pri)
RESTORE_COMPILER_WARNINGS

#include <utility>

QPixmap tintedSvgPixmap(const QString& svgResource, const QColor& color, QSize logicalSize, qreal dpr)
{
	QPixmap pm{ (QSizeF{ logicalSize } * dpr).toSize() };
	pm.setDevicePixelRatio(dpr);
	pm.fill(Qt::transparent);

	const QRectF logicalRect{ QPointF{ 0, 0 }, QSizeF{ logicalSize } };
	{
		QSvgRenderer renderer{ svgResource };
		QPainter painter{ &pm };
		renderer.render(&painter, logicalRect);
	}
	{
		// Tint by compositing over the rendered alpha, so it works for any monochrome source
		// regardless of how the colour is spelled inside the SVG.
		QPainter painter{ &pm };
		painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
		painter.fillRect(logicalRect, color);
	}
	return pm;
}

CTintedSvgIconEngine::CTintedSvgIconEngine(QString svgResource, ColorProvider color) :
	_resource{ std::move(svgResource) },
	_color{ std::move(color) }
{
	assert_r(_color);
}

QIconEngine* CTintedSvgIconEngine::clone() const
{
	return new CTintedSvgIconEngine{ _resource, _color }; // the cache does not travel
}

QColor CTintedSvgIconEngine::colorForMode(QIcon::Mode mode) const
{
	if (mode == QIcon::Disabled)
	{
		QColor c = QGuiApplication::palette().color(QPalette::Disabled, QPalette::WindowText);
		c.setAlphaF(0.5f);
		return c;
	}
	return _color();
}

QPixmap CTintedSvgIconEngine::scaledPixmap(const QSize& size, QIcon::Mode mode, QIcon::State /*state*/, qreal scale)
{
	const QColor color = colorForMode(mode);
	const CacheKey key{ .physicalSize = (QSizeF{ size } * scale).toSize(), .tint = color.rgba() };
	auto it = _cache.constFind(key);
	if (it == _cache.constEnd())
		it = _cache.insert(key, tintedSvgPixmap(_resource, color, size, scale));
	return *it;
}

QPixmap CTintedSvgIconEngine::pixmap(const QSize& size, QIcon::Mode mode, QIcon::State state)
{
	return scaledPixmap(size, mode, state, 1.0);
}

void CTintedSvgIconEngine::paint(QPainter* painter, const QRect& rect, QIcon::Mode mode, QIcon::State state)
{
	const qreal scale = painter->device() ? painter->device()->devicePixelRatioF() : 1.0;
	painter->drawPixmap(rect, scaledPixmap(rect.size(), mode, state, scale));
}

QIcon tintedSvgIcon(QString svgResource, CTintedSvgIconEngine::ColorProvider color)
{
	return QIcon{ new CTintedSvgIconEngine{ std::move(svgResource), std::move(color) } };
}
