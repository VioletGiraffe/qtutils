#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QColor>
#include <QHash>
#include <QIcon>
#include <QIconEngine>
#include <QPixmap>
RESTORE_COMPILER_WARNINGS

#include <functional>

// Rasterizes a monochrome SVG's shape at the given size and DPR, tinted with `color`. For delegates
// and other code that paints pixmaps directly; icons go through tintedSvgIcon() below.
[[nodiscard]] QPixmap tintedSvgPixmap(const QString& svgResource, const QColor& color, QSize logicalSize, qreal dpr);

// On-demand icon engine over a monochrome SVG: renders at each requested size and DPR rather than
// scaling one rasterization, and resolves its colour per render, so icons follow a live theme change
// with no invalidation - hand it a provider that reads the active theme, not a snapshot colour.
// Disabled icons use the palette's disabled tone at half opacity; other modes draw the provided colour.
class CTintedSvgIconEngine final : public QIconEngine
{
public:
	using ColorProvider = std::function<QColor ()>;

	CTintedSvgIconEngine(QString svgResource, ColorProvider color);

	[[nodiscard]] QIconEngine* clone() const override;
	[[nodiscard]] QPixmap scaledPixmap(const QSize& size, QIcon::Mode mode, QIcon::State state, qreal scale) override;
	[[nodiscard]] QPixmap pixmap(const QSize& size, QIcon::Mode mode, QIcon::State state) override;
	void paint(QPainter* painter, const QRect& rect, QIcon::Mode mode, QIcon::State state) override;

private:
	[[nodiscard]] QColor colorForMode(QIcon::Mode mode) const;

private:
	struct CacheKey
	{
		QSize physicalSize;
		QRgb tint; // the resolved colour is part of the key, or a theme switch would serve stale pixmaps

		bool operator==(const CacheKey& other) const noexcept = default;
		friend size_t qHash(const CacheKey& key, size_t seed = 0) noexcept {
			return qHashMulti(seed, key.physicalSize, key.tint);
		}
	};

	QString _resource;
	ColorProvider _color;
	QHash<CacheKey, QPixmap> _cache;
};

[[nodiscard]] QIcon tintedSvgIcon(QString svgResource, CTintedSvgIconEngine::ColorProvider color);
