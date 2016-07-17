#include "caboutdialog.h"

DISABLE_COMPILER_WARNINGS
#include "ui_caboutdialog.h"

#include <QApplication>
RESTORE_COMPILER_WARNINGS

CAboutDialog::CAboutDialog(const QString& versionString, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::CAboutDialog)
{
	ui->setupUi(this);

	ui->lblProgramName->setText(QApplication::applicationDisplayName());
	ui->lblVersion->setText(tr("Version %1 (%2 %3)\nUsing Qt version %4").arg(versionString).arg(__DATE__).arg(__TIME__).arg(QT_VERSION_STR));
}

CAboutDialog::~CAboutDialog()
{
	delete ui;
}
