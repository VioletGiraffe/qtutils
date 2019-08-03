#include "ctaskbarprogress.h"
#include "linux/qxttooltip.h"

CTaskBarProgress::CTaskBarProgress(QWidget * widget /*= 0*/) : _parent(widget)
{
}

CTaskBarProgress::~CTaskBarProgress()
{
}

void CTaskBarProgress::linkToWidgetsTaskbarButton(QWidget * widget)
{
}

void CTaskBarProgress::setProgress(int progress, int minValue /*= 0*/, int maxValue /*= 100*/)
{
}

void CTaskBarProgress::setState(ProgressState state)
{
}

