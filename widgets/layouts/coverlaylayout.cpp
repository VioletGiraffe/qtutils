#include "coverlaylayout.h"

COverlayLayout::~COverlayLayout()
{
	// TODO:
	for (auto item : _items)
		delete item;
}

void COverlayLayout::addItem(QLayoutItem *item)
{
	_items.push_back(item);
}

int COverlayLayout::count() const
{
	return static_cast<int>(_items.size());
}

QLayoutItem* COverlayLayout::itemAt(int index) const
{
	return index >= 0 && index < count() ? _items[index] : nullptr;
}

QLayoutItem* COverlayLayout::takeAt(int index)
{
	return index >= 0 && index < count() ? _items.takeAt(index) : nullptr;
}

void COverlayLayout::setGeometry(const QRect & rect)
{
	for (QLayoutItem* item : _items)
		item->setGeometry(rect);
}

QSize COverlayLayout::sizeHint() const
{
	QSize size;
	for (const QLayoutItem* item: _items)
		size = size.expandedTo(item->minimumSize());

	return size;
}

Qt::Orientations COverlayLayout::expandingDirections() const
{
	return Qt::Horizontal | Qt::Vertical;
}
