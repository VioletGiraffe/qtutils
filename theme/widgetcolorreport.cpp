#include "widgetcolorreport.h"

DISABLE_COMPILER_WARNINGS
#include <QApplication>
#include <QGuiApplication>
#include <QImage>
#include <QMetaEnum>
#include <QPalette>
#include <QPixmap>
#include <QPointF>
#include <QProxyStyle>
#include <QScreen>
#include <QSize>
#include <QStringList>
#include <QStyle>
#include <QStyleHints>
#include <QWidget>
#include <QtGlobal> // qVersion, QT_VERSION_STR
RESTORE_COMPILER_WARNINGS

#include <vector>

namespace {

QString colorName(const QColor& color)
{
	return color.alpha() == 255 ? color.name() : color.name(QColor::HexArgb);
}

QString flag(bool value)
{
	return value ? QStringLiteral("yes") : QStringLiteral("no");
}

template <typename Enum>
QString enumKey(Enum value)
{
	return QString::fromLatin1(QMetaEnum::fromType<Enum>().valueToKey(int(value)));
}

QString describeObject(const QObject& object)
{
	const QString className = QString::fromLatin1(object.metaObject()->className());
	return object.objectName().isEmpty() ? className : className + '#' + object.objectName();
}

QString describeStyleChain(const QStyle* style)
{
	QStringList chain;
	while (style)
	{
		chain.append(QStringLiteral("%1 \"%2\"").arg(describeObject(*style), style->name()));
		// QApplication::setStyleSheet() parents the style it wraps to the stylesheet style
		const auto* proxy = qobject_cast<const QProxyStyle*>(style);
		style = proxy ? proxy->baseStyle() : style->findChild<QStyle*>(QString{}, Qt::FindDirectChildrenOnly);
	}
	return chain.join(QStringLiteral(" -> "));
}

std::vector<QPalette::ColorRole> colorRoles()
{
	std::vector<QPalette::ColorRole> roles;
	for (int role = 0; role < QPalette::NColorRoles; ++role)
	{
		if (role != QPalette::NoRole)
			roles.push_back(QPalette::ColorRole(role));
	}
	return roles;
}

bool isRoleSetInAnyGroup(const QPalette& palette, QPalette::ColorRole role)
{
	return palette.isBrushSet(QPalette::Active, role) || palette.isBrushSet(QPalette::Inactive, role) || palette.isBrushSet(QPalette::Disabled, role);
}

// Rows at a quarter, half and three quarters of the height, seven points across each
void appendPixelSamples(QString& report, const QImage& image, QSize logicalSize)
{
	if (image.isNull() || logicalSize.isEmpty())
	{
		report += QStringLiteral("    nothing grabbed\n");
		return;
	}

	// Scaled by the image width, which covers both grabs whatever devicePixelRatio() each carries
	const qreal scale = qreal(image.width()) / logicalSize.width();
	for (int row = 1; row <= 3; ++row)
	{
		const int y = logicalSize.height() * row / 4;
		QStringList colors;
		for (int column = 1; column <= 7; ++column)
		{
			const QPoint pixel = (QPointF{ logicalSize.width() * column / 8.0, qreal(y) } * scale).toPoint();
			// A screen grab is clipped to the screen: a widget partly off it yields a smaller image
			colors.append(image.rect().contains(pixel) ? colorName(image.pixelColor(pixel)) : QStringLiteral("outside"));
		}
		report += QStringLiteral("    y %1: %2\n").arg(y).arg(colors.join(' '));
	}
}

} // namespace

QString widgetColorReport(QWidget& widget)
{
	const std::vector<QPalette::ColorRole> roles = colorRoles();
	const QPalette& palette = widget.palette();

	QString report = QStringLiteral("Colour report for %1, %2x%3\n").arg(describeObject(widget)).arg(widget.width()).arg(widget.height());

	report += QStringLiteral("State: enabled %1, focus %2, under mouse %3, WA_Hover %4, active window %5, focus proxy %6\n").arg(
		flag(widget.isEnabled()), flag(widget.hasFocus()), flag(widget.underMouse()), flag(widget.testAttribute(Qt::WA_Hover)),
		flag(widget.isActiveWindow()), widget.focusProxy() ? describeObject(*widget.focusProxy()) : QStringLiteral("none"));
	report += QStringLiteral("Background: autoFillBackground %1, WA_StyledBackground %2, WA_OpaquePaintEvent %3, WA_NoSystemBackground %4, "
		"WA_TranslucentBackground %5, WA_SetPalette %6, backgroundRole %7, foregroundRole %8, palette resolve mask 0x%9\n").arg(
		flag(widget.autoFillBackground()), flag(widget.testAttribute(Qt::WA_StyledBackground)), flag(widget.testAttribute(Qt::WA_OpaquePaintEvent)),
		flag(widget.testAttribute(Qt::WA_NoSystemBackground)), flag(widget.testAttribute(Qt::WA_TranslucentBackground)),
		flag(widget.testAttribute(Qt::WA_SetPalette)), enumKey(widget.backgroundRole()), enumKey(widget.foregroundRole()),
		QString::number(palette.resolveMask(), 16));

	report += QStringLiteral("Style: ") + describeStyleChain(widget.style()) + '\n';
	if (widget.style() != QApplication::style())
		report += QStringLiteral("Application style: ") + describeStyleChain(QApplication::style()) + '\n';
	report += QStringLiteral("Stylesheet: own %1 chars, application %2 chars\n").arg(widget.styleSheet().size()).arg(qApp->styleSheet().size());
	report += QStringLiteral("Colour scheme: ") + enumKey(QGuiApplication::styleHints()->colorScheme()) + '\n';
	// A style plugin is built with the runtime, not with this code: its drawing follows the runtime version
	report += QStringLiteral("Qt: runtime %1, compiled against %2\n").arg(QString::fromLatin1(qVersion()), QStringLiteral(QT_VERSION_STR));

	report += QStringLiteral("Palette, current group %1:\n").arg(enumKey(palette.currentColorGroup()));
	for (const QPalette::ColorRole role : roles)
	{
		report += QStringLiteral("    %1 active %2, inactive %3, disabled %4%5\n").arg(enumKey(role).leftJustified(16),
			colorName(palette.color(QPalette::Active, role)), colorName(palette.color(QPalette::Inactive, role)),
			colorName(palette.color(QPalette::Disabled, role)), isRoleSetInAnyGroup(palette, role) ? QStringLiteral(" (set)") : QString{});
	}

	const QPalette classPalette = QApplication::palette(&widget);
	QStringList classDifferences;
	for (const QPalette::ColorRole role : roles)
	{
		if (classPalette.color(QPalette::Active, role) != palette.color(QPalette::Active, role))
			classDifferences.append(enumKey(role) + ' ' + colorName(classPalette.color(QPalette::Active, role)));
	}
	report += QStringLiteral("Application palette for the class, active roles differing: ")
		+ (classDifferences.isEmpty() ? QStringLiteral("none") : classDifferences.join(QStringLiteral(", "))) + '\n';

	report += QStringLiteral("Ancestors:\n");
	for (const QWidget* ancestor = widget.parentWidget(); ancestor; ancestor = ancestor->parentWidget())
	{
		const QPalette& ancestorPalette = ancestor->palette();
		QStringList setRoles;
		for (const QPalette::ColorRole role : roles)
		{
			if (isRoleSetInAnyGroup(ancestorPalette, role))
				setRoles.append(enumKey(role));
		}
		report += QStringLiteral("    %1: Base %2, Window %3, WA_SetPalette %4, autoFillBackground %5, own stylesheet %6 chars, set roles: %7\n").arg(
			describeObject(*ancestor), colorName(ancestorPalette.color(QPalette::Active, QPalette::Base)),
			colorName(ancestorPalette.color(QPalette::Active, QPalette::Window)), flag(ancestor->testAttribute(Qt::WA_SetPalette)),
			flag(ancestor->autoFillBackground()), QString::number(ancestor->styleSheet().size()), setRoles.join(' '));
	}

	report += QStringLiteral("Pixels from grab():\n");
	appendPixelSamples(report, widget.grab().toImage(), widget.size());

	report += QStringLiteral("Pixels from the screen:\n");
	// Only while visible: winId() creates a native window for a hidden one
	if (widget.isVisible())
	{
		const QPoint topLeft = widget.mapTo(widget.window(), QPoint{});
		const QPixmap shown = widget.screen()->grabWindow(widget.window()->winId(), topLeft.x(), topLeft.y(), widget.width(), widget.height());
		appendPixelSamples(report, shown.toImage(), widget.size());
	}
	else
		report += QStringLiteral("    hidden\n");

	return report;
}
