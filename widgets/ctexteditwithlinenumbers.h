#pragma once
#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QTextEdit>
RESTORE_COMPILER_WARNINGS

class CTextEditWithLineNumbers : public QTextEdit
{
public:
	explicit CTextEditWithLineNumbers(QWidget *parent = nullptr) noexcept;

	void lineNumberAreaPaintEvent(QPaintEvent *event);
	[[nodiscard]] int lineNumberAreaWidth() const;

protected:
	void resizeEvent(QResizeEvent *event) override;

private:
	int getFirstVisibleBlockId();

	void updateLineNumberAreaWidth(int newBlockCount);
	void updateLineNumberArea();

private:
	QWidget *_lineNumberArea = nullptr;
};
