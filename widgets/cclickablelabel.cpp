#include "cclickablelabel.h"

DISABLE_COMPILER_WARNINGS
#include <QMouseEvent>
RESTORE_COMPILER_WARNINGS

void CClickableLabel::mouseDoubleClickEvent(QMouseEvent* e)
{
	QLabel::mouseDoubleClickEvent(e);
	emit doubleClicked(mapToGlobal(e->pos()));
}

void CClickableLabel::mouseReleaseEvent(QMouseEvent *ev)
{
	QLabel::mouseReleaseEvent(ev);
	emit singleClicked(mapToGlobal(ev->pos()));
}
