#include "ctaskbarprogress.h"
#include "compiler/compiler_warnings_control.h"
#include "assert/advanced_assert.h"

DISABLE_COMPILER_WARNINGS
#include <QApplication>
#include <QDebug>
RESTORE_COMPILER_WARNINGS

#include <Shobjidl.h>

CTaskBarProgress::CTaskBarProgress(QWidget *widget)
{
	if (widget)
		linkToWidgetsTaskbarButton(widget);
}

CTaskBarProgress::~CTaskBarProgress()
{
	ITaskbarList3 * iface = taskbarListInterface();
	if (iface)
		iface->Release();
}

void CTaskBarProgress::linkToWidgetsTaskbarButton(QWidget *widget)
{
	if (!widget)
	{
		qInfo() << __FUNCTION__ << ": widget is null";
		return;
	}

	QAbstractEventDispatcher * dispatcher = QAbstractEventDispatcher::instance();
	if (!dispatcher)
	{
		qInfo() << __FUNCTION__ << ": QAbstractEventDispatcher is null";
		return;
	}

	if (widgetAlreadyLinked(widget))
	{
		qInfo() << __FUNCTION__ << ": CProgressBarTaskbar instance" << hex << this << " is trying to link to QWidget " << hex << widget << ", whose taskbar button has already been linked to";
		return;
	}

	_registeredWidgetsList[this] = widget;
	_taskbarButtonCreatedMessageIdMap[widget->winId()] = RegisterWindowMessageW(L"TaskbarButtonCreated");

	// Care: creating winId leads to creating a window which leads to creating a Window which leads to window procedure starting up, so you should only register event filter afterwards
	qApp->installNativeEventFilter(this);
}

void CTaskBarProgress::setProgress(int progress, int minValue /* = 0*/, int maxValue /* = 100*/)
{
	ITaskbarList3 * iface = taskbarListInterface();
	if (iface)
		iface->SetProgressValue(HWND(_registeredWidgetsList[this]->winId()), progress, maxValue - minValue);
}

void CTaskBarProgress::setState(ProgressState state)
{
	ITaskbarList3 * iface = taskbarListInterface();
	TBPFLAG taskbarPRogressState;
	switch (state)
	{
	case psNormal:
		taskbarPRogressState = TBPF_NORMAL;
		break;
	case psPaused:
		taskbarPRogressState = TBPF_PAUSED;
		break;
	case psStopped:
		taskbarPRogressState = TBPF_ERROR;
		break;
	case psIndeterminate:
		taskbarPRogressState = TBPF_INDETERMINATE;
		break;
	case psNoProgress:
		taskbarPRogressState = TBPF_NOPROGRESS;
		break;
	default:
		taskbarPRogressState = TBPF_NOPROGRESS;
		break;
	}

	if (iface)
		iface->SetProgressState(HWND(_registeredWidgetsList[this]->winId()), taskbarPRogressState);
}

bool CTaskBarProgress::eventFilter(void *msg)
{
	MSG * message = static_cast<MSG*>(msg);
	assert_and_return_r(message, false);

	if (_taskbarButtonCreatedMessageIdMap.count(WId(message->hwnd)) == 0)
		return false;

	if (message->message == _taskbarButtonCreatedMessageIdMap[WId(message->hwnd)] && _taskbarListInterface.count(WId(message->hwnd)) == 0)
	{
		ITaskbarList3 * iface = 0;
		HRESULT result = CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_ALL, IID_ITaskbarList3,(void**)&iface);
		if (result != S_OK || !iface)
			qInfo() << "ITaskbarList3 creation failed";
		else
		{
			_taskbarListInterface[WId(message->hwnd)] = iface;
		}
	}

	return false;
}

bool CTaskBarProgress::widgetAlreadyLinked(const QWidget * widget)
{
	for (const auto& item: _registeredWidgetsList)
		if (item.second == widget)
			return true;

	return false;
}

ITaskbarList3 * CTaskBarProgress::taskbarListInterface()
{
	if (_registeredWidgetsList.count(this) == 0 || _registeredWidgetsList[this] == 0 || _taskbarListInterface.count(_registeredWidgetsList[this]->winId()) == 0)
		return 0;
	else
		return _taskbarListInterface[_registeredWidgetsList[this]->winId()];
}

#if QT_VERSION >= QT_VERSION_CHECK (5,0,0)
bool CTaskBarProgress::nativeEventFilter(const QByteArray &/*eventType*/, void *message, long * /*result*/)
{
	return eventFilter(message);
}
#endif

std::map<WId, ITaskbarList3*> CTaskBarProgress::_taskbarListInterface;
std::map<CTaskBarProgress*, QWidget*> CTaskBarProgress::_registeredWidgetsList;
std::map<WId, quint32 /* "taskbar button created" message ID */> CTaskBarProgress::_taskbarButtonCreatedMessageIdMap;
