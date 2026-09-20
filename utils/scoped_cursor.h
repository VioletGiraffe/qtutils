#pragma once

#include "compiler/compiler_warnings_control.h"
#include "lang/utils.hpp"

DISABLE_COMPILER_WARNINGS
#include <QCursor>
#include <QGuiApplication>
RESTORE_COMPILER_WARNINGS

// Override cursors stack application-wide: an unmatched restore pops an outer scope's.
struct ScopedWaitCursor
{
	NON_MOVABLE(ScopedWaitCursor);

	ScopedWaitCursor() { QGuiApplication::setOverrideCursor(Qt::WaitCursor); }
	~ScopedWaitCursor() { QGuiApplication::restoreOverrideCursor(); }
};
