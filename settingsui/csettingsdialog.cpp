#include "csettingsdialog.h"
#include "csettingspage.h"
#include "ui_csettingsdialog.h"
#include "../settings/csettings.h"
#include "assert/advanced_assert.h"

DISABLE_COMPILER_WARNINGS
#include <QMessageBox>
#include <QShortcut>
RESTORE_COMPILER_WARNINGS

CSettingsDialog::CSettingsDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::CSettingsDialog)
{
	ui->setupUi(this);

	// The list doesn't expand, the settings pane does
	ui->splitter->setStretchFactor(0, 0);
	ui->splitter->setStretchFactor(1, 1);

	connect(ui->pageList, &QListWidget::itemClicked, this, &CSettingsDialog::pageChanged);
	ui->pageList->setResizeMode(QListWidget::Adjust);

	new QShortcut(QKeySequence("Ctrl+Shift+W"), this, this, &CSettingsDialog::wipeSettings);
}

CSettingsDialog::~CSettingsDialog()
{
	delete ui;
}

CSettingsDialog& CSettingsDialog::addSettingsPage(CSettingsPage* page, const QString &pageName)
{
	page->setParent(this);
	ui->pages->addWidget(page);

	QListWidgetItem * item = new QListWidgetItem(pageName.isEmpty() ? page->windowTitle() : pageName);
	item->setData(Qt::UserRole, ui->pages->count()-1);
	ui->pageList->addItem(item);

	if (ui->pages->count() == 1)
		ui->pageList->setCurrentRow(0);

	ui->pageList->adjustSize();

	return *this;
}

void CSettingsDialog::pageChanged(QListWidgetItem * item)
{
	const int pageIndex = item->data(Qt::UserRole).toInt();
	ui->pages->setCurrentIndex(pageIndex);
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
