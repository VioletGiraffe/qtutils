#include "cfindmatchespopup.h"
#include "assert/advanced_assert.h"

DISABLE_COMPILER_WARNINGS
#include <QAbstractListModel>
#include <QApplication>
#include <QKeyEvent>
#include <QPainter>
#include <QStyle>
#include <QStyledItemDelegate>
RESTORE_COMPILER_WARNINGS

#include <algorithm>
#include <utility>

namespace {

constexpr int maxMatchListRows = 1'000'000; // Rows past it are not listed: QAbstractItemModel counts rows in int

// The display role holds the 1-based row number, for keyboard search; the delegate paints the rest
class MatchListModel final : public QAbstractListModel
{
public:
	MatchListModel(int rowCount, QObject* parent) : QAbstractListModel{ parent }, _rowCount{ rowCount } {}

	[[nodiscard]] int rowCount(const QModelIndex& parent = {}) const override { return parent.isValid() ? 0 : _rowCount; }

	[[nodiscard]] QVariant data(const QModelIndex& index, int role) const override
	{
		return role == Qt::DisplayRole && index.isValid() ? QVariant{ QString::number(index.row() + 1) } : QVariant{};
	}

private:
	const int _rowCount;
};

// Paints a row as its number, the match's location and an excerpt with the match in bold
class MatchRowDelegate final : public QStyledItemDelegate
{
public:
	using DescribeRow = std::function<MatchDescription (int row)>;

	MatchRowDelegate(DescribeRow describeRow, int rowCount, QObject* parent) :
		QStyledItemDelegate{ parent },
		_describeRow{ std::move(describeRow) },
		_rowCount{ rowCount }
	{}

	void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
	const DescribeRow _describeRow;
	const int _rowCount;
	mutable int _locationColumnWidth = -1; // Measured on the first paint
};

void MatchRowDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	QStyleOptionViewItem opt = option;
	initStyleOption(&opt, index);
	const QWidget* const widget = opt.widget;
	const QStyle* const style = widget ? widget->style() : QApplication::style();
	style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, widget);

	const bool selected = opt.state.testFlag(QStyle::State_Selected);
	const QColor textColor = opt.palette.color(selected ? QPalette::HighlightedText : QPalette::Text);
	const QColor dimColor = selected ? textColor : opt.palette.color(QPalette::PlaceholderText);

	const QFontMetrics metrics{ opt.font };
	QFont boldFont = opt.font;
	boldFont.setBold(true);
	const QFontMetrics boldMetrics{ boldFont };

	// Matches are in content order, so the last row has the widest line number or offset
	if (_locationColumnWidth < 0)
		_locationColumnWidth = metrics.horizontalAdvance(_describeRow(_rowCount - 1).location);

	const int textMargin = style->pixelMetric(QStyle::PM_FocusFrameHMargin, nullptr, widget) + 1;
	const QRect textRect = opt.rect.adjusted(textMargin, 0, -textMargin, 0);
	const int gap = 2 * metrics.horizontalAdvance(QLatin1Char{ ' ' });
	const int numberColumnWidth = metrics.horizontalAdvance(QString::number(_rowCount));
	int x = textRect.left();
	const auto remainingWidth = [&] { return std::max(textRect.right() + 1 - x, 0); };

	const auto drawText = [&](const QString& text, int width, Qt::Alignment alignment = Qt::AlignLeft) {
		painter->drawText(QRect{ x, textRect.top(), width, textRect.height() }, alignment | Qt::AlignVCenter, text);
		x += width;
	};

	const MatchDescription description = _describeRow(index.row());

	painter->save();
	painter->setFont(opt.font);
	painter->setPen(dimColor);
	drawText(QString::number(index.row() + 1), numberColumnWidth, Qt::AlignRight);
	x += gap;
	drawText(description.location, std::max(_locationColumnWidth, metrics.horizontalAdvance(description.location)));
	x += gap;

	QString excerpt = description.excerpt;
	for (QChar& ch : excerpt)
	{
		if (ch.isSpace() || ch.category() == QChar::Other_Control) // Tabs and line breaks would break the row's single line
			ch = QLatin1Char{ ' ' };
	}

	const qsizetype matchStart = std::clamp<qsizetype>(description.matchStart, 0, excerpt.size());
	const QString before = excerpt.left(matchStart);
	const QString match = excerpt.mid(matchStart, description.matchLength);
	const QString after = excerpt.mid(matchStart + match.size());

	// The match keeps its place in view: the text before it is elided first
	const int matchWidth = std::min(boldMetrics.horizontalAdvance(match), remainingWidth());
	const QString shownBefore = metrics.elidedText(before, Qt::ElideLeft, remainingWidth() - matchWidth);

	painter->setPen(textColor);
	drawText(shownBefore, metrics.horizontalAdvance(shownBefore));
	painter->setFont(boldFont);
	drawText(boldMetrics.elidedText(match, Qt::ElideRight, matchWidth), matchWidth);
	painter->setFont(opt.font);
	drawText(metrics.elidedText(after, Qt::ElideRight, remainingWidth()), remainingWidth());
	painter->restore();
}

} // namespace

CFindMatchesPopup::CFindMatchesPopup(std::vector<MatchRange> matches, int64_t currentIndex, DescribeMatch describeMatch, SelectMatch selectMatch, QWidget* parent) :
	QListView{ parent },
	_matches{ std::move(matches) },
	_describeMatch{ std::move(describeMatch) },
	_selectMatch{ std::move(selectMatch) }
{
	assert_r(!_matches.empty());
	if (_matches.size() > static_cast<size_t>(maxMatchListRows))
		_matches.resize(static_cast<size_t>(maxMatchListRows));

	setWindowFlags(Qt::Popup);
	setAttribute(Qt::WA_DeleteOnClose);
	setUniformItemSizes(true);
	setSelectionMode(QAbstractItemView::SingleSelection);
	setEditTriggers(QAbstractItemView::NoEditTriggers);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	const int rowCount = static_cast<int>(_matches.size());
	setModel(new MatchListModel{ rowCount, this });
	setItemDelegate(new MatchRowDelegate{ [this](int row) { return _describeMatch(_matches[static_cast<size_t>(row)]); }, rowCount, this });

	// Before the connection below: the host already has this match selected
	if (currentIndex >= 0 && currentIndex < rowCount)
		setCurrentIndex(model()->index(static_cast<int>(currentIndex), 0));

	const auto selectRowMatch = [this](const QModelIndex& row) {
		if (row.isValid())
			_selectMatch(row.row(), _matches[static_cast<size_t>(row.row())]);
	};
	connect(selectionModel(), &QItemSelectionModel::currentRowChanged, this, selectRowMatch);
	connect(this, &QAbstractItemView::clicked, this, selectRowMatch); // The current row too: the host may have scrolled away from its match
	connect(this, &QAbstractItemView::doubleClicked, this, &QWidget::close);
}

void CFindMatchesPopup::popUp(const QRect& globalArea, Qt::Edge anchorEdge)
{
	const int rowsHeight = sizeHintForRow(0) * model()->rowCount() + 2 * frameWidth();
	QRect popupRect = globalArea;
	if (rowsHeight < popupRect.height())
	{
		if (anchorEdge == Qt::BottomEdge)
			popupRect.setTop(popupRect.bottom() + 1 - rowsHeight);
		else
			popupRect.setHeight(rowsHeight);
	}

	setGeometry(popupRect);
	show();
	if (currentIndex().isValid())
		scrollTo(currentIndex(), QAbstractItemView::PositionAtCenter);
}

void CFindMatchesPopup::keyPressEvent(QKeyEvent* event)
{
	switch (event->key())
	{
	case Qt::Key_Return:
	case Qt::Key_Enter:
	case Qt::Key_Escape:
		close();
		return;
	default:
		QListView::keyPressEvent(event);
	}
}
