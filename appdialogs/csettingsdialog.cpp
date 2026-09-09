#include "csettingsdialog.h"

#include "csettingspage.h"
#include "assert/advanced_assert.h"

DISABLE_COMPILER_WARNINGS
#include <QAbstractScrollArea>
#include <QDialogButtonBox>
#include <QListWidget>
#include <QMessageBox>
#include <QSettings>
#include <QShortcut>
#include <QSplitter>
#include <QStackedWidget>
#include <QVBoxLayout>
RESTORE_COMPILER_WARNINGS

static void buildFocusChain(QList<QWidget*>& chain, QWidget* root)
{
	if (!root)
		return;

	QWidget* start = root;  // Can be any widget in the chain; we'll loop until back
	QWidget* current = start;

	do {
		const bool focusable = current->focusPolicy() != Qt::NoFocus && current->isVisible() && current->isEnabled();
		if (focusable)
			chain.push_back(current);

		current = current->nextInFocusChain();
	} while (current && current != start);
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
	_buttonBox->setFocusPolicy(Qt::TabFocus);

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

	// Everything below is for focus order control only
	QWidget* currentPage = _pages->widget(pageIndex);
	assert_and_return_r(currentPage, );

	if (!currentPage->isVisible())
		return;

	currentPage->clearFocus();

	QList<QWidget*> chain;
	chain.push_back(_pageList);
	buildFocusChain(chain, currentPage);
	chain.push_back(_buttonBox);

	for (size_t i = 1, n = chain.size(); i < n; ++i)
	{
		setTabOrder(chain[i - 1], chain[i]);
	}
	setTabOrder(chain.back(), chain.front());
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
