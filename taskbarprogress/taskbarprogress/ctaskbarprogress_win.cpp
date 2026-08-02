#include "ctaskbarprogress.h"
#include "compiler/compiler_warnings_control.h"
#include "assert/advanced_assert.h"

DISABLE_COMPILER_WARNINGS
#include <QApplication>
#include <QDebug>
RESTORE_COMPILER_WARNINGS

#include <ShObjIdl.h>

#include <utility>

using Microsoft::WRL::ComPtr;

CTaskBarProgress::CTaskBarProgress(QWidget *widget) noexcept
{
	if (widget)
		linkToWidgetsTaskbarButton(widget);
}

CTaskBarProgress::~CTaskBarProgress() noexcept
{
	qApp->removeNativeEventFilter(this);

	// The static maps must not keep entries for this instance or its window: Windows reuses HWND values,
	// so a leftover entry would be found - and called - for a future window with the same id.
	_taskbarListInterface.erase(_linkedWindowId);
	_taskbarButtonCreatedMessageIdMap.erase(_linkedWindowId);
	_registeredWidgetsList.erase(this);
}

void CTaskBarProgress::linkToWidgetsTaskbarButton(QWidget *widget)
{
	if (!widget)
	{
		qInfo() << __FUNCTION__ << ": widget is null";
		return;
	}

	// The taskbar button belongs to the top-level window, so that is what gets linked and tracked - two widgets
	// in the same window must collide in the check below. It also keeps winId() from turning a child widget
	// (and, by default, its siblings) into native windows.
	QWidget* const window = widget->window();

	if (widgetAlreadyLinked(window))
	{
		qInfo() << __FUNCTION__ << ": CTaskBarProgress instance" << Qt::hex << this << " is trying to link to QWidget " << Qt::hex << window << ", whose taskbar button has already been linked to";
		return;
	}

	_registeredWidgetsList[this] = window;
	_linkedWindowId = window->winId();
	_taskbarButtonCreatedMessageIdMap[_linkedWindowId] = RegisterWindowMessageW(L"TaskbarButtonCreated");

	// Care: creating winId leads to creating a window which leads to creating a Window which leads to window procedure starting up, so you should only register event filter afterwards
	qApp->installNativeEventFilter(this);
}

void CTaskBarProgress::setProgress(int progress, int minValue /* = 0*/, int maxValue /* = 100*/)
{
	ITaskbarList3 * iface = taskbarListInterface();
	if (iface)
		iface->SetProgressValue(HWND(_linkedWindowId), progress, maxValue - minValue);
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
		iface->SetProgressState(HWND(_linkedWindowId), taskbarProgressState);
}

bool CTaskBarProgress::eventFilter(void *msg)
{
	MSG * message = static_cast<MSG*>(msg);
	assert_and_return_r(message, false);

	if (!_taskbarButtonCreatedMessageIdMap.contains(WId(message->hwnd)))
		return false;

	if (message->message == _taskbarButtonCreatedMessageIdMap[WId(message->hwnd)] && !_taskbarListInterface.contains(WId(message->hwnd)))
	{
		ComPtr<ITaskbarList3> iface;
		HRESULT result = CoCreateInstance(CLSID_TaskbarList, nullptr, CLSCTX_ALL, IID_ITaskbarList3, reinterpret_cast<void**>(iface.GetAddressOf()));
		if (result != S_OK || !iface)
			qInfo() << "ITaskbarList3 creation failed";
		else
		{
			_taskbarListInterface[WId(message->hwnd)] = std::move(iface);
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
	// Looked up by the stored window id only - the linked widget must not be dereferenced here, this
	// runs on the destructor path where its QWidget part may already be destroyed.
	const auto found = _taskbarListInterface.find(_linkedWindowId);
	return found != _taskbarListInterface.end() ? found->second.Get() : nullptr;
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	bool CTaskBarProgress::nativeEventFilter(const QByteArray & /*eventType*/, void * message, qintptr * /*result*/)
#else
	bool CTaskBarProgress::nativeEventFilter(const QByteArray & /*eventType*/, void * message, long * /*result*/)
#endif
{
	return eventFilter(message);
}

std::unordered_map<WId, ComPtr<ITaskbarList3>> CTaskBarProgress::_taskbarListInterface;
std::unordered_map<CTaskBarProgress*, QWidget*> CTaskBarProgress::_registeredWidgetsList;
std::unordered_map<WId, quint32 /* "taskbar button created" message ID */> CTaskBarProgress::_taskbarButtonCreatedMessageIdMap;
