#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QDialog>
RESTORE_COMPILER_WARNINGS

namespace Ui {
class CSettingsDialog;
}

class CSettingsPage;

class QListWidgetItem;

class CSettingsDialog final : public QDialog
{
	Q_OBJECT
public:
	explicit CSettingsDialog(QWidget *parent = nullptr);
	~CSettingsDialog() override;

	CSettingsDialog& addSettingsPage(CSettingsPage * page, const QString& pageName = QString());

signals:
	void settingsChanged();

private:
	void pageChanged(QListWidgetItem *item);
	void wipeSettings();

private slots:
	void accept() override;

private:
	Ui::CSettingsDialog *ui;
};
