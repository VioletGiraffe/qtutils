#pragma once

#include "compiler/compiler_warnings_control.h"
#include "utility/named_type_wrapper.hpp"

DISABLE_COMPILER_WARNINGS
#include <QObject>
RESTORE_COMPILER_WARNINGS

#include <utility>

class QWidget;

class CPersistenceEnabler final : public QObject
{
public:
	using Delayed = UniqueNamedBoolType;
	using SetDefaultSize = UniqueNamedBoolType;

	// Installs itself on `widget` and is parented to it.
	// widgetSettingsPath must be application-wide unique: it is the QSettings path this widget's state and
	// position are stored under between launches.
	explicit CPersistenceEnabler(QString widgetSettingsPath, QWidget* widget, Delayed delayed = Delayed{ true }, SetDefaultSize setDefaultSize = SetDefaultSize{ true });

protected:
	bool eventFilter(QObject *watched, QEvent *event) override;

private:
	void restoreState();
	void saveState() const;

private:
	const QString _settingsPath;
	QWidget* const _widget; // the parent, so it outlives this filter
	bool _stateRestored = false;
	const bool _delayed;
	const bool _setDefaultSize;
};

// Persists the widget's geometry and state; see the constructor for widgetSettingsPath
inline void enablePersistence(QWidget* widget, QString widgetSettingsPath,
	CPersistenceEnabler::Delayed delayed = CPersistenceEnabler::Delayed{ true },
	CPersistenceEnabler::SetDefaultSize setDefaultSize = CPersistenceEnabler::SetDefaultSize{ true })
{
	new CPersistenceEnabler{ std::move(widgetSettingsPath), widget, delayed, setDefaultSize }; // parented to the widget, which owns it from here
}
