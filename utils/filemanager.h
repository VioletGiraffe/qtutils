#pragma once

// Requires QT += dbus on Linux and FreeBSD in every application that calls these: the reveal calls org.freedesktop.FileManager1.

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QString>
RESTORE_COMPILER_WARNINGS

// Which entry a path through a link reveals: the link's final target, or the link itself.
enum class LinkHandling
{
	Follow,
	NoFollow
};

// Selects path in the platform file manager. False means only that path did not exist; the handoff itself is best-effort,
// and a failed one opens the containing folder instead.
[[nodiscard]] bool revealInFileManager(const QString& path, LinkHandling links = LinkHandling::Follow);

// Platform-native menu wording for revealInFileManager.
[[nodiscard]] QString revealInFileManagerActionText();

// Opens the folder's contents, unlike revealInFileManager, which selects it in its parent.
// False means only that folderPath is not an existing directory; the handoff itself is best-effort.
[[nodiscard]] bool openFolderInFileManager(const QString& folderPath);
