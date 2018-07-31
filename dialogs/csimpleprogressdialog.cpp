#include "csimpleprogressdialog.h"

DISABLE_COMPILER_WARNINGS
#include "ui_csimpleprogressdialog.h"
RESTORE_COMPILER_WARNINGS

CSimpleProgressDialog::CSimpleProgressDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::CSimpleProgressDialog)
{
	ui->setupUi(this);

	connect(ui->_cancelButton, &QPushButton::clicked, this, &CSimpleProgressDialog::reject);
}

CSimpleProgressDialog::~CSimpleProgressDialog()
{
	delete ui;
}

void CSimpleProgressDialog::setValue(int value)
{
	ui->_progressBar->setValue(value);
	showOrHideAsNecessary();
}

void CSimpleProgressDialog::setMinValue(int value)
{
	ui->_progressBar->setMinimum(value);
	showOrHideAsNecessary();
}

void CSimpleProgressDialog::setMaxValue(int value)
{
	ui->_progressBar->setMaximum(value);
	showOrHideAsNecessary();
}

void CSimpleProgressDialog::setCancellable(bool visible)
{
	ui->_cancelButton->setVisible(visible);
	setWindowFlag(Qt::WindowCloseButtonHint, visible);
}

void CSimpleProgressDialog::setCancelButtonText(const QString& text)
{
	ui->_cancelButton->setText(text);
}

void CSimpleProgressDialog::setAutoShow(bool autoShow)
{
	_autoShow = autoShow;
	showOrHideAsNecessary();
}

void CSimpleProgressDialog::setAutoClose(bool autoClose)
{
	_autoClose = autoClose;
	showOrHideAsNecessary();
}

void CSimpleProgressDialog::showOrHideAsNecessary()
{
	if (_autoClose && ui->_progressBar->value() >= ui->_progressBar->maximum())
		close();
	else if (_autoShow && ui->_progressBar->value() > ui->_progressBar->minimum())
		show();
}
