#include "cclickablelabel.h"

DISABLE_COMPILER_WARNINGS
#include <QApplication>
#include <QMouseEvent>
#include <QStyle>
#include <QStyleHints>
RESTORE_COMPILER_WARNINGS

CClickableLabel::CClickableLabel(QWidget* parent) : QLabel{ parent }
{
	connect(QApplication::styleHints(), &QStyleHints::colorSchemeChanged, this, [this]() {
		QMetaObject::invokeMethod(this, [this]() {
			style()->unpolish(this);
			style()->polish(this);
		}, Qt::QueuedConnection);
	});
}

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
