#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QObject>
RESTORE_COMPILER_WARNINGS

class CPersistenceEnabler final : public QObject
{
public:
	// Set widgetSettingsPath to an application-wide-unique QSettings path+name for storing this widget's state and position between application launches
	CPersistenceEnabler(QString widgetSettingsPath, QObject* parent = nullptr);

protected:
	bool eventFilter(QObject *watched, QEvent *event) override;

private:
	const QString _settingsPath;
	bool _windowStateRestored = false;
};
