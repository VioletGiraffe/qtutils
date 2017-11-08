#include "cpersistentwindow.h"

#include "../settings/csettings.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QMainWindow>

#define GEOMETRY_KEY (_settingsPath + "_geometry")
#define STATE_KEY (_settingsPath + "_state")

CPersistenceEnabler::CPersistenceEnabler(const QString& widgetSettingsPath, QObject* parent) : QObject(parent), _settingsPath(widgetSettingsPath)
{
}

bool CPersistenceEnabler::eventFilter(QObject* watched, QEvent* event)
{
	if (event->type() == QEvent::Show)
	{
		auto widget = static_cast<QWidget*>(watched);
		auto window = dynamic_cast<QMainWindow*>(watched);
		CSettings s;

		if (!widget->restoreGeometry(s.value(GEOMETRY_KEY).toByteArray()) || !window || !window->restoreState(s.value(STATE_KEY).toByteArray()))
			widget->resize(QApplication::desktop()->screenGeometry().size() / 2);
	}
	else if (event->type() == QEvent::Close)
	{
		auto widget = static_cast<QWidget*>(watched);
		auto window = dynamic_cast<QMainWindow*>(watched);
		CSettings s;

		s.setValue(GEOMETRY_KEY, widget->saveGeometry());
		if (window)
			s.setValue(STATE_KEY, window->saveState());
	}

	return QObject::eventFilter(watched, event);
}
