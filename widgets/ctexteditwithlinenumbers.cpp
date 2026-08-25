#include "ctexteditwithlinenumbers.h"

#include "assert/advanced_assert.h"

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

// QTextEdit scrolls by pixels, so the scrollbar value is the document y the viewport starts at
qreal CTextEditWithLineNumbers::documentYAtViewportTop() const
{
	return verticalScrollBar()->value();
}

// hitTest() lays the document out only as far as the y it is asked about, which painting the viewport
// requires regardless. blockBoundingRect() lays out as far as the block it is handed, so a search by block
// index lays out however deep its probes reach.
QTextBlock CTextEditWithLineNumbers::firstVisibleBlock() const
{
	const QAbstractTextDocumentLayout* layout = document()->documentLayout();
	// The probe must land inside a block: hitTest() walks from the first block and stops only at a block containing y.
	// A block below the lazily laid out region has an empty QTextLayout at y 0, so a probe at y 0 stops on that one.
	// At scroll position 0 the viewport top is above the first block: the document's top margin belongs to no block.
	const qreal y = qMax(documentYAtViewportTop(), layout->blockBoundingRect(document()->firstBlock()).top());
	const int position = layout->hitTest(QPointF{ 0.0, y }, Qt::FuzzyHit);
	assert_r(position >= 0); // Qt::FuzzyHit always resolves to a position
	return document()->findBlock(position);
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
	// The number area shares the viewport's top and height, so the same shift places a block in both
	const qreal offsetY = -documentYAtViewportTop();
	const int numberHeight = fontMetrics().height();
	const int numberWidth = _lineNumberArea->width() - RightNumberMargin;

	for (QTextBlock block = firstVisibleBlock(); block.isValid(); block = block.next())
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
