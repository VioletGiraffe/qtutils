#pragma once
#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QPlainTextEdit>
RESTORE_COMPILER_WARNINGS

class CPlainTextEditWithLineNumbers : public QPlainTextEdit
{
public:
	explicit CPlainTextEditWithLineNumbers(QWidget *parent = nullptr) noexcept;

	void lineNumberAreaPaintEvent(QPaintEvent *event);
	[[nodiscard]] int lineNumberAreaWidth() const;

protected:
	void resizeEvent(QResizeEvent *event) override;
	void changeEvent(QEvent *event) override;

private:
	void updateLineNumberAreaWidth();
	void updateLineNumberAreaGeometry();
	// Mirrors the viewport update QPlainTextEdit reports: dy is the distance it scrolled, 0 for a repaint in place
	void updateLineNumberArea(const QRect& rect, int dy);

private:
	QWidget *_lineNumberArea = nullptr;
	int _lineNumberAreaWidth = 0; // the viewport's left margin, and so the number area's width
};
