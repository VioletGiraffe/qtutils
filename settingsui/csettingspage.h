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
};
