#include "windowsutils.h"
#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QString>
RESTORE_COMPILER_WARNINGS

QString toUncPath(QString somePath)
{
	somePath.replace('/', '\\');

	if (somePath.contains(':')) // Quick and dirty test for absolute path
		return somePath.prepend(R"(\\?\)");
	else
		return somePath;
}
