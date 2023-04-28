#include "cmouseclickdetector.h"
#include "assert/advanced_assert.h"

DISABLE_COMPILER_WARNINGS
#include <QApplication>
#include <QMouseEvent>
#include <QTimer>
RESTORE_COMPILER_WARNINGS

CMouseClickDetector::CMouseClickDetector(QObject *parent) :
	QObject(parent)
{
}

CMouseClickDetector *CMouseClickDetector::globalInstance()
{
	static CMouseClickDetector inst;
	return &inst;
}

void CMouseClickDetector::notify(QObject * object, QEvent * event)
{
	eventFilter(object, event);
}

bool CMouseClickDetector::eventFilter(QObject * object, QEvent * event)
{
	if (event->type() == QEvent::MouseButtonRelease)
	{
		QMouseEvent * mouseEvent = dynamic_cast<QMouseEvent*>(event);
		assert_and_return_r(mouseEvent, false);
		if (mouseEvent->buttons() == Qt::LeftButton)
		{
			const QPoint pos = mouseEvent->pos();

			if (!_lastClickTimestampForObject.contains(object))
				_lastClickTimestampForObject[object] = 0;

			if (mouseEvent->timestamp() - _lastClickTimestampForObject[object] <= (unsigned int)QApplication::doubleClickInterval())
			{
				_lastClickTimestampForObject[object] = 0;
				emit doubleLeftClickDetected(object, pos);
			}
			else
			{
				_lastClickTimestampForObject[object] = (qint64)mouseEvent->timestamp();
				QTimer::singleShot(QApplication::doubleClickInterval() + 1, this, [this, object, pos](){
					if (_lastClickTimestampForObject[object] != 0)
					{
						_lastClickTimestampForObject[object] = 0;
						emit singleLeftClickDetected(object, pos);
					}
				});
			}
		}
	}

	return QObject::eventFilter(object, event);
}
