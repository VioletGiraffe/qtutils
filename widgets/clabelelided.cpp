#include "clabelelided.h"

#include <QEvent>
#include <QFontMetrics>
#include <QHelpEvent>
#include <QPainter>
#include <QStringList>
#include <QStyle>
#include <QTextLayout>
#include <QTextOption>
#include <QToolTip>

#include <algorithm>
#include <vector>

namespace {

struct TextLine
{
	int start = 0; // Index into the label's text
	int length = 0;
};

// A QTextLayout covers a single paragraph, so the newlines are split here rather than left to the wrapping.
std::vector<TextLine> wrapIntoLines(const QString& text, const QFont& font, int width)
{
	std::vector<TextLine> lines;

	const QStringList paragraphs = text.split(QChar::LineFeed);
	int paragraphStart = 0;
	for (const QString& paragraph : paragraphs)
	{
		QTextLayout layout(paragraph, font);
		QTextOption textOption = layout.textOption();
		textOption.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere); // What QLabel does
		layout.setTextOption(textOption);

		layout.beginLayout();
		for (QTextLine line = layout.createLine(); line.isValid(); line = layout.createLine())
		{
			line.setLineWidth(width); // Decides where this line breaks, so it must precede reading its extents
			lines.push_back({ paragraphStart + line.textStart(), line.textLength() });
		}
		layout.endLayout();

		paragraphStart += (int)paragraph.size() + 1; // The +1 is the separator that split() consumed
	}

	return lines;
}

}

void CLabelElided::setElideMode(Qt::TextElideMode mode)
{
	if (mode == _elideMode)
		return;

	_elideMode = mode;
	updateGeometry(); // minimumSizeHint() depends on the mode
	update();
}

Qt::TextElideMode CLabelElided::elideMode() const
{
	return _elideMode;
}

QSize CLabelElided::minimumSizeHint() const
{
	if (_elideMode == Qt::ElideNone)
		return QLabel::minimumSizeHint();

	constexpr QChar ellipsis{0x2026}; // What QFontMetrics::elidedText substitutes
	const QFontMetrics fm(font());
	const int widthUnavailableForText = width() - textRect().width();
	return { fm.horizontalAdvance(ellipsis) + widthUnavailableForText, QLabel::minimumSizeHint().height() };
}

bool CLabelElided::event(QEvent* e)
{
	// Show automatic tooltip if no custom tooltip is set
	if (e->type() == QEvent::ToolTip && toolTip().isEmpty())
	{
		if (_textIsTruncated)
			QToolTip::showText(static_cast<QHelpEvent*>(e)->globalPos(), text());
		else
			QToolTip::hideText();

		return true;
	}

	return QLabel::event(e);
}

void CLabelElided::paintEvent(QPaintEvent*)
{
	QPainter painter(this);
	drawFrame(&painter);
	painter.setFont(font());

	const QRect textArea = textRect();
	if (wordWrap())
		paintWrapped(painter, textArea);
	else
		paintSingleLine(painter, textArea);
}

void CLabelElided::paintSingleLine(QPainter& painter, const QRect& textArea)
{
	const QString fullText = text();
	const QFontMetrics fm(font());
	const int flags = textFlags();

	_textIsTruncated = fm.horizontalAdvance(fullText) > textArea.width();
	const QString elidedText = fm.elidedText(fullText, _elideMode, textArea.width(), flags & Qt::TextShowMnemonic);
	style()->drawItemText(&painter, textArea, flags, palette(), isEnabled(), elidedText, foregroundRole());
}

void CLabelElided::paintWrapped(QPainter& painter, const QRect& textArea)
{
	const QString fullText = text();
	const QFontMetrics fm(font());
	const int lineHeight = fm.lineSpacing();
	const int visibleLineCount = lineHeight > 0 ? textArea.height() / lineHeight : 0; // Whole lines only
	if (visibleLineCount <= 0 || textArea.width() <= 0)
	{
		_textIsTruncated = !fullText.isEmpty();
		return;
	}

	const std::vector<TextLine> lines = wrapIntoLines(fullText, font(), textArea.width());
	const int lineCount = (int)lines.size();

	_textIsTruncated = lineCount > visibleLineCount;
	const bool elide = _textIsTruncated && _elideMode != Qt::ElideNone;

	int headLineCount = std::min(lineCount, visibleLineCount);
	int tailLineCount = 0;
	if (elide)
	{
		// The ellipsis takes a line of its own, with as much of the text dropped around it as fits on either side
		switch (_elideMode)
		{
		case Qt::ElideRight:
			headLineCount = visibleLineCount - 1;
			break;
		case Qt::ElideLeft:
			headLineCount = 0;
			tailLineCount = visibleLineCount - 1;
			break;
		default: // ElideMiddle
			headLineCount = visibleLineCount / 2;
			tailLineCount = visibleLineCount - 1 - headLineCount;
			break;
		}
	}

	int y = textArea.top();
	const int flags = textFlags();
	const int blockHeight = (headLineCount + tailLineCount + (elide ? 1 : 0)) * lineHeight;
	if (flags & Qt::AlignVCenter)
		y += (textArea.height() - blockHeight) / 2;
	else if (flags & Qt::AlignBottom)
		y += textArea.height() - blockHeight;

	const auto drawLine = [&](const QString& lineText) {
		const QRect lineRect{ textArea.left(), y, textArea.width(), lineHeight };
		style()->drawItemText(&painter, lineRect, flags, palette(), isEnabled(), lineText, foregroundRole());
		y += lineHeight;
	};

	for (int i = 0; i < headLineCount; ++i)
		drawLine(fullText.mid(lines[i].start, lines[i].length));

	if (elide)
	{
		const int seamStart = lines[headLineCount].start;
		const int seamEnd = tailLineCount > 0 ? lines[lineCount - tailLineCount].start : (int)fullText.size();
		QString seam = fullText.mid(seamStart, seamEnd - seamStart);
		seam.replace(QChar::LineFeed, QChar::Space); // The seam is one line, but the text it spans may not be
		drawLine(fm.elidedText(seam, _elideMode, textArea.width(), flags & Qt::TextShowMnemonic));
	}

	for (int i = lineCount - tailLineCount; i < lineCount; ++i)
		drawLine(fullText.mid(lines[i].start, lines[i].length));
}

QRect CLabelElided::textRect() const
{
	const int m = margin();
	QRect r = contentsRect().adjusted(m, m, -m, -m);

	int textIndent = indent();
	if (textIndent < 0 && frameWidth() != 0) // The implicit indent QLabel applies to framed labels
		textIndent = fontMetrics().horizontalAdvance(QChar{u'x'}) / 2 - m;

	if (textIndent > 0)
	{
		// indent() only applies to the edge the text is aligned to
		const Qt::Alignment align = visualAlignment();
		if (align & Qt::AlignLeft)
			r.setLeft(r.left() + textIndent);
		if (align & Qt::AlignRight)
			r.setRight(r.right() - textIndent);
		if (align & Qt::AlignTop)
			r.setTop(r.top() + textIndent);
		if (align & Qt::AlignBottom)
			r.setBottom(r.bottom() - textIndent);
	}

	return r;
}

Qt::Alignment CLabelElided::visualAlignment() const
{
	return QStyle::visualAlignment(layoutDirection(), alignment());
}

int CLabelElided::textFlags() const
{
	int flags = (int)visualAlignment();
	if (buddy()) // QLabel only reads '&' as a mnemonic marker when there's a buddy for it to activate
	{
		flags |= Qt::TextShowMnemonic;
		if (!style()->styleHint(QStyle::SH_UnderlineShortcut, nullptr, this))
			flags |= Qt::TextHideMnemonic;
	}

	return flags;
}
