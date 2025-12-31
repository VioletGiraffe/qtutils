#pragma once

#include <QLabel>

class CLabelMidElision final : public QLabel
{
public:
	using QLabel::QLabel;

protected:
	bool event(QEvent* e) override;
	void paintEvent(QPaintEvent* e) override;
};
