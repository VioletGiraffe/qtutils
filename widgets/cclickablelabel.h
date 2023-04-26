#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QLabel>
RESTORE_COMPILER_WARNINGS

class CClickableLabel : public QLabel
{
	Q_OBJECT

public:
	using QLabel::QLabel;

signals:
	void doubleClicked(QPoint pos);
	void singleClicked(QPoint pos);

protected:
	void mouseDoubleClickEvent(QMouseEvent* e) override;
	void mouseReleaseEvent(QMouseEvent *ev) override;
};
