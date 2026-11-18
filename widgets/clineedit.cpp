#include "clineedit.h"
#include "theme/cstylefixups.h"

DISABLE_COMPILER_WARNINGS
#include <QApplication>
#include <QFocusEvent>
#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>
#include <QRectF>
#include <QTimer>
RESTORE_COMPILER_WARNINGS

CLineEdit::~CLineEdit() = default;

void CLineEdit::setSelectAllOnFocus(bool select)
{
	_bSelectAllOnFocus = select;
}

bool CLineEdit::selectAllOnFocus() const
{
	return _bSelectAllOnFocus;
}

void CLineEdit::setBlockCursor(const std::optional<BlockCursor>& cursor)
{
	_blockCursor = cursor;
	setProperty(hiddenNativeTextCursorProperty, cursor.has_value());

	if (cursor && !_glowUnderlay)
	{
		_glowUnderlay = std::make_unique<CGlowUnderlay>(*this, [this]() -> std::optional<CGlowUnderlay::Glow> {
			const std::optional<BlockCell> cell = shownBlockCursor();
			if (!cell || _blockCursor->glowRadius <= 0)
				return {};

			QPainterPath block;
			block.addRect(QRectF{ cell->rect });
			return CGlowUnderlay::Glow{ .shape = block, .color = _blockCursor->fill, .radius = _blockCursor->glowRadius };
		});
	}

	if (cursor && !_blinkTimer)
	{
		_blinkTimer = new QTimer{ this };
		connect(_blinkTimer, &QTimer::timeout, this, [this] {
			_blockCursorShown = !_blockCursorShown;
			updateBlockCursorArea();
		});
		// Solid while the user types or moves it, as the native cursor is
		connect(this, &QLineEdit::cursorPositionChanged, this, &CLineEdit::restartBlockCursorBlinking);
		connect(this, &QLineEdit::textChanged, this, &CLineEdit::restartBlockCursorBlinking);
	}

	restartBlockCursorBlinking();
}

void CLineEdit::restartBlockCursorBlinking()
{
	_blockCursorShown = true;
	if (!_blinkTimer)
		return;

	// The flash time is a whole on-off cycle; 0 or less: no blinking
	const int halfCycle = QApplication::cursorFlashTime() / 2;
	if (_blockCursor && hasFocus() && halfCycle > 0)
		_blinkTimer->start(halfCycle);
	else
		_blinkTimer->stop();
	updateBlockCursorArea();
}

std::optional<CLineEdit::BlockCell> CLineEdit::shownBlockCursor() const
{
	if (!_blockCursor || !_blockCursorShown || !hasFocus() || isReadOnly())
		return {};

	const QString text = displayText();
	const qsizetype position = cursorPosition();
	QString character;
	if (position < text.size() && text[position] != QLatin1Char('\t'))
		character = text.mid(position, text[position].isHighSurrogate() ? 2 : 1);

	const QFontMetrics metrics = fontMetrics();
	const int width = metrics.horizontalAdvance(character.isEmpty() ? QStringLiteral(" ") : character);
	// cursorRect() is centred on the cursor, and its top is the text line's
	const QRect around = cursorRect();
	return BlockCell{ .rect = QRect{ around.center().x(), around.top(), width, metrics.height() }, .character = character };
}

void CLineEdit::updateBlockCursorArea()
{
	update();
	if (_glowUnderlay)
		_glowUnderlay->refresh();
}

void CLineEdit::focusInEvent(QFocusEvent * event)
{
	QLineEdit::focusInEvent(event);
	restartBlockCursorBlinking();

	if (!_bSelectAllOnFocus)
		return;

	// Only when focus moves into the field: focus returning from a popup, the menu bar or another window may find the text mid-edit
	switch (event->reason())
	{
	case Qt::MouseFocusReason:
	case Qt::TabFocusReason:
	case Qt::BacktabFocusReason:
	case Qt::ShortcutFocusReason:
	case Qt::OtherFocusReason:
		QMetaObject::invokeMethod(this, &CLineEdit::selectAll, Qt::QueuedConnection);
		break;
	default:
		break;
	}
}

void CLineEdit::focusOutEvent(QFocusEvent* event)
{
	QLineEdit::focusOutEvent(event);
	restartBlockCursorBlinking();
}

void CLineEdit::paintEvent(QPaintEvent* event)
{
	QLineEdit::paintEvent(event);
	paintBlockCursor();
}

void CLineEdit::paintBlockCursor()
{
	const std::optional<BlockCell> cell = shownBlockCursor();
	if (!cell)
		return;

	QPainter painter{ this };
	painter.fillRect(cell->rect, _blockCursor->fill);
	if (!cell->character.isEmpty())
	{
		painter.setFont(font());
		painter.setPen(_blockCursor->text);
		painter.drawText(QPoint{ cell->rect.left(), cell->rect.top() + fontMetrics().ascent() }, cell->character);
	}
}
