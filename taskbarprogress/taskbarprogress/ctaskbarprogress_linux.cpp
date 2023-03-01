#include "ctaskbarprogress.h"

DISABLE_COMPILER_WARNINGS
#include "linux/qxttooltip.h"
RESTORE_COMPILER_WARNINGS

CTaskBarProgress::CTaskBarProgress(QWidget * widget /*= 0*/) : _parent(widget)
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

