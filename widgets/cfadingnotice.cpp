#include "cfadingnotice.h"

DISABLE_COMPILER_WARNINGS
#include <QPainter>
#include <QVariantAnimation>
RESTORE_COMPILER_WARNINGS

namespace {

constexpr int DisplayMs = 1200;
constexpr qreal FadeStart = 0.7; // fraction of DisplayMs
constexpr qreal FontScale = 1.5;
constexpr int FillAlpha = 210; // the content shows through faintly
constexpr int CornerRadius = 8;

} // namespace

CFadingNotice::CFadingNotice(QWidget* parent) :
	QWidget{ parent },
	_opacityAnimation{ new QVariantAnimation{ this } }
{
	setAttribute(Qt::WA_TransparentForMouseEvents);
	hide(); // a child is shown with its parent unless it is explicitly hidden

	_opacityAnimation->setDuration(DisplayMs);
	_opacityAnimation->setKeyValueAt(0.0, 1.0);
	_opacityAnimation->setKeyValueAt(FadeStart, 1.0);
	_opacityAnimation->setKeyValueAt(1.0, 0.0);
	connect(_opacityAnimation, &QVariantAnimation::valueChanged, this, [this] { update(); });
	connect(_opacityAnimation, &QVariantAnimation::finished, this, &QWidget::hide);
}

void CFadingNotice::showMessage(const QString& text, const QRect& area)
{
	_text = text;

	// Derived from the parent's font on every call, so the notice follows font changes
	QFont noticeFont = parentWidget()->font();
	if (noticeFont.pointSizeF() > 0)
		noticeFont.setPointSizeF(noticeFont.pointSizeF() * FontScale);
	else
		noticeFont.setPixelSize(qRound(noticeFont.pixelSize() * FontScale));
	setFont(noticeFont);

	const int lineHeight = fontMetrics().height();
	QRect noticeRect{ QPoint{}, fontMetrics().size(Qt::TextSingleLine, text).grownBy(QMargins{ lineHeight, lineHeight / 2, lineHeight, lineHeight / 2 }) };
	noticeRect.moveCenter(area.center());
	setGeometry(noticeRect);

	_opacityAnimation->stop(); // start() does nothing on a running animation
	_opacityAnimation->start();
	raise();
	show();
	update();
}

void CFadingNotice::paintEvent(QPaintEvent*)
{
	QColor fill = palette().color(QPalette::Text);
	fill.setAlpha(FillAlpha);

	QPainter painter{ this };
	painter.setOpacity(_opacityAnimation->currentValue().toReal());
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setPen(Qt::NoPen);
	painter.setBrush(fill);
	painter.drawRoundedRect(rect(), CornerRadius, CornerRadius);

	painter.setPen(palette().color(QPalette::Base));
	painter.drawText(rect(), Qt::AlignCenter, _text);
}
