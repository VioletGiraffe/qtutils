#include "csettingsdialog.h"
#include "ui_csettingsdialog.h"

#include "csettingspage.h"
#include "../settings/csettings.h"
#include "assert/advanced_assert.h"

DISABLE_COMPILER_WARNINGS
#include <QMessageBox>
#include <QShortcut>
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
	QDialog(parent),
	ui(new Ui::CSettingsDialog)
{
	ui->setupUi(this);

	// The list doesn't expand, the settings pane does
	ui->splitter->setStretchFactor(0, 0);
	ui->splitter->setStretchFactor(1, 1);

	connect(ui->pageList, &QListWidget::currentItemChanged, this, &CSettingsDialog::pageChanged);
	ui->pageList->setResizeMode(QListWidget::Adjust);

	new QShortcut(QKeySequence("Ctrl+Shift+W"), this, this, &CSettingsDialog::wipeSettings);
}

CSettingsDialog::~CSettingsDialog()
{
	delete ui;
}

CSettingsDialog& CSettingsDialog::addSettingsPage(CSettingsPage* page, const QString &pageName)
{
	page->setParent(ui->pages);
	ui->pages->addWidget(page);

	QListWidgetItem * item = new QListWidgetItem(pageName.isEmpty() ? page->windowTitle() : pageName);
	item->setData(Qt::UserRole, ui->pages->count()-1);
	ui->pageList->addItem(item);
	ui->pageList->adjustSize();

	return *this;
}

void CSettingsDialog::showEvent(QShowEvent* event)
{
	QDialog::showEvent(event);

	if (_firstShow)
	{
		ui->pageList->setFocus();
		ui->pageList->setCurrentItem(ui->pageList->item(0));
		_firstShow = false;
	}
}

void CSettingsDialog::pageChanged(QListWidgetItem * item)
{
	if (!item)
		return;

	const int pageIndex = item->data(Qt::UserRole).toInt();
	ui->pages->setCurrentIndex(pageIndex);

	// Everything below is for focus order control only
	QWidget* currentPage = ui->pages->widget(pageIndex);
	assert_and_return_r(currentPage, );

	if (!currentPage->isVisible())
		return;

	currentPage->clearFocus();

	QList<QWidget*> chain;
	chain.push_back(ui->pageList);
	buildFocusChain(chain, currentPage);
	chain.push_back(ui->buttonBox);

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
		CSettings{}.clear();
		exit(0); // Exiting immediately so that the current application state cannot be re-saved upon next normal exit
	}
}

void CSettingsDialog::accept()
{
	for (int i = 0; i < ui->pages->count(); ++i)
	{
		CSettingsPage * page = dynamic_cast<CSettingsPage*>(ui->pages->widget(i));
		assert_r(page);
		page->acceptSettings();
	}

	emit settingsChanged();

	QDialog::accept();
}
