#include "clineedit.h"

DISABLE_COMPILER_WARNINGS
#include <QTimer>
RESTORE_COMPILER_WARNINGS

CLineEdit::CLineEdit(QWidget *parent) :
	QLineEdit(parent),
	_bSelectAllOnFocus(true)
{
}

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
		QTimer::singleShot(0, this, SLOT(selectAll()));
}
