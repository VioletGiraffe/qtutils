#include "cpersistenceenabler.h"

#include "assert/advanced_assert.h"

DISABLE_COMPILER_WARNINGS
#include <QApplication>
#include <QHeaderView>
#include <QMainWindow>
#include <QScreen>
#include <QSettings>
#include <QSplitter>
#include <QStyle>
RESTORE_COMPILER_WARNINGS

#define GEOMETRY_KEY (_settingsPath + "_geometry")
#define STATE_KEY (_settingsPath + "_state")
#define SPLITTER_KEY(splitter) (_settingsPath + "_splitter_" + (splitter)->objectName())

namespace {

// Empty for a header without sections or with an unnamed view.
// The section count is part of the key: QHeaderView::restoreState does not check it, and would apply a state saved for
// other columns.
QString headerKey(const QString& settingsPath, const QHeaderView* header)
{
	const QWidget* view = header->parentWidget();
	if (!view || view->objectName().isEmpty() || header->count() == 0)
		return {};

	return settingsPath + "_header_" + view->objectName() + (header->orientation() == Qt::Horizontal ? "_h" : "_v") + QString::number(header->count());
}

} // namespace

CPersistenceEnabler::CPersistenceEnabler(QString widgetSettingsPath, QWidget* widget, Delayed delayed, SetDefaultSize setDefaultSize) :
	QObject(widget),
	_settingsPath{std::move(widgetSettingsPath)},
	_widget{ widget },
	_delayed{ delayed },
	_setDefaultSize{ setDefaultSize }
{
	assert_and_return_r(_widget, );
	_widget->installEventFilter(this);

	// A quit closes no window, so a widget still visible at that point has not been saved by its hide event
	connect(qApp, &QCoreApplication::aboutToQuit, this, [this] {
		if (_widget->isVisible())
			saveState();
	});

	if (!_delayed)
		restoreState();
}

bool CPersistenceEnabler::eventFilter(QObject* watched, QEvent* e)
{
	if (_delayed && !_stateRestored && e->type() == QEvent::Show) [[unlikely]]
		restoreState();
	// Hide, not Close: QDialog::done() filters out its own close event, and an ignored close must not save
	else if (e->type() == QEvent::Hide) [[unlikely]]
		saveState();

	return QObject::eventFilter(watched, e);
}

void CPersistenceEnabler::saveState() const
{
	QSettings s;
	s.setValue(GEOMETRY_KEY, _widget->saveGeometry());
	if (auto* window = dynamic_cast<QMainWindow*>(_widget))
		s.setValue(STATE_KEY, window->saveState());

	for (const QSplitter* splitter : _widget->findChildren<QSplitter*>())
	{
		if (!splitter->objectName().isEmpty())
			s.setValue(SPLITTER_KEY(splitter), splitter->saveState());
	}

	for (const QHeaderView* header : _widget->findChildren<QHeaderView*>())
	{
		if (const QString key = headerKey(_settingsPath, header); !key.isEmpty())
			s.setValue(key, header->saveState());
	}
}

void CPersistenceEnabler::restoreState()
{
	_stateRestored = true;

	auto* window = dynamic_cast<QMainWindow*>(_widget);
	QSettings s;

	if (!_widget->restoreGeometry(s.value(GEOMETRY_KEY).toByteArray()) && _setDefaultSize)
	{
		const auto* const currentScreen = QApplication::screenAt(_widget->geometry().center());
		const auto availableGeometry = currentScreen ? currentScreen->availableGeometry() : QApplication::primaryScreen()->availableGeometry();
		_widget->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, availableGeometry.size() / 2, availableGeometry));
	}

	if (window)
		window->restoreState(s.value(STATE_KEY).toByteArray());

	// Restored before the widget reaches its final size, as in Qt's saveState example: a splitter without stretch factors
	// scales its panes in proportion when resized
	for (QSplitter* splitter : _widget->findChildren<QSplitter*>())
	{
		if (!splitter->objectName().isEmpty())
			splitter->restoreState(s.value(SPLITTER_KEY(splitter)).toByteArray());
	}

	for (QHeaderView* header : _widget->findChildren<QHeaderView*>())
	{
		if (const QString key = headerKey(_settingsPath, header); !key.isEmpty())
			header->restoreState(s.value(key).toByteArray());
	}
}
