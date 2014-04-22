#ifndef CSETTINGS_H
#define CSETTINGS_H

#include "../QtIncludes"
#include <memory>

class QSettings;

class CSettings
{
public:
	CSettings();

	void setValue(const QString &key, const QVariant &value);
	QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const;

	static void setApplicationName(const QString& name);
	static void setOrganizationName(const QString& name);

private:
	static std::shared_ptr<QSettings> _settings;
	static QString _applicationName;
	static QString _organizationName;
#ifndef NO_THREADSAFE_CSETTINGS
	static QMutex _settingsMutex;
#endif
};

#endif // CSETTINGS_H
