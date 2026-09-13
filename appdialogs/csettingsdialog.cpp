#include "csettingsdialog.h"

#include "csettingsnotifier.h"
#include "csettingspage.h"
#include "assert/advanced_assert.h"

DISABLE_COMPILER_WARNINGS
#include <QAbstractScrollArea>
#include <QDialogButtonBox>
#include <QList>
#include <QListWidget>
#include <QMessageBox>
#include <QSettings>
#include <QShortcut>
#include <QSplitter>
#include <QStackedWidget>
#include <QVBoxLayout>
RESTORE_COMPILER_WARNINGS

// Moves the tab stops inside container to directly after anchor, keeping their focus chain order
static void moveTabStopsAfter(QWidget* anchor, QWidget* container)
{
	QList<QWidget*> tabStops;
	for (QWidget* w = container->nextInFocusChain(); w != container; w = w->nextInFocusChain())
	{
		// Widgets with a focus proxy are skipped: setTabOrder() on a compound widget moves its proxied children along
		if (container->isAncestorOf(w) && (w->focusPolicy() & Qt::TabFocus) && !w->focusProxy())
			tabStops.push_back(w);
	}

	QWidget* previous = anchor;
	for (QWidget* tabStop : tabStops)
	{
		QWidget::setTabOrder(previous, tabStop);
		previous = tabStop;
	}
}

CSettingsDialog::CSettingsDialog(QWidget *parent) noexcept :
	QDialog(parent)
{
	setWindowTitle(tr("Settings"));

	// Pages arrive after construction, so the automatic adjustSize() on first show is what fits the dialog to them.
	// A minimum rather than resize(): resize() sets WA_Resized, which suppresses that.
	setMinimumSize(600, 400);

	QSplitter* splitter = new QSplitter(Qt::Horizontal, this);
	splitter->setOpaqueResize(false);
	splitter->setChildrenCollapsible(false);

	_pageList = new QListWidget(splitter);
	_pageList->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
	_pageList->setResizeMode(QListWidget::Adjust);
	_pageList->setStyleSheet("QListWidget::item { padding: 6px; }");

	_pages = new QStackedWidget(splitter);
	_pages->setMinimumWidth(50);

	splitter->addWidget(_pageList);
	splitter->addWidget(_pages);
	// The list doesn't expand, the settings pane does
	splitter->setStretchFactor(0, 0);
	splitter->setStretchFactor(1, 1);

	_buttonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addWidget(splitter);
	layout->addWidget(_buttonBox);

	connect(_pageList, &QListWidget::currentItemChanged, this, &CSettingsDialog::pageChanged);
	connect(_buttonBox, &QDialogButtonBox::accepted, this, &CSettingsDialog::accept);
	connect(_buttonBox, &QDialogButtonBox::rejected, this, &CSettingsDialog::reject);

	new QShortcut(QKeySequence("Ctrl+Shift+W"), this, this, &CSettingsDialog::wipeSettings);
}

CSettingsDialog& CSettingsDialog::addSettingsPage(CSettingsPage* page, const QString &pageName)
{
	// The page comes in parented to the dialog; addWidget() would re-parent it anyway, but warns while doing so
	page->setParent(_pages);
	_pages->addWidget(page);

	QListWidgetItem * item = new QListWidgetItem(pageName.isEmpty() ? page->windowTitle() : pageName);
	item->setData(Qt::UserRole, _pages->count()-1);
	_pageList->addItem(item);
	_pageList->adjustSize();

	return *this;
}

void CSettingsDialog::showEvent(QShowEvent* event)
{
	QDialog::showEvent(event);

	if (_firstShow)
	{
		// Page widgets follow the button box in the focus chain: pages are added after construction.
		// One order serves every page: Tab skips widgets on hidden pages.
		moveTabStopsAfter(_pageList, _pages);

		_pageList->setFocus();
		_pageList->setCurrentItem(_pageList->item(0));
		_firstShow = false;
	}
}

void CSettingsDialog::pageChanged(QListWidgetItem * item)
{
	if (!item)
		return;

	const int pageIndex = item->data(Qt::UserRole).toInt();
	_pages->setCurrentIndex(pageIndex);
}

void CSettingsDialog::wipeSettings()
{
	if (QMessageBox::question(this, tr("Wipe settings"), tr("Wipe all settings?")) == QMessageBox::Yes)
	{
		QSettings{}.clear();
		_Exit(0); // Exiting immediately so that the current application state cannot be re-saved upon next normal exit
	}
}

void CSettingsDialog::accept()
{
	for (int i = 0; i < _pages->count(); ++i)
	{
		CSettingsPage * page = dynamic_cast<CSettingsPage*>(_pages->widget(i));
		assert_r(page);
		page->acceptSettings();
	}

	CSettingsNotifier::instance().notifySettingsChanged();

	QDialog::accept();
}

void CSettingsDialog::reject()
{
	for (int i = 0; i < _pages->count(); ++i)
	{
		CSettingsPage * page = dynamic_cast<CSettingsPage*>(_pages->widget(i));
		assert_r(page);
		page->rejectSettings();
	}

	QDialog::reject();
}
