#include "cthemeselectioncontrols.h"
#include "cthemecontroller.h"

DISABLE_COMPILER_WARNINGS
#include <QComboBox>
#include <QFormLayout>
RESTORE_COMPILER_WARNINGS

CThemeSelectionControls::CThemeSelectionControls(QFormLayout& form, const QStringList& lightThemeNames, const QStringList& darkThemeNames) :
	_initialScheme{ CThemeController::instance().schemePreference() },
	_initialLightTheme{ CThemeController::instance().themeName(false) },
	_initialDarkTheme{ CThemeController::instance().themeName(true) }
{
	auto* scheme = new QComboBox;
	scheme->addItem(tr("System"), static_cast<int>(Qt::ColorScheme::Unknown));
	scheme->addItem(tr("Light"), static_cast<int>(Qt::ColorScheme::Light));
	scheme->addItem(tr("Dark"), static_cast<int>(Qt::ColorScheme::Dark));
	scheme->setCurrentIndex(scheme->findData(static_cast<int>(_initialScheme)));
	QObject::connect(scheme, &QComboBox::currentIndexChanged, scheme, [scheme] {
		CThemeController::instance().setSchemePreference(static_cast<Qt::ColorScheme>(scheme->currentData().toInt()));
	});
	form.addRow(tr("Color scheme:"), scheme);

	const auto addThemeRow = [&form](const QString& label, bool dark, const QStringList& names, const QString& storedName) {
		auto* theme = new QComboBox;
		theme->addItems(names);
		if (const auto storedIndex = names.indexOf(storedName); storedIndex >= 0)
			theme->setCurrentIndex(static_cast<int>(storedIndex));
		QObject::connect(theme, &QComboBox::currentTextChanged, theme, [dark](const QString& name) {
			CThemeController::instance().setThemeName(dark, name);
		});
		form.addRow(label, theme);
	};
	addThemeRow(tr("Light theme:"), false, lightThemeNames, _initialLightTheme);
	addThemeRow(tr("Dark theme:"), true, darkThemeNames, _initialDarkTheme);
}

void CThemeSelectionControls::restoreInitialSelection() const
{
	// Names before the scheme: restoring the scheme can change the polarity, and the theme is then resolved from the names in force
	CThemeController& controller = CThemeController::instance();
	controller.setThemeName(false, _initialLightTheme);
	controller.setThemeName(true, _initialDarkTheme);
	controller.setSchemePreference(_initialScheme);
}
