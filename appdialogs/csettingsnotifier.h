#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QObject>
RESTORE_COMPILER_WARNINGS

// Announces stored settings to whoever holds state derived from them - fonts, cached layout, a
// process path. A dialog announces on accept, once every page has stored its values; a setting a
// page applies live instead, a theme preview say, needs a notification of its own.
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
