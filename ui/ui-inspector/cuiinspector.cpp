#include "cuiinspector.h"
#include "assert/advanced_assert.h"
#include "compiler/compiler_warnings_control.h"
#include "settings/csettings.h"
#include "widgets/cpersistentwindow.h"

DISABLE_COMPILER_WARNINGS
#include "ui_cuiinspector.h"

#include <QDebug>
#include <QLayout>
#include <QTimer>
RESTORE_COMPILER_WARNINGS

#include <assert.h>

#define KEY_IGNORED_CLASSES "Tools/UiInspector/IgnoredClasses"
#define KEY_IGNORED_WINDOW_STATE "Tools/UiInspector/Window"

struct WidgetHierarchy {
	QWidget* widget = nullptr;
	QLayout* layout = nullptr;

	std::vector<WidgetHierarchy> children;
};

CUiInspector::CUiInspector(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::CUiInspector)
{
	ui->setupUi(this);

	installEventFilter(new CPersistenceEnabler(KEY_IGNORED_WINDOW_STATE, this));

	CSettings s;
	_ignoredClasses = s.value(KEY_IGNORED_CLASSES, QStringList{"CUiInspector", "QMenu"}).toStringList();

	QTimer::singleShot(500, this, &CUiInspector::inspect);

	connect(ui->action_Close, &QAction::triggered, this, &CUiInspector::close);
	connect(ui->action_Refresh, &QAction::triggered, this, &CUiInspector::inspect);
}

CUiInspector::~CUiInspector()
{
	delete ui;
}

void CUiInspector::inspect()
{
	std::vector<WidgetHierarchy> hierarchy;
	for (QWidget* widget : QApplication::topLevelWidgets())
		inspectWidgetHierarchy(widget, hierarchy);

	visualize(hierarchy);
}

inline QTreeWidgetItem* createTreeItem(const WidgetHierarchy& hierarchy, QTreeWidgetItem* parent = nullptr)
{
	auto item = new QTreeWidgetItem(parent);

	QString description;
	QDebug detailedInfoWriter(&description);
	if (hierarchy.widget)
	{
		detailedInfoWriter << hierarchy.widget;
		if (!hierarchy.widget->isVisible())
			description += "- HIDDEN";
	}
	else if (hierarchy.layout)
		detailedInfoWriter << hierarchy.layout;
	else
		assert_unconditional_r("Both widget and layout are nullptr");
	
	item->setText(0, description);
	for (const auto& child : hierarchy.children)
		createTreeItem(child, item);

	return item;
}

void CUiInspector::visualize(const std::vector<WidgetHierarchy>& hierarchy)
{
	QList<QTreeWidgetItem*> items;
	for (const auto& h : hierarchy)
	{
		auto item = createTreeItem(h);
		items.push_back(item);
	}

	ui->treeWidget->clear();
	ui->treeWidget->addTopLevelItems(items);
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
			if (auto childWidget = dynamic_cast<QWidget*>(child))
				inspectWidgetHierarchy(childWidget, thisItem.children);
		}
	}
}

void CUiInspector::inspectWidgetHierarchy(QLayout* layout, std::vector<struct WidgetHierarchy>& root) const
{
	root.emplace_back();
	WidgetHierarchy& thisItem = root.back();

	thisItem.layout = layout;
	for (int i = 0; true; ++i)
	{
		const auto item = layout->itemAt(i);
		if (!item)
			break;

		assert((item->layout() != nullptr) != (item->widget() != nullptr));

		if (item->layout())
			inspectWidgetHierarchy(item->layout(), thisItem.children);
		else if (item->widget())
			inspectWidgetHierarchy(item->widget(), thisItem.children);
	}
}
