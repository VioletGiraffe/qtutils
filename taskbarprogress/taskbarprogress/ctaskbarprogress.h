#pragma once

#include "compiler/compiler_warnings_control.h"

enum ProgressState { psNormal, psPaused, psStopped, psIndeterminate, psNoProgress };

#if defined _WIN32

DISABLE_COMPILER_WARNINGS
#include <QWidget>
#include <qt_windows.h>
#include <QAbstractEventDispatcher>

#include <QAbstractNativeEventFilter>
RESTORE_COMPILER_WARNINGS

#include <map>

struct ITaskbarList3;

class CTaskBarProgress final : protected QAbstractNativeEventFilter
{
public:
	explicit CTaskBarProgress(QWidget * widget = nullptr);
	~CTaskBarProgress() override;

	void linkToWidgetsTaskbarButton (QWidget * widget);
	void setProgress (int progress, int minValue = 0, int maxValue = 100);
	void setState (ProgressState state);

private:
	ITaskbarList3 * taskbarListInterface ();

	static bool eventFilter (void * msg);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	bool nativeEventFilter(const QByteArray & eventType, void * message, qintptr * result) override;
#else
	bool nativeEventFilter(const QByteArray & eventType, void * message, long * result) override;
#endif
	static bool widgetAlreadyLinked (const QWidget * widget);

private:
	static std::map<WId, quint32 /* "taskbar button created" message ID */> _taskbarButtonCreatedMessageIdMap;
	// List of the widgets with which linkWithWidgetstaskbarButton have already been called.
	// It's used to guard against linking different progress bar instances to the same taskbar button.
	static std::map<CTaskBarProgress*, QWidget*> _registeredWidgetsList;
	static std::map<WId, ITaskbarList3*> _taskbarListInterface;
};

#elif defined __APPLE__

class QWidget;

class CTaskBarProgress
{
public:
	explicit CTaskBarProgress(QWidget * widget = nullptr);

	void linkToWidgetsTaskbarButton (QWidget * widget);
	void setProgress (int progress, int minValue = 0, int maxValue = 100);
	void setState (ProgressState state);
};

#elif defined __linux__ || defined __FreeBSD__

class QWidget;

class CTaskBarProgress
{
public:
	explicit CTaskBarProgress(QWidget * widget = nullptr);

	void linkToWidgetsTaskbarButton(QWidget * widget);
	void setProgress(int progress, int minValue = 0, int maxValue = 100);
	void setState(ProgressState state);

private:
	QWidget      * _parent = nullptr;
};

#endif // _WIN32 / __APPLE__
