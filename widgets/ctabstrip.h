#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QPoint>
#include <QRect>
#include <QString>
#include <QWidget>
RESTORE_COMPILER_WARNINGS

#include <vector>

class CFlowLayout;
class QFrame;

// Tabs in rows that wrap to the strip's width, in a fixed order. Indexed as in QTabBar.
// Each tab is a QPushButton child, checked while current, with a QPushButton inside it that closes it.
// Pressing a tab makes it current; dragging it moves it, and a QFrame child marks where it lands.
// A middle click on a tab requests its closing.
// A context menu or tooltip event on a tab goes on to the strip, in the strip's coordinates.
class CTabStrip final : public QWidget
{
	Q_OBJECT

public:
	explicit CTabStrip(QWidget* parent = nullptr);

	// The first tab added becomes current
	int addTab(const QString& text);
	// A removed current tab passes current to the tab now at its index, or to the last tab
	void removeTab(int index);
	[[nodiscard]] int count() const;

	// -1 while there are no tabs
	[[nodiscard]] int currentIndex() const;
	void setCurrentIndex(int index);

	void setTabText(int index, const QString& text);
	void setTabData(int index, qulonglong value);
	[[nodiscard]] qulonglong tabData(int index) const;

	// -1 where no tab is
	[[nodiscard]] int tabAt(QPoint pos) const;

	// Between tabs, across and down
	void setSpacing(int spacing);

signals:
	// Not emitted when a removal or a move only shifts the current tab's index
	void currentChanged(int index);
	void tabCloseRequested(int index);
	// `to` is the index after the move
	void tabMoved(int from, int to);

private:
	class Tab;

	// slot: the index a dropped tab is inserted before; count() is after the last tab
	struct DropTarget {
		int slot = 0;
		QRect marker;
	};
	// Requires a tab
	[[nodiscard]] DropTarget dropTargetAt(QPoint pos) const;
	[[nodiscard]] int indexOf(const Tab& tab) const;

	// For Tab; positions are in the strip's coordinates
	void tabPressed(Tab& tab, QPoint pos);
	void tabDragged(QPoint pos);
	void tabReleased(QPoint pos);
	void requestTabClosing(const Tab& tab);
	void endDrag();

private:
	CFlowLayout* _layout = nullptr;
	QFrame* _dropMarker = nullptr;
	std::vector<Tab*> _tabs;
	Tab* _currentTab = nullptr;
	// The tab under a held left button, and where it was pressed
	Tab* _pressedTab = nullptr;
	QPoint _pressPos;
	bool _dragging = false;
};
