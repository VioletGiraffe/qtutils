#include "cplaintexteditwithlinenumbers.h"

DISABLE_COMPILER_WARNINGS
#include <QColor>
#include <QEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QPalette>
#include <QTextBlock>
RESTORE_COMPILER_WARNINGS

// Padding on either side of the number within the number area
static constexpr int LeftNumberMargin = 3;
static constexpr int RightNumberMargin = 4;

// Band on a dark surface: an absolute lift plus a share of the surface's own lightness, so the darkest themes get the smallest one
static constexpr int BandLightnessLift = 20;
static constexpr int BandLightnessLiftPercent = 80;
// Band on a light surface: a share off every channel, which leaves hue and saturation untouched
static constexpr int BandShadePercent = 12;

// A shade is a share of each channel; a lift cannot be, since a near-black surface has almost nothing to scale.
static QColor bandColorFor(const QColor& surface)
{
	int h, s, l, a;
	surface.getHsl(&h, &s, &l, &a);

	if (l >= 128)
	{
		const float shade = (100 - BandShadePercent) / 100.0f;
		return QColor::fromRgbF(surface.redF() * shade, surface.greenF() * shade, surface.blueF() * shade, surface.alphaF());
	}

	// Below mid-lightness HSL saturation tracks the chroma-to-lightness ratio, so raising l alone preserves the surface's tint.
	// l < 128 here, so the lift cannot overshoot 255.
	return QColor::fromHsl(h, s, l + BandLightnessLift + l * BandLightnessLiftPercent / 100, a).toRgb();
}

class CLineNumberArea final : public QWidget
{
public:
	inline CLineNumberArea(CPlainTextEditWithLineNumbers* editor) noexcept : QWidget(editor), codeEditor{ editor }
	{
	}

	inline QSize sizeHint() const override {
		return QSize{ codeEditor->lineNumberAreaWidth(), 0 };
	}

protected:
	inline void paintEvent(QPaintEvent* event) override {
		codeEditor->lineNumberAreaPaintEvent(event);
	}

private:
	CPlainTextEditWithLineNumbers* codeEditor = nullptr;
};

CPlainTextEditWithLineNumbers::CPlainTextEditWithLineNumbers(QWidget* parent) noexcept :
	QPlainTextEdit(parent)
{
	_lineNumberArea = new CLineNumberArea(this);

	// Neither connection binds to the document: setDocument() may replace it
	connect(this, &QPlainTextEdit::blockCountChanged, this, &CPlainTextEditWithLineNumbers::updateLineNumberAreaWidth);
	connect(this, &QPlainTextEdit::updateRequest, this, &CPlainTextEditWithLineNumbers::updateLineNumberArea);

	updateLineNumberAreaWidth();
}

int CPlainTextEditWithLineNumbers::lineNumberAreaWidth() const
{
	int digits = 1;
	int max = qMax(1, document()->blockCount());
	while (max >= 10)
	{
		max /= 10;
		++digits;
	}

	return LeftNumberMargin + RightNumberMargin + fontMetrics().horizontalAdvance(QLatin1Char('M')) * digits;
}

void CPlainTextEditWithLineNumbers::updateLineNumberAreaWidth()
{
	const int width = lineNumberAreaWidth();
	// setViewportMargins relayouts the scroll area, and this runs on every block count change
	if (width == _lineNumberAreaWidth)
		return;

	_lineNumberAreaWidth = width;
	setViewportMargins(width, 0, 0, 0);
	updateLineNumberAreaGeometry(); // setViewportMargins makes the margin but does not fill it
}

void CPlainTextEditWithLineNumbers::updateLineNumberAreaGeometry()
{
	const QRect cr = contentsRect();
	_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), _lineNumberAreaWidth, cr.height()));
}

void CPlainTextEditWithLineNumbers::updateLineNumberArea(const QRect& rect, int dy)
{
	if (dy != 0)
		_lineNumberArea->scroll(0, dy);
	else
		_lineNumberArea->update(0, rect.y(), _lineNumberArea->width(), rect.height());
}

void CPlainTextEditWithLineNumbers::resizeEvent(QResizeEvent* e)
{
	QPlainTextEdit::resizeEvent(e);

	updateLineNumberAreaGeometry();
}

void CPlainTextEditWithLineNumbers::changeEvent(QEvent* e)
{
	QPlainTextEdit::changeEvent(e);

	if (e->type() != QEvent::FontChange && e->type() != QEvent::ApplicationFontChange)
		return;

	// The number area's width follows the font, and nothing else recomputes it
	updateLineNumberAreaWidth();
	// The call above returns early when the digit count is unchanged, but the glyphs moved regardless
	_lineNumberArea->update();
}

void CPlainTextEditWithLineNumbers::lineNumberAreaPaintEvent(QPaintEvent* event)
{
	QPainter painter{ _lineNumberArea };
	painter.fillRect(event->rect(), bandColorFor(palette().color(QPalette::Base)));
	// Qt derives PlaceholderText from Text at 50% alpha where a palette leaves it unset, so any palette yields dim text here
	painter.setPen(palette().color(QPalette::PlaceholderText));

	const int numberHeight = fontMetrics().height();
	const int numberWidth = _lineNumberArea->width() - RightNumberMargin;

	QTextBlock block = firstVisibleBlock();
	// The number area shares the viewport's top and height, so the viewport's own offset places a block in both
	int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());

	for (; block.isValid() && top <= event->rect().bottom(); block = block.next())
	{
		const int bottom = top + qRound(blockBoundingRect(block).height()); // Zero-height for a hidden block, so it takes no room
		// Against the top of the block, so a wrapped block numbers its first line only
		if (block.isVisible() && bottom >= event->rect().top())
			painter.drawText(0, top, numberWidth, numberHeight, Qt::AlignRight, QString::number(block.blockNumber() + 1));

		top = bottom;
	}
}
