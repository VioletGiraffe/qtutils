#ifndef CMOUSECLICKDETECTOR_H
#define CMOUSECLICKDETECTOR_H

#include "utils/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QObject>
#include <QPoint>
RESTORE_COMPILER_WARNINGS

#include <map>

class CMouseClickDetector : public QObject
{
	Q_OBJECT
public:
	explicit CMouseClickDetector(QObject *parent = 0);

	static CMouseClickDetector * globalInstance();

	void notify(QObject* object, QEvent* event);

signals:
	void singleLeftClickDetected(QObject* object, QPoint pos);
	void doubleLeftClickDetected(QObject* object, QPoint pos);

protected:
	bool eventFilter(QObject* object, QEvent* event) override;

private slots:

private:
	std::map<QObject*, ulong> _lastClickTimestampForObject;
};

#endif // CMOUSECLICKDETECTOR_H
