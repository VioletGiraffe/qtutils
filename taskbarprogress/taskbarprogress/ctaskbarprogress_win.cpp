#include "ctaskbarprogress.h"
#include "compiler/compiler_warnings_control.h"
#include "assert/advanced_assert.h"

DISABLE_COMPILER_WARNINGS
#include <QApplication>
#include <QDebug>
RESTORE_COMPILER_WARNINGS

#include <ShObjIdl.h>

CTaskBarProgress::CTaskBarProgress(QWidget *widget) noexcept
{
	if (widget)
		linkToWidgetsTaskbarButton(widget);
}

CTaskBarProgress::~CTaskBarProgress() noexcept
{
	qApp->removeNativeEventFilter(this);

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

	if (widgetAlreadyLinked(widget))
	{
		qInfo() << __FUNCTION__ << ": CProgressBarTaskbar instance" << Qt::hex << this << " is trying to link to QWidget " << Qt::hex << widget << ", whose taskbar button has already been linked to";
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
	TBPFLAG taskbarProgressState = TBPF_NOPROGRESS;
	switch (state)
	{
	case psNormal:
		taskbarProgressState = TBPF_NORMAL;
		break;
	case psPaused:
		taskbarProgressState = TBPF_PAUSED;
		break;
	case psStopped:
		taskbarProgressState = TBPF_ERROR;
		break;
	case psIndeterminate:
		taskbarProgressState = TBPF_INDETERMINATE;
		break;
	case psNoProgress: [[fallthrough]];
	default:
		taskbarProgressState = TBPF_NOPROGRESS;
		break;
	}

	if (iface)
		iface->SetProgressState(HWND(_registeredWidgetsList[this]->winId()), taskbarProgressState);
}

bool CTaskBarProgress::eventFilter(void *msg)
{
	MSG * message = static_cast<MSG*>(msg);
	assert_and_return_r(message, false);

	if (!_taskbarButtonCreatedMessageIdMap.contains(WId(message->hwnd)))
		return false;

	if (message->message == _taskbarButtonCreatedMessageIdMap[WId(message->hwnd)] && !_taskbarListInterface.contains(WId(message->hwnd)))
	{
		ITaskbarList3 * iface = nullptr;
		HRESULT result = CoCreateInstance(CLSID_TaskbarList, nullptr, CLSCTX_ALL, IID_ITaskbarList3, reinterpret_cast<void**>(&iface));
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
	{
		if (item.second == widget)
			return true;
	}

	return false;
}

ITaskbarList3 * CTaskBarProgress::taskbarListInterface()
{
	if (!_registeredWidgetsList.contains(this) || _registeredWidgetsList[this] == nullptr || !_taskbarListInterface.contains(_registeredWidgetsList[this]->winId()))
		return nullptr;
	else
		return _taskbarListInterface[_registeredWidgetsList[this]->winId()];
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	bool CTaskBarProgress::nativeEventFilter(const QByteArray & /*eventType*/, void * message, qintptr * /*result*/)
#else
	bool CTaskBarProgress::nativeEventFilter(const QByteArray & /*eventType*/, void * message, long * /*result*/)
#endif
{
	return eventFilter(message);
}

std::unordered_map<WId, ITaskbarList3*> CTaskBarProgress::_taskbarListInterface;
std::unordered_map<CTaskBarProgress*, QWidget*> CTaskBarProgress::_registeredWidgetsList;
std::unordered_map<WId, quint32 /* "taskbar button created" message ID */> CTaskBarProgress::_taskbarButtonCreatedMessageIdMap;
