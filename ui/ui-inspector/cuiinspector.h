#pragma once
#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QMainWindow>
#include <QStringList>
RESTORE_COMPILER_WARNINGS

#include <vector>

class QTreeWidget;
class QAction;
class QLayout;

struct WidgetHierarchy;

class CUiInspector final : public QMainWindow
{
public:
	explicit CUiInspector(QWidget *parent = nullptr) noexcept;

private:
	void inspect();
	void visualize(const std::vector<WidgetHierarchy>& hierarchy);

	void inspectWidgetHierarchy(QWidget* widget, std::vector<WidgetHierarchy>& root) const;
	void inspectWidgetHierarchy(QLayout* layout, std::vector<WidgetHierarchy>& root) const;

	void showItemContextMenu(const QPoint& p);

private:
	QStringList _ignoredClasses;

	QTreeWidget* _tree;
	QAction* _actShowHiddenItems;
};
