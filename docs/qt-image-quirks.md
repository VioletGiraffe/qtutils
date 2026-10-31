# Qt image quirks

Known `QImage` traps with their remedies.

## Format queries

- `QImage::isGrayscale()` reports the content, not the format (Qt 6.11.2):
  - 16-, 24- and 32-bit color formats: `allGray()`, a scan of every pixel for R == G == B. An RGB image of gray content
    reports true, and a large one costs a full scan per call.
  - `Format_Indexed8`: true when the color table is the gray ramp.
  - 64-bit and floating-point formats: always false, whatever the content.

  Never call it for the layout: read `pixelFormat()` (`colorModel()`, the channel sizes, `alphaUsage()`) instead. When
  the content is the question, call `allGray()` explicitly, off the paint path.
