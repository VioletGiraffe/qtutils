#include "ctexteditwithlinenumbers.h"

#include <QPainter>
#include <QTextBlock>

// Space between the edge of the number and the right edge of the number area column
static constexpr int RightNumberMargin = 4;

class CLineNumberArea final : public QWidget
{
public:
	CLineNumberArea(CTextEditWithLineNumbers *editor) :
		QWidget(editor), _codeEditor(editor)
	{}

	QSize sizeHint() const override
	{
		return QSize(_codeEditor->lineNumberAreaWidth(), 0);
	}

protected:
	void paintEvent(QPaintEvent *event) override
	{
		_codeEditor->lineNumberAreaPaintEvent(event);
	}

private:
	CTextEditWithLineNumbers *_codeEditor;
};

CTextEditWithLineNumbers::CTextEditWithLineNumbers(QWidget *parent) :
	QPlainTextEdit(parent)
{
	_lineNumberArea = new CLineNumberArea(this);

	connect(this, &CTextEditWithLineNumbers::blockCountChanged, this, &CTextEditWithLineNumbers::updateLineNumberAreaWidth);
	connect(this, &CTextEditWithLineNumbers::updateRequest, this, &CTextEditWithLineNumbers::updateLineNumberArea);

	updateLineNumberAreaWidth(0);
}

int CTextEditWithLineNumbers::lineNumberAreaWidth()
{
	int digits = 1;
	int max = qMax(1, blockCount());
	while (max >= 10) {
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

void CTextEditWithLineNumbers::updateLineNumberArea(const QRect &rect, int dy)
{
	if (dy)
		_lineNumberArea->scroll(0, dy);
	else
		_lineNumberArea->update(0, rect.y(), _lineNumberArea->width(), rect.height());

	if (rect.contains(viewport()->rect()))
		updateLineNumberAreaWidth(0);
}

void CTextEditWithLineNumbers::resizeEvent(QResizeEvent *e)
{
	QPlainTextEdit::resizeEvent(e);

	QRect cr = contentsRect();
	_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CTextEditWithLineNumbers::lineNumberAreaPaintEvent(QPaintEvent *event)
{
	QPainter painter(_lineNumberArea);
	painter.fillRect(event->rect(), QColor(127, 127, 127, 20));
	painter.setPen(QColor(100, 100, 100));

	QTextBlock block = firstVisibleBlock();
	int blockNumber = block.blockNumber();
	int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
	int bottom = top + qRound(blockBoundingRect(block).height());

	while (block.isValid() && top <= event->rect().bottom())
	{
		if (block.isVisible() && bottom >= event->rect().top())
		{
			const QString number = QString::number(blockNumber + 1);
			painter.drawText(0, top,
							 _lineNumberArea->width() - RightNumberMargin, fontMetrics().height(),
							 Qt::AlignRight,
							 number);
		}

		block = block.next();
		top = bottom;
		bottom = top + qRound(blockBoundingRect(block).height());
		++blockNumber;
	}
}
