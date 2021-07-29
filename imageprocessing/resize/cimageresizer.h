#pragma once

class QImage;
class QSize;

namespace ImageResizing
{

	enum ResizeMethod {
		DefaultQimageFast,
		DefaultQimageSmooth,
		Smart
	};

	enum AspectRatio {
		KeepAspectRatio,
		IgnoreAspectRatio
	};

	QImage resize(const QImage& source, const QSize& targetSize, ResizeMethod method, AspectRatio aspectRatio = KeepAspectRatio);

} // namespace ImageResizing
