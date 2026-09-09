#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QWidget>
RESTORE_COMPILER_WARNINGS

class CSettingsPage : public QWidget
{
public:
	using QWidget::QWidget;

	virtual void acceptSettings() = 0;

	// Undoes whatever a page applied the moment it was picked, without waiting for acceptSettings().
	// Called on every page when the dialog is cancelled, by the button, Escape or the close button.
	virtual void rejectSettings() {}
};
