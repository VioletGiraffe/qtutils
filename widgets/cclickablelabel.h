#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QLabel>
RESTORE_COMPILER_WARNINGS

class CClickableLabel final : public QLabel
{
	Q_OBJECT
public:
	explicit CClickableLabel(QWidget* parent = nullptr);

signals:
	void doubleClicked(QPoint pos);
	void singleClicked(QPoint pos);

protected:
	void mouseDoubleClickEvent(QMouseEvent* e) override;
	void mouseReleaseEvent(QMouseEvent *ev) override;
};
