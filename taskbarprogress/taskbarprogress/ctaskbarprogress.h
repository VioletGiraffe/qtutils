#ifndef CTASKBARPROGRESS_H
#define CTASKBARPROGRESS_H

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

class CTaskBarProgress : protected QAbstractNativeEventFilter
{
public:
	explicit CTaskBarProgress(QWidget * widget = 0);
	~CTaskBarProgress();

	void linkToWidgetsTaskbarButton (QWidget * widget);
	void setProgress (int progress, int minValue = 0, int maxValue = 100);
	void setState (ProgressState state);

private:
	ITaskbarList3 * taskbarListInterface ();

	static bool eventFilter (void * msg);
	virtual bool nativeEventFilter(const QByteArray & eventType, void * message, long * result);
	static bool widgetAlreadyLinked (const QWidget * widget);

private:
	static std::map<WId, quint32 /* "taskbar button created" message ID */> _taskbarButtonCreatedMessageIdMap;
	static std::map<CTaskBarProgress*, QWidget*> _registeredWidgetsList; // List of the widgets with which linkWithWidgetstaskbarButton have already been called
													  // is used to guard against linking different progress bar instances to the same taskbar button
	static std::map<WId, ITaskbarList3*> _taskbarListInterface;
};

#elif defined __APPLE__

class QWidget;

class CTaskBarProgress
{
public:
	explicit CTaskBarProgress(QWidget * widget = 0);
	~CTaskBarProgress();

	void linkToWidgetsTaskbarButton (QWidget * widget);
	void setProgress (int progress, int minValue = 0, int maxValue = 100);
	void setState (ProgressState state);
};

#elif defined __linux__ || defined __FreeBSD__

class QWidget;

class CTaskBarProgress
{
public:
	explicit CTaskBarProgress(QWidget * widget = 0);
	~CTaskBarProgress();

	void linkToWidgetsTaskbarButton(QWidget * widget);
	void setProgress(int progress, int minValue = 0, int maxValue = 100);
	void setState(ProgressState state);

private:
	QWidget      * _parent = 0;
};

#else

class QWidget;

class CTaskBarProgress
{
public:
	explicit CTaskBarProgress(QWidget * widget = 0);
	~CTaskBarProgress();

	void linkToWidgetsTaskbarButton(QWidget * widget);
	void setProgress(int progress, int minValue = 0, int maxValue = 100);
	void setState(ProgressState state);
};

#endif // _WIN32

#endif // CTASKBARPROGRESS_H
