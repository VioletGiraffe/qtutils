#include "cpersistenceenabler.h"

#include "assert/advanced_assert.h"

DISABLE_COMPILER_WARNINGS
#include <QApplication>
#include <QDockWidget>
#include <QHeaderView>
#include <QMainWindow>
#include <QScreen>
#include <QSet>
#include <QSettings>
#include <QSplitter>
#include <QStringList>
#include <QStyle>
RESTORE_COMPILER_WARNINGS

#define GEOMETRY_KEY (_settingsPath + "_geometry")
#define STATE_KEY (_settingsPath + "_state")

namespace {

// A floating dock widget is a window, but its contents belong to the main window's state
bool isSeparateWindow(const QWidget* widget)
{
	return widget->isWindow() && !qobject_cast<const QDockWidget*>(widget);
}

// The child's objectName, preceded by those of its named ancestors below root.
// Empty for an unnamed child, or one inside another window such as a dialog parented to root: that window persists its own state.
QString namePath(const QWidget* root, const QWidget* child)
{
	if (child == root || child->objectName().isEmpty())
		return child->objectName();

	QStringList names;
	for (const QWidget* widget = child; widget != root; widget = widget->parentWidget())
	{
		if (isSeparateWindow(widget))
			return {};
		if (!widget->objectName().isEmpty())
			names.prepend(widget->objectName());
	}

	return names.join('.');
}

// Empty for a splitter namePath() excludes
QString splitterKey(const QString& settingsPath, const QWidget* root, const QSplitter* splitter)
{
	const QString path = namePath(root, splitter);
	if (path.isEmpty())
		return {};

	return settingsPath + "_splitter_" + path;
}

// Empty for a header without sections, or of a view namePath() excludes.
// The section count is part of the key: QHeaderView::restoreState does not check it, and would apply a state saved for
// other columns.
QString headerKey(const QString& settingsPath, const QWidget* root, const QHeaderView* header)
{
	if (header->count() == 0)
		return {};

	const QString path = namePath(root, header->parentWidget());
	if (path.isEmpty())
		return {};

	return settingsPath + "_header_" + path + (header->orientation() == Qt::Horizontal ? "_h" : "_v") + QString::number(header->count());
}

} // namespace

CPersistenceEnabler::CPersistenceEnabler(QString widgetSettingsPath, QWidget* widget, Delayed delayed, SetDefaultSize setDefaultSize, PersistViewHeaders persistViewHeaders) :
	QObject(widget),
	_settingsPath{std::move(widgetSettingsPath)},
	_widget{ widget },
	_delayed{ delayed },
	_setDefaultSize{ setDefaultSize },
	_persistViewHeaders{ persistViewHeaders }
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

	QSet<QString> childKeys;
	const auto saveChildState = [&](const QString& key, const QByteArray& state) {
		assert_message_r(!childKeys.contains(key), QString{ "Persisted children share the key " + key + ": rename one of them or an ancestor" }.toStdString());
		childKeys.insert(key);
		s.setValue(key, state);
	};

	for (const QSplitter* splitter : _widget->findChildren<QSplitter*>())
	{
		if (const QString key = splitterKey(_settingsPath, _widget, splitter); !key.isEmpty())
			saveChildState(key, splitter->saveState());
	}

	if (!_persistViewHeaders)
		return;

	for (const QHeaderView* header : _widget->findChildren<QHeaderView*>())
	{
		if (const QString key = headerKey(_settingsPath, _widget, header); !key.isEmpty())
			saveChildState(key, header->saveState());
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
		if (const QString key = splitterKey(_settingsPath, _widget, splitter); !key.isEmpty())
			splitter->restoreState(s.value(key).toByteArray());
	}

	if (!_persistViewHeaders)
		return;

	for (QHeaderView* header : _widget->findChildren<QHeaderView*>())
	{
		if (const QString key = headerKey(_settingsPath, _widget, header); !key.isEmpty())
			header->restoreState(s.value(key).toByteArray());
	}
}
