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

}
