#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QDialog>
#include <QString>
RESTORE_COMPILER_WARNINGS

class CSettingsPage;

class QDialogButtonBox;
class QListWidget;
class QListWidgetItem;
class QStackedWidget;

class CSettingsDialog : public QDialog
{
public:
	explicit CSettingsDialog(QWidget *parent = nullptr) noexcept;

	CSettingsDialog& addSettingsPage(CSettingsPage * page, const QString& pageName = QString());

protected:
	void showEvent(QShowEvent* event) override;

private:
	void pageChanged(QListWidgetItem *item);
	void wipeSettings();
	void accept() override;
	void reject() override;

	QListWidget* _pageList = nullptr;
	QStackedWidget* _pages = nullptr;
	QDialogButtonBox* _buttonBox = nullptr;
	bool _firstShow = true;
};
