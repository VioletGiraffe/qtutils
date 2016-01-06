#ifndef CPROGRESSBARTASKBAR_H
#define CPROGRESSBARTASKBAR_H

#include "taskbarprogress/ctaskbarprogress.h"

DISABLE_COMPILER_WARNINGS
#include <QProgressBar>
RESTORE_COMPILER_WARNINGS

#include <memory>

class CProgressBarTaskbar : public QProgressBar
{
public:
	explicit CProgressBarTaskbar(QWidget *parent = 0);
	void linkToWidgetstaskbarButton(QWidget * widget);

	void setState(ProgressState state);
	virtual void setVisible(bool visible);

private:
	void updateState();
	void progressChanged(int progress);

private:
	std::shared_ptr<CTaskBarProgress> _taskBarProgress;
	ProgressState _state;
};

#endif // CPROGRESSBARTASKBAR_H
