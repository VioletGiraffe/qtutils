#include "chistorycombobox.h"
#include "container/set_operations.hpp"

DISABLE_COMPILER_WARNINGS
#include <QKeyEvent>
#include <QLineEdit>
#include <QSettings>
#include <QStyle>
#include <QStyleOptionComboBox>
#include <QWheelEvent>
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

// Keys arrive here, not at the line edit: QComboBox is its focus proxy
// Enter never reaches QLineEdit, which would only accept a pending inline completion; the activated text includes the completion shown
// The base handler emits activated for the browsing keys; browsing here does not
void CHistoryComboBox::keyPressEvent(QKeyEvent* e)
{
	const bool browsingModifiers = !(e->modifiers() & (Qt::AltModifier | Qt::ControlModifier)); // Alt+Down opens the popup, Ctrl goes to the completer
	if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter)
		currentItemActivated(e->modifiers());
	else if (browsingModifiers && (e->key() == Qt::Key_Up || e->key() == Qt::Key_PageUp))
		browseHistory(-1);
	else if (browsingModifiers && (e->key() == Qt::Key_Down || e->key() == Qt::Key_PageDown))
		browseHistory(1);
	else
	{
		QComboBox::keyPressEvent(e);
		return;
	}

	e->accept();
}

// The base handler emits activated; browsing here does not
void CHistoryComboBox::wheelEvent(QWheelEvent* e)
{
	QStyleOptionComboBox option;
	initStyleOption(&option);
	if (!style()->styleHint(QStyle::SH_ComboBox_AllowWheelScrolling, &option, this))
	{
		QComboBox::wheelEvent(e);
		return;
	}

	if (const int delta = e->angleDelta().y(); delta != 0)
		browseHistory(delta > 0 ? -1 : 1);
	e->accept();
}

void CHistoryComboBox::browseHistory(int step)
{
	if (const int index = currentIndex() + step; index >= 0 && index < count())
		setCurrentIndex(index);
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
