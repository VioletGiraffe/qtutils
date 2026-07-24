#include "messagebox.h"

#include "assert/advanced_assert.h"

DISABLE_COMPILER_WARNINGS
#include <QAbstractButton>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFontMetrics>
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QStyle>
#include <QTextDocument>
#include <QVBoxLayout>
RESTORE_COMPILER_WARNINGS

#include <algorithm>
#include <vector>

namespace MessageBox {

namespace {

// The platform style's own message-box icon, so the box looks like every other one. Only called for a box that
// has an icon at all.
QStyle::StandardPixmap standardPixmapFor(QMessageBox::Icon icon)
{
	switch (icon)
	{
	case QMessageBox::Information: return QStyle::SP_MessageBoxInformation;
	case QMessageBox::Critical:    return QStyle::SP_MessageBoxCritical;
	case QMessageBox::Question:    return QStyle::SP_MessageBoxQuestion;
	default:                       return QStyle::SP_MessageBoxWarning;
	}
}

}

std::optional<int> question(QWidget* parent, const QString& title, const QString& text,
	const QStringList& options, int defaultIndex, bool cancellable, QMessageBox::Icon icon)
{
	assert_and_return_r(!options.empty(), std::nullopt);

	QMessageBox box(icon, title, text, QMessageBox::NoButton, parent);

	// All option buttons share one role so QDialogButtonBox keeps them contiguous and in insertion order on
	// every platform - that is what lets the returned index map back to `options`. Cancel alone takes
	// RejectRole, so Escape maps to it and each platform still positions it conventionally.
	std::vector<QPushButton*> optionButtons;
	optionButtons.reserve(static_cast<size_t>(options.size()));
	for (const QString& label : options)
		optionButtons.push_back(box.addButton(label, QMessageBox::ActionRole));

	if (cancellable)
		box.addButton(QMessageBox::Cancel);

	if (defaultIndex >= 0 && defaultIndex < static_cast<int>(optionButtons.size()))
		box.setDefaultButton(optionButtons[static_cast<size_t>(defaultIndex)]);

	box.exec();

	const QAbstractButton* clicked = box.clickedButton();
	for (size_t i = 0; i < optionButtons.size(); ++i)
		if (optionButtons[i] == clicked)
			return static_cast<int>(i);

	return std::nullopt;   // Cancel, Escape, or the dialog was closed
}

void notice(QWidget* parent, const QString& title, const QString& text, const QString& details, QMessageBox::Icon icon)
{
	QDialog dialog(parent);
	dialog.setWindowTitle(title);
	dialog.setWindowFlags(dialog.windowFlags() & ~Qt::WindowContextHelpButtonHint);

	QVBoxLayout* layout = new QVBoxLayout(&dialog);

	QHBoxLayout* headerRow = new QHBoxLayout;
	if (icon != QMessageBox::NoIcon)
	{
		const int iconSize = dialog.style()->pixelMetric(QStyle::PM_MessageBoxIconSize, nullptr, &dialog);
		QLabel* iconLabel = new QLabel(&dialog);
		iconLabel->setPixmap(dialog.style()->standardIcon(standardPixmapFor(icon), nullptr, &dialog)
			.pixmap(QSize{ iconSize, iconSize }, dialog.devicePixelRatio()));
		headerRow->addWidget(iconLabel, 0, Qt::AlignTop);
	}

	QLabel* textLabel = new QLabel(text, &dialog);
	textLabel->setWordWrap(true);
	headerRow->addWidget(textLabel, 1);
	layout->addLayout(headerRow);

	if (!details.isEmpty())
	{
		QPlainTextEdit* detailsView = new QPlainTextEdit(details, &dialog);
		detailsView->setReadOnly(true);
		// Wrap a long entry instead of scrolling sideways for it: vertical scrolling alone then reaches every entry,
		// and a wrapped path stays readable.
		detailsView->setLineWrapMode(QPlainTextEdit::WidgetWidth);

		const QFontMetrics metrics = detailsView->fontMetrics();
		detailsView->setMinimumWidth(metrics.averageCharWidth() * 80);   // a typical path fits without wrapping

		// The height is what bounds the box: past the cap the details scroll rather than growing the dialog
		// off-screen, while a short list still gets a small box. Counting explicit newlines under-counts entries
		// that wrap, which only brings the scrollbar in earlier - the bounded behaviour wanted anyway.
		constexpr int MIN_DETAIL_ROWS = 3;
		constexpr int MAX_DETAIL_ROWS = 12;
		const int rows = std::clamp(static_cast<int>(details.count('\n')) + 1, MIN_DETAIL_ROWS, MAX_DETAIL_ROWS);
		detailsView->setFixedHeight(rows * metrics.lineSpacing()
			+ 2 * (detailsView->frameWidth() + static_cast<int>(detailsView->document()->documentMargin())));

		layout->addWidget(detailsView);
	}

	QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok, &dialog);
	QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
	layout->addWidget(buttons);

	dialog.exec();
}

}
