#include "cclickablelabel.h"

CClickableLabel::CClickableLabel(QWidget* parent) : QLabel(parent)
{
}

void CClickableLabel::mouseDoubleClickEvent(QMouseEvent* e)
{
	QLabel::mouseDoubleClickEvent(e);
	emit doubleClicked(e ? mapToGlobal(e->pos()) : QPoint());
}


