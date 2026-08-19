#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QObject>
#include <QString>
RESTORE_COMPILER_WARNINGS

// Persists the user's theming choices and announces when the theme in effect changes: a scheme
// preference (system / light / dark) and one theme name per polarity. The names are stored as opaque
// strings - the app owns the theme list and resolves a name against it, including falling back when
// a stored name no longer names anything. This never sees a theme, let alone a colour.
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

	// Qt::ColorScheme::Unknown means "follow the system".
	[[nodiscard]] Qt::ColorScheme schemePreference() const { return _schemePreference; }
	void setSchemePreference(Qt::ColorScheme preference);

	[[nodiscard]] bool darkActive() const;

	// The stored selection for that polarity, verbatim - possibly empty, possibly naming a theme
	// that no longer exists.
	[[nodiscard]] QString themeName(bool dark) const;
	void setThemeName(bool dark, const QString& name);

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
