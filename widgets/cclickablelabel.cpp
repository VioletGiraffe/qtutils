#include "cclickablelabel.h"

DISABLE_COMPILER_WARNINGS
#include <QMouseEvent>
RESTORE_COMPILER_WARNINGS

CClickableLabel::CClickableLabel(QWidget* parent) : QLabel(parent)
{
}

void CClickableLabel::mouseDoubleClickEvent(QMouseEvent* e)
{
	QLabel::mouseDoubleClickEvent(e);
	emit doubleClicked(e ? mapToGlobal(e->pos()) : QPoint());
}


