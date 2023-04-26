#if __has_include("resize/cimageinterpolationkernel.h")

#include "cimageresizer.h"
#include "resize/cimageinterpolationkernel.h"

#include "assert/advanced_assert.h"
#include "math/math.hpp"
#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QColor>
#include <QDebug>
#include <QImage>
RESTORE_COMPILER_WARNINGS

#include <math.h>
#include <stdint.h>

QSize scaled(const QSize& source, const QSize& dest)
{
	const float xScaleFactor = (float)source.width() / (float)dest.width();
	const float yScaleFactor = (float)source.height() / (float)dest.height();

	const float actualFactor = (xScaleFactor > 1.0f && yScaleFactor > 1.0f) ? std::max(xScaleFactor, yScaleFactor) : std::min(xScaleFactor, yScaleFactor);

	return source / actualFactor;
}

inline QRgb applyKernel(const CImageInterpolationKernelBase<float>& kernel, const QImage& source, uint32_t x, uint32_t y)
{
	assert_r(x >= 0 && y >= 0); // Tested - doesn't affect performance within 1 ms resolution

	float r = 0.0f, g = 0.0f, b = 0.0f, a = 0.0f;

	const uint32_t srcHeight = (uint32_t)source.height(), srcWidth = (uint32_t)source.width();
	const uint32_t kernelSize = kernel.size();

	using FloatType = decltype(kernel.coeff(0, 0));

	if (source.depth() == 32)
	{
		for (uint32_t k = y, k_kernel = 0; k < y + kernelSize && k < srcHeight; ++k, ++ k_kernel)
		{
			// TODO: strict aliasong violation!
			const QRgb * line = reinterpret_cast<const QRgb*>(source.constScanLine((int)k));
			for (uint32_t i = x, i_kernel = 0; i < x + kernelSize && i < srcWidth; ++i, ++i_kernel)
			{
				const auto coeff = kernel.coeff(i_kernel, k_kernel);
				r += static_cast<FloatType>(qRed(line[i])) * coeff;
				g += static_cast<FloatType>(qGreen(line[i])) * coeff;
				b += static_cast<FloatType>(qBlue(line[i])) * coeff;
				a += static_cast<FloatType>(qAlpha(line[i])) * coeff;
			}
		}
	}
	else
	{
		for (uint32_t i = x, i_kernel = 0; i < x + kernelSize && i < srcWidth; ++i, ++i_kernel)
			for (uint32_t k = y, k_kernel = 0; k < y + kernelSize && k < srcHeight; ++k, ++ k_kernel)
			{
				const QColor pixel(source.pixel(i, k));
				const auto coeff = kernel.coeff(i_kernel, k_kernel);

				r += static_cast<FloatType>(pixel.red()) * coeff;
				g += static_cast<FloatType>(pixel.green()) * coeff;
				b += static_cast<FloatType>(pixel.blue()) * coeff;
				a += static_cast<FloatType>(pixel.alpha()) * coeff;
			}
	}

	r = Math::clamp(FloatType{0.0f}, r, FloatType{255.0f});
	g = Math::clamp(FloatType{0.0f}, g, FloatType{255.0f});
	b = Math::clamp(FloatType{0.0f}, b, FloatType{255.0f});
	a = Math::clamp(FloatType{0.0f}, a, FloatType{255.0f});

	return qRgba(Math::round<int>(r),
				 Math::round<int>(g),
				 Math::round<int>(b),
				 Math::round<int>(a)
			);
}

inline float ratio(const QSize& size)
{
	return (float)size.width() / (float)size.height();
}

inline QSize operator*(const QSize& origSize, int k)
{
	return QSize(origSize.width() * k, origSize.height() * k);
}

QImage bicubicInterpolation(const QImage& source, const QSize& targetSize, ImageResizing::AspectRatio aspectRatio)
{
	using namespace ImageResizing;

	if (targetSize.width() == source.width() && targetSize.height() == source.height())
		return source;
	else if (targetSize.width() >= source.width() || targetSize.height() >= source.height())
		return source.scaled(targetSize, aspectRatio == KeepAspectRatio ? Qt::KeepAspectRatio : Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

	QSize actualTargetSize = targetSize;
	if (::fabs(ratio(targetSize) - ratio(source.size())) < 0.01f)
		actualTargetSize = source.size().scaled(targetSize, aspectRatio == KeepAspectRatio ? Qt::KeepAspectRatio : Qt::IgnoreAspectRatio);

	if (actualTargetSize.isEmpty())
		return QImage();

	const float scaleFactor = source.width() > actualTargetSize.width() ? source.width() / (float)actualTargetSize.width() : actualTargetSize.width() / (float)source.width();

	const int intScaleFactor = (int)ceilf(scaleFactor);
	const QSize upscaledSourceSize = ::fabs(scaleFactor - 1.0f) < 0.001f ? scaled(source.size(), actualTargetSize * intScaleFactor) : source.size();

	QImage dest(actualTargetSize, source.format());
	QImage upscaledSource(upscaledSourceSize == source.size() ? source : source.scaled(upscaledSourceSize, Qt::IgnoreAspectRatio, source.depth() == 32 ? Qt::SmoothTransformation : Qt::FastTransformation));

	// TODO: refactor this. There's no need to create all the kernels, we're only going to need one.
	const CLanczosKernel lanczosKernel((uint32_t)upscaledSource.width() / (uint32_t)actualTargetSize.width(), 3);
	const CBicubicKernel bicubicKernel((uint32_t)upscaledSource.width() / (uint32_t)actualTargetSize.width(), 0.5f);

	const CImageInterpolationKernelBase<float>& kernel = upscaledSourceSize.width() / actualTargetSize.width() >= 30 ?
		(CImageInterpolationKernelBase<float>&)bicubicKernel :
		(CImageInterpolationKernelBase<float>&)bicubicKernel;

	const uint32_t kernelSize = kernel.size();
	const uint32_t targetWidth = (uint32_t)actualTargetSize.width(), targetHeight = (uint32_t)actualTargetSize.height();
	if (upscaledSource.depth() == 32)
	{
		for (uint32_t y = 0; y < targetHeight; ++y)
		{
			auto* scanLine = dest.scanLine(y);
			for (uint32_t x = 0; x < targetWidth; ++x)
			{
				const auto rgb = applyKernel(kernel, upscaledSource, x*kernelSize, y*kernelSize);
				auto* pixelAddress = scanLine + sizeof(QRgb) * x;
				memcpy(pixelAddress, &rgb, sizeof(rgb));
			}
		}
	}
	else
	{
		for (uint32_t x = 0; x < targetWidth; ++x)
		{
			for (uint32_t y = 0; y < targetHeight; ++y)
			{
				dest.setPixel(x, y, applyKernel(kernel, upscaledSource, x*kernelSize, y*kernelSize));
			}
		}
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
