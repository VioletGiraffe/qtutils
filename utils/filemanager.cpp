#include "filemanager.h"

DISABLE_COMPILER_WARNINGS
#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QObject>
#include <QProcess>
#include <QStringList>
#include <QUrl>

#if !defined Q_OS_WIN && !defined Q_OS_MACOS
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#endif
RESTORE_COMPILER_WARNINGS

#ifdef Q_OS_WIN
#include <Windows.h>
#include <ShlObj.h>

#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")
#endif

namespace
{

void openContainingFolder(const QString& path)
{
	QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo{ path }.absolutePath()));
}

} // namespace

bool revealInFileManager(const QString& path, const LinkHandling links)
{
	const QFileInfo fi{ path };
	// exists() follows links, so a link whose target is gone needs its own check
	const bool linkItself = links == LinkHandling::NoFollow && (fi.isSymLink() || fi.isJunction());
	if (!fi.exists() && !linkItself)
		return false;

	const QString revealedPath = links == LinkHandling::Follow ? fi.canonicalFilePath() : fi.absoluteFilePath();

#if defined Q_OS_WIN
	// Not explorer.exe /select: its command line splits a path at a comma unless quoted, and QProcess quotes only for spaces
	const QString nativePath = QDir::toNativeSeparators(revealedPath);
	HRESULT shown = E_FAIL;
	if (PIDLIST_ABSOLUTE item = ILCreateFromPathW(reinterpret_cast<const wchar_t*>(nativePath.utf16())))
	{
		// SHOpenFolderAndSelectItems requires COM; each successful init, S_FALSE included, needs an uninit
		const HRESULT comInitialized = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
		shown = SHOpenFolderAndSelectItems(item, 0, nullptr, 0);
		if (SUCCEEDED(comInitialized))
			CoUninitialize();
		ILFree(item);
	}
	if (FAILED(shown))
		openContainingFolder(revealedPath);
#elif defined Q_OS_MACOS
	if (!QProcess::startDetached(QStringLiteral("/usr/bin/open"), { QStringLiteral("-R"), revealedPath }))
		openContainingFolder(revealedPath);
#else
	// This D-Bus interface selects the item and activates a supported file manager if necessary. Construct the
	// call directly to avoid QDBusInterface's blocking introspection.
	QDBusMessage showItems = QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.FileManager1"), QStringLiteral("/org/freedesktop/FileManager1"),
		QStringLiteral("org.freedesktop.FileManager1"), QStringLiteral("ShowItems"));
	showItems.setArguments({ QStringList{ QUrl::fromLocalFile(revealedPath).toString() }, QString{} });

	// Activation can take seconds. qApp bounds the async watcher's lifetime and keeps the fallback on the GUI thread.
	auto* watcher = new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(showItems), qApp);
	QObject::connect(watcher, &QDBusPendingCallWatcher::finished, qApp, [revealedPath](QDBusPendingCallWatcher* self) {
		if (self->isError())
			openContainingFolder(revealedPath);
		self->deleteLater();
	});
#endif

	return true;
}

QString revealInFileManagerActionText()
{
#if defined Q_OS_WIN
	return QObject::tr("Open in Explorer");
#elif defined Q_OS_MACOS
	return QObject::tr("Reveal in Finder");
#else
	return QObject::tr("Show in file manager");
#endif
}

bool openFolderInFileManager(const QString& folderPath)
{
	if (!QFileInfo(folderPath).isDir())
		return false;

	QDesktopServices::openUrl(QUrl::fromLocalFile(folderPath));
	return true;
}
