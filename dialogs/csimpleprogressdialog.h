#pragma once
#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QDialog>
#include <QString>
RESTORE_COMPILER_WARNINGS

class QLabel;
class QProgressBar;
class QPushButton;

class CSimpleProgressDialog : public QDialog
{
public:
	explicit CSimpleProgressDialog(QWidget *parent = nullptr) noexcept;

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
	QLabel* _label = nullptr;
	QProgressBar* _progressBar = nullptr;
	QPushButton* _cancelButton = nullptr;
	bool _autoShow = false;
	bool _autoClose = false;
};
