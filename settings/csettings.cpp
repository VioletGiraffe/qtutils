#include "csettings.h"

DISABLE_COMPILER_WARNINGS
#include <QApplication>
RESTORE_COMPILER_WARNINGS

QString CSettings::_applicationName;
QString CSettings::_organizationName;
QSettings::Format CSettings::_settingsFormat = QSettings::NativeFormat;

CSettings::CSettings() : _impl(_settingsFormat, QSettings::UserScope,
							   _organizationName.isEmpty() ? qApp->organizationName() : _organizationName,
							   _applicationName.isEmpty() ? qApp->applicationName() : _applicationName)
{
}

void CSettings::setApplicationName(const QString &name)
{
	_applicationName = name;
}

void CSettings::setOrganizationName(const QString &name)
{
	_organizationName = name;
}

void CSettings::setFormat(QSettings::Format format)
{
	_settingsFormat = format;
}

void CSettings::setValue(const QString & key, const QVariant & value)
{
	_impl.setValue(key, value);
}

QVariant CSettings::value(const QString & key, const QVariant & defaultValue) const
{
	return _impl.value(key, defaultValue);
}

QStringList CSettings::allKeys() const
{
	return _impl.allKeys();
}

void CSettings::clear()
{
	_impl.clear();
}
