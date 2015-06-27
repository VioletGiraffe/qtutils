#ifndef CSETTINGSPAGE_H
#define CSETTINGSPAGE_H

#include "utils/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QWidget>
RESTORE_COMPILER_WARNINGS

class CSettingsPage : public QWidget
{
	Q_OBJECT
public:
	explicit CSettingsPage(QWidget *parent = 0);
	virtual ~CSettingsPage();

	virtual void acceptSettings() = 0;
};

#endif // CSETTINGSPAGE_H
