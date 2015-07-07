#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QLabel>
RESTORE_COMPILER_WARNINGS

class CClickableLabel : public QLabel
{
	Q_OBJECT

public:
	explicit CClickableLabel(QWidget* parent = 0);

signals:
	void doubleClicked(QPoint pos);

protected:
	void mouseDoubleClickEvent(QMouseEvent* e) override;
};
