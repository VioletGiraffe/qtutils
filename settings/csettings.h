#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QSettings>
RESTORE_COMPILER_WARNINGS

class CSettings
{
public:
	CSettings();

	static void setApplicationName(const QString& name);
	static void setOrganizationName(const QString& name);
	static void setFormat(QSettings::Format format);

	void setValue(const QString& key, const QVariant& value);
	QVariant value(const QString& key, const QVariant& defaultValue = QVariant()) const;


private:
	QSettings _impl;

	static QString _applicationName;
	static QString _organizationName;
	static QSettings::Format _settingsFormat;
};

