#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QBasicTimer>
#include <QByteArray>
#include <QElapsedTimer>
#include <QIcon>
#include <QImage>
#include <QImageReader>
#include <QPointer>
#include <QRect>
#include <QSize>
#include <QString>
#include <QVariantAnimation>
#include <QWidget>
RESTORE_COMPILER_WARNINGS

#include <cstdint>
#include <functional>
#include <optional>

class QPainter;

class CImageViewerWidget final : public QWidget
{
public:
	// Scales srcRect of source into dest, which arrives at the exact target size and the source's format, premultiplied
	// if that has straight alpha. An empty srcRect means the whole source. An implementation may replace dest, so its
	// format is not guaranteed on return. Qt::FastTransformation requests nearest neighbor, used once source pixels are large on screen.
	using ImageScaleFunction = std::function<void (QImage& dest, const QImage& source, const QRect& srcRect, Qt::TransformationMode mode)>;

	// An image already turned upright; format names the file type in the info strip.
	struct DecodedFile
	{
		QImage image;
		QString format;
		QByteArray iccProfile; // the color profile of image; empty when unknown
	};

	// One image on its way into the view, possibly through a preview first. GUI thread only; copyable.
	// Ignored once superseded by a newer expectImage(), displayImage() or displayFrame().
	class PendingImage
	{
	public:
		// Laid out at finalSize, so zoom, pan and the info strip already use the final image's geometry.
		// finalSize is ignored when empty or when its aspect ratio differs from the preview's by more than 2%.
		// A later preview replaces an earlier one. Ignored after showFinal() or showFailure(). Requires a non-null image.
		void showPreview(DecodedFile preview, QSize finalSize = {}) const;
		// After a preview: the view stays when the image matches the preview's layout size, is rescaled when only the
		// aspect ratio matches, and is refitted otherwise. Without a preview: a new image, fitted. Requires a non-null image.
		void showFinal(DecodedFile image) const;
		// Clears the view unless a preview was shown, which then stays.
		void showFailure() const;
		// Shows a bar centered at the top, filled to fraction (0 to 1), until the image is shown, fails or is superseded.
		void showProgress(double fraction) const;

	private:
		friend class CImageViewerWidget;
		PendingImage(CImageViewerWidget* widget, uint64_t generation) noexcept;

		[[nodiscard]] CImageViewerWidget* acceptingWidget() const; // null once ended

		QPointer<CImageViewerWidget> _widget;
		uint64_t _generation = 0;
	};

	// Converts image, scaled from the source, from the source's ICC profile for display. Only called with a profile; may replace image.
	using DisplayColorConverter = std::function<void (QImage& image, const QByteArray& iccProfile)>;

	explicit CImageViewerWidget(QWidget* parent = nullptr);

public:
	// Without one, every scale goes through scaleWithQt().
	void setImageScaler(ImageScaleFunction scaler) noexcept;
	// Without one, images are shown unconverted. sourceImage() keeps the original colors either way.
	// The profile comes from a DecodedFile, or from the QImage's color space: for files the widget reads and for displayFrame().
	void setDisplayColorConverter(DisplayColorConverter converter);
	// QImage::scaled. Public so an injected scaler can delegate the formats it cannot handle.
	static void scaleWithQt(QImage& dest, const QImage& source, const QRect& srcRect, Qt::TransformationMode mode);

	// resetViewParameters: refits and recenters on the new image; false keeps the current zoom and pan, re-clamped to it.
	bool displayFrame(const QImage& image, bool resetViewParameters = true);
	// Reads the file through QImageReader, which plays animated files. Reports nothing to the user: the caller owns the error UI.
	// Fails without replacing the displayed image, but any animation already playing stops.
	bool displayImage(const QString& imagePath, bool resetViewParameters = true);
	// Supersedes the current pending image. The displayed image stays until the new one shows something.
	// An animation ends on the frame shown and its file is closed, as closeFile() does.
	[[nodiscard]] PendingImage expectImage();
	// Empty while only a preview is shown.
	[[nodiscard]] const QImage& sourceImage() const noexcept;
	[[nodiscard]] QString imageInfoString() const;

	[[nodiscard]] QSize sizeHint() const override;

	// Scales the source on demand, at the exact size the consumer asks for.
	[[nodiscard]] QIcon imageIcon() const;

	void copyToClipboard() noexcept;
	void copyDisplayedToClipboard() noexcept;

	void fitToWindow() noexcept;
	void zoomToActualPixels() noexcept;

	// True while an animated file is displayed, paused or not.
	// A single-frame GIF reports true until the first frame advance finds nothing to advance to.
	[[nodiscard]] bool isAnimated() const noexcept { return _animation.has_value(); }
	// No-op unless an animated image is displayed.
	void togglePause();
	// Pauses, then shows the adjacent frame, wrapping at either end. No-op unless an animated image is displayed.
	// Stepping back rewinds the file and decodes forward to the target: neither the GIF nor the WEBP handler can seek.
	void stepToNextFrame();
	void stepToPreviousFrame();
	// Closes the displayed file, which an animated image holds open. Its animation ends on the frame shown.
	void closeFile();

	// The strip along the bottom, showing imageInfoString() and the current magnification, plus the navigator
	// shown while the view can pan. Visible by default.
	void setOverlayVisible(bool visible);
	[[nodiscard]] bool isOverlayVisible() const noexcept { return _overlayVisible; }
	// Appended to the strip. The widget binds no shortcut, so naming the key that hides it is the caller's to do.
	void setInfoStripHint(QString hint);
	// Centered in the strip; the image info yields space to it.
	void setInfoStripCaption(QString caption);

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
	// How the view follows a replaced image:
	//   Reset - refits and recenters on the next paint
	//   KeepZoom - keeps the scale and the image point at the viewport center, re-clamped
	//   KeepLayout - keeps the on-screen geometry: the new image is the same picture, laid out at another size
	enum class ViewUpdate { Reset, KeepZoom, KeepLayout };

	// Replaces the displayed pixels, laid out at imageSize, leaving the animation and the file metadata untouched.
	bool setSourceImage(const QImage& image, QSize imageSize, ViewUpdate viewUpdate, Presentation presentation = Presentation::Deferred);
	// Replaces the displayed image and its file metadata; the animation is left untouched.
	void setDecodedFile(DecodedFile file, QSize imageSize, ViewUpdate viewUpdate, bool isPreview);
	// For the pending image's next rendition: KeepLayout after a preview of matching aspect ratio, Reset otherwise
	[[nodiscard]] ViewUpdate viewUpdateForPendingRendition(QSize imageSize) const noexcept;
	// The current PendingImage is ignored from now on and its progress bar is removed
	void endPendingImage();
	void setPaused(bool paused);                                                   // requires an engaged _animation
	bool decodeNextFrame();                                                        // requires an engaged _animation; false disengages it
	bool decodePrecedingFrame();                                                   // the same, for the frame before the displayed one
	void ensureFrameCountKnown();                                                  // requires an engaged _animation
	void presentPendingFrame();                                                    // requires an engaged _animation
	void scheduleNextFrame();                                                      // requires an engaged _animation
	void startOrStopFrameTimer();                                                  // no-op unless _animation is engaged

	// The view is an affine map from image coordinates to viewport device pixels: devicePos = _offset + imagePos * _scale.
	// Image coordinates span _imageSize; source pixels map onto them per axis, which only a preview makes differ from 1:1.
	[[nodiscard]] QSizeF viewportDeviceSize() const noexcept;
	[[nodiscard]] QSizeF scaledImageSize() const noexcept;                         // on-screen size of the whole image, device px
	[[nodiscard]] QSizeF sourceScale() const noexcept;                             // device px per source px, per axis
	[[nodiscard]] QRect visibleImageRect() const noexcept;                         // image coordinates the viewport shows, outset to whole units
	[[nodiscard]] QRect visibleSourceRect() const noexcept;                        // source pixels the viewport shows, outset to whole pixels
	// The viewport back-projected through devicePos = _offset + pos * unitScale, outset to whole units and clipped to bounds
	[[nodiscard]] QRect visibleRect(const QSizeF& unitScale, const QSize& bounds) const noexcept;
	[[nodiscard]] qreal fitScale() const noexcept;                                 // scale that fits the whole image
	[[nodiscard]] qreal minScale() const noexcept;                                 // most zoomed-out scale allowed
	[[nodiscard]] QPointF centeredOffset() const noexcept;                         // offset that centers the image at _scale
	[[nodiscard]] bool isPannable() const noexcept;                                // image larger than the viewport on some axis
	void clampOffset() noexcept;                                                   // per-axis: center if smaller, keep inside if larger
	void setScale(qreal scale) noexcept;                                           // the only writer of _scale and _fitToWindow
	void resetToFit() noexcept;
	// A view that fit the whole image refits; a zoomed one keeps its scale and the image point at the viewport center.
	// No-op until the first image is displayed.
	void refitOrKeepViewCenter(const QSizeF& previousViewportDeviceSize) noexcept;
	void invalidateDisplayImageCache(Presentation presentation);                   // repaints, so paintEvent must never call it
	void scaleSource(QImage& dest, const QRect& srcRect, const QSize& targetPx, Qt::TransformationMode mode) const; // an empty srcRect is the whole source

	[[nodiscard]] QString magnificationString() const;                             // on-screen size of the visible crop, and _scale as a percentage
	void paintInfoStrip(QPainter& painter) const;
	[[nodiscard]] QRect pendingProgressBarRect() const;                            // widget coords; empty when the widget is too narrow
	void paintPendingProgress(QPainter& painter) const;

	// The navigator: the whole image as a thumbnail in the top right corner, with the visible crop boxed on it.
	[[nodiscard]] QRect navigatorRect() const;                                     // widget coords; empty unless the overlay is on and the view can pan
	[[nodiscard]] QRect navigatorCropBox(const QRect& navigator) const noexcept;   // visibleImageRect() mapped into the navigator
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
	DisplayColorConverter _displayColorConverter;
	QString _infoStripHint;
	QString _infoStripCaption;
	bool _overlayVisible = true;
	// Tracked rather than read from isVisible(): a minimized window's children are sent a hide event but stay isVisible().
	bool _shown = false;
	QImage _sourceImage;
	QSize _imageSize;              // image coordinates' extent: the source's own size, or the final size a preview is laid out at
	bool _sourceIsPreview = false;
	QByteArray _sourceIccProfile;
	QImage _displayImage;
	QImage _navigatorThumbnail;    // the whole source at the navigator's device size; built on first use, rebuilt on a size change or a new image
	std::optional<size_t> _cacheKey;    // absent while _displayImage is stale

	QString _currentImageFormat;
	std::optional<Animation> _animation;

	// Bumped by endPendingImage(): a PendingImage of an older generation is ignored
	uint64_t _pendingGeneration = 0;
	// The current PendingImage's state; endPendingImage() resets it
	bool _pendingPreviewShown = false;
	bool _pendingProgressShown = false;
	QVariantAnimation _pendingProgressAnimation; // slides the drawn fraction, its current value, to the latest one reported

	qreal _scale = 1.0;    // device px per image px; 1.0 == 1:1 (native resolution)
	QPointF _offset;       // device-px position of the image's (0,0) within the viewport
	bool _viewInitialized = false;
	bool _fitToWindow = true; // _scale still fits the whole image, so a resize refits rather than preserving the zoom

	QPointF _panStartOffset;
	QPointF _panStartMouseDevice;
	bool _isPanning = false;
	bool _isNavigatorSteering = false;
};
