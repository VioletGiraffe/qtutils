#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QLineEdit>
RESTORE_COMPILER_WARNINGS

class CLineEdit : public QLineEdit
{
	Q_OBJECT

public:
	using QLineEdit::QLineEdit;

	void setSelectAllOnFocus(bool select);
	[[nodiscard]] bool selectAllOnFocus() const;

signals:
	// As returnPressed(), with the modifiers held
	void returnPressedWithModifiers(Qt::KeyboardModifiers modifiers);

protected:
	void focusInEvent (QFocusEvent * event) override;
	void keyPressEvent(QKeyEvent* event) override;

private:
	bool _bSelectAllOnFocus = true;
};
