#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QMessageBox>
#include <QStringList>
RESTORE_COMPILER_WARNINGS

#include <optional>

namespace MessageBox {

// Poses `text` with one button per entry of `options` - arbitrary labels, unlike QMessageBox::question's
// fixed Yes/No/... buttons - and returns the 0-based index of the chosen option, matching `options` order,
// or nullopt if the user cancelled or dismissed the dialog. `defaultIndex` is the option pre-selected for
// Enter. With `cancellable`, a Cancel button is added and Escape maps to it; without it the user must pick.
[[nodiscard]] std::optional<int> question(QWidget* parent, const QString& title, const QString& text,
	const QStringList& options, int defaultIndex = 0, bool cancellable = true,
	QMessageBox::Icon icon = QMessageBox::Question);

// An OK-only message box that shows `text` above `details` in a scrollable, height-capped body, where a plain
// QMessageBox instead grows to fit its whole message. Use it wherever the message carries a part whose length is
// driven by input rather than fixed - one line per offending file, per failure - so the box can't outgrow the screen.
// `details` is plain text the caller has already assembled, separator included; it is selectable, so paths can be
// copied out of it. Empty `details` (the process that failed had nothing to say) degrades to a plain message box,
// so a caller that may or may not have output needn't branch.
void notice(QWidget* parent, const QString& title, const QString& text, const QString& details,
	QMessageBox::Icon icon = QMessageBox::Warning);

}
