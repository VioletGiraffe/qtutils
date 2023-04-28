#include "cuiinspector.h"

DISABLE_COMPILER_WARNINGS
#include <QAction>
#include <QApplication>
#include <QDebug>
#include <QLayout>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QTimer>
#include <QTreeWidget>
RESTORE_COMPILER_WARNINGS


struct WidgetHierarchy {
	QWidget* widget = nullptr;
	QLayout* layout = nullptr;

	std::vector<WidgetHierarchy> children;
};

CUiInspector::CUiInspector(QWidget* parent) :
	QMainWindow(parent)
{
	QTimer::singleShot(500, this, &CUiInspector::inspect);

	setCentralWidget(_tree = new QTreeWidget);
	_tree->setSelectionMode(QAbstractItemView::NoSelection);
	_tree->setHeaderHidden(true);
	_tree->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(_tree, &QWidget::customContextMenuRequested, this, &CUiInspector::showItemContextMenu);

	QMenuBar* mainMenu = menuBar();
	mainMenu->addAction(tr("&Refresh"), this, &CUiInspector::inspect);
	_actShowHiddenItems = mainMenu->addAction(tr("&Show hidden items"), this, &CUiInspector::inspect);
	_actShowHiddenItems->setCheckable(true);
	_actShowHiddenItems->setChecked(true);

	mainMenu->show();
	statusBar()->show();

	connect(qApp, &QApplication::focusChanged, this, [this](QWidget* old, QWidget* n) {
		QString text;
		{
			QDebug infoWriter(&text);
			infoWriter << "Focus changed from" << old << "to" << n;
		}
		statusBar()->showMessage(text);
	});
}

void CUiInspector::inspect()
{
	std::vector<WidgetHierarchy> hierarchy;
	for (QWidget* widget : QApplication::topLevelWidgets())
	{
		if (widget != this) // Do not show self
			inspectWidgetHierarchy(widget, hierarchy);
	}

	visualize(hierarchy);
}

QTreeWidgetItem* createTreeItem(const WidgetHierarchy& hierarchy, const bool showHidden, QTreeWidgetItem* parent = nullptr)
{
	const bool isHiddenWidget = hierarchy.widget && !hierarchy.widget->isVisible();
	if (showHidden && isHiddenWidget) // Hidden widgets contain no visible children - do not iterate further
		return nullptr;

	auto* item = new QTreeWidgetItem(parent);

	QString description;

	// Scope to ensure destruction of the QDebug object
	{
		QDebug detailedInfoWriter(&description);
		if (hierarchy.widget)
		{
			item->setData(0, Qt::UserRole, (qulonglong)hierarchy.widget);
			detailedInfoWriter << hierarchy.widget;
			if (isHiddenWidget)
				detailedInfoWriter << "- HIDDEN";

			if (hierarchy.widget->testAttribute(Qt::WA_TransparentForMouseEvents))
				detailedInfoWriter << "(transparent for mouse events!)";
		}
		else if (hierarchy.layout)
		{
			item->setData(0, Qt::UserRole, (qulonglong)hierarchy.layout);
			detailedInfoWriter << hierarchy.layout;
		}
		else
			assert(!"Both widget and layout are nullptr");
	}

	item->setText(0, description);
	if (hierarchy.layout)
		item->setForeground(0, QColor(100, 255, 100));

	for (const auto& child : hierarchy.children)
		createTreeItem(child, showHidden, item);

	return item;
}

void CUiInspector::visualize(const std::vector<WidgetHierarchy>& hierarchy)
{
	QList<QTreeWidgetItem*> items;
	items.reserve((int)hierarchy.size());
	const bool showHidden = _actShowHiddenItems->isChecked();
	for (const auto& h : hierarchy)
	{
		auto* item = createTreeItem(h, showHidden);
		if (item)
			items.push_back(item);
	}

	_tree->clear();
	_tree->addTopLevelItems(items);
	_tree->expandAll();
}

void CUiInspector::inspectWidgetHierarchy(QWidget* widget, std::vector<struct WidgetHierarchy>& root) const
{
	if (_ignoredClasses.contains(widget->objectName()) || _ignoredClasses.contains(widget->metaObject()->className()))
		return;

	root.emplace_back();
	WidgetHierarchy& thisItem = root.back();
	thisItem.widget = widget;
	if (widget->layout())
		inspectWidgetHierarchy(widget->layout(), thisItem.children);
	else
	{
		for (QObject* child : widget->children())
		{
			if (auto* childWidget = dynamic_cast<QWidget*>(child))
				inspectWidgetHierarchy(childWidget, thisItem.children);
		}
	}
}

void CUiInspector::inspectWidgetHierarchy(QLayout* layout, std::vector<struct WidgetHierarchy>& root) const
{
	WidgetHierarchy& thisItem = root.emplace_back();

	thisItem.layout = layout;
	for (int i = 0; true; ++i)
	{
		QLayoutItem* item = layout->itemAt(i);
		if (!item)
			break;

		auto* l = item->layout();
		auto* w = item->widget();
		if (!l && !w)
			continue;

		assert(!(l && w));

		if (l)
			inspectWidgetHierarchy(l, thisItem.children);
		else if (w)
			inspectWidgetHierarchy(w, thisItem.children);
	}
}

inline void CUiInspector::showItemContextMenu(const QPoint& p)
{
	auto* item = _tree->itemAt(p);
	if (!item)
		return;

	QMenu contextMenu(_tree);
	auto* goToParent = contextMenu.addAction(tr("Go to parent"));
	auto* highlight = contextMenu.addAction(tr("Highlight item"));
	if (const auto* selectedAction = contextMenu.exec(mapToGlobal(p)); selectedAction == goToParent)
	{
		auto* parent = item->parent();
		if (!parent)
			return;

		_tree->setCurrentItem(parent);
		_tree->scrollTo(_tree->currentIndex());
	}
	else if (selectedAction == highlight)
	{
		auto* ptr = (QObject*)item->data(0, Qt::UserRole).toULongLong();
		QWidget* widget = nullptr;
		QRect qRect;
		if (auto* l = dynamic_cast<QLayout*>(ptr))
		{
			widget = l->widget();
			qRect = widget ? widget->rect() : QRect{};
		}
		else if (auto* w = dynamic_cast<QWidget*>(ptr))
		{
			widget = w;
			qRect = w->rect();
		}

		if (qRect.isEmpty() || !widget)
			return;

		widget->setStyleSheet(QStringLiteral("* { border: 4px solid red; }"));
	}
}
