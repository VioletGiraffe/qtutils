#ifndef CSETTINGS_H
#define CSETTINGS_H

#include "../QtIncludes"
#include <memory>

class CSettings
{
public:
	static std::unique_ptr<QSettings> instance();

	static void setApplicationName(const QString& name);
	static void setOrganizationName(const QString& name);

private:
	CSettings();

private:
	static QString _applicationName;
	static QString _organizationName;
};

#endif // CSETTINGS_H
