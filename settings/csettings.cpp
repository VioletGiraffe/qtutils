#include "csettings.h"

std::shared_ptr<QSettings> CSettings::_settings;
QString CSettings::_applicationName;
QString CSettings::_organizationName;

#ifndef NO_THREADSAFE_CSETTINGS
QMutex CSettings::_settingsMutex;
#endif

CSettings::CSettings()
{
#ifndef NO_THREADSAFE_CSETTINGS
	QMutexLocker locker (&_settingsMutex);
#endif
	if (_settings.get() == 0)
	{
		_settings = std::make_shared<QSettings>(QSettings::NativeFormat, QSettings::UserScope,
												_organizationName.isEmpty() ? qApp->organizationName() : _organizationName,
												_applicationName.isEmpty() ? qApp->applicationName() : _applicationName);
	}
}

void CSettings::setValue(const QString &key, const QVariant &value)
{
#ifndef NO_THREADSAFE_CSETTINGS
	QMutexLocker locker (&_settingsMutex);
#endif
	_settings->setValue( key, value );
}

QVariant CSettings::value(const QString &key, const QVariant &defaultValue) const
{
#ifndef NO_THREADSAFE_CSETTINGS
	QMutexLocker locker (&_settingsMutex);
#endif
	return _settings->value( key, defaultValue );
}

void CSettings::setApplicationName(const QString &name)
{
	_applicationName = name;
}

void CSettings::setOrganizationName(const QString &name)
{
	_organizationName = name;
}
