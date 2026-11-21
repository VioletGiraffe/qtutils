#include "appimagedesktopentry.h"
#include "assert/advanced_assert.h"

DISABLE_COMPILER_WARNINGS
#include <QAction>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QImageReader>
#include <QLatin1StringView>
#include <QMenu>
#include <QMessageBox>
#include <QPointer>
#include <QProcess>
#include <QSaveFile>
#include <QSettings>
#include <QStandardPaths>
#include <QStringList>
RESTORE_COMPILER_WARNINGS

namespace {

constexpr QLatin1StringView OfferedSettingsKey{ "desktopEntry/offered" };
constexpr QLatin1StringView MainGroup{ "[Desktop Entry]" };

QString appImagePath()
{
	return qEnvironmentVariable("APPIMAGE");
}

QString applicationsFolder()
{
	return QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);
}

QString entryPath()
{
	return applicationsFolder() + '/' + QGuiApplication::desktopFileName() + ".desktop";
}

QString iconsFolder()
{
	return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/icons/hicolor";
}

QString escapedStringValue(QString value)
{
	value.replace('\\', QStringLiteral("\\\\"));
	value.replace('\n', QStringLiteral("\\n"));
	value.replace('\r', QStringLiteral("\\r"));
	return value;
}

QString unescapedStringValue(QStringView value)
{
	QString result;
	result.reserve(value.size());
	for (qsizetype i = 0; i < value.size(); ++i)
	{
		if (value[i] != '\\' || i + 1 == value.size())
		{
			result += value[i];
			continue;
		}

		switch (value[++i].unicode())
		{
		case 's': result += ' '; break;
		case 'n': result += '\n'; break;
		case 't': result += '\t'; break;
		case 'r': result += '\r'; break;
		default: result += value[i]; break;
		}
	}
	return result;
}

// argument quoted as one Exec argument, then escaped as a string value
QString escapedExecArgument(const QString& argument)
{
	QString quoted{ u'"' };
	for (const QChar c : argument)
	{
		if (c == '"' || c == '`' || c == '$' || c == '\\')
			quoted += '\\';
		else if (c == '%') // a field code prefix
			quoted += '%';
		quoted += c;
	}
	quoted += '"';
	return escapedStringValue(quoted);
}

// The Exec value with its program replaced by this AppImage file; the arguments, field codes included, are kept
QString execOfThisAppImage(QStringView value)
{
	qsizetype programEnd = 0;
	if (value.startsWith(u'"'))
	{
		programEnd = 1;
		while (programEnd < value.size() && value[programEnd] != u'"')
			programEnd += value[programEnd] == u'\\' ? 2 : 1;
		++programEnd;
	}
	else
		programEnd = value.indexOf(u' ');

	if (programEnd < 0 || programEnd > value.size())
		programEnd = value.size();

	return escapedExecArgument(appImagePath()).append(value.sliced(programEnd));
}

QString tryExecLine()
{
	return "TryExec=" + escapedStringValue(appImagePath());
}

[[nodiscard]] std::expected<QStringList, QString> readLines(const QString& path)
{
	QFile file{ path };
	if (!file.open(QFile::ReadOnly | QFile::Text))
		return std::unexpected(QObject::tr("Cannot read %1: %2").arg(path, file.errorString()));
	return QString::fromUtf8(file.readAll()).split('\n');
}

// Unescaped; empty when the [Desktop Entry] group has no such key
QString mainGroupValue(const QStringList& lines, QLatin1StringView key)
{
	bool inMainGroup = false;
	for (const QString& line : lines)
	{
		if (line.startsWith('['))
			inMainGroup = line == MainGroup;
		else if (inMainGroup && line.startsWith(key) && QStringView{ line }.sliced(key.size()).startsWith('='))
			return unescapedStringValue(QStringView{ line }.sliced(key.size() + 1));
	}
	return {};
}

// The icon the installed entry shows, when this module installed it; empty otherwise
QString installedEntryIconPath()
{
	const auto lines = readLines(entryPath());
	if (!lines)
		return {};

	const QFileInfo icon{ mainGroupValue(*lines, QLatin1StringView{ "Icon" }) };
	const bool ownIcon = icon.isAbsolute() && icon.absoluteFilePath().startsWith(iconsFolder() + '/')
		&& icon.completeBaseName() == QGuiApplication::desktopFileName();
	return ownIcon ? icon.absoluteFilePath() : QString{};
}

// appimagetool requires the desktop file and its icon at the AppDir root
QString appDirFilePath(const QString& fileName)
{
	return qEnvironmentVariable("APPDIR") + '/' + fileName;
}

[[nodiscard]] std::expected<QString, QString> appDirIconPath(const QString& iconName)
{
	for (const QLatin1StringView suffix : { QLatin1StringView{ ".png" }, QLatin1StringView{ ".svg" } })
		if (const QString path = appDirFilePath(iconName + suffix); QFileInfo::exists(path))
			return path;
	return std::unexpected(QObject::tr("The AppImage holds no icon named %1").arg(iconName));
}

// Where install() copies sourceIcon: the hicolor theme's folder for its size
QString iconDestinationPath(const QString& sourceIcon)
{
	const QFileInfo source{ sourceIcon };
	QString sizeFolder = QStringLiteral("scalable");
	if (source.suffix() != QLatin1StringView{ "svg" })
	{
		const QSize size = QImageReader{ sourceIcon }.size();
		sizeFolder = QStringLiteral("%1x%2").arg(size.width()).arg(size.height());
	}
	return iconsFolder() + '/' + sizeFolder + "/apps/" + QGuiApplication::desktopFileName() + '.' + source.suffix();
}

// The AppImage's own entry, launching this AppImage file and showing icon
QString entryText(const QStringList& templateLines, const QString& icon)
{
	QStringList lines;
	bool inMainGroup = false;
	for (const QString& line : templateLines)
	{
		if (line.startsWith('['))
			inMainGroup = line == MainGroup;

		if (line.startsWith(QLatin1StringView{ "Exec=" }))
		{
			lines.push_back("Exec=" + execOfThisAppImage(QStringView{ line }.sliced(5)));
			// TryExec hides the entry from menus once this AppImage file is moved or deleted
			if (inMainGroup)
				lines.push_back(tryExecLine());
		}
		else if (inMainGroup && line.startsWith(QLatin1StringView{ "Icon=" }))
			lines.push_back("Icon=" + escapedStringValue(icon));
		else if (!(inMainGroup && line.startsWith(QLatin1StringView{ "TryExec=" })))
			lines.push_back(line);
	}
	return lines.join('\n');
}

[[nodiscard]] std::expected<void, QString> createParentFolder(const QString& filePath)
{
	const QString folder = QFileInfo{ filePath }.absolutePath();
	if (!QDir{}.mkpath(folder))
		return std::unexpected(QObject::tr("Cannot create the folder %1").arg(folder));
	return {};
}

[[nodiscard]] std::expected<void, QString> removeFile(const QString& path)
{
	QFile file{ path };
	if (file.exists() && !file.remove())
		return std::unexpected(QObject::tr("Cannot delete %1: %2").arg(path, file.errorString()));
	return {};
}

// Desktops that read mimeinfo.cache list the entry under its MIME types only after this runs
void updateDesktopDatabase()
{
	if (!QProcess::startDetached(QStringLiteral("update-desktop-database"), { applicationsFolder() }))
		qInfo() << "update-desktop-database could not be started";
}

void setInstalled(const bool installed, QWidget* dialogParent)
{
	const auto result = installed ? AppImageDesktopEntry::install() : AppImageDesktopEntry::uninstall();
	if (!result)
	{
		const QString title = installed ? QObject::tr("Cannot install the desktop entry") : QObject::tr("Cannot uninstall the desktop entry");
		QMessageBox::warning(dialogParent, title, result.error());
	}
}

} // namespace

bool AppImageDesktopEntry::runningFromAppImage()
{
	// The executable is checked too: the AppImage runtime's environment reaches every program started from the AppImage
	const QString appDir = QFileInfo{ qEnvironmentVariable("APPDIR") }.canonicalFilePath();
	return !appDir.isEmpty() && !appImagePath().isEmpty() && QCoreApplication::applicationFilePath().startsWith(appDir + '/');
}

bool AppImageDesktopEntry::installedForThisAppImage()
{
	const auto lines = readLines(entryPath());
	return lines && lines->contains(tryExecLine());
}

std::expected<void, QString> AppImageDesktopEntry::install()
{
	assert_r(runningFromAppImage());
	assert_r(!QGuiApplication::desktopFileName().isEmpty());

	const auto templateLines = readLines(appDirFilePath(QGuiApplication::desktopFileName() + ".desktop"));
	if (!templateLines)
		return std::unexpected(templateLines.error());

	const auto sourceIcon = appDirIconPath(mainGroupValue(*templateLines, QLatin1StringView{ "Icon" }));
	if (!sourceIcon)
		return std::unexpected(sourceIcon.error());

	// An entry for another AppImage file may show an icon of another size or format
	if (const auto removed = removeFile(installedEntryIconPath()); !removed)
		return removed;

	const QString icon = iconDestinationPath(*sourceIcon);
	if (const auto created = createParentFolder(icon); !created)
		return created;
	if (const auto removed = removeFile(icon); !removed)
		return removed;
	if (QFile source{ *sourceIcon }; !source.copy(icon))
		return std::unexpected(QObject::tr("Cannot write %1: %2").arg(icon, source.errorString()));

	// Atomic: the desktop watches this folder and may read the file at any time
	QSaveFile entry{ entryPath() };
	if (const auto created = createParentFolder(entry.fileName()); !created)
		return created;
	if (!entry.open(QFile::WriteOnly) || entry.write(entryText(*templateLines, icon).toUtf8()) < 0 || !entry.commit())
		return std::unexpected(QObject::tr("Cannot write %1: %2").arg(entry.fileName(), entry.errorString()));

	updateDesktopDatabase();
	return {};
}

std::expected<void, QString> AppImageDesktopEntry::uninstall()
{
	const QString icon = installedEntryIconPath();
	// The entry goes first: a menu never shows it without its icon
	if (const auto removed = removeFile(entryPath()); !removed)
		return removed;

	updateDesktopDatabase();
	return removeFile(icon);
}

void AppImageDesktopEntry::addToggleAction(QMenu& menu, QWidget* dialogParent, QAction* before)
{
	if (!runningFromAppImage())
		return;

	auto* action = new QAction{ &menu };
	menu.insertAction(before, action);
	const auto updateText = [action] {
		action->setText(installedForThisAppImage() ? QObject::tr("Uninstall Desktop Entry") : QObject::tr("Install Desktop Entry"));
	};
	updateText();
	// Read on every opening: the entry file can change outside this process
	QObject::connect(&menu, &QMenu::aboutToShow, action, updateText);
	QObject::connect(action, &QAction::triggered, action, [dialogParent = QPointer{ dialogParent }] {
		setInstalled(!installedForThisAppImage(), dialogParent);
	});
}

void AppImageDesktopEntry::offerOnce(QWidget* parent)
{
	QSettings settings;
	if (!runningFromAppImage() || settings.value(OfferedSettingsKey, false).toBool() || installedForThisAppImage())
		return;

	settings.setValue(OfferedSettingsKey, true);

	auto* question = new QMessageBox{ QMessageBox::Question, QObject::tr("Install a desktop entry?"),
		QObject::tr("A desktop entry adds %1 to the applications menu, and lets you pin it to the taskbar or dock.")
			.arg(QGuiApplication::applicationDisplayName()),
		QMessageBox::Yes | QMessageBox::No, parent };
	question->setInformativeText(QObject::tr("The Help menu can install or uninstall it later."));
	question->setAttribute(Qt::WA_DeleteOnClose);
	QObject::connect(question, &QDialog::finished, question, [question] {
		if (question->standardButton(question->clickedButton()) == QMessageBox::Yes)
			setInstalled(true, question->parentWidget());
	});
	question->open();
}
