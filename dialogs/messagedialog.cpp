#include "messagedialog.h"

#include "assert/advanced_assert.h"

DISABLE_COMPILER_WARNINGS
#include <QAbstractButton>
#include <QAccessible>
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

namespace MessageDialog {

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

// Sends QAccessible::Alert on show, matching QMessageBox.
class AlertDialog final : public QDialog
{
public:
	// Without `dismissable`, Escape and the window's close button do nothing.
	AlertDialog(QWidget* parent, bool dismissable) : QDialog(parent), _dismissable(dismissable) {}

	void reject() override
	{
		if (_dismissable)
			QDialog::reject();
	}

protected:
	void showEvent(QShowEvent* e) override
	{
		QDialog::showEvent(e);
#if QT_CONFIG(accessibility)
		QAccessibleEvent event(this, QAccessible::Alert);
		QAccessible::updateAccessibility(&event);
#endif
	}

private:
	const bool _dismissable;
};

// Titles `dialog` and lays out the style's icon beside `text`, with `details` below in a read-only view whose height
// is capped. Returns the layout for the caller to append its buttons to.
QVBoxLayout* buildAlertBody(QDialog& dialog, const QString& title, const QString& text, const QString& details, QMessageBox::Icon icon)
{
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

	return layout;
}

// The index of `clicked` among `optionButtons`; nullopt for Cancel, Escape, or a closed dialog.
std::optional<int> indexOfOption(const std::vector<QPushButton*>& optionButtons, const QAbstractButton* clicked)
{
	const auto it = std::find(optionButtons.cbegin(), optionButtons.cend(), clicked);
	return it != optionButtons.cend() ? std::optional<int>{ static_cast<int>(it - optionButtons.cbegin()) } : std::nullopt;
}

}

std::optional<int> question(QWidget* parent, const QString& title, const QString& text,
	const QStringList& options, int defaultIndex, bool cancellable, QMessageBox::Icon icon, const QString& details)
{
	assert_and_return_r(!options.empty(), std::nullopt);

	std::vector<QPushButton*> optionButtons;
	optionButtons.reserve(static_cast<size_t>(options.size()));
	const bool hasDefault = defaultIndex >= 0 && defaultIndex < static_cast<int>(options.size());

	if (details.isEmpty())
	{
		// A plain QMessageBox stays native where the platform has a native one.
		QMessageBox box(icon, title, text, QMessageBox::NoButton, parent);

		// All option buttons share one role so QDialogButtonBox keeps them contiguous and in insertion order on
		// every platform - that is what lets the returned index map back to `options`. Cancel alone takes
		// RejectRole, so Escape maps to it and each platform still positions it conventionally.
		for (const QString& label : options)
			optionButtons.push_back(box.addButton(label, QMessageBox::ActionRole));

		if (cancellable)
			box.addButton(QMessageBox::Cancel);

		if (hasDefault)
			box.setDefaultButton(optionButtons[static_cast<size_t>(defaultIndex)]);

		box.exec();
		return indexOfOption(optionButtons, box.clickedButton());
	}

	AlertDialog dialog(parent, cancellable);
	QVBoxLayout* layout = buildAlertBody(dialog, title, text, details, icon);

	QDialogButtonBox* buttons = new QDialogButtonBox(&dialog);
	for (const QString& label : options)
		optionButtons.push_back(buttons->addButton(label, QDialogButtonBox::ActionRole));

	if (cancellable)
		buttons->addButton(QDialogButtonBox::Cancel);

	if (hasDefault)
	{
		QPushButton* defaultButton = optionButtons[static_cast<size_t>(defaultIndex)];
		defaultButton->setDefault(true);
		// Focused as in QMessageBox: the details view comes first in the focus chain.
		defaultButton->setFocus();
	}

	// Cancel is recorded like an option and maps to nullopt.
	// Escape calls reject() and records nothing.
	const QAbstractButton* clicked = nullptr;
	QObject::connect(buttons, &QDialogButtonBox::clicked, &dialog, [&](QAbstractButton* button) {
		clicked = button;
		dialog.accept();
	});

	layout->addWidget(buttons);
	dialog.exec();
	return indexOfOption(optionButtons, clicked);
}

void notice(QWidget* parent, const QString& title, const QString& text, const QString& details, QMessageBox::Icon icon)
{
	AlertDialog dialog(parent, /*dismissable=*/true);
	QVBoxLayout* layout = buildAlertBody(dialog, title, text, details, icon);

	QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok, &dialog);
	QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
	layout->addWidget(buttons);

	dialog.exec();
}

}
