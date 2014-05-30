#include "cimageresizer.h"

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
	case Bicubic:
		return bicubicInterpolation(source, targetSize, aspectRatio);
	}
}

QImage CImageResizer::bicubicInterpolation(const QImage& source, const QSize& targetSize, CImageResizer::AspectRatio aspectRatio)
{
	const float xFactor = float(source.width()) / targetSize.width();
	const float yFactor = float(source.height()) / targetSize.height();

	const int numChannels = (source.isGrayscale() ? 1 : 3) + (source.hasAlphaChannel() ? 1 : 0);
	const int bytesPerLineSource = source.bytesPerLine();

	const int bytesPerLineDest = bytesPerLineSource / xFactor;

	QImage dest(targetSize, source.format());

	for (int i = 0; i < targetSize.height(); ++i)
	{
		for (int j = 0; j < targetSize.width(); ++j)
		{
			const int x = int(xFactor * j);
			const int y = int(yFactor * i);
			const float dx = xFactor * j - x;
			const float dy = yFactor * i - y;

			const int index = y * bytesPerLineSource + x * numChannels;
			const int a = y * bytesPerLineSource + (x + 1) * numChannels;
			const int b = (y + 1) * bytesPerLineSource + x * numChannels;
			const int c = (y + 1) * bytesPerLineSource + (x + 1) * numChannels;

			unsigned char C[5] = {0};
			for (int jj = 0; jj <= 3; ++jj)
			{
				QRgb pixel1(source.pixel(x-1, y-1+jj));
				QRgb pixel2(source.pixel(x+1, y-1+jj));
				QRgb pixel3(source.pixel(x+2, y-1+jj));
				QRgb pixel4(source.pixel(x, y-1+jj));
			}
		}
	}

	return dest;
}
