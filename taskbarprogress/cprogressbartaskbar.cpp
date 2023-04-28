#include "cprogressbartaskbar.h"

CProgressBarTaskbar::CProgressBarTaskbar(QWidget *parent) :
	QProgressBar(parent)
{
	connect (this, &CProgressBarTaskbar::valueChanged, this, &CProgressBarTaskbar::progressChanged);
}

void CProgressBarTaskbar::linkToWidgetstaskbarButton(QWidget *widget)
{
	if (widget)
		_taskBarProgress = std::make_shared<CTaskBarProgress>(widget);
}

void CProgressBarTaskbar::setState(ProgressState state)
{
	_state = state;
	updateState();
}

void CProgressBarTaskbar::setVisible(bool visible)
{
	QProgressBar::setVisible(visible);
	updateState();
}

void CProgressBarTaskbar::updateState()
{
	if (_taskBarProgress)
		_taskBarProgress->setState(isVisible() ? (_state) : psNoProgress);
}

void CProgressBarTaskbar::progressChanged(int progress)
{
	if (_taskBarProgress)
		_taskBarProgress->setProgress(progress, minimum(), maximum());
}

