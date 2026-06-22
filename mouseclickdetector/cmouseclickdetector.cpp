#include "cmouseclickdetector.h"
#include "assert/advanced_assert.h"

DISABLE_COMPILER_WARNINGS
#include <QApplication>
#include <QMouseEvent>
#include <QTimer>
RESTORE_COMPILER_WARNINGS

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

			auto it = _lastClickTimestampForObject.find(object);
			if (it == _lastClickTimestampForObject.end())
			{
				it = _lastClickTimestampForObject.emplace(object, 0).first;
				// Clean up the entry once the object goes away, otherwise the map grows without bound over the app's lifetime.
				connect(object, &QObject::destroyed, this, [this](QObject* destroyedObject){
					_lastClickTimestampForObject.erase(destroyedObject);
				});
			}

			if (mouseEvent->timestamp() - it->second <= (unsigned int)QApplication::doubleClickInterval())
			{
				it->second = 0;
				emit doubleLeftClickDetected(object, pos);
			}
			else
			{
				it->second = (qint64)mouseEvent->timestamp();
				QTimer::singleShot(QApplication::doubleClickInterval() + 1, this, [this, object, pos](){
					// Look up by find(), not operator[]: the object may have been destroyed (and its entry erased) by now,
					// and operator[] would silently re-insert a stray entry for the now-dangling pointer.
					const auto timerIt = _lastClickTimestampForObject.find(object);
					if (timerIt != _lastClickTimestampForObject.end() && timerIt->second != 0)
					{
						timerIt->second = 0;
						emit singleLeftClickDetected(object, pos);
					}
				});
			}
		}
	}

	return QObject::eventFilter(object, event);
}
