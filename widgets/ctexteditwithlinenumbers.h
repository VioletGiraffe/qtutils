#pragma once
#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QPlainTextEdit>
RESTORE_COMPILER_WARNINGS

class CTextEditWithLineNumbers : public QPlainTextEdit
{
public:
	explicit CTextEditWithLineNumbers(QWidget *parent = nullptr) noexcept;

	void lineNumberAreaPaintEvent(QPaintEvent *event);
	[[nodiscard]] int lineNumberAreaWidth() const;

protected:
	void resizeEvent(QResizeEvent *event) override;

private:
	void updateLineNumberAreaWidth(int newBlockCount);
	void updateLineNumberArea(const QRect &rect, int dy);

private:
	QWidget *_lineNumberArea = nullptr;
};
