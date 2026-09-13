#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QComboBox>
RESTORE_COMPILER_WARNINGS

class CHistoryComboBox final : public QComboBox
{
	Q_OBJECT

public:
	explicit CHistoryComboBox(QWidget * parent);
	~CHistoryComboBox() override;

	void enableAutoSave(const QString& settingName);
	void setClearEditorOnItemActivation(bool clear);
	// No effect outside history mode or while the text is empty
	void moveCurrentTextToTopOfHistory();

	[[nodiscard]] QStringList items() const;
	[[nodiscard]] bool historyMode() const;

public slots:
	// Enables or disables history mode (moving activated item to the top)
	void setHistoryMode(bool historyMode);
	// Switch to the next combobox item (which means going back through the history if history mode is set)
	void selectPreviousItem();
	void resetToLastSelected(bool clearLineEdit);

signals:
	// Emitted on Enter, with the modifiers held
	void itemActivated(QString itemText, Qt::KeyboardModifiers modifiers);

protected:
	void keyPressEvent(QKeyEvent * e) override;

private:
	void currentItemActivated(Qt::KeyboardModifiers modifiers);

	void saveState();

private:
	QString _settingName;
	bool _bHistoryMode = true;
	bool _bClearEditorOnItemActivation = false;
};
