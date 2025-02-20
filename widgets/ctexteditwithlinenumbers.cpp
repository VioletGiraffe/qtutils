#include "ctexteditwithlinenumbers.h"

DISABLE_COMPILER_WARNINGS
#include <QAbstractTextDocumentLayout>
#include <QPainter>
#include <QScrollBar>
#include <QTextBlock>
RESTORE_COMPILER_WARNINGS

// Space between the edge of the number and the right edge of the number area column
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

	connect(document(), &QTextDocument::blockCountChanged, this, &CTextEditWithLineNumbers::updateLineNumberAreaWidth);
	connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &CTextEditWithLineNumbers::updateLineNumberArea);
	connect(this, &CTextEditWithLineNumbers::textChanged, this, &CTextEditWithLineNumbers::updateLineNumberArea);
	connect(this, &CTextEditWithLineNumbers::cursorPositionChanged, this, &CTextEditWithLineNumbers::updateLineNumberArea);

	updateLineNumberAreaWidth(0);
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

	const int space = 3 + RightNumberMargin + fontMetrics().horizontalAdvance(QLatin1Char('M')) * digits;
	return space;
}

void CTextEditWithLineNumbers::updateLineNumberAreaWidth(int /* newBlockCount */)
{
	setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CTextEditWithLineNumbers::updateLineNumberArea()
{
	const QRect rect = contentsRect();
	_lineNumberArea->update(0, rect.y(), _lineNumberArea->width(), rect.height());
	updateLineNumberAreaWidth(0);
}

void CTextEditWithLineNumbers::resizeEvent(QResizeEvent* e)
{
	QTextEdit::resizeEvent(e);

	const QRect cr = contentsRect();
	_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

int CTextEditWithLineNumbers::getFirstVisibleBlockId()
{
	// Detect the first block for which bounding rect - once translated
	// in absolute coordinated - is contained by the editor's text area

	// Costly way of doing but since "blockBoundingGeometry(...)" doesn't
	// exists for "QTextEdit"...

	QTextCursor curs = QTextCursor(document());
	curs.movePosition(QTextCursor::Start);

	const QRect r1 = viewport()->geometry();

	for (int i = 0, n = document()->blockCount(); i < n; ++i)
	{
		QTextBlock block = curs.block();

		QRect r2 = document()->documentLayout()->blockBoundingRect(block).translated(
			r1.x(), r1.y() - verticalScrollBar()->sliderPosition()
		).toRect();

		if (r1.contains(r2, true))
			return i;

		curs.movePosition(QTextCursor::NextBlock);
	}

	return 0;
}

void CTextEditWithLineNumbers::lineNumberAreaPaintEvent(QPaintEvent* event)
{
	//verticalScrollBar()->setSliderPosition(verticalScrollBar()->sliderPosition());

	QPainter painter{ _lineNumberArea };
	auto baseColor = palette().color(QPalette::Base);
	const bool darkTheme = baseColor.lightness() < 128;
	baseColor = darkTheme ? baseColor.lighter(250) : baseColor.darker(113);

	painter.fillRect(event->rect(), baseColor);
	painter.setPen(!darkTheme ? baseColor.darker(250) : baseColor.lighter(200));

	int blockNumber = getFirstVisibleBlockId();

	QTextBlock block = document()->findBlockByNumber(blockNumber);
	QTextBlock prev_block = (blockNumber > 0) ? document()->findBlockByNumber(blockNumber - 1) : block;
	const int translate_y = (blockNumber > 0) ? (-verticalScrollBar()->sliderPosition()) : 0;

	int top = viewport()->geometry().top();

	// Adjust text position according to the previous "non entirely visible" block
	// if applicable. Also takes in consideration the document's margin offset.
	int additional_margin = 0;
	if (blockNumber == 0)
		// Simply adjust to document's margin
		additional_margin = (int)document()->documentMargin() - 1 - verticalScrollBar()->sliderPosition();
	else
		// Getting the height of the visible part of the previous "non entirely visible" block
		additional_margin = (int)document()->documentLayout()->blockBoundingRect(prev_block).translated(0, translate_y).intersected(viewport()->geometry()).height();

	// Shift the starting point
	top += additional_margin;

	const int fontHeight = fontMetrics().height();

	int bottom = top + (int)document()->documentLayout()->blockBoundingRect(block).height();

	// Draw the numbers (displaying the current line number in green)
	while (block.isValid() && top <= event->rect().bottom())
	{
		if (block.isVisible() && bottom >= event->rect().top())
		{
			const QString number = QString::number(blockNumber + 1);
			painter.drawText(-5, top, _lineNumberArea->width(), fontHeight, Qt::AlignRight, number);
		}

		block = block.next();
		top = bottom;
		bottom = top + (int)document()->documentLayout()->blockBoundingRect(block).height();
		++blockNumber;
	}
}
