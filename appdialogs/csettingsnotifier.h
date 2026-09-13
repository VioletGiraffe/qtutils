#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QObject>
RESTORE_COMPILER_WARNINGS

// Announces stored settings to holders of state derived from them: fonts, cached layout, a process path.
// CSettingsDialog emits it on accept, after every page has stored its values.
// A setting a page applies live, such as a theme preview, needs a separate notification.
class CSettingsNotifier final : public QObject
{
	Q_OBJECT

public:
	static CSettingsNotifier& instance()
	{
		static CSettingsNotifier notifier;
		return notifier;
	}

	void notifySettingsChanged() { emit settingsChanged(); }

signals:
	void settingsChanged();
};
