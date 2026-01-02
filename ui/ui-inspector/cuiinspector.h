#pragma once

#include <QtWidgets/QMainWindow>
#include <QtCore/QStringList>

#include <vector>

class QLayout;
class QTreeWidgetItem;

struct WidgetHierarchy;

class CUiInspector final : public QMainWindow
{
public:
	explicit CUiInspector(QWidget *parent = nullptr) noexcept;
	void setRootToInspect(QWidget* root);

private:
	void setupUi();

	void inspect();
	void visualize(const std::vector<WidgetHierarchy>& hierarchy);

	void inspectWidgetHierarchy(QWidget* widget, std::vector<WidgetHierarchy>& root) const;
	void inspectWidgetHierarchy(QLayout* layout, std::vector<WidgetHierarchy>& root) const;

	void showItemContextMenu(const QPoint& p);
	void highlightItem(QObject* item);
	void onItemSelected(QTreeWidgetItem* current, QTreeWidgetItem* previous);

private:
	QStringList _ignoredClasses;
	QWidget* _rootToInspect = nullptr;

	QWidget* _currentlyHighlighted = nullptr;
	QString _styleSheetToRestore;

	struct Ui;
	Ui* _ui = nullptr;
};
