#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QLayout>
#include <QList>
RESTORE_COMPILER_WARNINGS

class COverlayLayout : public QLayout
{
public:
	COverlayLayout(QWidget *parent);
	~COverlayLayout();

	void addItem(QLayoutItem *item) override;
	int count() const override;
	QLayoutItem* itemAt(int index) const override;
	QLayoutItem* takeAt(int index) override;

	void setGeometry(const QRect &rect) override;

	QSize sizeHint(void) const override;

	Qt::Orientations expandingDirections() const override;

private:
	QList<QLayoutItem*> _items;
};
