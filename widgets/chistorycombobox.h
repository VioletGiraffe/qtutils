#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QComboBox>
RESTORE_COMPILER_WARNINGS

// Editable combo box whose items are a most-recent-first history. The owner records entries with addToHistory; maxCount() caps the list.
class CHistoryComboBox final : public QComboBox
{
	Q_OBJECT

public:
	explicit CHistoryComboBox(QWidget * parent);
	~CHistoryComboBox() override;

	// Appends the list saved under settingsKey and saves every later change there
	void setSettingsKeyForHistoryPersistence(const QString& settingsKey);
	void setClearEditorOnItemActivation(bool clear);
	// Moves text to the top, adding it when absent, and makes it current. Empty text is ignored.
	void addToHistory(const QString& text);

	[[nodiscard]] QStringList items() const;

public slots:
	// Switch to the next combobox item, which is the previous entry in the history
	void selectPreviousItem();
	void resetToLastSelected(bool clearLineEdit);

signals:
	// Emitted on Enter, with the modifiers held; the text is not recorded
	void itemActivated(QString itemText, Qt::KeyboardModifiers modifiers);

protected:
	void keyPressEvent(QKeyEvent * e) override;

private:
	void currentItemActivated(Qt::KeyboardModifiers modifiers);

	void saveState();

private:
	QString _settingsKey;
	bool _bClearEditorOnItemActivation = false;
};
