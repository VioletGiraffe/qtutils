#include "chistorycombobox.h"
#include "container/set_operations.hpp"

DISABLE_COMPILER_WARNINGS
#include <QChildEvent>
#include <QCompleter>
#include <QKeyEvent>
#include <QLineEdit>
#include <QSettings>
#include <QStyle>
#include <QStyleOptionComboBox>
#include <QWheelEvent>
RESTORE_COMPILER_WARNINGS

namespace {

// QLineEdit's Enter accepts a pending inline completion instead of finishing the edit
bool inlineCompletionPending(const QLineEdit& lineEdit)
{
	const QCompleter* completer = lineEdit.completer();
	return completer && completer->completionMode() == QCompleter::InlineCompletion && lineEdit.hasSelectedText()
		&& lineEdit.selectionEnd() == lineEdit.text().size();
}

} // namespace

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

bool CHistoryComboBox::eventFilter(QObject* watched, QEvent* e)
{
	// Enter skips QLineEdit, whose editingFinished makes QComboBox emit activated for text matching an entry
	if (watched == lineEdit() && e->type() == QEvent::KeyPress)
	{
		const auto* keyEvent = static_cast<const QKeyEvent*>(e);
		if ((keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) && !inlineCompletionPending(*lineEdit()))
		{
			currentItemActivated(keyEvent->modifiers());
			return true;
		}
	}

	return QComboBox::eventFilter(watched, e);
}

void CHistoryComboBox::childEvent(QChildEvent* e)
{
	// Covers the line edit setEditable creates and any later one from setLineEdit
	if (e->type() == QEvent::ChildPolished && qobject_cast<QLineEdit*>(e->child()))
		e->child()->installEventFilter(this);

	QComboBox::childEvent(e);
}

// The base handlers of the browsing keys and the wheel emit activated; browsing here does not
void CHistoryComboBox::keyPressEvent(QKeyEvent* e)
{
	const bool browsingModifiers = !(e->modifiers() & (Qt::AltModifier | Qt::ControlModifier)); // Alt+Down opens the popup, Ctrl goes to the completer
	if (browsingModifiers && (e->key() == Qt::Key_Up || e->key() == Qt::Key_PageUp))
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
