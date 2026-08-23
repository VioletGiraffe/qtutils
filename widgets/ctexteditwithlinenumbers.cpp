#include "ctexteditwithlinenumbers.h"

DISABLE_COMPILER_WARNINGS
#include <QAbstractTextDocumentLayout>
#include <QEvent>
#include <QPainter>
#include <QScrollBar>
#include <QTextBlock>
RESTORE_COMPILER_WARNINGS

// Padding on either side of the number within the number area
static constexpr int LeftNumberMargin = 3;
static constexpr int RightNumberMargin = 4;

class CLineNumberArea final : public QWidget
{
public:
	inline CLineNumberArea(CTextEditWithLineNumbers* editor) noexcept : QWidget(editor), codeEditor{ editor }
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
	CTextEditWithLineNumbers* codeEditor = nullptr;
};

CTextEditWithLineNumbers::CTextEditWithLineNumbers(QWidget* parent) noexcept :
	QTextEdit(parent)
{
	_lineNumberArea = new CLineNumberArea(this);

	// No connection binds to the document: setDocument() may replace it.
	// textChanged is QTextEdit's own signal, and it covers every block count change.
	connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &CTextEditWithLineNumbers::updateLineNumberArea);
	connect(this, &CTextEditWithLineNumbers::textChanged, this, &CTextEditWithLineNumbers::updateLineNumberArea);

	updateLineNumberAreaWidth();
}

int CTextEditWithLineNumbers::lineNumberAreaWidth() const
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

void CTextEditWithLineNumbers::updateLineNumberAreaWidth()
{
	const int width = lineNumberAreaWidth();
	// setViewportMargins relayouts the scroll area, and this runs on every text change
	if (width == _lineNumberAreaWidth)
		return;

	_lineNumberAreaWidth = width;
	setViewportMargins(width, 0, 0, 0);
	updateLineNumberAreaGeometry(); // setViewportMargins makes the margin but does not fill it
}

void CTextEditWithLineNumbers::updateLineNumberAreaGeometry()
{
	const QRect cr = contentsRect();
	_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), _lineNumberAreaWidth, cr.height()));
}

void CTextEditWithLineNumbers::updateLineNumberArea()
{
	_lineNumberArea->update();
	updateLineNumberAreaWidth();
}

void CTextEditWithLineNumbers::resizeEvent(QResizeEvent* e)
{
	QTextEdit::resizeEvent(e);

	updateLineNumberAreaGeometry();
}

void CTextEditWithLineNumbers::changeEvent(QEvent* e)
{
	QTextEdit::changeEvent(e);

	// QTextEdit gives the document the new font on either event, so the blocks have moved by the time this runs.
	// The number area's width follows the font too, and nothing else recomputes it.
	if (e->type() == QEvent::FontChange || e->type() == QEvent::ApplicationFontChange)
		updateLineNumberArea();
}

// Blocks of a laid-out document are in non-decreasing vertical order, so bisection finds this one without
// walking them.
// blockBoundingRect() lays the document out as far as the block it is asked about, so any block's position
// can be had. QPlainTextEdit lays out only what is shown and answers with firstVisibleBlock() instead.
int CTextEditWithLineNumbers::firstVisibleBlockNumber() const
{
	const QAbstractTextDocumentLayout* layout = document()->documentLayout();
	const qreal scrollY = verticalScrollBar()->value();

	int low = 0, high = document()->blockCount() - 1;
	while (low < high)
	{
		const int middle = low + (high - low) / 2;
		if (layout->blockBoundingRect(document()->findBlockByNumber(middle)).bottom() > scrollY)
			high = middle;
		else
			low = middle + 1;
	}

	return low;
}

void CTextEditWithLineNumbers::lineNumberAreaPaintEvent(QPaintEvent* event)
{
	QPainter painter{ _lineNumberArea };
	auto baseColor = palette().color(QPalette::Base);
	const bool darkTheme = baseColor.lightness() < 128;
	baseColor = darkTheme ? baseColor.lighter(250) : baseColor.darker(113);

	painter.fillRect(event->rect(), baseColor);
	painter.setPen(!darkTheme ? baseColor.darker(250) : baseColor.lighter(200));

	const QAbstractTextDocumentLayout* layout = document()->documentLayout();
	// The viewport shows the document shifted up by the scroll position.
	// The number area shares the viewport's top and height, so the same shift places a block in both.
	const qreal offsetY = -qreal(verticalScrollBar()->value());
	const int numberHeight = fontMetrics().height();
	const int numberWidth = _lineNumberArea->width() - RightNumberMargin;

	for (QTextBlock block = document()->findBlockByNumber(firstVisibleBlockNumber()); block.isValid(); block = block.next())
	{
		// Before the rect is read: blockBoundingRect() answers a null one for a hidden block
		if (!block.isVisible())
			continue;

		const QRectF blockRect = layout->blockBoundingRect(block).translated(0.0, offsetY);
		if (blockRect.top() > event->rect().bottom())
			break;

		// Against the top of the block, so a wrapped block numbers its first line only
		if (blockRect.bottom() >= event->rect().top())
			painter.drawText(0, qRound(blockRect.top()), numberWidth, numberHeight, Qt::AlignRight, QString::number(block.blockNumber() + 1));
	}
}
