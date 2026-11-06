#pragma once

#include "findresult.h"

DISABLE_COMPILER_WARNINGS
#include <QListView>
RESTORE_COMPILER_WARNINGS

#include <functional>
#include <vector>

// A find bar's matches in a popup list, numbered from 1; a row shows the match's location and an excerpt with the match in bold.
//   A row's match is selected in the host when the row becomes current or is clicked; the list stays open for browsing.
//   Enter and double-click also close the list; Esc and a click outside it close it. Typing a number jumps to that row.
//   Deletes itself once closed.
class CFindMatchesPopup final : public QListView
{
public:
	using DescribeMatch = std::function<MatchDescription (MatchRange match)>;
	using SelectMatch = std::function<void (int64_t index, MatchRange match)>;

	// Requires at least one match. currentIndex: the row to start on, which is not selected in the host again; -1 for none.
	CFindMatchesPopup(std::vector<MatchRange> matches, int64_t currentIndex, DescribeMatch describeMatch, SelectMatch selectMatch, QWidget* parent);

	// Shows the list in 'globalArea' against its 'anchorEdge', Qt::TopEdge or Qt::BottomEdge, as tall as the rows need up to the area's height
	void popUp(const QRect& globalArea, Qt::Edge anchorEdge);

protected:
	void keyPressEvent(QKeyEvent* event) override;

private:
	std::vector<MatchRange> _matches;
	const DescribeMatch _describeMatch;
	const SelectMatch _selectMatch;
};
