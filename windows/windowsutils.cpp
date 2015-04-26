#include "windowsutils.h"

#include <Windows.h>

QString ErrorStringFromLastError()
{
	WCHAR* lpMsgBuf = 0;
	if (FormatMessageW(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT),
		(LPWSTR) &lpMsgBuf,
		0, NULL ) > 0)
	{
		QString msg = QString::fromWCharArray(lpMsgBuf);
		LocalFree(lpMsgBuf);
		return msg;
	}
	else
		return QString();
}
