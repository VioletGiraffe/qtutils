#include "cuiinspector.h"

#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtCore/QDebug>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QStatusBar>
#include <QtGui/QStyleHints>
#include <QtWidgets/QTextBrowser>
#include <QtCore/QTimer>
#include <QtWidgets/QTreeWidget>


struct WidgetHierarchy {
	QWidget* widget = nullptr;
	QLayout* layout = nullptr;

	std::vector<WidgetHierarchy> children;
};

struct CUiInspector::Ui final : QObject {
	using QObject::QObject;

	QTreeWidget* _tree = nullptr;
	QTextBrowser* _detailsText = nullptr;
	QAction* _actShowHiddenItems = nullptr;
};

inline QString getExtraWidgetString(const QWidget* widget)
{
	if (auto* b = dynamic_cast<const QAbstractButton*>(widget); b)
		return QString{ "[text: '%1', checked: %2]" }.arg(b->text()).arg(b->isChecked());
	else if (auto* l = dynamic_cast<const QLabel*>(widget); l)
		return QString{ "[text: '%1']" }.arg(l->text());
	else if (auto* le = dynamic_cast<const QLineEdit*>(widget); le)
		return QString{ "[text: '%1']" }.arg(le->text());
	else if (auto* cb = dynamic_cast<const QComboBox*>(widget); cb)
		return QString{ "[currentText: '%1', count: %2]" }.arg(cb->currentText()).arg(cb->count());
	else if (auto* mw = dynamic_cast<const QMainWindow*>(widget); mw)
		return QString{ "title: '%1'" }.arg(mw->windowTitle());
	else
		return {};
}

static QString getItemInfo(const QWidget* widget)
{
	if (!widget)
		return "<null QWidget>";

	QString description = QString{"%1(0x%2)"}.arg(widget->metaObject()->className(), QString::number((quintptr)widget, 16));
	if (QString objName = widget->objectName(); !objName.isEmpty())
		description += ", name: " + objName;
	if (QString extra = getExtraWidgetString(widget); !extra.isEmpty())
		description.append(" ").append(extra);

	if (!widget->isVisible())
		description += " - HIDDEN";
	if (widget->testAttribute(Qt::WA_TransparentForMouseEvents))
		description += " (transparent for mouse events!)";

	return description;
}

static QString getItemInfo(const QLayout* layout)
{
	if (!layout)
		return "<null QLayout>";

	QString description;
	{
		QDebug infoWriter(&description);
		infoWriter << layout << layout->count() << "items";
		if (!layout->isEnabled())
			infoWriter << "- DISABLED";
	}
	return description;
}

static QString getWidgetDetails(const QWidget* widget)
{
	QString details;
	if (widget)
	{
		QDebug infoWriter(&details);
		infoWriter << "Geometry:" << widget->geometry() << '\n';
		infoWriter << "Visible:" << widget->isVisible() << '\n';
		infoWriter << "Enabled:" << widget->isEnabled() << '\n';
		infoWriter << "Focus policy:" << widget->focusPolicy() << '\n';
		infoWriter << "Size policy:" << widget->sizePolicy() << '\n';
		infoWriter << "Minimum size:" << widget->minimumSize() << '\n';
		infoWriter << "Maximum size:" << widget->maximumSize() << '\n';
		infoWriter << "Size hint:" << widget->sizeHint() << '\n';
		infoWriter << "Min. size hint:" << widget->minimumSizeHint() << '\n';
		if (QString ss = widget->styleSheet(); !ss.isEmpty())
			infoWriter << "Style sheet:" << ss << '\n';
	}

	return details;
}

static QString getLayoutDetails(const QLayout* layout)
{
	QString details;
	if (layout)
	{
		QDebug infoWriter(&details);
		infoWriter << "Geometry:" << layout->geometry() << '\n';
		infoWriter << "Enabled:" << layout->isEnabled() << '\n';
		infoWriter << "Contents margins:" << layout->contentsMargins() << '\n';
		infoWriter << "Spacing:" << layout->spacing() << '\n';
		infoWriter << "item count:" << layout->count() << '\n';
	}
	return details;
}

CUiInspector::CUiInspector(QWidget* parent) noexcept :
	QMainWindow(parent)
{
	setupUi();

	QTimer::singleShot(500, this, &CUiInspector::inspect);

	connect(qApp, &QApplication::focusChanged, this, [this](QWidget* old, QWidget* n) {
		QString text;
		{
			QDebug infoWriter(&text);
			infoWriter << "Focus changed from" << old << "to" << n;
		}
		statusBar()->showMessage(text);
	});

	resize(800, 600);
	show();
}

void CUiInspector::setRootToInspect(QWidget* root)
{
	_rootToInspect = root;
	inspect();
}

void CUiInspector::setupUi()
{
	_ui = new Ui(this);
	QSplitter* centralSplitter = new QSplitter(Qt::Horizontal);
	_ui->_tree = new QTreeWidget;
	_ui->_tree->setSelectionMode(QAbstractItemView::SingleSelection);
	QHeaderView* header = _ui->_tree->header();
	header->setSectionResizeMode(QHeaderView::ResizeToContents);
	header->setStretchLastSection(false);  // Critical!

	centralSplitter->addWidget(_ui->_tree);

	_ui->_tree->setSelectionMode(QAbstractItemView::NoSelection);
	_ui->_tree->setHeaderHidden(true);
	_ui->_tree->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(_ui->_tree, &QWidget::customContextMenuRequested, this, &CUiInspector::showItemContextMenu);

	_ui->_detailsText = new QTextBrowser;
	_ui->_detailsText->setReadOnly(true);
	centralSplitter->addWidget(_ui->_detailsText);

	QMenuBar* mainMenu = menuBar();
	mainMenu->addAction(tr("&Refresh"), this, &CUiInspector::inspect);
	_ui->_actShowHiddenItems = mainMenu->addAction(tr("&Show hidden items"), this, &CUiInspector::inspect);
	_ui->_actShowHiddenItems->setCheckable(true);
	_ui->_actShowHiddenItems->setChecked(true);

	centralSplitter->setStretchFactor(0, 1);
	centralSplitter->setStretchFactor(1, 0);
	
	setCentralWidget(centralSplitter);

	connect(_ui->_tree, &QTreeWidget::currentItemChanged, this, &CUiInspector::onItemSelected);
}

void CUiInspector::inspect()
{
	std::vector<WidgetHierarchy> hierarchy;
	if (_rootToInspect == nullptr)
	{
		for (QWidget* widget : QApplication::topLevelWidgets())
		{
			if (widget != this) // Do not show self
				inspectWidgetHierarchy(widget, hierarchy);
		}
	}
	else
		inspectWidgetHierarchy(_rootToInspect, hierarchy);

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
		if (hierarchy.widget)
		{
			item->setData(0, Qt::UserRole, (qulonglong)hierarchy.widget);
			description = getItemInfo(hierarchy.widget);
		}
		else if (hierarchy.layout)
		{
			item->setData(0, Qt::UserRole, (qulonglong)hierarchy.layout);
			description = getItemInfo(hierarchy.layout);
		}
		else
			assert(!"Both widget and layout are nullptr");
	}

	item->setText(0, description);
	if (hierarchy.layout)
		item->setForeground(0, qApp->styleHints()->colorScheme() == Qt::ColorScheme::Dark ? QColor(100, 255, 0) : QColor(70, 178, 0));

	if (isHiddenWidget)
	{
		QFont font = item->font(0);
		font.setItalic(true);
		item->setFont(0, font);
	}

	for (const auto& child : hierarchy.children)
		createTreeItem(child, showHidden, item);

	return item;
}

void CUiInspector::visualize(const std::vector<WidgetHierarchy>& hierarchy)
{
	QList<QTreeWidgetItem*> items;
	items.reserve((int)hierarchy.size());
	const bool showHidden = _ui->_actShowHiddenItems->isChecked();
	for (const auto& h : hierarchy)
	{
		auto* item = createTreeItem(h, showHidden);
		if (item)
			items.push_back(item);
	}

	_ui->_tree->clear();
	_ui->_tree->addTopLevelItems(items);
	_ui->_tree->expandAll();
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
	auto* item = _ui->_tree->itemAt(p);
	if (!item)
		return;

	QMenu contextMenu(_ui->_tree);
	auto* goToParent = contextMenu.addAction(tr("Go to parent"));
	auto* highlight = contextMenu.addAction(tr("Highlight item"));
	if (const auto* selectedAction = contextMenu.exec(mapToGlobal(p)); selectedAction == goToParent)
	{
		auto* parent = item->parent();
		if (!parent)
			return;

		_ui->_tree->setCurrentItem(parent);
		_ui->_tree->scrollTo(_ui->_tree->currentIndex());
	}
	else if (selectedAction == highlight)
	{
		auto* ptr = reinterpret_cast<QObject*>(item->data(0, Qt::UserRole).toULongLong());
		highlightItem(ptr);
	}
}

void CUiInspector::highlightItem(QObject* item)
{
	QWidget* widget = nullptr;
	QRect qRect;
	if (auto* l = dynamic_cast<QLayout*>(item))
	{
		widget = l->widget();
		qRect = widget ? widget->rect() : QRect{};
	}
	else if (auto* w = dynamic_cast<QWidget*>(item))
	{
		widget = w;
		qRect = w->rect();
	}

	if (qRect.isEmpty() || !widget)
		return;

	if (_currentlyHighlighted == widget)
		return;

	if (_currentlyHighlighted && _currentlyHighlighted != widget)
		_currentlyHighlighted->setStyleSheet(_styleSheetToRestore);

	_currentlyHighlighted = widget;
	_styleSheetToRestore = widget->styleSheet();
	widget->setStyleSheet(QStringLiteral("* { border: 4px solid red; }"));
}

void CUiInspector::onItemSelected(QTreeWidgetItem* current, QTreeWidgetItem* /*previous*/)
{
	if (!current)
	{
		_ui->_detailsText->clear();
		return;
	}

	auto* ptr = reinterpret_cast<QObject*>(current->data(0, Qt::UserRole).toULongLong());
	if (auto* l = dynamic_cast<QLayout*>(ptr))
		_ui->_detailsText->setText(getLayoutDetails(l));
	else if (auto* w = dynamic_cast<QWidget*>(ptr))
		_ui->_detailsText->setText(getWidgetDetails(w));
}
