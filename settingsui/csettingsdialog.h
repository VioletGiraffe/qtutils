#ifndef CSETTINGSDIALOG_H
#define CSETTINGSDIALOG_H

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QDialog>
RESTORE_COMPILER_WARNINGS

#include <vector>

namespace Ui {
class CSettingsDialog;
}

class CSettingsPage;

class QListWidgetItem;

class CSettingsDialog : public QDialog
{
	Q_OBJECT

public:
	explicit CSettingsDialog(QWidget *parent = 0);
	~CSettingsDialog();

	CSettingsDialog& addSettingsPage(CSettingsPage * page, const QString& pageName = QString());

signals:
	void settingsChanged();

private:
	void pageChanged(QListWidgetItem *item);

private slots:
	void accept() override;

private:
	Ui::CSettingsDialog *ui;
};

#endif // CSETTINGSDIALOG_H
