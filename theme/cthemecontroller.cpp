#include "cthemecontroller.h"

#include "assert/advanced_assert.h"
#include "settings/csettings.h"

DISABLE_COMPILER_WARNINGS
#include <QGuiApplication>
#include <QStyleHints>
RESTORE_COMPILER_WARNINGS

namespace {

constexpr auto SchemeKey = "Theme/ColorScheme";
constexpr auto LightThemeKey = "Theme/LightTheme";
constexpr auto DarkThemeKey = "Theme/DarkTheme";

// Stored as text rather than the enum's number: reordering Qt::ColorScheme would otherwise silently
// repoint an existing user's setting at a different scheme.
QString schemeToString(Qt::ColorScheme scheme)
{
	switch (scheme)
	{
	case Qt::ColorScheme::Light: return QStringLiteral("light");
	case Qt::ColorScheme::Dark: return QStringLiteral("dark");
	case Qt::ColorScheme::Unknown: break;
	}
	return QStringLiteral("system");
}

Qt::ColorScheme schemeFromString(const QString& text)
{
	if (text == QLatin1String("light"))
		return Qt::ColorScheme::Light;
	if (text == QLatin1String("dark"))
		return Qt::ColorScheme::Dark;
	return Qt::ColorScheme::Unknown;
}

} // namespace

CThemeController& CThemeController::instance()
{
	assert_r(qApp != nullptr); // styleHints() and the settings storage both need the application
	static CThemeController controller;
	return controller;
}

CThemeController::CThemeController()
{
	const CSettings settings;
	_schemePreference = schemeFromString(settings.value(QLatin1String(SchemeKey)).toString());
	_lightThemeName = settings.value(QLatin1String(LightThemeKey)).toString();
	_darkThemeName = settings.value(QLatin1String(DarkThemeKey)).toString();

	QGuiApplication::styleHints()->setColorScheme(_schemePreference);

	connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged, this, [this] {
		emit themeChanged(); // the polarity flipped, so a different theme resolves by definition
	});
}

void CThemeController::setSchemePreference(Qt::ColorScheme preference)
{
	if (_schemePreference == preference)
		return;

	_schemePreference = preference;
	CSettings{}.setValue(QLatin1String(SchemeKey), schemeToString(preference));

	// Qt emits colorSchemeChanged if this alters the effective scheme, and that is what reaches
	// themeChanged(). When it does not - pinning Light while the system is already light - nothing
	// visual changed and there is nothing to announce.
	QGuiApplication::styleHints()->setColorScheme(preference);
}

bool CThemeController::darkActive() const
{
	return QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark;
}

QString CThemeController::themeName(bool dark) const
{
	return dark ? _darkThemeName : _lightThemeName;
}

void CThemeController::setThemeName(bool dark, const QString& name)
{
	QString& stored = dark ? _darkThemeName : _lightThemeName;
	if (stored == name)
		return;

	stored = name;
	CSettings{}.setValue(QLatin1String(dark ? DarkThemeKey : LightThemeKey), name);

	if (dark == darkActive())
		emit themeChanged(); // the other polarity's theme is not on screen, so nothing to redraw
}
