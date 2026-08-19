#include "cstylefixups.h"

#include "assert/advanced_assert.h"

DISABLE_COMPILER_WARNINGS
#include <QAbstractItemView>
#include <QComboBox>
#include <QEvent>
#include <QPainter>
#include <QPen>
#include <QPushButton>
#include <QStyleOption>
RESTORE_COMPILER_WARNINGS

#include <utility>

namespace {

// Qt parents a combo's popup container to the combo and gives it Qt::Popup. Both hold under any name
// Qt gives that private class, which is what makes this a usable cross-check on the name match below.
[[nodiscard]] bool isPopupOwnedByComboBox(const QWidget& w)
{
	return w.windowType() == Qt::Popup && qobject_cast<QComboBox*>(w.parentWidget()) != nullptr;
}

} // namespace

CComboPopupRounder::CComboPopupRounder(FrameProvider frame, QObject* parent) :
	QObject{ parent },
	_frame{ std::move(frame) }
{
	assert_r(_frame);
}

bool CComboPopupRounder::eventFilter(QObject* watched, QEvent* event)
{
	const QEvent::Type type = event->type();
	if (!watched->isWidgetType() || (type != QEvent::Show && type != QEvent::Paint))
		return QObject::eventFilter(watched, event);

	QWidget* container = static_cast<QWidget*>(watched);
	const bool isComboContainer = container->inherits("QComboBoxPrivateContainer");
	if (type == QEvent::Show)
	{
		// Failing this means Qt renamed the class and none of the popup styling runs any more.
		if (isPopupOwnedByComboBox(*container))
			assert_r(isComboContainer);

		if (isComboContainer)
		{
			container->setAttribute(Qt::WA_TranslucentBackground);
			QAbstractItemView* view = container->findChild<QAbstractItemView*>();
			assert_and_return_r(view != nullptr, QObject::eventFilter(watched, event));
			view->setStyleSheet(QStringLiteral("background: transparent; border: none;"));
			view->viewport()->setAutoFillBackground(false);
		}
	}
	else if (isComboContainer && container->isWindow())
	{
		const Frame frame = _frame();
		QPainter p{ container };
		p.setRenderHint(QPainter::Antialiasing);
		const QRectF r = QRectF{ container->rect() }.adjusted(0.5, 0.5, -0.5, -0.5);
		p.setPen(QPen{ frame.borderColor, 1.0 });
		p.setBrush(container->palette().base());
		p.drawRoundedRect(r, frame.radius, frame.radius);
		return true;
	}
	return QObject::eventFilter(watched, event);
}

bool CSplitterHandleHoverEnabler::eventFilter(QObject* watched, QEvent* event)
{
	if (event->type() == QEvent::Polish && watched->isWidgetType() && watched->inherits("QSplitterHandle"))
		static_cast<QWidget*>(watched)->setAttribute(Qt::WA_Hover);
	return QObject::eventFilter(watched, event);
}

CFocusFrameStyle::CFocusFrameStyle(OutsetProvider outset, QStyle* baseStyle) :
	QProxyStyle{ baseStyle },
	_outset{ std::move(outset) }
{
	assert_r(_outset);
}

void CFocusFrameStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter,
                                     const QWidget* widget) const
{
	if (element == PE_FrameFocusRect && qobject_cast<const QPushButton*>(widget))
	{
		if (const auto* focusOption = qstyleoption_cast<const QStyleOptionFocusRect*>(option))
		{
			QStyleOptionFocusRect widened{ *focusOption };
			const int pad = _outset();
			widened.rect = focusOption->rect.adjusted(-pad, -pad, pad, pad).intersected(widget->rect().adjusted(1, 1, -1, -1));
			QProxyStyle::drawPrimitive(element, &widened, painter, widget);
			return;
		}
	}
	QProxyStyle::drawPrimitive(element, option, painter, widget);
}
