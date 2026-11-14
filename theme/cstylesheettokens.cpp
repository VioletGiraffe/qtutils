#include "cstylesheettokens.h"
#include "cbasepalette.h"

#include "assert/advanced_assert.h"

DISABLE_COMPILER_WARNINGS
#include <QColor>
#include <QRegularExpression>
RESTORE_COMPILER_WARNINGS

void CStyleSheetTokens::add(const QString& name, const QString& value)
{
	static const QRegularExpression validName{ QStringLiteral("^\\w+$") };
	assert_message_r(validName.match(name).hasMatch(), QString{ "Not a token name: " + name }.toStdString());
	assert_message_r(!_values.contains(name), QString{ "Token added twice: " + name }.toStdString());

	_values.insert(name, value);
}

void CStyleSheetTokens::add(const QString& name, const QColor& color)
{
	// An invalid colour would be written as black
	assert_message_r(color.isValid(), QString{ "Invalid colour for token " + name }.toStdString());
	add(name, color.name(color.alpha() == 255 ? QColor::HexRgb : QColor::HexArgb));
}

void CStyleSheetTokens::addPalette(const CBasePalette& palette)
{
	add(QStringLiteral("windowBg"), palette.windowBg);
	add(QStringLiteral("surface"), palette.surface);
	add(QStringLiteral("surfaceAlt"), palette.surfaceAlt);
	add(QStringLiteral("text"), palette.text);
	add(QStringLiteral("textDim"), palette.textDim);
	add(QStringLiteral("button"), palette.button);
	add(QStringLiteral("accent"), palette.accent);
	add(QStringLiteral("selectionBg"), palette.selectionBg);
	add(QStringLiteral("selectionFg"), palette.selectionFg);

	add(QStringLiteral("border"), palette.border);
	add(QStringLiteral("borderSubtle"), palette.borderSubtle);
	add(QStringLiteral("borderStrong"), palette.borderStrong);
	add(QStringLiteral("accentFg"), palette.accentFg);
	add(QStringLiteral("accentText"), palette.accentText);
	add(QStringLiteral("accentBg"), palette.accentBg);
	add(QStringLiteral("buttonBorder"), palette.buttonBorder);
	add(QStringLiteral("errorText"), palette.errorText);
}

QString CStyleSheetTokens::expanded(const QString& sheet) const
{
	static const QRegularExpression placeholder{ QStringLiteral("@(\\w+)@") };

	QString result;
	result.reserve(sheet.size());

	const auto appendSheetText = [&](qsizetype from, qsizetype to) {
		const QStringView text = QStringView{ sheet }.sliced(from, to - from);
		assert_message_r(!text.contains(u'@'),
			QString{ "Stray '@' in the stylesheet: " + text.mid(text.indexOf(u'@') - 20, 40).toString() }.toStdString());
		result += text;
	};

	qsizetype expandedUpTo = 0;
	for (auto matches = placeholder.globalMatch(sheet); matches.hasNext(); )
	{
		const QRegularExpressionMatch match = matches.next();
		appendSheetText(expandedUpTo, match.capturedStart(0));

		const auto value = _values.constFind(match.captured(1));
		if (value != _values.constEnd())
			result += *value;
		else
		{
			assert_unconditional_r(QString{ "No token for the placeholder " + match.captured(0) }.toStdString());
			result += match.capturedView(0);
		}

		expandedUpTo = match.capturedEnd(0);
	}
	appendSheetText(expandedUpTo, sheet.size());

	return result;
}
