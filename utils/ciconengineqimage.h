#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QIconEngine>
#include <QImage>
RESTORE_COMPILER_WARNINGS

#include <functional>

// Renders `source` at the exact pixel size each consumer asks for, instead of holding a few pre-scaled pixmaps and
// letting QIcon pick the nearest one and resample it again: one scale, straight from the original, to the size that
// is actually going to be blitted. The aspect ratio is preserved, the rest of the requested rect stays transparent.
// Source and scaler-result DPR metadata is ignored: both are treated as raw pixels. Nothing is cached - every request
// re-scales. The application style generates mode variants; On and Off share the same source.
class CIconEngineQImage final : public QIconEngine
{
public:
	// Scales `source` to exactly `targetSize`, which already carries the source's aspect ratio.
	using ScaleFunction = std::function<QImage (const QImage& source, const QSize& targetSize)>;

	explicit CIconEngineQImage(QImage source, ScaleFunction scale) noexcept;

	[[nodiscard]] QIconEngine* clone() const override;
	[[nodiscard]] bool isNull() override;

	// Neither actualSize() nor availableSizes() is overridden: the base actualSize() reports the requested size,
	// which is exactly what gets rendered, and an empty size list makes the XCB plugin ask for 16/32/64/128.
	[[nodiscard]] QPixmap scaledPixmap(const QSize& size, QIcon::Mode mode, QIcon::State state, qreal scale) override;
	[[nodiscard]] QPixmap pixmap(const QSize& size, QIcon::Mode mode, QIcon::State state) override;

	void paint(QPainter* painter, const QRect& rect, QIcon::Mode mode, QIcon::State state) override;

private:
	[[nodiscard]] QImage renderFitted(const QSize& targetSize) const;

private:
	QImage _source;
	ScaleFunction _scale;
};
