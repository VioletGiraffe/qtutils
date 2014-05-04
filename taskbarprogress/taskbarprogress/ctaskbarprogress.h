#ifndef CTASKBARPROGRESS_H
#define CTASKBARPROGRESS_H

#include "../../QtIncludes"

enum ProgressState { psNormal, psPaused, psStopped, psIndeterminate, psNoProgress };

#if defined _WIN32

#include <map>

struct ITaskbarList3;

class CTaskBarProgress
#if QT_VERSION >= QT_VERSION_CHECK (5,0,0)
	: protected QAbstractNativeEventFilter
#endif
{
public:
	CTaskBarProgress(QWidget * widget = 0);
	~CTaskBarProgress();

	void linkToWidgetstaskbarButton (QWidget * widget);
	void setProgress (int progress, int minValue = 0, int maxValue = 100);
	void setState (ProgressState state);

private:
	ITaskbarList3 * taskbarListInterface ();

	static bool eventFilter (void * msg);
#if QT_VERSION >= QT_VERSION_CHECK (5,0,0)
	virtual bool nativeEventFilter(const QByteArray & eventType, void * message, long * result);
#endif
	static bool widgetAlreadyLinked (const QWidget * widget);

private:
#if QT_VERSION < QT_VERSION_CHECK (5,0,0)
	static QAbstractEventDispatcher::EventFilter _qtEventFilter;
#endif
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
	CTaskBarProgress(QWidget * widget = 0);
	~CTaskBarProgress();

	void linkToWidgetstaskbarButton (QWidget * widget);
	void setProgress (int progress, int minValue = 0, int maxValue = 100);
	void setState (ProgressState state);
};

#elif defined __linux__

class QWidget;

class CTaskBarProgress
{
public:
	CTaskBarProgress(QWidget * widget = 0);
	~CTaskBarProgress();

	void linkToWidgetsTaskbarButton(QWidget * widget);
	void setProgress(int progress, int minValue = 0, int maxValue = 100);
	void setState(ProgressState state);

private:
	QWidget      * _parent;
};

#else

class QWidget;

class CTaskBarProgress
{
public:
	CTaskBarProgress(QWidget * widget = 0);
	~CTaskBarProgress();

	void linkToWidgetsTaskbarButton(QWidget * widget);
	void setProgress(int progress, int minValue = 0, int maxValue = 100);
	void setState(ProgressState state);
};

#endif // _WIN32

#endif // CTASKBARPROGRESS_H
