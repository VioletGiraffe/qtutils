#include "chistorycombobox.h"
#include "container/set_operations.hpp"
#include "settings/csettings.h"

DISABLE_COMPILER_WARNINGS
#include <QAbstractItemView>
#include <QDebug>
#include <QKeyEvent>
#include <QLineEdit>
RESTORE_COMPILER_WARNINGS

#include "assert/advanced_assert.h"

CHistoryComboBox::CHistoryComboBox(QWidget* parent) :
	QComboBox(parent)
{
	// without this call lineEdit is not created so it would be impossible to access it
	setEditable(true);

	installEventFilter(this);
}

CHistoryComboBox::~CHistoryComboBox()
{
	if (!_settingName.isEmpty())
		CSettings().setValue(_settingName, itemsToSave());
}

void CHistoryComboBox::enableAutoSave(const QString& settingName)
{
	_settingName = settingName;

	if (!settingName.isEmpty())
		addItems(CSettings().value(settingName).toStringList());
}

void CHistoryComboBox::setClearEditorOnItemActivation(bool clear)
{
	_bClearEditorOnItemActivation = clear;
}

void CHistoryComboBox::setSaveCurrentText(bool save)
{
	_bSaveCurrentText = save;
}

void CHistoryComboBox::setSelectPreviousItemShortcut(const QKeySequence& selectPreviousItemShortcut)
{
	_selectPreviousItemShortcut = selectPreviousItemShortcut;
}

// Enables or disables history mode (moving activated item to the top)
void CHistoryComboBox::setHistoryMode(bool historyMode)
{
	_bHistoryMode = historyMode;
}

bool CHistoryComboBox::historyMode() const
{
	return _bHistoryMode;
}

// Switch to the next combobox item (which means going back through the history if history mode is set)
void CHistoryComboBox::selectPreviousItem()
{
	if (count() <= 0)
		return;

	if (currentText().isEmpty())
		setCurrentIndex(0);
	else if (currentIndex() < count() - 1)
		setCurrentIndex(currentIndex() + 1);

	lineEdit()->selectAll(); // Causes a bug
}

void CHistoryComboBox::resetToLastSelected(bool clearLineEdit)
{
	lineEdit()->clear(); // To clear any user input
	setCurrentIndex(currentIndex());
	if (clearLineEdit)
		lineEdit()->clear(); // To clear the current item text set by setCurrentIndex() 

	clearFocus();
}

// TODO: this does not belong inside this class
bool CHistoryComboBox::eventFilter(QObject* receiver, QEvent* e)
{
	if (e->type() == QEvent::KeyPress)
	{
		QKeyEvent * keyEvent = static_cast<QKeyEvent*>(e);

		if (keyEvent->text().isEmpty())
			return QComboBox::eventFilter(receiver, e);

		QString modifierString;
		if (keyEvent->modifiers() & Qt::ShiftModifier)
			modifierString = "Shift+";
		if (keyEvent->modifiers() & Qt::ControlModifier)
			modifierString = "Ctrl+";
		if (keyEvent->modifiers() & Qt::AltModifier)
			modifierString = "Alt+";
		if (keyEvent->modifiers() & Qt::MetaModifier)
			modifierString = "Meta+";

		QKeySequence fullSequence(modifierString + QKeySequence(keyEvent->key()).toString());
		if (!_selectPreviousItemShortcut.isEmpty() && fullSequence == _selectPreviousItemShortcut)
		{
			selectPreviousItem();
			return true;
		}
	}

	return QComboBox::eventFilter(receiver, e);
}

QStringList CHistoryComboBox::items() const
{
	QStringList itemsList;
	const QString currentItemText = currentText();
	if (!currentItemText.isEmpty() && currentIndex() >= 0 && currentItemText != itemText(currentIndex()))
		itemsList.push_back(currentItemText);

	for (int i = 0; i < count(); ++i)
		itemsList.push_back(itemText(i));

	return itemsList;
}

void CHistoryComboBox::keyPressEvent(QKeyEvent* e)
{
	if (lineEdit()->hasFocus() && (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter))
	{
		e->accept();
		currentItemActivated();
	}
	else
		QComboBox::keyPressEvent(e);

	if (!_settingName.isEmpty())
		CSettings().setValue(_settingName, itemsToSave());
}

// Moves the currently selected item to the top
void CHistoryComboBox::currentItemActivated()
{
	const QString newItem = currentText();
	emit itemActivated(newItem);

	if (_bHistoryMode)
	{
		// No longer necessary as of Qt 5.4?..
		//		removeItem(currentIndex());

		auto list = items();
		list.push_front(newItem);

		list = SetOperations::uniqueElements(list);
		clear();
		for (int i = list.size() - 1; i >= 0; --i)
			insertItem(0, list[i]);
		setCurrentIndex(0);

		if (_bClearEditorOnItemActivation)
			lineEdit()->clear();
	}
}

QStringList CHistoryComboBox::itemsToSave() const
{
	auto result = items();
	if (_bSaveCurrentText)
		result.push_front(currentText());

	return result;
}
