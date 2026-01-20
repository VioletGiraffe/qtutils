#include "cpersistentwindow.h"

#include "../settings/csettings.h"
#include "assert/advanced_assert.h"

DISABLE_COMPILER_WARNINGS
#include <QApplication>
#include <QMainWindow>
#include <QScreen>
#include <QStyle>
RESTORE_COMPILER_WARNINGS

#define GEOMETRY_KEY (_settingsPath + "_geometry")
#define STATE_KEY (_settingsPath + "_state")

CPersistenceEnabler::CPersistenceEnabler(QString widgetSettingsPath, QObject* parent, bool delayed) :
	QObject(parent),
	_settingsPath{std::move(widgetSettingsPath)},
	_delayed{ delayed }
{
	if (!_delayed)
	{
		QWidget* widget = qobject_cast<QWidget*>(parent);
		assert_and_return_r(widget, );
		restoreState(widget);
	}
}

bool CPersistenceEnabler::eventFilter(QObject* watched, QEvent* e)
{
	if (_delayed && !_windowStateRestored && e->type() == QEvent::Show) [[unlikely]]
	{
		_windowStateRestored = true;

		auto* widget = static_cast<QWidget*>(watched);
		assert_and_return_r(widget, QObject::eventFilter(watched, e));
		restoreState(widget);
	}
	else if (e->type() == QEvent::Close) [[unlikely]]
	{
		auto* widget = static_cast<QWidget*>(watched);
		auto* window = dynamic_cast<QMainWindow*>(watched);
		CSettings s;

		s.setValue(GEOMETRY_KEY, widget->saveGeometry());
		if (window)
			s.setValue(STATE_KEY, window->saveState());
	}

	return QObject::eventFilter(watched, e);
}

void CPersistenceEnabler::restoreState(QWidget* widget)
{
	auto* window = dynamic_cast<QMainWindow*>(widget);
	CSettings s;

	if (!widget->restoreGeometry(s.value(GEOMETRY_KEY).toByteArray()) || !window || !window->restoreState(s.value(STATE_KEY).toByteArray()))
	{
		const auto* const currentScreen = QApplication::screenAt(widget->geometry().center());
		const auto availableGeometry = currentScreen ? currentScreen->availableGeometry() : QApplication::primaryScreen()->availableGeometry();
		widget->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, availableGeometry.size() / 2, availableGeometry));
	}
}
