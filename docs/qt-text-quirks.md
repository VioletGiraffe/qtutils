# Qt text metrics quirks

Known `QFontMetrics` traps with their remedies.

## Rounded widths

- `QFontMetrics::horizontalAdvance()` returns the text's fractional advance rounded to the nearest pixel (Qt 6.11.2).
- `QFontMetrics::elidedText()` returns the text whole only if its unrounded width fits the width given.
  Text elided to its own `horizontalAdvance()` loses its tail to an ellipsis whenever the fraction is below .5.
- `QPainter::drawText()` into a rect clips text wider than the rect, unless `Qt::TextDontClip` is set.
- Only fonts with fractional advances are affected, so the symptom depends on the font and the exact string.
- Remedy: size a rect that must hold text whole with `roundedUpTextWidth()` (`utils/painterutils.h`): the `QFontMetricsF`
  advance, rounded up.
