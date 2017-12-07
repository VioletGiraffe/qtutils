#pragma once

#include <QMainWindow>
#include <QTimer>

namespace Ui {
class CUiInspector;
}

class CUiInspector : public QMainWindow
{
public:
	explicit CUiInspector(QWidget *parent = 0);
	~CUiInspector();

private:
	void inspect();

private:
	QTimer _timer;

	Ui::CUiInspector *ui;
};

