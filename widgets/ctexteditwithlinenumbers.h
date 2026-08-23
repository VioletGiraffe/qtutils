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
	void changeEvent(QEvent *event) override;

private:
	// The number of the first block reaching below the top of the viewport
	[[nodiscard]] int firstVisibleBlockNumber() const;

	void updateLineNumberAreaWidth();
	void updateLineNumberAreaGeometry();
	void updateLineNumberArea();

private:
	QWidget *_lineNumberArea = nullptr;
	int _lineNumberAreaWidth = 0; // the viewport's left margin, and so the number area's width
};
