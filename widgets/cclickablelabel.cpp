#include "cclickablelabel.h"

#include <QMouseEvent>

CClickableLabel::CClickableLabel(QWidget* parent) : QLabel(parent)
{
}

void CClickableLabel::mouseDoubleClickEvent(QMouseEvent* e)
{
	QLabel::mouseDoubleClickEvent(e);
	emit doubleClicked(e ? mapToGlobal(e->pos()) : QPoint());
}


