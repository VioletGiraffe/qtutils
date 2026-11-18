#include "cmenubar.h"

DISABLE_COMPILER_WARNINGS
#include <QAction>
#include <QFontMetricsF>
#include <QPainter>
#include <QString>
#include <QStyle>
#include <QStyleOptionMenuItem>
RESTORE_COMPILER_WARNINGS

#include <algorithm>
#include <iterator>
#include <utility>

namespace {

// A label without its '&' markers, and the index of its mnemonic letter in that text; -1 for none. "&&" is a literal '&'.
[[nodiscard]] std::pair<QString, qsizetype> labelAndMnemonic(const QString& text)
{
	QString label;
	label.reserve(text.size());
	qsizetype mnemonic = -1;
	for (qsizetype i = 0; i < text.size(); ++i)
	{
		if (text[i] == QLatin1Char('&') && i + 1 < text.size())
		{
			++i;
			if (text[i] != QLatin1Char('&') && mnemonic < 0)
				mnemonic = label.size();
		}
		label += text[i];
	}
	return { label, mnemonic };
}

} // namespace

void CMenuBar::setLabelColors(const std::optional<LabelColors>& colors)
{
	_labelColors = colors;
	update();
}

void CMenuBar::initStyleOption(QStyleOptionMenuItem* option, const QAction* action) const
{
	QMenuBar::initStyleOption(option, action);
	const bool highlightedOrDisabled = !option->state.testFlag(QStyle::State_Enabled) || (option->state & (QStyle::State_Selected | QStyle::State_Sunken));
	// An item with an icon shows only the icon
	if (!_paintingItems || highlightedOrDisabled || option->text.isEmpty() || !option->icon.isNull())
		return;

	_ownLabels.push_back({ .action = action, .text = option->text });
	option->text.clear();
}

void CMenuBar::paintEvent(QPaintEvent* event)
{
	if (!_labelColors)
	{
		QMenuBar::paintEvent(event);
		return;
	}

	// Brings the items' geometry up to date before QMenuBar paints: computing it calls initStyleOption() for every item, the hidden ones too
	if (!actions().isEmpty())
		actionGeometry(actions().constFirst());

	_ownLabels.clear();
	_paintingItems = true;
	QMenuBar::paintEvent(event);
	_paintingItems = false;

	QPainter painter{ this };
	painter.setFont(font());
	const QFontMetricsF metrics{ font() };
	// Through actions(): actionGeometry() takes a non-const action
	for (QAction* action : actions())
	{
		const auto ownLabel = std::ranges::find(_ownLabels, action, &OwnLabel::action);
		if (ownLabel == _ownLabels.end())
			continue;

		const QRect itemRect = actionGeometry(action);
		const auto [label, mnemonic] = labelAndMnemonic(ownLabel->text);
		qreal x = itemRect.left() + (itemRect.width() - metrics.horizontalAdvance(label)) / 2.0;
		const qreal baseline = itemRect.top() + (itemRect.height() - metrics.height()) / 2.0 + metrics.ascent();

		// Three runs: before the mnemonic, the mnemonic, after it
		const qsizetype runEnds[] = { mnemonic < 0 ? label.size() : mnemonic, mnemonic < 0 ? label.size() : mnemonic + 1, label.size() };
		qsizetype runStart = 0;
		for (size_t run = 0; run < std::size(runEnds); ++run)
		{
			const QString runText = label.sliced(runStart, runEnds[run] - runStart);
			runStart = runEnds[run];
			if (runText.isEmpty())
				continue;

			painter.setPen(run == 1 ? _labelColors->mnemonic : _labelColors->text);
			painter.drawText(QPointF{ x, baseline }, runText);
			x += metrics.horizontalAdvance(runText);
		}
	}
}
