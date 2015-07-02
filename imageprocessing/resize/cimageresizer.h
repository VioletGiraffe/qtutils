#ifndef CIMAGERESIZER_H
#define CIMAGERESIZER_H

#include "utils/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QImage>
RESTORE_COMPILER_WARNINGS

class CImageResizer
{
public:
	enum ResizeMethod {
		DefaultQimageFast,
		DefaultQimageSmooth,
		Smart
	};

	enum AspectRatio {
		KeepAspectRatio,
		IgnoreAspectRatio
	};

	CImageResizer();

	static QImage resize(const QImage& source, const QSize& targetSize, ResizeMethod method, AspectRatio aspectRatio = KeepAspectRatio);

private:
	static QImage bicubicInterpolation(const QImage& source, const QSize& targetSize, AspectRatio aspectRatio = KeepAspectRatio);
};

#endif // CIMAGERESIZER_H
