#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QHash>
#include <QString>
RESTORE_COMPILER_WARNINGS

#include <type_traits>

class QColor;
struct CBasePalette;

// Values for the @name@ placeholders of a stylesheet. QSS has no variables: a themed sheet is written with
// placeholders and expanded once per theme.
// A sheet must contain '@' in placeholders only.
class CStyleSheetTokens
{
public:
	// A name consists of ASCII letters, digits and underscores. Each name is added once.
	void add(const QString& name, const QString& value);
	// Translucent colours are written with their alpha.
	void add(const QString& name, const QColor& color);
	// Takes every arithmetic type: a number the overloads did not take would convert to a QColor through QRgb.
	// Not a plain int parameter: that would take a Qt::GlobalColor ahead of the QColor overload.
	template <typename Number> requires std::is_arithmetic_v<Number>
	void add(const QString& name, Number number) { add(name, QString::number(number)); }

	// One token per field, named after the field. The palette must be resolved.
	void addPalette(const CBasePalette& palette);

	// Fails on a placeholder that has no token, and on a stray '@': left in the sheet, either voids its rule silently.
	[[nodiscard]] QString expanded(const QString& sheet) const;

private:
	QHash<QString, QString> _values;
};
