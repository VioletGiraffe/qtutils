#include "csimpleprogressdialog.h"

DISABLE_COMPILER_WARNINGS
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>
RESTORE_COMPILER_WARNINGS

CSimpleProgressDialog::CSimpleProgressDialog(QWidget *parent) noexcept :
	QDialog(parent)
{
	_label = new QLabel(tr("Making everything better, please wait..."), this);

	_progressBar = new QProgressBar(this);
	_progressBar->setValue(0); // Not the default of -1, which paints an empty bar with no percentage

	_cancelButton = new QPushButton(tr("&Cancel"), this);

	QHBoxLayout* buttonRow = new QHBoxLayout;
	buttonRow->setContentsMargins(0, 0, 0, 0); // A nested layout takes the parent layout's spacing as its margin, insetting the row
	buttonRow->addStretch();
	buttonRow->addWidget(_cancelButton);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addWidget(_label);
	layout->addWidget(_progressBar);
	layout->addLayout(buttonRow);
	layout->addStretch();

	// Fixed, not derived from the label: setLabelText may be called while the dialog is visible, and the dialog must not jump
	resize(307, 92);

	connect(_cancelButton, &QPushButton::clicked, this, &CSimpleProgressDialog::reject);
}

void CSimpleProgressDialog::setLabelText(const QString &text)
{
	_label->setText(text);
}

void CSimpleProgressDialog::setValue(int value)
{
	_progressBar->setValue(value);
	showOrHideAsNecessary();
}

void CSimpleProgressDialog::setMinValue(int value)
{
	_progressBar->setMinimum(value);
	showOrHideAsNecessary();
}

void CSimpleProgressDialog::setMaxValue(int value)
{
	_progressBar->setMaximum(value);
	showOrHideAsNecessary();
}

void CSimpleProgressDialog::setCancellable(bool visible)
{
	_cancelButton->setVisible(visible);
	setWindowFlag(Qt::WindowCloseButtonHint, visible);
}

void CSimpleProgressDialog::setCancelButtonText(const QString& text)
{
	_cancelButton->setText(text);
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
	if (_autoClose && _progressBar->value() >= _progressBar->maximum())
		close();
	else if (_autoShow && _progressBar->value() > _progressBar->minimum())
		show();
}
