#pragma once

#include "taskbarprogress/ctaskbarprogress.h"

DISABLE_COMPILER_WARNINGS
#include <QProgressBar>
RESTORE_COMPILER_WARNINGS

#include <memory>

class CProgressBarTaskbar final : public QProgressBar
{
public:
	explicit CProgressBarTaskbar(QWidget *parent = nullptr) noexcept;
	void linkToWidgetstaskbarButton(QWidget * widget);

	void setState(ProgressState state);
	void setVisible(bool visible) override;

private:
	void updateState();
	void progressChanged(int progress);

private:
	std::shared_ptr<CTaskBarProgress> _taskBarProgress;
	ProgressState _state = psNormal;
};
