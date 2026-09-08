#include "cpersistentwindow.h"

#include "assert/advanced_assert.h"

DISABLE_COMPILER_WARNINGS
#include <QApplication>
#include <QMainWindow>
#include <QScreen>
#include <QSettings>
#include <QStyle>
RESTORE_COMPILER_WARNINGS

#define GEOMETRY_KEY (_settingsPath + "_geometry")
#define STATE_KEY (_settingsPath + "_state")

CPersistenceEnabler::CPersistenceEnabler(QString widgetSettingsPath, QWidget* widget, Delayed delayed, SetDefaultSize setDefaultSize) :
	QObject(widget),
	_settingsPath{std::move(widgetSettingsPath)},
	_widget{ widget },
	_delayed{ delayed },
	_setDefaultSize{ setDefaultSize }
{
	assert_and_return_r(_widget, );
	_widget->installEventFilter(this);

	// A quit closes no window, so the state is saved from here as well as on the close event
	connect(qApp, &QCoreApplication::aboutToQuit, this, [this] {
		// Before the restore the widget holds a geometry that is not the stored one, and must not replace it
		if (_windowStateRestored)
			saveState();
	});

	if (!_delayed)
		restoreState();
}

bool CPersistenceEnabler::eventFilter(QObject* watched, QEvent* e)
{
	if (_delayed && !_windowStateRestored && e->type() == QEvent::Show) [[unlikely]]
		restoreState();
	else if (e->type() == QEvent::Close) [[unlikely]]
		saveState();

	return QObject::eventFilter(watched, e);
}

void CPersistenceEnabler::saveState() const
{
	QSettings s;
	s.setValue(GEOMETRY_KEY, _widget->saveGeometry());
	if (auto* window = dynamic_cast<QMainWindow*>(_widget))
		s.setValue(STATE_KEY, window->saveState());
}

void CPersistenceEnabler::restoreState()
{
	_windowStateRestored = true;

	auto* window = dynamic_cast<QMainWindow*>(_widget);
	QSettings s;

	if (!_widget->restoreGeometry(s.value(GEOMETRY_KEY).toByteArray()))
	{
		if (!_setDefaultSize)
			return;

		const auto* const currentScreen = QApplication::screenAt(_widget->geometry().center());
		const auto availableGeometry = currentScreen ? currentScreen->availableGeometry() : QApplication::primaryScreen()->availableGeometry();
		_widget->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, availableGeometry.size() / 2, availableGeometry));
	}

	if (window)
		window->restoreState(s.value(STATE_KEY).toByteArray());
}
