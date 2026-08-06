#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QHash>
#include <QIconEngine>
#include <QImage>
#include <QList>
#include <QPixmap>
RESTORE_COMPILER_WARNINGS

#include <functional>

// Renders `source` at the exact pixel size each consumer asks for, instead of holding a few pre-scaled pixmaps and
// letting QIcon pick the nearest one and resample it again: one scale, straight from the original, to the size that
// is actually going to be blitted. The aspect ratio is preserved, the rest of the requested rect stays transparent.
// Source and scaler-result DPR metadata is ignored: both are treated as raw pixels. Raw renders are cached by final
// physical size. The application style generates mode variants; On and Off share the same source.
class CIconEngineQImage final : public QIconEngine
{
public:
	// Scales `source` to exactly `targetSize`, which already carries the source's aspect ratio.
	using ScaleFunction = std::function<QImage (const QImage& source, const QSize& targetSize)>;

	explicit CIconEngineQImage(QImage source, ScaleFunction scale = {}) noexcept;

	[[nodiscard]] QIconEngine* clone() const override;
	[[nodiscard]] bool isNull() override;

	// actualSize() is not overridden: the base reports the requested size, which is exactly what gets rendered.
	[[nodiscard]] QPixmap scaledPixmap(const QSize& size, QIcon::Mode mode, QIcon::State state, qreal scale) override;
	[[nodiscard]] QPixmap pixmap(const QSize& size, QIcon::Mode mode, QIcon::State state) override;

	// Empty everywhere but macOS: an empty list lets the consumer pick the sizes (the XCB plugin falls back to
	// 16/32/64/128, a Wayland compositor uses the ones it advertised), but the Cocoa helper drops the size AppKit
	// asked for and iterates this list instead, so an empty one leaves the window with no icon at all.
	[[nodiscard]] QList<QSize> availableSizes(QIcon::Mode mode, QIcon::State state) override;

	void paint(QPainter* painter, const QRect& rect, QIcon::Mode mode, QIcon::State state) override;

private:
	[[nodiscard]] QImage renderFitted(const QSize& targetSize) const;

private:
	struct CacheKey
	{
		QSize targetSize;
		QIcon::Mode mode;

		bool operator==(const CacheKey& other) const noexcept = default;
		friend size_t qHash(const CacheKey& key, size_t seed = 0) noexcept {
			return qHashMulti(seed, key.targetSize, static_cast<int>(key.mode));
		}
	};

	QImage _source;
	ScaleFunction _scale;
	QHash<CacheKey, QPixmap> _cache;
};
