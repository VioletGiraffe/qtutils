#include "caboutdialog.h"

DISABLE_COMPILER_WARNINGS
#include "ui_caboutdialog.h"

#include <QApplication>
RESTORE_COMPILER_WARNINGS

CAboutDialog::CAboutDialog(QWidget *parent /*= 0*/) : CAboutDialog(QString(), parent)
{
}

CAboutDialog::CAboutDialog(const QString& versionString, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::CAboutDialog)
{
	ui->setupUi(this);

	ui->lblProgramName->setText(QApplication::applicationDisplayName());

	if (!versionString.isEmpty())
		ui->lblVersion->setText(tr("Version %1 (%2 %3)\nUsing Qt version %4").arg(versionString).arg(__DATE__).arg(__TIME__).arg(QT_VERSION_STR));
	else
		ui->lblVersion->setText(tr("Built on %1 at %2\nUsing Qt version %3").arg(__DATE__).arg(__TIME__).arg(QT_VERSION_STR));
}

CAboutDialog::~CAboutDialog()
{
	delete ui;
}
