#ifndef CLINEEDIT_H
#define CLINEEDIT_H

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QLineEdit>
RESTORE_COMPILER_WARNINGS

class CLineEdit : public QLineEdit
{
public:
	explicit CLineEdit(QWidget * parent = 0);
	void setSelectAllOnFocus(bool select);
	bool selectAllOnFocus() const;

protected:
	void focusInEvent (QFocusEvent * event) override;

private:
	bool _bSelectAllOnFocus;
};

#endif // CLINEEDIT_H
