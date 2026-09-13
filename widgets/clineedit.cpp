#include "clineedit.h"

DISABLE_COMPILER_WARNINGS
#include <QKeyEvent>
RESTORE_COMPILER_WARNINGS

void CLineEdit::setSelectAllOnFocus(bool select)
{
	_bSelectAllOnFocus = select;
}

bool CLineEdit::selectAllOnFocus() const
{
	return _bSelectAllOnFocus;
}

void CLineEdit::focusInEvent(QFocusEvent * event)
{
	QLineEdit::focusInEvent(event);

	if (_bSelectAllOnFocus)
		QMetaObject::invokeMethod(this, &CLineEdit::selectAll, Qt::QueuedConnection);
}

void CLineEdit::keyPressEvent(QKeyEvent* event)
{
	QLineEdit::keyPressEvent(event);

	// Checked after the base handler, which has applied the validator's fixup: the condition returnPressed() is emitted under
	if ((event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) && hasAcceptableInput())
		emit returnPressedWithModifiers(event->modifiers());
}
