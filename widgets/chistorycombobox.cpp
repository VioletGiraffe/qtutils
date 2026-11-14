#include "chistorycombobox.h"
#include "utils/painterutils.h"
#include "container/set_operations.hpp"

DISABLE_COMPILER_WARNINGS
#include <QAbstractItemView>
#include <QFontMetrics>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMargins>
#include <QSettings>
#include <QSize>
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

	connect(this, &QComboBox::currentTextChanged, this, [this] {
		if (_sizedToCurrentText)
			updateGeometry();
	});
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

void CHistoryComboBox::setSizedToCurrentText(const bool sized)
{
	if (_sizedToCurrentText == sized)
		return;

	_sizedToCurrentText = sized;
	if (!sized)
		view()->window()->setMinimumWidth(0); // showPopup() set it
	updateGeometry();
}

void CHistoryComboBox::showPopup()
{
	// QComboBox makes the popup as wide as itself, or as the popup window's minimum width if that is larger
	if (_sizedToCurrentText)
	{
		const int scrollBarWidth = style()->pixelMetric(QStyle::PM_ScrollBarExtent, nullptr, this);
		view()->window()->setMinimumWidth(view()->sizeHintForColumn(0) + 2 * view()->frameWidth() + scrollBarWidth);
	}

	QComboBox::showPopup();
}

QSize CHistoryComboBox::sizeHint() const
{
	return _sizedToCurrentText ? sizeHintForText(currentText()) : QComboBox::sizeHint();
}

QSize CHistoryComboBox::minimumSizeHint() const
{
	return _sizedToCurrentText ? sizeHintForText({}) : QComboBox::minimumSizeHint();
}

QSize CHistoryComboBox::sizeHintForText(const QString& text) const
{
	ensurePolished();

	const QLineEdit* const editor = lineEdit();
	if (editor)
		editor->ensurePolished(); // a stylesheet may give the editor its own font
	const QFont textFont = editor ? editor->font() : font();
	const QFontMetrics metrics{ textFont };
	int contentWidth = roundedUpTextWidth(textFont, text);
	if (editor)
	{
		// QLineEdit insets its text by 2 px a side and needs room for the cursor
		static constexpr int textInset = 2 * 2 + 2;
		const QMargins textMargins = editor->textMargins(), frameMargins = editor->contentsMargins();
		contentWidth += textInset + textMargins.left() + textMargins.right() + frameMargins.left() + frameMargins.right();
	}

	QStyleOptionComboBox option;
	initStyleOption(&option);

	// The height stays the base class's
	QSize hint = QComboBox::sizeHint();
	hint.setWidth(style()->sizeFromContents(QStyle::CT_ComboBox, &option, QSize{ contentWidth, metrics.height() }, this).width());
	return hint;
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
