#include "ciconengineqimage.h"

#include "assert/advanced_assert.h"

DISABLE_COMPILER_WARNINGS
#include <QApplication>
#include <QGuiApplication>
#include <QPainter>
#include <QPixmap>
#include <QStyle>
#include <QStyleOption>
RESTORE_COMPILER_WARNINGS

#include <utility>

CIconEngineQImage::CIconEngineQImage(QImage source, ScaleFunction scale) noexcept :
	_source{ std::move(source) },
	_scale{ std::move(scale) }
{
	_source.setDevicePixelRatio(1.0);

	if (!_scale)
	{
		_scale = [](const QImage& source, const QSize& targetSize) -> QImage {
			return source.scaled(targetSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
		};
	}
}

QIconEngine* CIconEngineQImage::clone() const
{
	return new CIconEngineQImage{ *this };
}

bool CIconEngineQImage::isNull()
{
	return _source.isNull();
}

QPixmap CIconEngineQImage::scaledPixmap(const QSize& size, QIcon::Mode mode, QIcon::State /*state*/, qreal scale)
{
	// QIcon::pixmap() re-derives the returned pixmap's dpr from its pixel count, and it comes back as `scale` only
	// if that count is exactly size * scale. Too few pixels lower the dpr instead of being upscaled, which shrinks
	// the icon rather than blurring it.
	const QSize targetSize = size * scale;
	if (_source.isNull() || targetSize.isEmpty())
		return {};

	const CacheKey key{ targetSize, mode };

	QPixmap pixmap = _cache.value(key);
	if (pixmap.isNull())
	{
		pixmap = QPixmap::fromImage(renderFitted(targetSize));

		if (mode != QIcon::Normal)
		{
			QStyleOption option;
			option.palette = QGuiApplication::palette();

			const QPixmap styled = QApplication::style()->generatedIconPixmap(mode, pixmap, &option);
			if (!styled.isNull())
				pixmap = styled;
		}

		_cache.insert(key, pixmap);
	}

	pixmap.setDevicePixelRatio(scale);
	return pixmap;
}

QPixmap CIconEngineQImage::pixmap(const QSize& size, QIcon::Mode mode, QIcon::State state)
{
	return scaledPixmap(size, mode, state, 1.0);
}

QList<QSize> CIconEngineQImage::availableSizes(QIcon::Mode /*mode*/, QIcon::State /*state*/)
{
#ifdef Q_OS_MACOS
	return { QSize{ 16, 16 } }; // The title bar's document proxy icon button; the request arrives scaled by the screen's DPR.
#else
	return {};
#endif
}

void CIconEngineQImage::paint(QPainter* painter, const QRect& rect, QIcon::Mode mode, QIcon::State state)
{
	const QPaintDevice* device = painter->device();
	const qreal dpr = device ? device->devicePixelRatio() : qApp->devicePixelRatio();
	painter->drawPixmap(rect, scaledPixmap(rect.size(), mode, state, dpr));
}

QImage CIconEngineQImage::renderFitted(const QSize& targetSize) const
{
	// QSize::scaled truncates, and an extreme aspect ratio fits into an icon-sized box at zero height.
	const QSize fittedSize = _source.size().scaled(targetSize, Qt::KeepAspectRatio).expandedTo({ 1, 1 });

	QImage fitted = _scale(_source, fittedSize);
	assert_r(fitted.size() == fittedSize);
	fitted.setDevicePixelRatio(1.0);

	if (fitted.size() == targetSize)
		return fitted;

	QImage padded{ targetSize, QImage::Format_ARGB32_Premultiplied };
	padded.fill(Qt::transparent);

	QPainter painter{ &padded };
	painter.drawImage(QPoint{ (targetSize.width() - fitted.width()) / 2, (targetSize.height() - fitted.height()) / 2 }, fitted);

	return padded;
}
