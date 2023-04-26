#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QLineEdit>
RESTORE_COMPILER_WARNINGS

class CLineEdit : public QLineEdit
{
public:
	using QLineEdit::QLineEdit;

	void setSelectAllOnFocus(bool select);
	[[nodiscard]] bool selectAllOnFocus() const;

protected:
	void focusInEvent (QFocusEvent * event) override;

private:
	bool _bSelectAllOnFocus = true;
};
