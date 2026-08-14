#pragma once

#include <QLabel>

class CLabelElided final : public QLabel
{
public:
	using QLabel::QLabel;

	void setElideMode(Qt::TextElideMode mode);
	[[nodiscard]] Qt::TextElideMode elideMode() const;

protected:
	bool event(QEvent* e) override;
	void paintEvent(QPaintEvent* e) override;

private:
	Qt::TextElideMode _elideMode = Qt::ElideMiddle;
};
