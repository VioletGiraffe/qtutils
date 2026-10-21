# Qt icon and pixmap quirks

Known `QIcon` and `QPixmap` traps with their remedies.

## Identity

- `QIcon::cacheKey()` identifies the icon engine instance, not the image. `QFileIconProvider` creates an engine per
  file, so files showing the same icon get distinct keys. Equal keys imply equal icons; unequal keys imply nothing.
- `QIcon::pixmap()` sets the device pixel ratio on its result. The DPR lives in the shared pixel data, so on a pixmap
  also held by `QPixmapCache` this detaches and copies the whole image, and the result's `QPixmap::cacheKey()` is
  unique per call.
- For content identity, hash the pixels (`QImage::constBits()`) together with the format and dimensions.

## Scaling

- `QIcon::paint` draws the engine's pixmap into the target rect with the painter's own render hints. Engines may return
  a pixmap far larger than requested - Windows file icons arrive at 256 px for any request above 48 px - so without
  `QPainter::SmoothPixmapTransform` the downscale is nearest-neighbour and diagonals turn jagged.
- Whether `QIcon::pixmap()` returns more pixels than requested depends on the engine (Qt 6.11.2):
  - Windows file icons (`QWindowsFileIconEngine`): a native image-list size, never scaled.
  - macOS file icons (`QAppleFileIconEngine`): the system renders the `NSImage` at the requested size.
  - Theme icons (`QIconLoaderEngine`), SVG entries: rendered at the requested size.
  - Theme bitmap entries and `QPixmapIconEngine`: a larger bitmap downscaled with `QPixmap::scaled(..., Qt::SmoothTransformation)`.

  Only Windows file icons reach a caller's own downscaler at more than the requested size. Requesting a larger size to get
  a theme bitmap's native pixels would render SVG entries oversized: the public API does not tell the two apart.
- `SmoothPixmapTransform` is bilinear: 2x2 source pixels per destination pixel, so below 0.5x it skips source pixels
  and still aliases. For quality, downscale once with an area-averaging scaler (`QImage::scaled` with
  `Qt::SmoothTransformation`) and draw the result unscaled.
