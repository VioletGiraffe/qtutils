#include "cimageviewerwidget.h"
#include "utils/ciconengineqimage.h"
#include "utils/painterutils.h"
#include "assert/advanced_assert.h"

DISABLE_COMPILER_WARNINGS
#include <QApplication>
#include <QClipboard>
#include <QColor>
#include <QColorSpace>
#include <QDebug>
#include <QEasingCurve>
#include <QFontMetrics>
#include <QHash>
#include <QHideEvent>
#include <QImageReader>
#include <QMouseEvent>
#include <QPainter>
#include <QRegion>
#include <QResizeEvent>
#include <QScreen>
#include <QShowEvent>
#include <QTimerEvent>
#include <QWheelEvent>
#include <QtMath>
RESTORE_COMPILER_WARNINGS

#include <algorithm>
#include <array>
#include <math.h>
#include <utility>

static constexpr qreal kMaxScale = 40.0; // device px per image px; the maximum magnification.
static constexpr qreal kMinScaledImageSize = 6.0; // logical px; the longer side of the image at the most zoomed-out scale.
static constexpr qreal kNearestNeighborMinScale = 3.0; // logical px per source px; from here up the source pixels show plainly, so the view scales nearest neighbor.
// A small preview's rounding stays below it; a camera's preview cropped to another aspect ratio is far above it.
static constexpr qreal kPreviewAspectTolerance = 0.02;

static constexpr int kNavigatorMaxSize = 100;       // logical px, longer side of the navigator
static constexpr int kNavigatorMinSize = 40;       // below this the navigator is not shown at all: too small to aim at
static constexpr int kNavigatorCropMinSize = 3;     // logical px; a thinner crop box would have nothing left to outline
static constexpr int kPendingProgressBarWidth = 300; // logical px; narrowed to fit the widget
static constexpr int kPendingProgressBarHeight = 8; // logical px
static constexpr int kPendingProgressAnimationMs = 200; // the fill's slide to each newly reported fraction

namespace
{
	// Position of one axis of the image within the viewport, in device pixels: center it when it is smaller than the
	// viewport (letterbox), otherwise keep its edges inside (crop/pan). Applied per axis, this is what lets a single
	// view letterbox one axis while cropping the other.
	[[nodiscard]] inline qreal clampAxis(qreal offset, qreal imageLength, qreal viewportLength) noexcept
	{
		if (imageLength <= viewportLength)
			return (viewportLength - imageLength) / 2.0;

		return std::clamp(offset, viewportLength - imageLength, 0.0);
	}

	[[nodiscard]] inline QPointF centerOf(const QSizeF& size) noexcept
	{
		return QPointF{ size.width() / 2.0, size.height() / 2.0 };
	}

	// Scaled images come back premultiplied, so a buffer allocated in this format is reused as is
	[[nodiscard]] constexpr QImage::Format premultipliedFormat(QImage::Format format) noexcept
	{
		switch (format)
		{
		case QImage::Format_ARGB32: return QImage::Format_ARGB32_Premultiplied;
		case QImage::Format_RGBA8888: return QImage::Format_RGBA8888_Premultiplied;
		default: return format;
		}
	}

	[[nodiscard]] bool aspectRatiosMatch(const QSize& a, const QSize& b) noexcept
	{
		if (a.isEmpty() || b.isEmpty())
			return false;

		const qreal aspectA = (qreal)a.width() / a.height(), aspectB = (qreal)b.width() / b.height();
		return qAbs(aspectA - aspectB) <= kPreviewAspectTolerance * aspectB;
	}

	// Read from the format alone: QImage::isGrayscale() scans the pixels (docs/qt-image-quirks.md)
	[[nodiscard]] QString pixelLayoutString(const QPixelFormat& format)
	{
		if (format.colorModel() == QPixelFormat::Indexed)
			return CImageViewerWidget::tr("%1-bit indexed").arg(format.bitsPerPixel());

		// redSize() to blackSize() read the first four fields whatever the color model; an ignored alpha field is padding
		const std::array<int, 5> fieldBits{ format.redSize(), format.greenSize(), format.blueSize(), format.blackSize(),
			format.alphaUsage() == QPixelFormat::UsesAlpha ? format.alphaSize() : 0 };
		int channels = 0, totalBits = 0, firstChannelBits = 0;
		bool uniform = true;
		for (const int bits : fieldBits)
		{
			if (bits == 0)
				continue;

			if (channels++ == 0)
				firstChannelBits = bits;
			else if (bits != firstChannelBits)
				uniform = false;
			totalBits += bits;
		}

		QString layout = uniform ? CImageViewerWidget::tr("%1×%2 bpp").arg(channels).arg(firstChannelBits) : CImageViewerWidget::tr("%1 bpp").arg(totalBits);
		if (format.typeInterpretation() == QPixelFormat::FloatingPoint)
			layout += CImageViewerWidget::tr(" float");
		return layout;
	}

	// Indexed and 16-bit-per-channel images are shown as 32-bit ones
	void convertToDisplayableFormat(QImage& image)
	{
		const auto format = image.format();
		if (format != QImage::Format_Indexed8 && format != QImage::Format_Grayscale16 && format != QImage::Format_RGBA64 && format != QImage::Format_RGBX64)
			return;

		image.convertTo(image.hasAlphaChannel() ? QImage::Format_ARGB32 : QImage::Format_RGB32);
		qInfo() << "CImageViewerWidget: converted image format from" << format << "to" << image.format();
	}

	// Reallocates dest only when its size or format differs from the target
	inline void scaleForDisplay(const CImageViewerWidget::ImageScaleFunction& scaler, const CImageViewerWidget::DisplayColorConverter& colorConverter,
		const QByteArray& iccProfile, QImage& dest, const QImage& source, const QRect& srcRect, const QSize& targetPx, Qt::TransformationMode mode)
	{
		const QImage::Format destFormat = premultipliedFormat(source.format());
		if (dest.size() != targetPx || dest.format() != destFormat)
			dest = QImage(targetPx, destFormat);

		if (scaler)
			scaler(dest, source, srcRect, mode);
		else
			CImageViewerWidget::scaleWithQt(dest, source, srcRect, mode);

		if (colorConverter && !iccProfile.isEmpty())
			colorConverter(dest, iccProfile);
	}
}

CImageViewerWidget::CImageViewerWidget(QWidget* parent)
	: QWidget{ parent }
{
	_pendingProgressAnimation.setDuration(kPendingProgressAnimationMs);
	_pendingProgressAnimation.setEasingCurve(QEasingCurve::InOutSine);
	connect(&_pendingProgressAnimation, &QVariantAnimation::valueChanged, this, [this] { update(pendingProgressBarRect()); });
}

void CImageViewerWidget::setImageScaler(ImageScaleFunction scaler) noexcept
{
	_imageScaler = std::move(scaler);
}

void CImageViewerWidget::setDisplayColorConverter(DisplayColorConverter converter)
{
	_displayColorConverter = std::move(converter);
	_navigatorThumbnail = QImage{}; // paintNavigator only compares sizes
	invalidateDisplayImageCache(Presentation::Deferred);
}

void CImageViewerWidget::scaleWithQt(QImage& dest, const QImage& source, const QRect& srcRect, Qt::TransformationMode mode)
{
	dest = (srcRect.isEmpty() ? source : source.copy(srcRect)).scaled(dest.size(), Qt::IgnoreAspectRatio, mode);
}

CImageViewerWidget::Animation::Animation(QString imagePath) :
	path{ std::move(imagePath) },
	reader{ path }
{
	reader.setAutoDetectImageFormat(true);
	reader.setAutoTransform(true);
}

bool CImageViewerWidget::setSourceImage(const QImage& image, QSize imageSize, ViewUpdate viewUpdate, Presentation presentation)
{
	const QSize previousImageSize = _imageSize;
	_sourceImage = image;
	_imageSize = imageSize;
	_navigatorThumbnail = QImage{}; // paintNavigator only compares sizes, which a new image may share

	switch (viewUpdate)
	{
	case ViewUpdate::Reset:
		if (_isPanning)
			unsetCursor();

		_isPanning = false;
		_isNavigatorSteering = false;
		_viewInitialized = false; // Refit to the new image on the next paint.
		break;
	case ViewUpdate::KeepZoom:
		refitOrKeepViewCenter(viewportDeviceSize());
		break;
	case ViewUpdate::KeepLayout:
		if (_sourceImage.isNull() || !_viewInitialized || size().isEmpty() || _imageSize == previousImageSize)
			break;

		if (_fitToWindow)
			resetToFit();
		else
		{
			// The image's origin stays put and the scale follows the layout size, so every image point keeps its screen position
			setScale(std::clamp(_scale * previousImageSize.width() / _imageSize.width(), minScale(), kMaxScale));
			clampOffset();
		}
		break;
	}

	if (_imageSize != previousImageSize)
		updateGeometry(); // The image size drives sizeHint().

	invalidateDisplayImageCache(presentation);
	return !_sourceImage.isNull();
}

void CImageViewerWidget::setDecodedFile(DecodedFile file, QSize imageSize, ViewUpdate viewUpdate, bool isPreview)
{
	_sourcePixelFormat = file.image.pixelFormat();
	convertToDisplayableFormat(file.image);
	_sourceIccProfile = std::move(file.iccProfile);
	_currentImageFormat = std::move(file.format);
	_sourceIsPreview = isPreview;
	setSourceImage(file.image, imageSize, viewUpdate);
}

bool CImageViewerWidget::displayFrame(const QImage& image, bool resetViewParameters)
{
	endPendingImage();
	_animation.reset();
	_currentImageFormat.clear(); // No file behind a bare image
	_sourceIccProfile = image.colorSpace().iccProfile();
	_sourcePixelFormat = image.pixelFormat();
	_sourceIsPreview = false;

	return setSourceImage(image, image.size(), resetViewParameters ? ViewUpdate::Reset : ViewUpdate::KeepZoom);
}

bool CImageViewerWidget::displayImage(const QString& imagePath, bool resetViewParameters)
{
	endPendingImage();

	// Frame 0 is read through the animation's own reader: an animated file continues from where this read leaves it.
	_animation.emplace(imagePath);
	DecodedFile file;
	file.format = QString::fromLatin1(_animation->reader.format());
	file.image = _animation->reader.read();
	if (file.image.isNull())
	{
		_animation.reset();
		return false;
	}
	file.iccProfile = file.image.colorSpace().iccProfile(); // every frame of a file shares it

	if (_animation->reader.supportsAnimation())
	{
		_animation->clock.start();
		_animation->displayedDelayMs = _animation->reader.nextImageDelay();
		startOrStopFrameTimer(); // A file can be loaded before the widget is shown: playback then waits for that.
	}
	else
		_animation.reset();

	const QSize imageSize = file.image.size();
	setDecodedFile(std::move(file), imageSize, resetViewParameters ? ViewUpdate::Reset : ViewUpdate::KeepZoom, false);
	return true;
}

CImageViewerWidget::PendingImage CImageViewerWidget::expectImage()
{
	closeFile();
	endPendingImage();
	return PendingImage{ this, _pendingGeneration };
}

const QImage& CImageViewerWidget::sourceImage() const noexcept
{
	static const QImage none;
	return _sourceIsPreview ? none : _sourceImage;
}

CImageViewerWidget::ViewUpdate CImageViewerWidget::viewUpdateForPendingRendition(QSize imageSize) const noexcept
{
	return _pendingPreviewShown && aspectRatiosMatch(imageSize, _imageSize) ? ViewUpdate::KeepLayout : ViewUpdate::Reset;
}

void CImageViewerWidget::endPendingImage()
{
	++_pendingGeneration;
	_pendingPreviewShown = false;
	_pendingProgressAnimation.stop();
	if (std::exchange(_pendingProgressShown, false))
		update(pendingProgressBarRect());
}

CImageViewerWidget::PendingImage::PendingImage(CImageViewerWidget* widget, uint64_t generation) noexcept
	: _widget{ widget }, _generation{ generation }
{
}

CImageViewerWidget* CImageViewerWidget::PendingImage::acceptingWidget() const
{
	CImageViewerWidget* widget = _widget.data();
	return widget && widget->_pendingGeneration == _generation ? widget : nullptr;
}

void CImageViewerWidget::PendingImage::showPreview(DecodedFile preview, QSize finalSize) const
{
	CImageViewerWidget* widget = acceptingWidget();
	if (!widget)
		return;
	assert_and_return_r(!preview.image.isNull(), );

	const QSize previewSize = preview.image.size();
	const QSize imageSize = aspectRatiosMatch(finalSize, previewSize) ? finalSize : previewSize;
	const ViewUpdate viewUpdate = widget->viewUpdateForPendingRendition(imageSize);
	widget->_pendingPreviewShown = true;
	widget->setDecodedFile(std::move(preview), imageSize, viewUpdate, true);
}

void CImageViewerWidget::PendingImage::showFinal(DecodedFile image) const
{
	CImageViewerWidget* widget = acceptingWidget();
	if (!widget)
		return;
	assert_and_return_r(!image.image.isNull(), );

	const QSize imageSize = image.image.size();
	const ViewUpdate viewUpdate = widget->viewUpdateForPendingRendition(imageSize);
	widget->endPendingImage();
	widget->setDecodedFile(std::move(image), imageSize, viewUpdate, false);
}

void CImageViewerWidget::PendingImage::showFailure() const
{
	CImageViewerWidget* widget = acceptingWidget();
	if (!widget)
		return;

	const bool previewShown = widget->_pendingPreviewShown;
	widget->endPendingImage();
	if (!previewShown)
		widget->setDecodedFile({}, {}, ViewUpdate::Reset, false);
}

void CImageViewerWidget::PendingImage::showProgress(double fraction) const
{
	CImageViewerWidget* widget = acceptingWidget();
	if (!widget)
		return;

	QVariantAnimation& animation = widget->_pendingProgressAnimation;
	const bool firstReport = !std::exchange(widget->_pendingProgressShown, true);
	// The first report slides from empty: the animation still holds the previous pending image's last fraction
	const double shownFraction = firstReport ? 0.0 : animation.currentValue().toDouble();
	animation.stop();
	animation.setStartValue(shownFraction);
	animation.setEndValue(std::clamp(fraction, 0.0, 1.0));
	animation.start();
	if (firstReport)
		widget->update(widget->pendingProgressBarRect()); // the animation repaints only when its value changes
}

QString CImageViewerWidget::imageInfoString() const
{
	if (_sourceImage.isNull())
		return QString();

	QString imageInfo = tr("%1x%2 (%3 MP)").
		arg(_imageSize.width()).
		arg(_imageSize.height()).
		arg(_imageSize.width() * _imageSize.height() * 1e-6, 0, 'f', 1);

	// A preview's pixel layout is its own, not the final image's
	if (!_sourceIsPreview)
		imageInfo += ' ' + pixelLayoutString(_sourcePixelFormat);

	if (_currentImageFormat.isEmpty()) // Displayed from a QImage: there is no file to name a format for
		return imageInfo;

	QString fileInfo = _currentImageFormat.toUpper() + ' ' + imageInfo;

	if (_animation && _animation->frameCount > 1)
		fileInfo += tr(", frame %1 of %2").arg(_animation->displayedFrameNumber + 1).arg(_animation->frameCount);

	return fileInfo;
}

QSize CImageViewerWidget::sizeHint() const
{
	if (_sourceImage.isNull())
		return QWidget::sizeHint();

	const auto maxSize = screen()->availableGeometry().size() - QSize(60, 60);
	const qreal dpr = devicePixelRatioF();
	return QSize{
		std::clamp(qCeil(_imageSize.width() / dpr), 150, maxSize.width()),
		std::clamp(qCeil(_imageSize.height() / dpr), 150, maxSize.height())
	};
}

QIcon CImageViewerWidget::imageIcon() const
{
	if (_sourceImage.isNull())
		return QIcon{};

	// Copied intentionally: probably not necessary, but the lambda is stored in the icon engine and might outlive this widget
	const auto resizeImage = [scaler = _imageScaler, colorConverter = _displayColorConverter, iccProfile = _sourceIccProfile](const QImage& src, const QSize& targetSize) -> QImage {
		QImage dst;
		scaleForDisplay(scaler, colorConverter, iccProfile, dst, src, QRect{}, targetSize, Qt::SmoothTransformation);
		return dst;
	};

	return QIcon{ new CIconEngineQImage{_sourceImage, resizeImage} };
}

void CImageViewerWidget::setOverlayVisible(bool visible)
{
	_overlayVisible = visible;
	update();
}

void CImageViewerWidget::setInfoStripHint(QString hint)
{
	_infoStripHint = std::move(hint);
	update();
}

void CImageViewerWidget::setInfoStripCaption(QString caption)
{
	_infoStripCaption = std::move(caption);
	update();
}

void CImageViewerWidget::copyToClipboard() noexcept
{
	if (const QImage& image = sourceImage(); !image.isNull())
		QApplication::clipboard()->setImage(image);
}

void CImageViewerWidget::copyDisplayedToClipboard() noexcept
{
	if (!_displayImage.isNull())
		QApplication::clipboard()->setImage(_displayImage);
}

QSizeF CImageViewerWidget::viewportDeviceSize() const noexcept
{
	return QSizeF{ size() } * devicePixelRatioF();
}

QSizeF CImageViewerWidget::scaledImageSize() const noexcept
{
	return QSizeF{ _imageSize } * _scale;
}

QSizeF CImageViewerWidget::sourceScale() const noexcept
{
	return QSizeF{ _scale * _imageSize.width() / _sourceImage.width(), _scale * _imageSize.height() / _sourceImage.height() };
}

QRect CImageViewerWidget::visibleImageRect() const noexcept
{
	return visibleRect(QSizeF{ _scale, _scale }, _imageSize);
}

QRect CImageViewerWidget::visibleSourceRect() const noexcept
{
	return visibleRect(sourceScale(), _sourceImage.size());
}

QRect CImageViewerWidget::visibleRect(const QSizeF& unitScale, const QSize& bounds) const noexcept
{
	// Back-project the viewport through the inverse transform pos = (devicePx - offset) / unitScale, clipped to the bounds.
	const QSizeF viewport = viewportDeviceSize();
	const int x0 = qFloor(std::clamp(-_offset.x() / unitScale.width(), 0.0, (qreal)bounds.width()));
	const int y0 = qFloor(std::clamp(-_offset.y() / unitScale.height(), 0.0, (qreal)bounds.height()));
	const int x1 = qCeil(std::clamp((viewport.width() - _offset.x()) / unitScale.width(), 0.0, (qreal)bounds.width()));
	const int y1 = qCeil(std::clamp((viewport.height() - _offset.y()) / unitScale.height(), 0.0, (qreal)bounds.height()));

	return QRect{ x0, y0, x1 - x0, y1 - y0 };
}

qreal CImageViewerWidget::fitScale() const noexcept
{
	const QSizeF viewport = viewportDeviceSize();
	return std::min(viewport.width() / _imageSize.width(), viewport.height() / _imageSize.height());
}

qreal CImageViewerWidget::minScale() const noexcept
{
	// Zoom out until the longer side is down to kMinScaledImageSize. Fit and 1:1 lower the floor further for an image
	// already at or below that size, so both stay reachable.
	const qreal longerSide = (qreal)std::max(_imageSize.width(), _imageSize.height());
	return std::min({ kMinScaledImageSize * devicePixelRatioF() / longerSide, fitScale(), 1.0 });
}

QPointF CImageViewerWidget::centeredOffset() const noexcept
{
	return centerOf(viewportDeviceSize() - scaledImageSize());
}

bool CImageViewerWidget::isPannable() const noexcept
{
	const QSizeF image = scaledImageSize(), viewport = viewportDeviceSize();
	// paintEvent snaps the blit to whole device pixels, so an overflow of half a pixel or less cannot move anything.
	return image.width() > viewport.width() + 0.5 || image.height() > viewport.height() + 0.5;
}

void CImageViewerWidget::clampOffset() noexcept
{
	const QSizeF image = scaledImageSize(), viewport = viewportDeviceSize();
	_offset.setX(clampAxis(_offset.x(), image.width(), viewport.width()));
	_offset.setY(clampAxis(_offset.y(), image.height(), viewport.height()));
}

void CImageViewerWidget::setScale(qreal scale) noexcept
{
	_scale = scale;
	// Exact comparison: a scale that fits always comes from this same expression, either directly or through the minScale() clamp.
	_fitToWindow = qFuzzyCompare(_scale, fitScale());
}

void CImageViewerWidget::resetToFit() noexcept
{
	setScale(fitScale());
	_offset = centeredOffset();
	_viewInitialized = true;
}

void CImageViewerWidget::refitOrKeepViewCenter(const QSizeF& previousViewportDeviceSize) noexcept
{
	if (_sourceImage.isNull() || !_viewInitialized || size().isEmpty())
		return;

	if (_fitToWindow)
	{
		resetToFit();
		return;
	}

	const QPointF centerImagePos = (centerOf(previousViewportDeviceSize) - _offset) / _scale;

	setScale(std::clamp(_scale, minScale(), kMaxScale));
	_offset = centerOf(viewportDeviceSize()) - centerImagePos * _scale;
	clampOffset();
}

void CImageViewerWidget::invalidateDisplayImageCache(Presentation presentation)
{
	_cacheKey.reset();

	if (presentation == Presentation::Immediate)
		repaint();
	else
		update();
}

void CImageViewerWidget::scaleSource(QImage& dest, const QRect& srcRect, const QSize& targetPx, Qt::TransformationMode mode) const
{
	scaleForDisplay(_imageScaler, _displayColorConverter, _sourceIccProfile, dest, _sourceImage, srcRect, targetPx, mode);
}

void CImageViewerWidget::fitToWindow() noexcept
{
	if (_sourceImage.isNull() || size().isEmpty())
		return;

	resetToFit();
	update();
}

void CImageViewerWidget::zoomToActualPixels() noexcept
{
	if (_sourceImage.isNull() || size().isEmpty())
		return;

	setScale(1.0); // 1:1 == one image pixel per one device pixel; always within [minScale(), kMaxScale].
	_offset = centeredOffset();
	_viewInitialized = true;
	update();
}

void CImageViewerWidget::togglePause()
{
	if (!_animation)
		return;

	setPaused(!_animation->userPaused);
}

void CImageViewerWidget::stepToNextFrame()
{
	if (!_animation)
		return;

	setPaused(true);

	// Only a step taken before the first tick has to decode: otherwise the lookahead already holds the next frame.
	if (_animation->pendingFrame.isNull() && !decodeNextFrame())
		return;

	presentPendingFrame();
	decodeNextFrame();
}

void CImageViewerWidget::stepToPreviousFrame()
{
	if (!_animation)
		return;

	setPaused(true);

	if (!decodePrecedingFrame())
		return;

	presentPendingFrame();
	decodeNextFrame();
}

void CImageViewerWidget::closeFile()
{
	if (!_animation)
		return;

	_animation.reset();
	update(); // The info strip loses its frame number
}

void CImageViewerWidget::setPaused(bool paused)
{
	_animation->userPaused = paused;
	startOrStopFrameTimer();
}

void CImageViewerWidget::startOrStopFrameTimer()
{
	if (!_animation)
		return;

	const bool shouldRun = _shown && !_animation->userPaused;
	if (shouldRun == _animation->frameTimer.isActive())
		return;

	if (!shouldRun)
	{
		_animation->frameTimer.stop();
		return;
	}

	// Resuming restarts the current frame's delay in full: the remainder is not tracked.
	_animation->nextFrameDueMs = _animation->clock.elapsed() + _animation->displayedDelayMs;
	scheduleNextFrame();
}

bool CImageViewerWidget::decodeNextFrame()
{
	QImage frame = _animation->reader.read();
	if (frame.isNull())
	{
		// Every GIF reports itself as animated, single-frame ones included.
		if (_animation->reader.currentImageNumber() <= 0)
		{
			_animation.reset();
			return false;
		}

		// The handler cannot seek, so a fresh device and decoder are the only way back to frame 0.
		_animation->reader.setFileName(_animation->path);
		frame = _animation->reader.read();
		if (frame.isNull())
		{
			_animation.reset(); // The file became unreadable; the last frame stays on screen.
			return false;
		}
	}

	ensureFrameCountKnown();

	_animation->pendingFrame = std::move(frame);
	_animation->pendingDelayMs = _animation->reader.nextImageDelay();
	return true;
}

bool CImageViewerWidget::decodePrecedingFrame()
{
	int targetFrame = _animation->displayedFrameNumber - 1;
	if (targetFrame < 0)
	{
		ensureFrameCountKnown();
		targetFrame = _animation->frameCount - 1;
	}

	// The handler cannot seek, so frame 0 is the only entry point and every frame up to the target has to be decoded.
	_animation->reader.setFileName(_animation->path);

	// Stops at the last decodable frame: a scan counts frame headers, so a malformed trailing frame inflates imageCount().
	QImage frame;
	for (int i = 0; i <= targetFrame; ++i)
	{
		QImage next = _animation->reader.read();
		if (next.isNull())
			break;

		frame = std::move(next);
	}

	if (frame.isNull())
	{
		_animation.reset(); // The file became unreadable; the last frame stays on screen.
		return false;
	}

	_animation->pendingFrame = std::move(frame);
	_animation->pendingDelayMs = _animation->reader.nextImageDelay();
	return true;
}

void CImageViewerWidget::ensureFrameCountKnown()
{
	// Deferred rather than resolved at load: imageCount() scans the whole file, and frame 0 must not wait for it.
	if (_animation->frameCount == 0)
		_animation->frameCount = std::max(1, _animation->reader.imageCount());
}

void CImageViewerWidget::presentPendingFrame()
{
	// The info strip is painted inside the presentation below, so its frame number must be set first.
	_animation->displayedFrameNumber = _animation->reader.currentImageNumber();
	_animation->displayedDelayMs = _animation->pendingDelayMs;

	setSourceImage(_animation->pendingFrame, _animation->pendingFrame.size(), ViewUpdate::KeepZoom, Presentation::Immediate);
}

void CImageViewerWidget::scheduleNextFrame()
{
	const qint64 nowMs = _animation->clock.elapsed();

	// A stall is not chased down: catching up would burst through every frame the schedule already covered.
	if (_animation->nextFrameDueMs < nowMs - _animation->displayedDelayMs)
		_animation->nextFrameDueMs = nowMs;

	_animation->frameTimer.start((int)std::max<qint64>(0, _animation->nextFrameDueMs - nowMs), Qt::PreciseTimer, this);
}

void CImageViewerWidget::paintEvent(QPaintEvent*)
{
	QPainter p{ this };
	p.fillRect(rect(), palette().color(QPalette::Window));

	if (_sourceImage.isNull())
	{
		QFont bigFont = font();
		bigFont.setPointSize(28);
		p.setFont(bigFont);
		p.drawText(rect(), Qt::AlignCenter, tr("No image loaded"));
		paintPendingProgress(p);
		return;
	}

	if (size().isEmpty())
		return;

	if (!_viewInitialized)
		resetToFit();

	const QRect sourceRect = visibleSourceRect();
	if (sourceRect.isEmpty())
		return;

	// The visible crop occupies sourceRect * sourceScale device pixels; render the buffer at exactly that resolution for a 1:1 blit.
	// Scaled in source pixels, not image coordinates: the scaler's source rectangle is whole pixels.
	const QSizeF scale = sourceScale();
	const QSize bufferPx{
		std::max(1, qRound(sourceRect.width() * scale.width())),
		std::max(1, qRound(sourceRect.height() * scale.height()))
	};

	const qreal dpr = devicePixelRatioF();
	const Qt::TransformationMode mode = std::min(scale.width(), scale.height()) / dpr >= kNearestNeighborMinScale ? Qt::FastTransformation : Qt::SmoothTransformation;
	// The mode is keyed: it depends on the device pixel ratio, which neither the crop nor the buffer size reflects.
	const size_t newCacheKey = qHashMulti(4 /* true random seed, chosen by a fair dice throw */, sourceRect, bufferPx, mode);
	if (newCacheKey != _cacheKey)
	{
		_cacheKey = newCacheKey;
		scaleSource(_displayImage, sourceRect, bufferPx, mode);
	}

	// Set after scaling: a scaler is free to replace the buffer.
	_displayImage.setDevicePixelRatio(dpr);

	const QPointF targetDevice = _offset + QPointF{ sourceRect.x() * scale.width(), sourceRect.y() * scale.height() };
	p.drawImage(snappedToDevicePixel(p, targetDevice / dpr), _displayImage);

	paintNavigator(p);

	if (_overlayVisible)
		paintInfoStrip(p);

	paintPendingProgress(p);
}

QString CImageViewerWidget::magnificationString() const
{
	// _scale is the magnification: on-screen (device) pixels per source pixel, so 1.0 is native resolution.
	return tr("Viewing at %1x%2 (%3% / %4x)").arg(_displayImage.width()).arg(_displayImage.height()).
		arg(qRound(_scale * 100.0)).arg(_scale, 0, 'f', 2);
}

void CImageViewerWidget::paintInfoStrip(QPainter& painter) const
{
	const QFontMetrics fm = painter.fontMetrics();
	const int padding = fm.height() / 2;
	const QRect strip{ 0, height() - fm.height() - padding, width(), fm.height() + padding };
	const QRect textRect = strip.adjusted(padding, 0, -padding, 0);

	painter.fillRect(strip, QColor(0, 0, 0, 140));
	// Light on a dark scrim whatever the palette: what shows through is image content, not the window.
	painter.setPen(QColor(0xe8, 0xe8, 0xe8));

	QString magnification = magnificationString();
	if (!_infoStripHint.isEmpty())
		magnification += " • " + _infoStripHint;

	painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignRight, magnification);

	int infoRight = textRect.x() + textRect.width() - fm.horizontalAdvance(magnification) - padding;

	// Centered on the strip: it stays put as the magnification text changes width.
	const int center = strip.center().x();
	const int captionHalfWidth = std::min((fm.horizontalAdvance(_infoStripCaption) + 1) / 2, infoRight - center);
	if (captionHalfWidth > 0)
	{
		const QRect captionRect{ center - captionHalfWidth, textRect.y(), 2 * captionHalfWidth, textRect.height() };
		painter.drawText(captionRect, Qt::AlignCenter, fm.elidedText(_infoStripCaption, Qt::ElideRight, captionRect.width()));
		infoRight = captionRect.x() - padding;
	}

	const int infoWidth = infoRight - textRect.x();
	if (infoWidth > 0)
		painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, fm.elidedText(imageInfoString(), Qt::ElideRight, infoWidth));
}

QRect CImageViewerWidget::pendingProgressBarRect() const
{
	const int padding = fontMetrics().height() / 2; // Same inset as the info strip's.
	const int barWidth = std::min(kPendingProgressBarWidth, width() - 2 * padding);
	return barWidth > 0 ? QRect{ (width() - barWidth) / 2, padding, barWidth, kPendingProgressBarHeight } : QRect{};
}

void CImageViewerWidget::paintPendingProgress(QPainter& painter) const
{
	const QRect bar = pendingProgressBarRect();
	if (!_pendingProgressShown || bar.isEmpty())
		return;

	const double shownFraction = _pendingProgressAnimation.currentValue().toDouble();
	// The info strip's colors
	painter.fillRect(bar, QColor(0, 0, 0, 140));
	painter.fillRect(QRect{ bar.topLeft(), QSize{ qRound(bar.width() * shownFraction), bar.height() } }, palette().color(QPalette::Highlight));
	painter.setPen(QColor(0xe8, 0xe8, 0xe8));
	painter.setBrush(Qt::NoBrush);
	painter.drawRect(bar.adjusted(0, 0, -1, -1)); // drawRect strokes one pixel past the rect's right and bottom edges
}

QRect CImageViewerWidget::navigatorRect() const
{
	if (!_overlayVisible || !isPannable())
		return {};

	const int longerSide = std::min(std::min(width(), height()) / 4, kNavigatorMaxSize);
	if (longerSide < kNavigatorMinSize)
		return {};

	const QSize navigatorSize = _imageSize.scaled(longerSide, longerSide, Qt::KeepAspectRatio);
	if (navigatorSize.isEmpty()) // An aspect ratio past kNavigatorMaxSize:1 truncates the shorter side to nothing.
		return {};

	const int padding = fontMetrics().height() / 2; // Same inset as the info strip's.
	return QRect{ QPoint{ width() - padding - navigatorSize.width(), padding }, navigatorSize };
}

QRect CImageViewerWidget::navigatorCropBox(const QRect& navigator) const noexcept
{
	const QRect crop = visibleImageRect();
	const qreal xScale = (qreal)navigator.width() / _imageSize.width();
	const qreal yScale = (qreal)navigator.height() / _imageSize.height();

	// At high magnification the crop maps to less than a navigator pixel, so the box is floored and then kept
	// inside the navigator by position: clipping it at the far edge would leave nothing to outline.
	const int boxWidth = std::min(navigator.width(), std::max(kNavigatorCropMinSize, qRound(crop.width() * xScale)));
	const int boxHeight = std::min(navigator.height(), std::max(kNavigatorCropMinSize, qRound(crop.height() * yScale)));

	return QRect{
		std::clamp(navigator.x() + qRound(crop.x() * xScale), navigator.x(), navigator.right() + 1 - boxWidth),
		std::clamp(navigator.y() + qRound(crop.y() * yScale), navigator.y(), navigator.bottom() + 1 - boxHeight),
		boxWidth,
		boxHeight
	};
}

void CImageViewerWidget::paintNavigator(QPainter& painter)
{
	const QRect navigator = navigatorRect();
	if (navigator.isEmpty())
		return;

	const qreal dpr = devicePixelRatioF();
	const QSize thumbnailPx = (QSizeF{ navigator.size() } * dpr).toSize();
	if (_navigatorThumbnail.size() != thumbnailPx)
		scaleSource(_navigatorThumbnail, QRect{}, thumbnailPx, Qt::SmoothTransformation);

	_navigatorThumbnail.setDevicePixelRatio(dpr);

	painter.save();

	painter.drawImage(snappedToDevicePixel(painter, navigator.topLeft()), _navigatorThumbnail);

	// Dim outside the crop rather than only outlining it: an outline alone is lost against busy image content.
	const QRect cropBox = navigatorCropBox(navigator);
	QRegion outsideCrop{ navigator };
	outsideCrop -= cropBox;
	for (const QRect& r : outsideCrop)
		painter.fillRect(r, QColor(0, 0, 0, 110));

	painter.setPen(QColor(0xe8, 0xe8, 0xe8));
	painter.drawRect(cropBox.adjusted(0, 0, -1, -1));
	painter.setPen(QColor(0, 0, 0, 140));
	painter.drawRect(navigator.adjusted(0, 0, -1, -1));

	painter.restore();
}

void CImageViewerWidget::centerViewOnNavigatorPoint(QPointF widgetPos)
{
	const QRect navigator = navigatorRect();
	if (navigator.isEmpty())
		return;

	const QPointF imagePos{
		(widgetPos.x() - navigator.x()) * _imageSize.width() / navigator.width(),
		(widgetPos.y() - navigator.y()) * _imageSize.height() / navigator.height()
	};

	_offset = centerOf(viewportDeviceSize()) - imagePos * _scale;
	clampOffset();
	update();
}

void CImageViewerWidget::resizeEvent(QResizeEvent* e)
{
	QWidget::resizeEvent(e);

	if (!e->oldSize().isValid())
		return;

	refitOrKeepViewCenter(QSizeF{ e->oldSize() } * devicePixelRatioF());
}

void CImageViewerWidget::showEvent(QShowEvent* e)
{
	QWidget::showEvent(e);

	_shown = true;
	startOrStopFrameTimer();
}

void CImageViewerWidget::hideEvent(QHideEvent* e)
{
	QWidget::hideEvent(e);

	// Sent to every child when the window is minimized, too.
	_shown = false;
	startOrStopFrameTimer();
}

void CImageViewerWidget::wheelEvent(QWheelEvent* e)
{
	const int delta = e->angleDelta().y();
	if (_sourceImage.isNull() || !_viewInitialized || size().isEmpty() || delta == 0 || !e->modifiers().testFlag(Qt::ControlModifier))
	{
		QWidget::wheelEvent(e);
		return;
	}

	const QPointF cursorDevice = e->position() * devicePixelRatioF();
	const qreal newScale = std::clamp(_scale * std::pow(1.0015, (qreal)delta), minScale(), kMaxScale);

	if (!qFuzzyCompare(newScale, _scale))
	{
		// Keep the source pixel under the cursor pinned in place as the scale changes.
		_offset = cursorDevice - (cursorDevice - _offset) * (newScale / _scale);
		setScale(newScale);
		clampOffset();
		update();
	}

	e->accept();
}

void CImageViewerWidget::mousePressEvent(QMouseEvent* e)
{
	if (_sourceImage.isNull() || !_viewInitialized || e->button() != Qt::LeftButton || !isPannable())
	{
		QWidget::mousePressEvent(e);
		return;
	}

	if (navigatorRect().contains(e->position().toPoint()))
	{
		_isNavigatorSteering = true;
		centerViewOnNavigatorPoint(e->position());
		e->accept();
		return;
	}

	_isPanning = true;
	_panStartOffset = _offset;
	_panStartMouseDevice = e->position() * devicePixelRatioF();
	setCursor(Qt::ClosedHandCursor);
	e->accept();
}

void CImageViewerWidget::mouseMoveEvent(QMouseEvent* e)
{
	if (_isNavigatorSteering)
	{
		centerViewOnNavigatorPoint(e->position());
		e->accept();
		return;
	}

	if (!_isPanning || _sourceImage.isNull())
	{
		QWidget::mouseMoveEvent(e);
		return;
	}

	_offset = _panStartOffset + (e->position() * devicePixelRatioF() - _panStartMouseDevice);
	clampOffset();
	update();
	e->accept();
}

void CImageViewerWidget::mouseReleaseEvent(QMouseEvent* e)
{
	if (e->button() == Qt::LeftButton && _isNavigatorSteering)
	{
		_isNavigatorSteering = false;
		e->accept();
		return;
	}

	if (e->button() == Qt::LeftButton && _isPanning)
	{
		_isPanning = false;
		unsetCursor();
		e->accept();
		return;
	}

	QWidget::mouseReleaseEvent(e);
}

void CImageViewerWidget::timerEvent(QTimerEvent* e)
{
	if (!_animation || e->timerId() != _animation->frameTimer.timerId())
	{
		QWidget::timerEvent(e);
		return;
	}

	// Only the first tick decodes here: later ones present a frame decoded during the previous interval.
	if (_animation->pendingFrame.isNull() && !decodeNextFrame())
		return;

	_animation->nextFrameDueMs += _animation->pendingDelayMs;
	presentPendingFrame();

	// Decoded after the presentation: this work belongs in the idle interval, not ahead of the next deadline.
	if (decodeNextFrame())
		scheduleNextFrame();
}
