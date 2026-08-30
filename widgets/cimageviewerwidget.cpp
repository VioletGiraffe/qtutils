#include "cimageviewerwidget.h"
#include "utils/ciconengineqimage.h"

DISABLE_COMPILER_WARNINGS
#include <QApplication>
#include <QClipboard>
#include <QColor>
#include <QDebug>
#include <QFontMetrics>
#include <QHash>
#include <QImageReader>
#include <QMouseEvent>
#include <QPainter>
#include <QRegion>
#include <QResizeEvent>
#include <QScreen>
#include <QWheelEvent>
#include <QtMath>
RESTORE_COMPILER_WARNINGS

#include <algorithm>
#include <math.h>
#include <utility>

static constexpr qreal kMaxScale = 40.0; // device px per source px; the maximum magnification.
static constexpr qreal kMinScaledImageSize = 6.0; // logical px; the longer side of the image at the most zoomed-out scale.

static constexpr int kNavigatorMaxSize = 100;       // logical px, longer side of the navigator
static constexpr int kNavigatorMinSize = 40;       // below this the navigator is not shown at all: too small to aim at
static constexpr int kNavigatorThumbnailSize = kNavigatorMaxSize * 2; // device px; covers kNavigatorMaxSize at a device pixel ratio of 2
static constexpr int kNavigatorCropMinSize = 3;     // logical px; a thinner crop box would have nothing left to outline

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

	inline void scaleImage(const CImageViewerWidget::ImageScaleFunction& scaler, QImage& dest, const QImage& source, const QRect& srcRect)
	{
		if (scaler)
			scaler(dest, source, srcRect);
		else
			CImageViewerWidget::smoothScale(dest, source, srcRect);
	}
}

void CImageViewerWidget::setImageScaler(ImageScaleFunction scaler) noexcept
{
	_imageScaler = std::move(scaler);
}

void CImageViewerWidget::smoothScale(QImage& dest, const QImage& source, const QRect& srcRect)
{
	const QSize targetSize = dest.size();
	dest = (srcRect.isEmpty() ? source : source.copy(srcRect)).scaled(targetSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

bool CImageViewerWidget::displayImage(const QImage& image)
{
	_sourceImage = image;
	// No file behind a bare image; the overload below fills these in after calling here.
	_currentImageFormat.clear();
	_currentImageFileSize = 0;
	_isPanning = false;
	_isNavigatorSteering = false;
	_navigatorThumbnail = QImage{};
	_cacheKey = 0;
	_viewInitialized = false; // Refit to the new image on the next paint.
	updateGeometry(); // Because the image affects sizeHint()
	update();
	return !_sourceImage.isNull();
}

bool CImageViewerWidget::displayImage(const QString& imagePath)
{
	QImageReader reader(imagePath);
	reader.setAutoDetectImageFormat(true);
	reader.setAutoTransform(true);

	const QString fileFormat = QString::fromLatin1(reader.format());
	QImage img = reader.read();
	if (img.isNull())
		return false;

	if (const auto format = img.format(); format == QImage::Format_Indexed8 || format == QImage::Format_Grayscale16 || format == QImage::Format_RGBA64 || format == QImage::Format_RGBX64)
	{
		img.convertTo(img.hasAlphaChannel() ? QImage::Format_ARGB32 : QImage::Format_RGB32);
		qInfo() << "CImageViewerWidget::displayImage: converted image format from" << format << "to" << img.format();
	}

	const qint64 fileSize = reader.device()->size();
	const bool displayed = displayImage(img);
	_currentImageFormat = fileFormat;
	_currentImageFileSize = fileSize;
	return displayed;
}

QString CImageViewerWidget::imageInfoString() const
{
	if (_sourceImage.isNull())
		return QString();

	const int numChannels = _sourceImage.isGrayscale() ? 1 : (3 + (_sourceImage.hasAlphaChannel() ? 1 : 0));
	const QString imageInfo = tr("%1x%2 (%3 MP), %4 channels, %5 bits per pixel").
		arg(_sourceImage.width()).
		arg(_sourceImage.height()).
		arg(_sourceImage.width() * _sourceImage.height() * 1e-6, 0, 'f', 1).
		arg(numChannels).
		arg(_sourceImage.bitPlaneCount());

	if (_currentImageFileSize <= 0) // Displayed from a QImage: there is no file to name a format or a compression ratio for
		return imageInfo;

	return _currentImageFormat.toUpper() + ' ' + imageInfo + tr(", compressed to %1 bits per pixel").
		arg(QString::number(8.0 * (double)_currentImageFileSize / double(_sourceImage.width() * _sourceImage.height()), 'f', 2));
}

QSize CImageViewerWidget::sizeHint() const
{
	if (_sourceImage.isNull())
		return QWidget::sizeHint();

	const auto maxSize = screen()->availableGeometry().size() - QSize(60, 60);
	const qreal dpr = devicePixelRatioF();
	return QSize{
		std::clamp(qCeil(_sourceImage.width() / dpr), 150, maxSize.width()),
		std::clamp(qCeil(_sourceImage.height() / dpr), 150, maxSize.height())
	};
}

QIcon CImageViewerWidget::imageIcon() const
{
	if (_sourceImage.isNull())
		return QIcon{};

	// Copied intentionally: probably not necessary, but the lambda is stored in the icon engine and might outlive this widget
	const auto resizeImage = [scaler = _imageScaler](const QImage& src, const QSize& targetSize) -> QImage {
		QImage dst(targetSize, src.format());
		scaleImage(scaler, dst, src, QRect{});
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

void CImageViewerWidget::copyToClipboard() noexcept
{
	if (!_sourceImage.isNull())
		QApplication::clipboard()->setImage(_sourceImage);
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
	return QSizeF{ _sourceImage.size() } * _scale;
}

QRect CImageViewerWidget::visibleSourceRect() const noexcept
{
	// Back-project the viewport through the inverse transform sourcePx = (devicePx - offset) / scale, clipped to the image.
	const QSizeF viewport = viewportDeviceSize();
	const int x0 = qFloor(std::clamp(-_offset.x() / _scale, 0.0, (qreal)_sourceImage.width()));
	const int y0 = qFloor(std::clamp(-_offset.y() / _scale, 0.0, (qreal)_sourceImage.height()));
	const int x1 = qCeil(std::clamp((viewport.width() - _offset.x()) / _scale, 0.0, (qreal)_sourceImage.width()));
	const int y1 = qCeil(std::clamp((viewport.height() - _offset.y()) / _scale, 0.0, (qreal)_sourceImage.height()));

	return QRect{ x0, y0, x1 - x0, y1 - y0 };
}

qreal CImageViewerWidget::fitScale() const noexcept
{
	const QSizeF viewport = viewportDeviceSize();
	return std::min(viewport.width() / _sourceImage.width(), viewport.height() / _sourceImage.height());
}

qreal CImageViewerWidget::minScale() const noexcept
{
	// Zoom out until the longer side is down to kMinScaledImageSize. Fit and 1:1 lower the floor further for an image
	// already at or below that size, so both stay reachable.
	const qreal longerSide = (qreal)std::max(_sourceImage.width(), _sourceImage.height());
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
	_fitToWindow = _scale == fitScale();
}

void CImageViewerWidget::resetToFit() noexcept
{
	setScale(fitScale());
	_offset = centeredOffset();
	_viewInitialized = true;
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

	setScale(1.0); // 1:1 == one source pixel per one device pixel; always within [minScale(), kMaxScale].
	_offset = centeredOffset();
	_viewInitialized = true;
	update();
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
		return;
	}

	if (size().isEmpty())
		return;

	if (!_viewInitialized)
		resetToFit();

	const QRect sourceRect = visibleSourceRect();
	if (sourceRect.isEmpty())
		return;

	// The visible crop occupies sourceRect * scale device pixels; render the buffer at exactly that resolution for a 1:1 blit.
	const QSize bufferPx{
		std::max(1, qRound(sourceRect.width() * _scale)),
		std::max(1, qRound(sourceRect.height() * _scale))
	};

	if (_displayImage.size() != bufferPx || _displayImage.format() != _sourceImage.format())
		_displayImage = QImage(bufferPx.width(), bufferPx.height(), _sourceImage.format());

	const size_t newCacheKey = qHashMulti(4 /* true random number, chosen by a fair dice throw */, sourceRect, bufferPx);
	if (newCacheKey != _cacheKey)
	{
		_cacheKey = newCacheKey;
		scaleImage(_imageScaler, _displayImage, _sourceImage, sourceRect);
	}

	// Set after scaling: a scaler is free to replace the buffer.
	const qreal dpr = devicePixelRatioF();
	_displayImage.setDevicePixelRatio(dpr);

	// Snap the blit to a whole device pixel so a 1:1 view stays pixel-exact; a fractional origin would make the painter resample the buffer.
	const QPoint targetDevice = (_offset + QPointF{ (qreal)sourceRect.x(), (qreal)sourceRect.y() } * _scale).toPoint();
	p.drawImage(QPointF{ targetDevice } / dpr, _displayImage);

	paintNavigator(p);

	if (_overlayVisible)
		paintInfoStrip(p);
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

	const int infoWidth = textRect.width() - fm.horizontalAdvance(magnification) - padding;
	if (infoWidth > 0)
		painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, fm.elidedText(imageInfoString(), Qt::ElideRight, infoWidth));
}

QRect CImageViewerWidget::navigatorRect() const
{
	if (!_overlayVisible || !isPannable())
		return {};

	const int longerSide = std::min(std::min(width(), height()) / 4, kNavigatorMaxSize);
	if (longerSide < kNavigatorMinSize)
		return {};

	const QSize navigatorSize = _sourceImage.size().scaled(longerSide, longerSide, Qt::KeepAspectRatio);
	if (navigatorSize.isEmpty()) // An aspect ratio past kNavigatorMaxSize:1 truncates the shorter side to nothing.
		return {};

	const int padding = fontMetrics().height() / 2; // Same inset as the info strip's.
	return QRect{ QPoint{ width() - padding - navigatorSize.width(), padding }, navigatorSize };
}

QRect CImageViewerWidget::navigatorCropBox(const QRect& navigator) const noexcept
{
	const QRect crop = visibleSourceRect();
	const qreal xScale = (qreal)navigator.width() / _sourceImage.width();
	const qreal yScale = (qreal)navigator.height() / _sourceImage.height();

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

	// Not the injected scaler: that one serves the view, and a thumbnail this small does not need it.
	if (_navigatorThumbnail.isNull())
		_navigatorThumbnail = _sourceImage.scaled(kNavigatorThumbnailSize, kNavigatorThumbnailSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

	painter.save();

	painter.setRenderHint(QPainter::SmoothPixmapTransform);
	painter.drawImage(navigator, _navigatorThumbnail);

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

	const QPointF sourcePos{
		(widgetPos.x() - navigator.x()) * _sourceImage.width() / navigator.width(),
		(widgetPos.y() - navigator.y()) * _sourceImage.height() / navigator.height()
	};

	_offset = centerOf(viewportDeviceSize()) - sourcePos * _scale;
	clampOffset();
	update();
}

void CImageViewerWidget::resizeEvent(QResizeEvent* e)
{
	QWidget::resizeEvent(e);

	if (_sourceImage.isNull() || !_viewInitialized || !e->oldSize().isValid() || size().isEmpty())
		return;

	// A view that showed the whole image keeps fitting it; a zoomed-in view keeps its scale and the source point at the viewport center.
	if (_fitToWindow)
	{
		resetToFit();
		return;
	}

	const QSizeF oldViewport = QSizeF{ e->oldSize() } * devicePixelRatioF();
	const QPointF centerSource = (centerOf(oldViewport) - _offset) / _scale;

	setScale(std::clamp(_scale, minScale(), kMaxScale));
	_offset = centerOf(viewportDeviceSize()) - centerSource * _scale;
	clampOffset();
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

	if (newScale != _scale)
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
