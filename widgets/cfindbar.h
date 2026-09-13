#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QFrame>
RESTORE_COMPILER_WARNINGS

class QCheckBox;
class QKeyEvent;
class QLabel;
class CLightningFastViewerWidget;
class CLineEdit;

// Find bar for a CLightningFastViewerWidget: a pattern field, previous and next, case, whole-word and regex toggles, a status label.
//   Hidden until the Find key. The find shortcuts are installed on the viewer's window, so the host only places the bar.
//   Enter and FindNext search forward, Shift+Enter and FindPrevious backward. The search wraps around at either end.
//   Esc in the bar hides it and returns focus to the viewer.
//   Derives QFrame: a stylesheet background does not paint on a plain QWidget subclass.
class CFindBar final : public QFrame
{
public:
	explicit CFindBar(CLightningFastViewerWidget* viewer, QWidget* parent = nullptr);

	// Shows the bar and focuses the pattern field, its text selected
	void activate();

protected:
	void keyPressEvent(QKeyEvent* event) override;

private:
	// Opens the bar instead while the pattern is empty
	void findMatch(bool backward);

private:
	CLightningFastViewerWidget* const _viewer;
	CLineEdit* _patternEdit = nullptr;
	QCheckBox* _caseSensitiveBox = nullptr;
	QCheckBox* _wholeWordsBox = nullptr;
	QCheckBox* _regexBox = nullptr;
	QLabel* _statusLabel = nullptr;
};
