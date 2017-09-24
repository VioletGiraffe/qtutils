#include "high_dpi_support.h"

#include "compiler/compiler_warnings_control.h"
#include "assert/advanced_assert.h"

DISABLE_COMPILER_WARNINGS
#include <QAbstractNativeEventFilter>
#include <QApplication>
RESTORE_COMPILER_WARNINGS

#ifdef _WIN32

#include <Windows.h>

static BOOL enableNonClientDpiScaling(HWND hWnd) {
	typedef BOOL(WINAPI *EnableNonClientDpiScalingFunc) (HWND);

	HMODULE hUser32 = LoadLibraryA("user32");
	if (hUser32 == nullptr)
		return FALSE;

	auto pEnableNonClientDpiScaling = (EnableNonClientDpiScalingFunc) ::GetProcAddress(hUser32, "EnableNonClientDpiScaling");
	if (pEnableNonClientDpiScaling == nullptr)
		return FALSE;

	return pEnableNonClientDpiScaling(hWnd);
}

class WinEventFilter : public QAbstractNativeEventFilter, QObject
{
public:
	inline WinEventFilter(QObject* parent): QObject(parent) {}

	inline bool nativeEventFilter(const QByteArray& /*eventType*/, void *message, long* /*result*/) override
	{
		auto winMsg = static_cast<MSG*>(message);
		if (winMsg->message == WM_NCCREATE)
		{
			const auto r = enableNonClientDpiScaling(winMsg->hwnd);
			if (r == FALSE)
			{
				WCHAR title[256] = {0};
				GetWindowTextW(winMsg->hwnd, title, 255);
				assert_unconditional_r((QString("EnableNonClientDpiScaling failed for window") + QString::fromWCharArray(title) + ", last error " + GetLastError()).toUtf8().data());
			}
		}

		return false;
	}
};

void enable_high_dpi_support()
{
	assert_and_return_r(qApp, );
	qApp->installNativeEventFilter(new WinEventFilter(qApp));
}

#else

void enable_high_dpi_support()
{
}

#endif
