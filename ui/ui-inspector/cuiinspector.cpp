#include "cuiinspector.h"

#include "ui_cuiinspector.h"

#include <QLayout>

#include <assert.h>
#include <vector>

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

	connect(&_timer, &QTimer::timeout, this, &CUiInspector::inspect);
	_timer.start(500);
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
		hierarchy.emplace_back();
		inspectWidgetHierarchy(widget, hierarchy.back());
	}
}
