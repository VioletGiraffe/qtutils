#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QBasicTimer>
#include <QElapsedTimer>
#include <QIcon>
#include <QImage>
#include <QImageReader>
#include <QRect>
#include <QSize>
#include <QWidget>
RESTORE_COMPILER_WARNINGS

#include <functional>
#include <optional>

class QPainter;

class CImageViewerWidget final : public QWidget
{
public:
	// Scales srcRect of source into dest, which arrives at the exact target size and the source's format. An empty
	// srcRect means the whole source. An implementation may replace dest, so its format is not guaranteed on return.
	using ImageScaleFunction = std::function<void (QImage& dest, const QImage& source, const QRect& srcRect)>;

	using QWidget::QWidget;

public:
	// Without one, every scale goes through smoothScaleQt().
	void setImageScaler(ImageScaleFunction scaler) noexcept;
	// QImage::scaled with a smooth transform. Public so an injected scaler can delegate the formats it cannot handle.
	static void smoothScaleQt(QImage& dest, const QImage& source, const QRect& srcRect);
	// While enabled, upscales bypass the scaler for a nearest-neighbor blit; downscales are unaffected.
	void setNearestNeighborUpscaling(bool enabled);

	// resetViewParameters: refits and recenters on the new image; false keeps the current zoom and pan, re-clamped to it.
	bool displayFrame(const QImage& image, bool resetViewParameters = true);
	// Reports nothing to the user: the caller owns the error UI.
	// Fails without replacing the displayed image, but any animation already playing stops.
	bool displayImage(const QString& imagePath, bool resetViewParameters = true);
	[[nodiscard]] const QImage& sourceImage() const noexcept { return _sourceImage; }
	[[nodiscard]] QString imageInfoString() const;

	[[nodiscard]] QSize sizeHint() const override;

	// Scales the source on demand, at the exact size the consumer asks for.
	[[nodiscard]] QIcon imageIcon() const;

	void copyToClipboard() noexcept;
	void copyDisplayedToClipboard() noexcept;

	void fitToWindow() noexcept;
	void zoomToActualPixels() noexcept;

	// No-op unless an animated image is displayed.
	void togglePause();

	// The strip along the bottom, showing imageInfoString() and the current magnification, plus the navigator
	// shown while the view can pan. Visible by default.
	void setOverlayVisible(bool visible);
	[[nodiscard]] bool isOverlayVisible() const noexcept { return _overlayVisible; }
	// Appended to the strip. The widget binds no shortcut, so naming the key that hides it is the caller's to do.
	void setInfoStripHint(QString hint);

protected:
	void paintEvent(QPaintEvent* e) override;
	void resizeEvent(QResizeEvent* e) override;
	void showEvent(QShowEvent* e) override;
	void hideEvent(QHideEvent* e) override;
	void wheelEvent(QWheelEvent* e) override;
	void mousePressEvent(QMouseEvent* e) override;
	void mouseMoveEvent(QMouseEvent* e) override;
	void mouseReleaseEvent(QMouseEvent* e) override;
	void timerEvent(QTimerEvent* e) override;

private:
	// Immediate: paints inside the call, for a frame that has to land on its deadline.
	enum class Presentation { Deferred, Immediate };

	// Replaces the displayed pixels, leaving the animation and the file metadata untouched.
	bool setSourceImage(const QImage& image, bool resetViewParameters, Presentation presentation = Presentation::Deferred);
	bool decodeNextFrame();                                                        // requires an engaged _animation; false disengages it
	void scheduleNextFrame();                                                      // requires an engaged _animation
	void startOrStopFrameTimer();                                                  // no-op unless _animation is engaged

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
	// A view that fit the whole image refits; a zoomed one keeps its scale and the source point at the viewport center.
	// No-op until the first image is displayed.
	void refitOrKeepViewCenter(const QSizeF& previousViewportDeviceSize) noexcept;
	void invalidateDisplayImageCache(Presentation presentation);                   // repaints, so paintEvent must never call it

	[[nodiscard]] QString magnificationString() const;                             // on-screen size of the visible crop, and _scale as a percentage
	void paintInfoStrip(QPainter& painter) const;

	// The navigator: the whole image as a thumbnail in the top right corner, with the visible crop boxed on it.
	[[nodiscard]] QRect navigatorRect() const;                                     // widget coords; empty unless the overlay is on and the view can pan
	[[nodiscard]] QRect navigatorCropBox(const QRect& navigator) const noexcept;   // visibleSourceRect() mapped into the navigator
	void paintNavigator(QPainter& painter);
	void centerViewOnNavigatorPoint(QPointF widgetPos);

private:
	// Engaged only while an animated file is displayed.
	// The frame timer runs only while the widget is shown and userPaused is false.
	struct Animation
	{
		explicit Animation(QString imagePath);

		QString path;
		QImageReader reader;
		QBasicTimer frameTimer;
		bool userPaused = false;

		QElapsedTimer clock;            // Timebase for nextFrameDueMs, started when the first frame is scheduled
		qint64 nextFrameDueMs = 0;      // When pendingFrame is due on screen

		QImage pendingFrame;            // Decoded during the previous interval; null only until the first tick primes it
		int pendingDelayMs = 0;         // How long pendingFrame stays on screen once presented
		int displayedDelayMs = 0;       // The same, for the frame on screen now
		int displayedFrameNumber = 0;   // currentImageNumber() reports the pending frame, not this one
		int frameCount = 0;             // 0 until the first frame advance queries it
	};

private:
	ImageScaleFunction _imageScaler;
	QString _infoStripHint;
	bool _overlayVisible = true;
	bool _nearestNeighborUpscaling = false;
	// Tracked rather than read from isVisible(): a minimized window's children are sent a hide event but stay isVisible().
	bool _shown = false;
	QImage _sourceImage;
	QImage _displayImage;
	QImage _navigatorThumbnail;    // the whole source, scaled down; built on first use, dropped with the image
	std::optional<size_t> _cacheKey;    // absent while _displayImage is stale

	QString _currentImageFormat;
	std::optional<Animation> _animation;

	qreal _scale = 1.0;    // device px per source px; 1.0 == 1:1 (native resolution)
	QPointF _offset;       // device-px position of source (0,0) within the viewport
	bool _viewInitialized = false;
	bool _fitToWindow = true; // _scale still fits the whole image, so a resize refits rather than preserving the zoom

	QPointF _panStartOffset;
	QPointF _panStartMouseDevice;
	bool _isPanning = false;
	bool _isNavigatorSteering = false;
};
