#include "clineedit.h"

DISABLE_COMPILER_WARNINGS
#include <QFocusEvent>
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

	if (!_bSelectAllOnFocus)
		return;

	// Only when focus moves into the field: focus returning from a popup, the menu bar or another window may find the text mid-edit
	switch (event->reason())
	{
	case Qt::MouseFocusReason:
	case Qt::TabFocusReason:
	case Qt::BacktabFocusReason:
	case Qt::ShortcutFocusReason:
	case Qt::OtherFocusReason:
		QMetaObject::invokeMethod(this, &CLineEdit::selectAll, Qt::QueuedConnection);
		break;
	default:
		break;
	}
}
