#pragma once

#include "assert/advanced_assert.h"
#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QObject>
#include <QString>
RESTORE_COMPILER_WARNINGS

#include <algorithm>
#include <ranges>

// Persists the user's theming choices and announces when the theme in effect changes: a scheme
// preference (system / light / dark) and one theme name per polarity. The names are stored as opaque
// strings - the app owns the theme list, and selectedTheme() resolves the stored name against it.
// Of a theme, this reads the name and the polarity only.
//
// Qt owns the polarity itself, through QStyleHints::setColorScheme(), so the platform follows too -
// on Windows that includes the title bar. The preference is kept separately because Qt reports the
// effective scheme, which cannot express "following the system".
class CThemeController final : public QObject
{
	Q_OBJECT

public:
	// Must not be called before the QApplication exists.
	[[nodiscard]] static CThemeController& instance();

	// Persists to QSettings{ organization, application } instead of the application's own store.
	// Must be called before the first instance().
	static void setSettingsLocation(const QString& organization, const QString& application);

	// Qt::ColorScheme::Unknown means "follow the system".
	[[nodiscard]] Qt::ColorScheme schemePreference() const { return _schemePreference; }
	void setSchemePreference(Qt::ColorScheme preference);

	[[nodiscard]] bool darkActive() const;

	// The stored selection for that polarity, verbatim - possibly empty, possibly naming a theme
	// that no longer exists.
	[[nodiscard]] QString themeName(bool dark) const;
	void setThemeName(bool dark, const QString& name);

	struct Selection
	{
		Qt::ColorScheme schemePreference = Qt::ColorScheme::Unknown;
		QString lightThemeName;
		QString darkThemeName;
	};

	// For a settings dialog that applies the choices as they are picked: restoring the saved selection undoes them on cancel.
	[[nodiscard]] Selection selection() const;
	void setSelection(const Selection& selection);

	// The theme of 'themes' in effect: the one stored for the polarity on screen, or else the first of that polarity.
	// A fresh install and a stored name that outlived its theme both resolve to that first one.
	// A theme type has `name` and `dark` members. Requires a theme of each polarity. Returns a reference into 'themes'.
	template <std::ranges::forward_range Themes>
	[[nodiscard]] const std::ranges::range_value_t<Themes>& selectedTheme(const Themes& themes) const
	{
		const bool dark = darkActive();
		const QString storedName = themeName(dark);

		auto theme = std::ranges::find_if(themes, [&](const auto& t) { return t.dark == dark && t.name == storedName; });
		if (theme == std::ranges::end(themes))
			theme = std::ranges::find_if(themes, [dark](const auto& t) { return t.dark == dark; });

		assert_and_return_r(theme != std::ranges::end(themes), *std::ranges::begin(themes));
		return *theme;
	}

signals:
	// The polarity flipped, or the on-screen polarity's selection changed. The app re-resolves and
	// re-applies; anything caching theme-derived state (highlighting formats, pixmaps) rebuilds here.
	void themeChanged();

private:
	CThemeController();

private:
	QString _lightThemeName;
	QString _darkThemeName;
	Qt::ColorScheme _schemePreference = Qt::ColorScheme::Unknown;
};
