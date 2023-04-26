#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QLayout>
#include <QList>
RESTORE_COMPILER_WARNINGS

class COverlayLayout final : public QLayout
{
public:
	using QLayout::QLayout;
	~COverlayLayout() override;

	void addItem(QLayoutItem *item) override;
	[[nodiscard]] int count() const override;
	[[nodiscard]] QLayoutItem* itemAt(int index) const override;
	QLayoutItem* takeAt(int index) override;

	void setGeometry(const QRect &rect) override;

	[[nodiscard]] QSize sizeHint() const override;

	[[nodiscard]] Qt::Orientations expandingDirections() const override;

private:
	QList<QLayoutItem*> _items;
};
