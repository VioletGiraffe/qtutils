#include "ctabstrip.h"
#include "layouts/cflowlayout.h"
#include "assert/advanced_assert.h"

DISABLE_COMPILER_WARNINGS
#include <QApplication>
#include <QFrame>
#include <QIcon>
#include <QMouseEvent>
#include <QPushButton>
#include <QResizeEvent>
#include <QStyle>
#include <QStyleOptionButton>
#include <QStylePainter>
RESTORE_COMPILER_WARNINGS

#include <algorithm>
#include <limits>

// A tab's text is at most this many average characters wide; a longer text loses its middle
static constexpr int kMaxTextChars = 28;
// The strip's minimum width allows a tab this many average characters wide
static constexpr int kMinTextChars = 4;
static constexpr int kCloseButtonGap = 4;
static constexpr int kCloseIconSize = 16;
static constexpr int kDropMarkerWidth = 2;

// Never sets QAbstractButton's text: it would take a mnemonic from a '&' in the tab's text.
// Selection and dragging bypass QAbstractButton's press handling, so a tab is never down.
class CTabStrip::Tab final : public QPushButton
{
public:
	Tab(CTabStrip& strip, const QString& text);

	void setFullText(const QString& text);

	qulonglong tabData = 0;

	[[nodiscard]] QSize sizeHint() const override;
	[[nodiscard]] QSize minimumSizeHint() const override;

protected:
	void paintEvent(QPaintEvent* event) override;
	void resizeEvent(QResizeEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;

private:
	[[nodiscard]] QSize sizeForTextWidth(int textWidth) const;
	[[nodiscard]] QRect contentsRectFromStyle() const;

private:
	CTabStrip& _strip;
	QPushButton* _closeButton = nullptr;
	QString _fullText;
};

CTabStrip::Tab::Tab(CTabStrip& strip, const QString& text) :
	QPushButton(&strip),
	_strip(strip),
	_closeButton(new QPushButton(this))
{
	setCheckable(true);
	setFocusPolicy(Qt::NoFocus);
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	setFullText(text);

	_closeButton->setFocusPolicy(Qt::NoFocus);
	_closeButton->setToolTip(CTabStrip::tr("Close tab"));
	// A stylesheet icon replaces this one when drawn, but QPushButton's size hint counts only its own icon
	_closeButton->setIcon(style()->standardIcon(QStyle::SP_TabCloseButton, nullptr, this));
	_closeButton->setIconSize(QSize{ kCloseIconSize, kCloseIconSize });
	assert_r(connect(_closeButton, &QPushButton::clicked, this, [this] { _strip.requestTabClosing(*this); }));
}

void CTabStrip::Tab::setFullText(const QString& text)
{
	_fullText = text;
	setAccessibleName(text);
	updateGeometry();
	update();
}

QSize CTabStrip::Tab::sizeHint() const
{
	ensurePolished();
	const QFontMetrics metrics = fontMetrics();
	return sizeForTextWidth(std::min(metrics.horizontalAdvance(_fullText), metrics.averageCharWidth() * kMaxTextChars));
}

QSize CTabStrip::Tab::minimumSizeHint() const
{
	ensurePolished();
	return sizeForTextWidth(fontMetrics().averageCharWidth() * kMinTextChars);
}

QSize CTabStrip::Tab::sizeForTextWidth(const int textWidth) const
{
	const QSize closeSize = _closeButton->sizeHint();
	const QSize contentsSize{ textWidth + kCloseButtonGap + closeSize.width(), std::max(fontMetrics().height(), closeSize.height()) };

	QStyleOptionButton option;
	initStyleOption(&option);
	return style()->sizeFromContents(QStyle::CT_PushButton, &option, contentsSize, this);
}

QRect CTabStrip::Tab::contentsRectFromStyle() const
{
	QStyleOptionButton option;
	initStyleOption(&option);
	return style()->subElementRect(QStyle::SE_PushButtonContents, &option, this);
}

void CTabStrip::Tab::paintEvent(QPaintEvent* /*event*/)
{
	QStyleOptionButton option;
	initStyleOption(&option);
	QStylePainter painter{ this };
	painter.drawControl(QStyle::CE_PushButtonBevel, option);

	// The text is drawn as a label beside the close button, at the text colour the stylesheet gives this state
	option.rect = contentsRectFromStyle();
	const QRect closeRect = _closeButton->geometry();
	if (isRightToLeft())
		option.rect.setLeft(closeRect.right() + 1 + kCloseButtonGap);
	else
		option.rect.setRight(closeRect.left() - 1 - kCloseButtonGap);

	// A '&' is doubled so that the style draws it instead of taking it as a mnemonic
	option.text = option.fontMetrics.elidedText(_fullText, Qt::ElideMiddle, option.rect.width()).replace(QLatin1Char('&'), QStringLiteral("&&"));
	painter.drawControl(QStyle::CE_PushButtonLabel, option);
}

void CTabStrip::Tab::resizeEvent(QResizeEvent* event)
{
	QPushButton::resizeEvent(event);
	_closeButton->setGeometry(QStyle::alignedRect(layoutDirection(), Qt::AlignRight | Qt::AlignVCenter, _closeButton->sizeHint(), contentsRectFromStyle()));
}

void CTabStrip::Tab::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
		_strip.tabPressed(*this, mapTo(&_strip, event->position().toPoint()));
}

void CTabStrip::Tab::mouseMoveEvent(QMouseEvent* event)
{
	if (event->buttons() & Qt::LeftButton)
		_strip.tabDragged(mapTo(&_strip, event->position().toPoint()));
}

void CTabStrip::Tab::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
		_strip.tabReleased(mapTo(&_strip, event->position().toPoint()));
	// On release: a press dragged off the tab closes nothing
	else if (event->button() == Qt::MiddleButton && rect().contains(event->position().toPoint()))
		_strip.requestTabClosing(*this);
}

CTabStrip::CTabStrip(QWidget* parent) :
	QWidget(parent),
	_layout(new CFlowLayout(this, 0)),
	_dropMarker(new QFrame(this))
{
	_dropMarker->hide();
}

int CTabStrip::addTab(const QString& text)
{
	auto* tab = new Tab(*this, text);
	_tabs.push_back(tab);
	_layout->addWidget(tab);

	const int index = count() - 1;
	if (!_currentTab)
		setCurrentIndex(index);
	return index;
}

void CTabStrip::removeTab(const int index)
{
	assert_and_return_r(index >= 0 && index < count(), );

	Tab* tab = _tabs[(size_t)index];
	if (tab == _pressedTab)
		endDrag();

	_tabs.erase(_tabs.begin() + index);
	_layout->removeWidget(tab);
	tab->hide();
	// Deleted later: the removal may run in a handler of the tab or of its close button
	tab->deleteLater();

	if (tab == _currentTab)
	{
		_currentTab = nullptr;
		if (!_tabs.empty())
			setCurrentIndex(std::min(index, count() - 1));
	}
}

int CTabStrip::count() const
{
	return (int)_tabs.size();
}

int CTabStrip::currentIndex() const
{
	return _currentTab ? indexOf(*_currentTab) : -1;
}

void CTabStrip::setCurrentIndex(const int index)
{
	assert_and_return_r(index >= 0 && index < count(), );

	Tab* tab = _tabs[(size_t)index];
	if (tab == _currentTab)
		return;

	if (_currentTab)
		_currentTab->setChecked(false);
	_currentTab = tab;
	tab->setChecked(true);
	emit currentChanged(index);
}

void CTabStrip::setTabText(const int index, const QString& text)
{
	assert_and_return_r(index >= 0 && index < count(), );
	_tabs[(size_t)index]->setFullText(text);
}

void CTabStrip::setTabData(const int index, const qulonglong value)
{
	assert_and_return_r(index >= 0 && index < count(), );
	_tabs[(size_t)index]->tabData = value;
}

qulonglong CTabStrip::tabData(const int index) const
{
	assert_and_return_r(index >= 0 && index < count(), 0);
	return _tabs[(size_t)index]->tabData;
}

int CTabStrip::tabAt(const QPoint pos) const
{
	const auto tab = std::ranges::find_if(_tabs, [pos](const Tab* t) { return t->geometry().contains(pos); });
	return tab != _tabs.end() ? (int)(tab - _tabs.begin()) : -1;
}

void CTabStrip::setSpacing(const int spacing)
{
	_layout->setSpacing(spacing);
}

CTabStrip::DropTarget CTabStrip::dropTargetAt(const QPoint pos) const
{
	assert_debug_only(!_tabs.empty());

	// A row is a run of tabs with the same top. The target row is the one nearest to pos vertically.
	size_t rowStart = 0;
	int rowDistance = std::numeric_limits<int>::max();
	for (size_t i = 0; i < _tabs.size(); ++i)
	{
		if (i > 0 && _tabs[i]->y() == _tabs[i - 1]->y())
			continue;

		const QRect tabRect = _tabs[i]->geometry();
		const int distance = pos.y() < tabRect.top() ? tabRect.top() - pos.y() : std::max(pos.y() - tabRect.bottom(), 0);
		if (distance < rowDistance)
		{
			rowDistance = distance;
			rowStart = i;
		}
	}

	const int rowTop = _tabs[rowStart]->y();
	size_t slot = rowStart;
	while (slot < _tabs.size() && _tabs[slot]->y() == rowTop && pos.x() >= _tabs[slot]->geometry().center().x())
		++slot;

	// The marker is centred in the gap before the slot's tab, or after the row's last tab
	const bool afterRow = slot == _tabs.size() || _tabs[slot]->y() != rowTop;
	const QRect neighbour = _tabs[afterRow ? slot - 1 : slot]->geometry();
	const int spacing = std::max(_layout->horizontalSpacing(), 0);
	const int gapCentre = afterRow ? neighbour.right() + 1 + spacing / 2 : neighbour.left() - (spacing + 1) / 2;
	const int markerLeft = std::clamp(gapCentre - kDropMarkerWidth / 2, 0, std::max(width() - kDropMarkerWidth, 0));
	return { (int)slot, QRect{ markerLeft, neighbour.top(), kDropMarkerWidth, neighbour.height() } };
}

int CTabStrip::indexOf(const Tab& tab) const
{
	const auto it = std::ranges::find(_tabs, &tab);
	return it != _tabs.end() ? (int)(it - _tabs.begin()) : -1;
}

void CTabStrip::tabPressed(Tab& tab, const QPoint pos)
{
	// A drag whose release never arrived, such as one cut off by a lost mouse grab, ends here
	endDrag();
	_pressedTab = &tab;
	_pressPos = pos;
	setCurrentIndex(indexOf(tab));
}

void CTabStrip::tabDragged(const QPoint pos)
{
	if (!_pressedTab)
		return;
	if (!_dragging && (pos - _pressPos).manhattanLength() < QApplication::startDragDistance())
		return;

	_dragging = true;
	const DropTarget target = dropTargetAt(pos);
	const int from = indexOf(*_pressedTab);
	_dropMarker->setGeometry(target.marker);
	_dropMarker->raise();
	// No marker where a drop leaves the tab in place
	_dropMarker->setVisible(target.slot != from && target.slot != from + 1);
}

void CTabStrip::tabReleased(const QPoint pos)
{
	if (!_pressedTab)
		return;

	const bool dragged = _dragging;
	const int from = indexOf(*_pressedTab);
	endDrag();
	if (!dragged)
		return;

	const int slot = dropTargetAt(pos).slot;
	const int to = slot > from ? slot - 1 : slot;
	if (to == from)
		return;

	Tab* tab = _tabs[(size_t)from];
	_tabs.erase(_tabs.begin() + from);
	_tabs.insert(_tabs.begin() + to, tab);
	_layout->removeWidget(tab);
	_layout->insertWidget(to, tab);
	emit tabMoved(from, to);
}

void CTabStrip::requestTabClosing(const Tab& tab)
{
	emit tabCloseRequested(indexOf(tab));
}

void CTabStrip::endDrag()
{
	_pressedTab = nullptr;
	_dragging = false;
	_dropMarker->hide();
}
