#include "cimageresizer.h"
#include "cimageinterpolationkernel.h"
#include "assert/advanced_assert.h"

DISABLE_COMPILER_WARNINGS
#include <QColor>
RESTORE_COMPILER_WARNINGS

#include <math.h>
#include <time.h>
#include <memory>

QSize scaled(const QSize& source, const QSize& dest)
{
	const float xScaleFactor = source.width() / (float)dest.width();
	const float yScaleFactor = source.height() / (float)dest.height();

	const float actualFactor = (xScaleFactor > 1.0f && yScaleFactor > 1.0f) ? std::max(xScaleFactor, yScaleFactor) : std::min(xScaleFactor, yScaleFactor);

	return source / actualFactor;
}

#define CLAMP_TO_255(x) if (x > 255.0f) x = 255.0f; else if (x < 0.0f) x = 0.0f;

inline QRgb applyKernel(const CImageInterpolationKernelBase<float>* kernel, const QImage& source, int x, int y)
{
	assert_r(x >= 0 && y >= 0);

	float r = 0.0f, g = 0.0f, b = 0.0f, a = 0.0f;

	if (source.depth() == 32)
	{
		for (int k = y, k_kernel = 0; k < y + kernel->size() && k < source.height(); ++k, ++ k_kernel)
		{
			const QRgb * line = (const QRgb*)source.constScanLine(k);
			for (int i = x, i_kernel = 0; i < x + kernel->size() && i < source.width(); ++i, ++i_kernel)
			{
				r += qRed(line[i]) * kernel->coeff(i_kernel, k_kernel);
				g += qGreen(line[i]) * kernel->coeff(i_kernel, k_kernel);
				b += qBlue(line[i]) * kernel->coeff(i_kernel, k_kernel);
				a += qAlpha(line[i]) * kernel->coeff(i_kernel, k_kernel);
			}
		}
	}
	else
	{
		for (int i = x, i_kernel = 0; i < x + kernel->size() && i < source.width(); ++i, ++i_kernel)
			for (int k = y, k_kernel = 0; k < y + kernel->size() && k < source.height(); ++k, ++ k_kernel)
			{
				const QColor pixel(source.pixel(i, k));
				r += pixel.red() * kernel->coeff(i_kernel, k_kernel);
				g += pixel.green() * kernel->coeff(i_kernel, k_kernel);
				b += pixel.blue() * kernel->coeff(i_kernel, k_kernel);
				a += pixel.alpha() * kernel->coeff(i_kernel, k_kernel);
			}
	}
	
	CLAMP_TO_255(r);
	CLAMP_TO_255(g);
	CLAMP_TO_255(b);
	CLAMP_TO_255(a);
	return qRgba((int)(r + 0.5f), (int)(g + 0.5f), (int)(b + 0.5f), (int)(a + 0.5f));
}

static QSize operator* (const QSize& origSize, int k)
{
	return QSize(origSize.width()*k, origSize.height()*k);
}


static float ratio(const QSize& size)
{
	return (float)size.width() / size.height();
}

CImageResizer::CImageResizer()
{
}

QImage CImageResizer::resize(const QImage& source, const QSize& targetSize, CImageResizer::ResizeMethod method, AspectRatio aspectRatio)
{
	switch(method)
	{
	case DefaultQimageFast:
		return source.scaled(targetSize, aspectRatio == KeepAspectRatio ? Qt::KeepAspectRatio : Qt::IgnoreAspectRatio, Qt::FastTransformation);
	case DefaultQimageSmooth:
		return source.scaled(targetSize, aspectRatio == KeepAspectRatio ? Qt::KeepAspectRatio : Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	case Smart:
		return bicubicInterpolation(source, targetSize, aspectRatio);
	default:
		return QImage();
	}
}

QImage CImageResizer::bicubicInterpolation(const QImage& source, const QSize& targetSize, CImageResizer::AspectRatio aspectRatio)
{
	if (targetSize.width() >= source.width() || targetSize.height() >= source.height())
		return source.scaled(targetSize, aspectRatio == KeepAspectRatio ? Qt::KeepAspectRatio : Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

	QSize actualTargetSize = targetSize;
	if (ratio(targetSize) != ratio(source.size()))
		actualTargetSize = source.size().scaled(targetSize, aspectRatio == KeepAspectRatio ? Qt::KeepAspectRatio : Qt::IgnoreAspectRatio);

	float scaleFactor = source.width() / (float)actualTargetSize.width();
	if (scaleFactor < 1.0f)
		scaleFactor = 1.0f / scaleFactor;

	const int intScaleFactor = (int)(scaleFactor + 0.97f);
	const QSize upscaledSourceSize = scaleFactor != 1 ? scaled(source.size(), actualTargetSize * intScaleFactor) : source.size();

	QImage dest(actualTargetSize, source.format());
	QImage upscaledSource(upscaledSourceSize == source.size() ? source : source.scaled(upscaledSourceSize, Qt::IgnoreAspectRatio, source.depth() == 32 ? Qt::SmoothTransformation : Qt::FastTransformation));

	std::shared_ptr<CImageInterpolationKernelBase<float>> kernel;
	if (upscaledSourceSize.width() / actualTargetSize.width() >= 30) 
		kernel = std::make_shared<CLanczosKernel>(upscaledSource.width() / actualTargetSize.width(), 3);
	else
		kernel = std::make_shared<CBicubicKernel>(upscaledSource.width() / actualTargetSize.width(), 0.5f);

	if (upscaledSource.depth() == 32)
	{
		for (int y = 0; y < actualTargetSize.height(); ++y)
		{
			QRgb * destLine = (QRgb*)dest.scanLine(y);
			for (int x = 0; x < actualTargetSize.width(); ++x)
				destLine[x] = applyKernel(kernel.get(), upscaledSource, x*kernel->size(), y*kernel->size());
		}
	}
	else
	{
		for (int x = 0; x < actualTargetSize.width(); ++x)
			for (int y = 0; y < actualTargetSize.height(); ++y)
				dest.setPixel(x, y, applyKernel(kernel.get(), upscaledSource, x*kernel->size(), y*kernel->size()));
	}

	return dest;
}
