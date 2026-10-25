#include "chistorycombobox.h"
#include "container/set_operations.hpp"

DISABLE_COMPILER_WARNINGS
#include <QKeyEvent>
#include <QLineEdit>
#include <QSettings>
RESTORE_COMPILER_WARNINGS

CHistoryComboBox::CHistoryComboBox(QWidget* parent) :
	QComboBox(parent)
{
	// without this call lineEdit is not created so it would be impossible to access it
	setEditable(true);
	// Qt inserts nothing on Enter: the owner records entries
	setInsertPolicy(QComboBox::NoInsert);
}

CHistoryComboBox::~CHistoryComboBox()
{
	saveState();
}

void CHistoryComboBox::setSettingsKeyForHistoryPersistence(const QString& settingsKey)
{
	_settingsKey = settingsKey;

	if (!settingsKey.isEmpty())
		addItems(QSettings().value(settingsKey).toStringList());
}

void CHistoryComboBox::setClearEditorOnItemActivation(bool clear)
{
	_bClearEditorOnItemActivation = clear;
}

void CHistoryComboBox::addToHistory(const QString& text)
{
	if (text.isEmpty())
		return;

	QStringList list = items();
	list.push_front(text);
	list = SetOperations::uniqueElements(list);

	setUpdatesEnabled(false);
	clear();
	addItems(list);
	setUpdatesEnabled(true);

	setCurrentIndex(0);
	saveState();
}

void CHistoryComboBox::selectPreviousItem()
{
	if (count() <= 0)
		return;

	if (currentText().isEmpty())
		setCurrentIndex(0);
	else if (currentIndex() < count() - 1)
		setCurrentIndex(currentIndex() + 1);

	lineEdit()->selectAll();
}

void CHistoryComboBox::resetToLastSelected(bool clearLineEdit)
{
	lineEdit()->clear(); // To clear any user input
	setCurrentIndex(currentIndex());
	if (clearLineEdit)
		lineEdit()->clear(); // To clear the current item text set by setCurrentIndex()

	clearFocus();
}

QStringList CHistoryComboBox::items() const
{
	const auto nItems = count();

	QStringList itemsList;
	itemsList.reserve(nItems);
	for (int i = 0; i < nItems; ++i)
		itemsList.push_back(itemText(i));

	return itemsList;
}

void CHistoryComboBox::keyPressEvent(QKeyEvent* e)
{
	if (lineEdit()->hasFocus() && (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter))
	{
		e->accept();
		currentItemActivated(e->modifiers());
	}
	else
		QComboBox::keyPressEvent(e);
}

void CHistoryComboBox::currentItemActivated(Qt::KeyboardModifiers modifiers)
{
	const QString newItem = currentText();
	if (_bClearEditorOnItemActivation)
		lineEdit()->clear();

	emit itemActivated(newItem, modifiers);
}

void CHistoryComboBox::saveState()
{
	if (!_settingsKey.isEmpty())
		QSettings().setValue(_settingsKey, items());
}
