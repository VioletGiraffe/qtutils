#include "cuiinspector.h"
#include "assert/advanced_assert.h"
#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include "ui_cuiinspector.h"

#include <QDebug>
#include <QLayout>
#include <QTimer>
RESTORE_COMPILER_WARNINGS

#include <assert.h>

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

	QTimer::singleShot(500, this, &CUiInspector::inspect);

	connect(ui->action_Close, &QAction::triggered, this, &CUiInspector::close);
	connect(ui->action_Refresh, &QAction::triggered, this, &CUiInspector::inspect);
}

CUiInspector::~CUiInspector()
{
	delete ui;
}

void inspectWidgetHierarchy(QWidget* widget, WidgetHierarchy& root);
void inspectWidgetHierarchy(QLayout* widget, WidgetHierarchy& root);

inline void inspectWidgetHierarchy(QLayout* layout, WidgetHierarchy& root)
{
	root.layout = layout;
	for (int i = 0; true;++i)
	{
		const auto item = layout->itemAt(i);
		if (!item)
			break;

		assert((item->layout() != nullptr) != (item->widget() != nullptr));

		root.children.emplace_back();
		if (item->layout())
			inspectWidgetHierarchy(item->layout(), root.children.back());
		else if (item->widget())
			inspectWidgetHierarchy(item->widget(), root.children.back());
	}
}

inline void inspectWidgetHierarchy(QWidget* widget, WidgetHierarchy& root)
{
	root.widget = widget;
	if (widget->layout())
	{
		root.children.emplace_back();
		inspectWidgetHierarchy(widget->layout(), root.children.back());
	}
	
}

void CUiInspector::inspect()
{
	std::vector<WidgetHierarchy> hierarchy;
	for (QWidget* widget : QApplication::topLevelWidgets())
	{
		if (widget->objectName() == "CUiInspector")
			continue; // Skip self

		hierarchy.emplace_back();
		inspectWidgetHierarchy(widget, hierarchy.back());
	}

	visualize(hierarchy);
}

inline QTreeWidgetItem* createTreeItem(const WidgetHierarchy& hierarchy, QTreeWidgetItem* parent = nullptr)
{
	auto item = new QTreeWidgetItem(parent);

	QString description;
	QDebug detailedInfoWriter(&description);
	if (hierarchy.widget)
		detailedInfoWriter << hierarchy.widget;
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
