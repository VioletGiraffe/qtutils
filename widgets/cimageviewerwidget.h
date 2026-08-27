#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QIcon>
#include <QImage>
#include <QRect>
#include <QSize>
#include <QWidget>
RESTORE_COMPILER_WARNINGS

#include <functional>

class QPainter;

class CImageViewerWidget final : public QWidget
{
public:
	// Scales srcRect of source into dest, which arrives at the exact target size and the source's format. An empty
	// srcRect means the whole source. An implementation may replace dest, so its format is not guaranteed on return.
	using ImageScaleFunction = std::function<void (QImage& dest, const QImage& source, const QRect& srcRect)>;

	using QWidget::QWidget;

public:
	// Without one, every scale goes through smoothScale().
	void setImageScaler(ImageScaleFunction scaler) noexcept;
	// QImage::scaled with a smooth transform. Public so an injected scaler can delegate the formats it cannot handle.
	static void smoothScale(QImage& dest, const QImage& source, const QRect& srcRect);

	bool displayImage(const QImage& image);
	// Reports nothing to the user: the caller owns the error UI.
	bool displayImage(const QString& imagePath);
	[[nodiscard]] const QImage& sourceImage() const noexcept { return _sourceImage; }
	[[nodiscard]] QString imageInfoString() const;

	[[nodiscard]] QSize sizeHint() const override;

	// Scales the source on demand, at the exact size the consumer asks for.
	[[nodiscard]] QIcon imageIcon() const;

	void copyToClipboard() noexcept;
	void copyDisplayedToClipboard() noexcept;

	void fitToWindow() noexcept;
	void zoomToActualPixels() noexcept;

	// The strip along the bottom, showing imageInfoString() and the current magnification, plus the navigator
	// shown while the view can pan. Visible by default.
	void setOverlayVisible(bool visible);
	[[nodiscard]] bool isOverlayVisible() const noexcept { return _overlayVisible; }
	// Appended to the strip. The widget binds no shortcut, so naming the key that hides it is the caller's to do.
	void setInfoStripHint(QString hint);

protected:
	void paintEvent(QPaintEvent* e) override;
	void resizeEvent(QResizeEvent* e) override;
	void wheelEvent(QWheelEvent* e) override;
	void mousePressEvent(QMouseEvent* e) override;
	void mouseMoveEvent(QMouseEvent* e) override;
	void mouseReleaseEvent(QMouseEvent* e) override;

private:
	// The view is an affine map from source pixels to viewport device pixels: devicePos = _offset + sourcePos * _scale.
	[[nodiscard]] QSizeF viewportDeviceSize() const noexcept;
	[[nodiscard]] QSizeF scaledImageSize() const noexcept;                         // on-screen size of the whole image, device px
	[[nodiscard]] QRect visibleSourceRect() const noexcept;                        // source pixels the viewport shows, outset to whole pixels
	[[nodiscard]] qreal fitScale() const noexcept;                                 // scale that fits the whole image
	[[nodiscard]] qreal minScale() const noexcept;                                 // most zoomed-out scale allowed
	[[nodiscard]] QPointF centeredOffset() const noexcept;                         // offset that centers the image at _scale
	[[nodiscard]] bool isPannable() const noexcept;                                // image larger than the viewport on some axis
	void clampOffset() noexcept;                                                   // per-axis: center if smaller, keep inside if larger
	void setScale(qreal scale) noexcept;                                           // the only writer of _scale and _fitToWindow
	void resetToFit() noexcept;

	[[nodiscard]] QString magnificationString() const;                             // on-screen size of the visible crop, and _scale as a percentage
	void paintInfoStrip(QPainter& painter) const;

	// The navigator: the whole image as a thumbnail in the top right corner, with the visible crop boxed on it.
	[[nodiscard]] QRect navigatorRect() const;                                     // widget coords; empty unless the overlay is on and the view can pan
	[[nodiscard]] QRect navigatorCropBox(const QRect& navigator) const noexcept;   // visibleSourceRect() mapped into the navigator
	void paintNavigator(QPainter& painter);
	void centerViewOnNavigatorPoint(QPointF widgetPos);

private:
	ImageScaleFunction _imageScaler;
	QString _infoStripHint;
	bool _overlayVisible = true;
	QImage _sourceImage;
	QImage _displayImage;
	QImage _navigatorThumbnail;    // the whole source, scaled down; built on first use, dropped with the image
	size_t _cacheKey = 0;

	QString _currentImageFormat;
	qint64 _currentImageFileSize = 0;

	qreal _scale = 1.0;    // device px per source px; 1.0 == 1:1 (native resolution)
	QPointF _offset;       // device-px position of source (0,0) within the viewport
	bool _viewInitialized = false;
	bool _fitToWindow = true; // _scale still fits the whole image, so a resize refits rather than preserving the zoom

	QPointF _panStartOffset;
	QPointF _panStartMouseDevice;
	bool _isPanning = false;
	bool _isNavigatorSteering = false;
};
