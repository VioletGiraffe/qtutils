#include "chighlightoverlay.h"

DISABLE_COMPILER_WARNINGS
#include <QPainter>
RESTORE_COMPILER_WARNINGS

namespace {

constexpr int FrameWidth = 2;
constexpr int FillAlpha = 40;

} // namespace

CHighlightOverlay::CHighlightOverlay(QWidget* parent, int cornerRadius) :
	QWidget{ parent },
	_cornerRadius{ cornerRadius }
{
	setAttribute(Qt::WA_TransparentForMouseEvents);
	hide(); // a child is shown with its parent unless it is explicitly hidden
}

void CHighlightOverlay::showOverParent()
{
	if (isVisible())
		return;

	setGeometry(parentWidget()->rect());
	raise();
	show();
}

void CHighlightOverlay::paintEvent(QPaintEvent*)
{
	const QColor accent = palette().color(QPalette::Accent);
	QColor fill = accent;
	fill.setAlpha(FillAlpha); // translucent: what is being marked stays legible

	QPainter painter{ this };
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setPen(QPen{ accent, FrameWidth });
	painter.setBrush(fill);
	// Half the pen inside the edge, so none of the frame falls outside the widget
	const qreal inset = FrameWidth / 2.0;
	painter.drawRoundedRect(QRectF{ rect() }.adjusted(inset, inset, -inset, -inset), _cornerRadius, _cornerRadius);
}
