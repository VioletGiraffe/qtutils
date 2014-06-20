#include "cmouseclickdetector.h"
#include <assert.h>

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
		assert(mouseEvent);
		if (mouseEvent->buttons() == Qt::LeftButton)
		{
			const QPoint pos = mouseEvent->pos();

			if (_lastClickTimestampForObject.count(object) < 1)
				_lastClickTimestampForObject[object] = 0;

			if (mouseEvent->timestamp() - _lastClickTimestampForObject[object] <= (unsigned int)QApplication::doubleClickInterval())
			{
				_lastClickTimestampForObject[object] = 0;
				emit doubleLeftClickDetected(object, pos);
			}
			else
			{
				_lastClickTimestampForObject[object] = mouseEvent->timestamp();
				QTimer * timer = new QTimer;
				timer->setSingleShot(true);
				QObject::connect(timer, &QTimer::timeout, [=](){
					if (_lastClickTimestampForObject[object] != 0)
					{
						_lastClickTimestampForObject[object] = 0;
						timer->deleteLater();
						emit singleLeftClickDetected(object, pos);
					}
				});
				timer->start(QApplication::doubleClickInterval()+1);
			}
		}
	}

	return QObject::eventFilter(object, event);
}
