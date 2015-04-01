#include "csettings.h"

QString CSettings::_applicationName;
QString CSettings::_organizationName;


std::unique_ptr<QSettings> CSettings::instance()
{
	return std::move(std::unique_ptr<QSettings>(new QSettings(QSettings::NativeFormat, QSettings::UserScope,
		_organizationName.isEmpty() ? qApp->organizationName() : _organizationName,
		_applicationName.isEmpty() ? qApp->applicationName() : _applicationName)));
}



CSettings::CSettings()
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
