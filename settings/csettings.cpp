#include "csettings.h"

QString CSettings::_applicationName;
QString CSettings::_organizationName;

CSettings::CSettings() : _impl(QSettings::NativeFormat, QSettings::UserScope,
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

void CSettings::setValue(const QString & key, const QVariant & value)
{
	_impl.setValue(key, value);
}

QVariant CSettings::value(const QString & key, const QVariant & defaultValue) const
{
	return _impl.value(key, defaultValue);
}
