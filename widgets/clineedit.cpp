#include "clineedit.h"

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
