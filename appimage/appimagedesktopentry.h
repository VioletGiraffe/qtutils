#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QString>
RESTORE_COMPILER_WARNINGS

#include <expected>

class QMenu;
class QWidget;

// The user's applications menu entry for the AppImage this process runs from.
// The entry's file ID is QGuiApplication::desktopFileName(), which must be set first:
//   - the AppImage's own desktop file must be named after it;
//   - desktops match the app's windows to the entry by it.
namespace AppImageDesktopEntry {

[[nodiscard]] bool runningFromAppImage();
// False for an entry that launches another AppImage file
[[nodiscard]] bool installedForThisAppImage();

// Requires runningFromAppImage(). Replaces an entry for another AppImage file.
[[nodiscard]] std::expected<void, QString> install();
[[nodiscard]] std::expected<void, QString> uninstall();

// Adds an install / uninstall toggle to menu, only when runningFromAppImage(). Failures are reported over dialogParent.
// The toggle belongs in the Help menu: offerOnce() names that menu.
void addToggleAction(QMenu& menu, QWidget* dialogParent);
// Asks once per settings store, only when runningFromAppImage() and no entry is installed for this AppImage file.
// Returns at once: the question is not modal to the event loop.
void offerOnce(QWidget* parent);

} // namespace AppImageDesktopEntry
