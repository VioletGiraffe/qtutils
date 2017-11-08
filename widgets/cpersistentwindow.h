#pragma once

#include <QObject>

class CPersistenceEnabler : public QObject
{
public:
	// Set widgetSettingsPath to an application-wide-unique QSettings path+name for storing this widget's state and position between application launches
	CPersistenceEnabler(const QString& widgetSettingsPath, QObject* parent = nullptr);

protected:
	bool eventFilter(QObject *watched, QEvent *event) override;

private:
	const QString _settingsPath;
};
