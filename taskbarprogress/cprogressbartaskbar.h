#ifndef CPROGRESSBARTASKBAR_H
#define CPROGRESSBARTASKBAR_H

#include "taskbarprogress/ctaskbarprogress.h"
#include <memory>
#include "../QtIncludes"

class CProgressBarTaskbar : public QProgressBar
{
	Q_OBJECT
public:
	explicit CProgressBarTaskbar(QWidget *parent = 0);
	void linkToWidgetstaskbarButton(QWidget * widget);

	void setState(ProgressState state);
	virtual void setVisible(bool visible);

private:
	void updateState();

private slots:
	void progressChanged(int progress);

private:
	std::shared_ptr<CTaskBarProgress> _taskBarProgress;
	ProgressState _state;
};

#endif // CPROGRESSBARTASKBAR_H
