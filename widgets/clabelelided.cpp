#include "clabelelided.h"

DISABLE_COMPILER_WARNINGS
#include <QEvent>
#include <QFontMetrics>
#include <QHelpEvent>
#include <QPainter>
#include <QStyle>
#include <QTextLayout>
#include <QTextOption>
#include <QToolTip>
RESTORE_COMPILER_WARNINGS

#include <algorithm>
#include <vector>

namespace {

struct TextLine
{
	int start = 0; // Index into the label's text
	int length = 0;
};

// Always yields at least one line, so an empty paragraph keeps its height
std::vector<TextLine> splitIntoParagraphs(const QString& text)
{
	std::vector<TextLine> paragraphs;
	for (int start = 0;;)
	{
		const int end = (int)text.indexOf(QChar::LineFeed, start);
		if (end < 0)
		{
			paragraphs.push_back({ start, (int)text.size() - start });
			return paragraphs;
		}

		paragraphs.push_back({ start, end - start });
		start = end + 1;
	}
}

// A QTextLayout covers a single paragraph, so the newlines are split off before the wrapping rather than by it.
std::vector<TextLine> wrapIntoLines(const QString& text, const QFont& font, int width)
{
	std::vector<TextLine> lines;

	for (const TextLine& paragraph : splitIntoParagraphs(text))
	{
		QTextLayout layout(text.mid(paragraph.start, paragraph.length), font);
		QTextOption textOption = layout.textOption();
		textOption.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere); // What QLabel does
		layout.setTextOption(textOption);

		layout.beginLayout();
		for (QTextLine line = layout.createLine(); line.isValid(); line = layout.createLine())
		{
			line.setLineWidth(width); // Decides where this line breaks, so it must precede reading its extents
			lines.push_back({ paragraph.start + line.textStart(), line.textLength() });
		}
		layout.endLayout();
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

void CLabelElided::setMinimumTextSample(const QString& text)
{
	if (text == _minimumTextSample)
		return;

	_minimumTextSample = text;
	updateGeometry();
}

QString CLabelElided::minimumTextSample() const
{
	return _minimumTextSample;
}

QSize CLabelElided::minimumSizeHint() const
{
	if (_elideMode == Qt::ElideNone)
		return QLabel::minimumSizeHint();

	constexpr QChar ellipsis{0x2026}; // What QFontMetrics::elidedText substitutes
	const QFontMetrics fm(font());
	const int textWidth = fm.horizontalAdvance(_minimumTextSample.isEmpty() ? QString{ ellipsis } : _minimumTextSample);
	const int widthUnavailableForText = width() - textRect().width();
	return { textWidth + widthUnavailableForText, QLabel::minimumSizeHint().height() };
}

bool CLabelElided::hasHeightForWidth() const
{
	return false; // Wrapped text elides into the height it's given instead of demanding more of it
}

int CLabelElided::heightForWidth(int) const
{
	return -1; // Must agree with hasHeightForWidth()
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

	paintText(painter, textRect());
}

void CLabelElided::paintText(QPainter& painter, const QRect& textArea)
{
	const QString fullText = text();
	const QFontMetrics fm(font());
	const int lineHeight = fm.lineSpacing();
	if (textArea.width() <= 0 || lineHeight <= 0)
	{
		_textIsTruncated = !fullText.isEmpty();
		return;
	}

	// Whole lines only, except that a rect too short for even one line still gets that line rather than nothing
	const int visibleLineCount = std::max(1, textArea.height() / lineHeight);

	const std::vector<TextLine> lines = wordWrap() ? wrapIntoLines(fullText, font(), textArea.width()) : splitIntoParagraphs(fullText);
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
		if (!_textIsTruncated)
			_textIsTruncated = fm.horizontalAdvance(lineText) > textArea.width(); // Also covers ElideNone, which elides nothing

		const QRect lineRect{ textArea.left(), y, textArea.width(), lineHeight };
		const QString elidedText = fm.elidedText(lineText, _elideMode, textArea.width(), flags & Qt::TextShowMnemonic);
		style()->drawItemText(&painter, lineRect, flags, palette(), isEnabled(), elidedText, foregroundRole());
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
		drawLine(seam);
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
