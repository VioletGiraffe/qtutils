#if __has_include("resize/cimageinterpolationkernel.h")

#include "cimageresizer.h"
#include "resize/cimageinterpolationkernel.h"
#include "assert/advanced_assert.h"
#include "math/math.hpp"
#include "compiler/compiler_warnings_control.h"
#include "system/ctimeelapsed.h"
#include "utility/on_scope_exit.hpp"

DISABLE_COMPILER_WARNINGS
#include <QColor>
#include <QDebug>
#include <QImage>
RESTORE_COMPILER_WARNINGS

#include <math.h>
#include <stdint.h>

QSize scaled(const QSize& source, const QSize& dest)
{
	const float xScaleFactor = source.width() / (float)dest.width();
	const float yScaleFactor = source.height() / (float)dest.height();

	const float actualFactor = (xScaleFactor > 1.0f && yScaleFactor > 1.0f) ? std::max(xScaleFactor, yScaleFactor) : std::min(xScaleFactor, yScaleFactor);

	return source / actualFactor;
}

inline QRgb applyKernel(const CImageInterpolationKernelBase<float>& kernel, const QImage& source, int x, int y)
{
	assert_r(x >= 0 && y >= 0); // Tested - doesn't affect performance within 1 ms resolution

	float r = 0.0f, g = 0.0f, b = 0.0f, a = 0.0f;

	const auto srcHeight = source.height(), srcWidth = source.width();
	const auto kernelSize = kernel.size();
	if (source.depth() == 32)
	{
		for (int k = y, k_kernel = 0; k < y + kernelSize && k < srcHeight; ++k, ++ k_kernel)
		{
			const QRgb * line = (const QRgb*)source.constScanLine(k);
			for (int i = x, i_kernel = 0; i < x + kernelSize && i < srcWidth; ++i, ++i_kernel)
			{
				const auto coeff = kernel.coeff(i_kernel, k_kernel);
				r += qRed(line[i]) * coeff;
				g += qGreen(line[i]) * coeff;
				b += qBlue(line[i]) * coeff;
				a += qAlpha(line[i]) * coeff;
			}
		}
	}
	else
	{
		for (int i = x, i_kernel = 0; i < x + kernelSize && i < srcWidth; ++i, ++i_kernel)
			for (int k = y, k_kernel = 0; k < y + kernelSize && k < srcHeight; ++k, ++ k_kernel)
			{
				const QColor pixel(source.pixel(i, k));
				const auto coeff = kernel.coeff(i_kernel, k_kernel);

				r += pixel.red() * coeff;
				g += pixel.green() * coeff;
				b += pixel.blue() * coeff;
				a += pixel.alpha() * coeff;
			}
	}

	r = Math::clamp(0.0f, r, 255.0f);
	g = Math::clamp(0.0f, g, 255.0f);
	b = Math::clamp(0.0f, b, 255.0f);
	a = Math::clamp(0.0f, a, 255.0f);
	return qRgba((int)(r + 0.5f), (int)(g + 0.5f), (int)(b + 0.5f), (int)(a + 0.5f));
}

inline float ratio(const QSize& size)
{
	return (float)size.width() / size.height();
}

inline QSize operator*(const QSize& origSize, int k)
{
	return QSize(origSize.width()*k, origSize.height()*k);
}

QImage bicubicInterpolation(const QImage& source, const QSize& targetSize, ImageResizing::AspectRatio aspectRatio)
{
	using namespace ImageResizing;

	if (targetSize.width() == source.width() && targetSize.height() == source.height())
		return source;
	else if (targetSize.width() >= source.width() || targetSize.height() >= source.height())
		return source.scaled(targetSize, aspectRatio == KeepAspectRatio ? Qt::KeepAspectRatio : Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

	QSize actualTargetSize = targetSize;
	if (ratio(targetSize) != ratio(source.size()))
		actualTargetSize = source.size().scaled(targetSize, aspectRatio == KeepAspectRatio ? Qt::KeepAspectRatio : Qt::IgnoreAspectRatio);

	if (actualTargetSize.isEmpty())
		return QImage();

	const float scaleFactor = source.width() > actualTargetSize.width() ? source.width() / (float)actualTargetSize.width() : actualTargetSize.width() / (float)source.width();

	const int intScaleFactor = (int)ceilf(scaleFactor);
	const QSize upscaledSourceSize = scaleFactor != 1 ? scaled(source.size(), actualTargetSize * intScaleFactor) : source.size();

	QImage dest(actualTargetSize, source.format());
	QImage upscaledSource(upscaledSourceSize == source.size() ? source : source.scaled(upscaledSourceSize, Qt::IgnoreAspectRatio, source.depth() == 32 ? Qt::SmoothTransformation : Qt::FastTransformation));

	// TODO: refactor this. There's no need to create all the kernels, we're only going to need one.
	const CLanczosKernel lanczosKernel(upscaledSource.width() / actualTargetSize.width(), 3);
	const CBicubicKernel bicubicKernel(upscaledSource.width() / actualTargetSize.width(), 0.5f);

	const CImageInterpolationKernelBase<float>& kernel = upscaledSourceSize.width() / actualTargetSize.width() >= 30 ?
		(CImageInterpolationKernelBase<float>&)bicubicKernel :
		(CImageInterpolationKernelBase<float>&)bicubicKernel;

	const auto kernelSize = kernel.size();
	const auto targetWidth = actualTargetSize.width(), targetHeight = actualTargetSize.height();
	if (upscaledSource.depth() == 32)
	{
		for (int y = 0; y < targetHeight; ++y)
		{
			auto* scanLine = dest.scanLine(y);
			for (int x = 0; x < targetWidth; ++x)
			{
				const auto rgb = applyKernel(kernel, upscaledSource, x*kernelSize, y*kernelSize);
				auto* pixelAddress = scanLine + sizeof(QRgb) * x;
				memcpy(pixelAddress, &rgb, sizeof(rgb));
			}
		}
	}
	else
	{
		for (int x = 0; x < targetWidth; ++x)
			for (int y = 0; y < targetHeight; ++y)
				dest.setPixel(x, y, applyKernel(kernel, upscaledSource, x*kernelSize, y*kernelSize));
	}
	return dest;
}

QImage ImageResizing::resize(const QImage& source, const QSize& targetSize, ResizeMethod method, AspectRatio aspectRatio)
{
//	CTimeElapsed timer(true);
//	EXEC_ON_SCOPE_EXIT([&]{
//		qInfo() << "Image resized from" << source.size() << "to" << targetSize << "in" << timer.elapsed() << "ms";
//	});

	switch(method)
	{
	case DefaultQimageFast:
		return source.scaled(targetSize, aspectRatio == KeepAspectRatio ? Qt::KeepAspectRatio : Qt::IgnoreAspectRatio, Qt::FastTransformation);
	case DefaultQimageSmooth:
		return source.scaled(targetSize, aspectRatio == KeepAspectRatio ? Qt::KeepAspectRatio : Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	case Smart:
	{
		auto actualTargetSize = source.size().scaled(targetSize, aspectRatio == KeepAspectRatio ? Qt::KeepAspectRatio : Qt::IgnoreAspectRatio);
		if (source.size().width() < actualTargetSize.width())
			return source.scaled(targetSize, aspectRatio == KeepAspectRatio ? Qt::KeepAspectRatio : Qt::IgnoreAspectRatio, Qt::FastTransformation);
		else
			return source.scaled(targetSize, aspectRatio == KeepAspectRatio ? Qt::KeepAspectRatio : Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	}
	default:
		return {};
	}
}

#endif
