#include "windowsutils.h"

#include <Windows.h>

QString ErrorStringFromLastError()
{
	WCHAR* lpMsgBuf = nullptr;
	if (FormatMessageW(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,
		GetLastError(),
		MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT),
		(LPWSTR) &lpMsgBuf,
		0, nullptr ) > 0)
	{
		QString msg = QString::fromWCharArray(lpMsgBuf);
		LocalFree(lpMsgBuf);
		return msg;
	}
	else
		return QString();
}

QString toUncPath(const QString& somePath)
{
	QString unc = somePath;
	unc.replace('/', '\\');

	if (somePath.contains(':')) // Quick and dirty test for absolute path
		return QLatin1String("\\\\?\\") + unc;
	else
		return unc;
}
