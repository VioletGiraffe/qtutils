#include "cglowunderlay.h"

#include "assert/advanced_assert.h"
#include "utils/painterutils.h"

DISABLE_COMPILER_WARNINGS
#include <QEvent>
#include <QPainter>
#include <QWidget>
RESTORE_COMPILER_WARNINGS

#include <algorithm>
#include <utility>

CGlowUnderlay::CGlowUnderlay(QWidget& owner, GlowProvider provider) :
	_owner{ owner },
	_provider{ std::move(provider) }
{
	assert_r(_provider);
	_owner.installEventFilter(this);
	watchAncestors();
	refresh();
}

void CGlowUnderlay::refresh()
{
	if (_paintedIn)
		_paintedIn->update(_paintedArea);

	_glow = _provider();
	if (const std::optional<Placement> where = placement())
		where->painter->update(where->area);
}

std::optional<CGlowUnderlay::Placement> CGlowUnderlay::placement() const
{
	if (!_glow || _glow->radius <= 0 || _glow->shape.isEmpty() || !_owner.isVisible())
		return {};

	const QRect shapeRect = _glow->shape.boundingRect().toAlignedRect();
	for (QWidget* ancestor = _owner.parentWidget(); ancestor; ancestor = ancestor->parentWidget())
	{
		const QPoint ownerOrigin = _owner.mapTo(ancestor, QPoint{});
		const QRect shape = shapeRect.translated(ownerOrigin);
		const QRect bounds = ancestor->rect();
		const int room = std::min({ shape.left() - bounds.left(), shape.top() - bounds.top(), bounds.right() - shape.right(), bounds.bottom() - shape.bottom() });
		if (room < _glow->radius && !ancestor->isWindow())
			continue;

		const int radius = std::min(_glow->radius, room);
		if (radius <= 0)
			return {};
		return Placement{ .painter = ancestor, .ownerOrigin = ownerOrigin, .radius = radius, .area = shape.adjusted(-radius, -radius, radius, radius) };
	}

	return {};
}

void CGlowUnderlay::watchAncestors()
{
	std::vector<QWidget*> ancestors;
	for (QWidget* ancestor = _owner.parentWidget(); ancestor; ancestor = ancestor->isWindow() ? nullptr : ancestor->parentWidget())
		ancestors.emplace_back(ancestor);

	// The filters stay on the ancestors that remain: this can run inside an ancestor's ParentChange, and Qt documents removing a filter
	// during filter dispatch as safe, but not installing one
	for (QWidget* const former : _watchedAncestors)
	{
		if (former && std::ranges::find(ancestors, former) == ancestors.end())
			former->removeEventFilter(this);
	}
	for (QWidget* const ancestor : ancestors)
	{
		if (std::ranges::find(_watchedAncestors, ancestor, &QPointer<QWidget>::data) == _watchedAncestors.end())
			ancestor->installEventFilter(this);
	}
	_watchedAncestors.assign(ancestors.begin(), ancestors.end());
}

bool CGlowUnderlay::eventFilter(QObject* watched, QEvent* event)
{
	// The owner's and its ancestors' events: the glow's place in the painter depends on the geometry of all of them
	switch (event->type())
	{
	case QEvent::ParentChange:
		watchAncestors();
		refresh();
		return false;
	case QEvent::Move:
		// The window's position on the screen moves no glow within it
		if (!static_cast<QWidget*>(watched)->isWindow())
			refresh();
		return false;
	case QEvent::Resize:
	case QEvent::Show:
	case QEvent::Hide:
		refresh();
		return false;
	case QEvent::Paint:
		break;
	default:
		return false;
	}

	const std::optional<Placement> where = placement();
	if (!where || watched != where->painter)
		return false;

	// Before the ancestor paints itself; the widgets above paint over the glow wherever they are opaque
	QPainter painter{ where->painter };
	paintGlow(painter, _glow->shape.translated(where->ownerOrigin), _glow->color, where->radius, _glow->opacity);
	_paintedIn = where->painter;
	_paintedArea = where->area;
	return false;
}
