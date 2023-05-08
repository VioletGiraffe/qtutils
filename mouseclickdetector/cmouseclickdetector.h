#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QObject>
#include <QPoint>
RESTORE_COMPILER_WARNINGS

#include <map>

class CMouseClickDetector final : public QObject
{
	Q_OBJECT
public:
	explicit CMouseClickDetector(QObject *parent = nullptr) noexcept;

	[[nodiscard]] static CMouseClickDetector * globalInstance();

	void notify(QObject* object, QEvent* event);

signals:
	void singleLeftClickDetected(QObject* object, QPoint pos);
	void doubleLeftClickDetected(QObject* object, QPoint pos);

protected:
	bool eventFilter(QObject* object, QEvent* event) override;

private:
	std::map<QObject*, qint64> _lastClickTimestampForObject;
};

