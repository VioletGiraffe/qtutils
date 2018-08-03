#pragma once
#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QDialog>
RESTORE_COMPILER_WARNINGS

namespace Ui {
class CSimpleProgressDialog;
}

class CSimpleProgressDialog : public QDialog
{
public:
	explicit CSimpleProgressDialog(QWidget *parent = nullptr);
	~CSimpleProgressDialog() override;

	void setLabelText(const QString& text);

	void setValue(int value);
	void setMinValue(int value);
	void setMaxValue(int value);

	void setCancellable(bool visible);
	void setCancelButtonText(const QString& text);

	void setAutoShow(bool autoShow);
	void setAutoClose(bool autoClose);

private:
	void showOrHideAsNecessary();

private:
	Ui::CSimpleProgressDialog *ui;
	bool _autoShow = false;
	bool _autoClose = false;
};
