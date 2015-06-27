#pragma once

#include <QLabel>

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
