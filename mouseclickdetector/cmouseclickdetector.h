#ifndef CMOUSECLICKDETECTOR_H
#define CMOUSECLICKDETECTOR_H

#include <QObject>
#include <QPoint>

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
