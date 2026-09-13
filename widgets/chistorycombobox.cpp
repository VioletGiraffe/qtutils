#include "chistorycombobox.h"
#include "container/set_operations.hpp"

DISABLE_COMPILER_WARNINGS
#include <QAbstractItemView>
#include <QDebug>
#include <QKeyEvent>
#include <QLineEdit>
#include <QSettings>
RESTORE_COMPILER_WARNINGS

CHistoryComboBox::CHistoryComboBox(QWidget* parent) :
	QComboBox(parent)
{
	// without this call lineEdit is not created so it would be impossible to access it
	setEditable(true);

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
		addItems(QSettings().value(settingName).toStringList());
}

void CHistoryComboBox::setClearEditorOnItemActivation(bool clear)
{
	_bClearEditorOnItemActivation = clear;
}

void CHistoryComboBox::setSaveCurrentText(bool save)
{
	_bSaveCurrentText = save;
}

void CHistoryComboBox::setHistoryMode(bool historyMode)
{
	_bHistoryMode = historyMode;
}

bool CHistoryComboBox::historyMode() const
{
	return _bHistoryMode;
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
		QSettings().setValue(_settingName, itemsToSave());
}
