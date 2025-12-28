#include "chistorycombobox.h"
#include "container/set_operations.hpp"
#include "settings/csettings.h"

#include "../qtcore_helpers/qstring_helpers.hpp"

DISABLE_COMPILER_WARNINGS
#include <QAbstractItemView>
#include <QDebug>
#include <QKeyEvent>
#include <QLineEdit>
RESTORE_COMPILER_WARNINGS

CHistoryComboBox::CHistoryComboBox(QWidget* parent) :
	QComboBox(parent)
{
	// without this call lineEdit is not created so it would be impossible to access it
	setEditable(true);

	installEventFilter(this);

	connect(this, &QComboBox::activated, this, &CHistoryComboBox::onItemSelected);
}

CHistoryComboBox::~CHistoryComboBox()
{
	saveState();
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
			modifierString = QSL("Shift+");
		if (keyEvent->modifiers() & Qt::ControlModifier)
			modifierString = QSL("Ctrl+");
		if (keyEvent->modifiers() & Qt::AltModifier)
			modifierString = QSL("Alt+");
		if (keyEvent->modifiers() & Qt::MetaModifier)
			modifierString = QSL("Meta+");

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
	const QString currentItemText = currentText();
	const auto nItems = count();

	QStringList itemsList;
	itemsList.reserve(nItems + 1);

	if (!currentItemText.isEmpty() && currentIndex() >= 0 && currentItemText != itemText(currentIndex()))
		itemsList.push_back(currentItemText);

	for (int i = 0; i < nItems; ++i)
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

	saveState();
}

// Moves the currently selected item to the top
void CHistoryComboBox::currentItemActivated()
{
	const QString newItem = currentText();
	onItemSelected();
	emit itemActivated(newItem);
}

void CHistoryComboBox::onItemSelected()
{
	if (_bHistoryMode)
	{
		auto list = items();
		list.push_front(currentText());

		list = SetOperations::uniqueElements(list);

		setUpdatesEnabled(false);
		clear();
		addItems(list);
		setUpdatesEnabled(true);

		setCurrentIndex(0);

		if (_bClearEditorOnItemActivation)
			lineEdit()->clear();
	}

	saveState();
}

QStringList CHistoryComboBox::itemsToSave() const
{
	auto result = items();
	if (_bSaveCurrentText)
		result.push_front(currentText());

	return result;
}

void CHistoryComboBox::saveState()
{
	if (!_settingName.isEmpty())
		CSettings().setValue(_settingName, itemsToSave());
}
