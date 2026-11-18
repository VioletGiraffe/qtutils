#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QColor>
#include <QMenuBar>
#include <QString>
RESTORE_COMPILER_WARNINGS

#include <optional>
#include <vector>

class QStyleOptionMenuItem;

// A menu bar that can draw each item's mnemonic letter in a colour of its own, as a text-mode menu bar does.
// The style draws every item's background, so the stylesheet's item rules apply as to a QMenuBar.
class CMenuBar final : public QMenuBar
{
public:
	using QMenuBar::QMenuBar;

	struct LabelColors
	{
		QColor text, mnemonic;
	};
	// The labels of the items neither highlighted nor disabled are drawn here: the stylesheet's text colour then does not apply to them.
	// Empty: every label is drawn by the style.
	// Requires even horizontal padding on the items: the label is centred in the item's rect, as the style centres it in the padded one.
	void setLabelColors(const std::optional<LabelColors>& colors);

protected:
	void paintEvent(QPaintEvent* event) override;
	void initStyleOption(QStyleOptionMenuItem* option, const QAction* action) const override;

private:
	struct OwnLabel
	{
		const QAction* action = nullptr;
		QString text; // with its '&' markers
	};

private:
	std::optional<LabelColors> _labelColors;
	// Set while QMenuBar paints the items: the style is given no text for the labels drawn here
	bool _paintingItems = false;
	// The labels to draw, of the items QMenuBar painted: those it moved to its extension menu are not among them.
	// Filled by initStyleOption(), which QMenuBar calls for each item it paints.
	mutable std::vector<OwnLabel> _ownLabels;
};
