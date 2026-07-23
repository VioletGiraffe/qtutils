#include "messagebox.h"

#include "assert/advanced_assert.h"

DISABLE_COMPILER_WARNINGS
#include <QAbstractButton>
#include <QPushButton>
RESTORE_COMPILER_WARNINGS

#include <vector>

namespace MessageBox {

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

}
