#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QCoreApplication>
#include <QString>
#include <QStringList>
RESTORE_COMPILER_WARNINGS

class QFormLayout;

// The theming rows of a settings page: the colour scheme, and one theme per polarity.
// Each choice goes to CThemeController the moment it is picked, so it is judged on the real windows.
class CThemeSelectionControls
{
	Q_DECLARE_TR_FUNCTIONS(CThemeSelectionControls)

public:
	// Adds the three rows to 'form'. A stored theme name that is not listed shows as the first name,
	// the theme CThemeController::selectedTheme() resolves it to.
	CThemeSelectionControls(QFormLayout& form, const QStringList& lightThemeNames, const QStringList& darkThemeNames);

	// Puts back the choices that were in force at construction. For CSettingsPage::rejectSettings().
	void restoreInitialSelection() const;

private:
	Qt::ColorScheme _initialScheme;
	QString _initialLightTheme;
	QString _initialDarkTheme;
};
