#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QComboBox>
#include <QKeySequence>
RESTORE_COMPILER_WARNINGS

class CHistoryComboBox : public QComboBox
{
	Q_OBJECT

public:
	explicit CHistoryComboBox(QWidget * parent);
	~CHistoryComboBox() override;

	void enableAutoSave(const QString& settingName);
	void setClearEditorOnItemActivation(bool clear);
	void setSaveCurrentText(bool save);

	void setSelectPreviousItemShortcut(const QKeySequence& selectPreviousItemShortcut);

	[[nodiscard]] QStringList items() const;
	[[nodiscard]] bool historyMode() const;

public slots:
	// Enables or disables history mode (moving activated item to the top)
	void setHistoryMode(bool historyMode);
	// Switch to the next combobox item (which means going back through the history if history mode is set)
	void selectPreviousItem();
	void resetToLastSelected(bool clearLineEdit);

signals:
	void itemActivated(QString itemText);

protected:
	bool eventFilter(QObject *, QEvent *) override;
	void keyPressEvent(QKeyEvent * e) override;

private:
	// Moves the currently selected item to the top
	void currentItemActivated();

	[[nodiscard]] QStringList itemsToSave() const;

private:
	// QShortcut doesn't work properly with this class for some reason, so here's a hack for creating a keyboard shortcut to selectPreviousItem
	QKeySequence _selectPreviousItemShortcut;
	QString _settingName;
	bool _bHistoryMode = true;
	bool _bClearEditorOnItemActivation = false;
	bool _bSaveCurrentText = false;
};
