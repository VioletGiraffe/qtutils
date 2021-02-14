#include "windowsutils.h"
#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QString>
RESTORE_COMPILER_WARNINGS

QString toUncPath(const QString& somePath)
{
	QString unc = somePath;
	unc.replace('/', '\\');

	if (somePath.contains(':')) // Quick and dirty test for absolute path
		return QLatin1String("\\\\?\\") + unc;
	else
		return unc;
}
