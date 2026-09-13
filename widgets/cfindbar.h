#pragma once

#include "compiler/compiler_warnings_control.h"
#include "findresult.h"

DISABLE_COMPILER_WARNINGS
#include <QFrame>
#include <QKeySequence>
#include <QList>
#include <QPointer>
#include <QString>
#include <QTextDocument>
RESTORE_COMPILER_WARNINGS

#include <functional>

class CHistoryComboBox;
class QAction;
class QCheckBox;
class QLabel;
class QRegularExpression;
class QShowEvent;

// Find bar: a pattern field with history, previous and next, case, whole-word and regex toggles, a status label, a close button.
//   Searches through the host's find functions, which must wrap around at either end.
//   The host must add findActions() to a visible widget of its window, e.g. a menu: shortcuts on the hidden bar never fire.
//   Hidden until any of its actions; only Find moves the focus into it. Enter searches forward, Shift+Enter backward.
//   While visible, the bar takes Esc from its window's shortcuts and hides on it; a widget that handles Esc itself still gets it first.
//   Hiding on Esc or the close button moves focus from the bar back to the widget focused before activate().
//   Derives QFrame: a stylesheet background does not paint on a plain QWidget subclass.
class CFindBar final : public QFrame
{
public:
	using FindText = std::function<FindResult (const QString& pattern, QTextDocument::FindFlags flags)>;
	using FindRegex = std::function<FindResult (const QRegularExpression& pattern, QTextDocument::FindFlags flags)>;

	struct Keys
	{
		QKeySequence find;
		QKeySequence findNext;
		QKeySequence findPrevious;
	};

	// settingsRootKey: the QSettings group for the pattern history and the toggles; empty for no persistence
	CFindBar(FindText findText, FindRegex findRegex, const Keys& keys, QString settingsRootKey, QWidget* parent = nullptr);

	// Find, Find next and Find previous
	[[nodiscard]] QList<QAction*> findActions() const;

	// Shows the bar and focuses the pattern field, its text selected
	void activate();

protected:
	bool eventFilter(QObject* watched, QEvent* event) override;
	void showEvent(QShowEvent* event) override;

private:
	void deactivate();

	// Shows the bar; activates it instead of searching while the pattern is empty
	void findMatch(bool backward);

private:
	const FindText _findText;
	const FindRegex _findRegex;
	const QString _settingsRootKey;
	QPointer<QWidget> _focusBeforeActivation;

	QAction* _findAction = nullptr;
	QAction* _findNextAction = nullptr;
	QAction* _findPreviousAction = nullptr;

	CHistoryComboBox* _patternBox = nullptr;
	QCheckBox* _caseSensitiveBox = nullptr;
	QCheckBox* _wholeWordsBox = nullptr;
	QCheckBox* _regexBox = nullptr;
	QLabel* _statusLabel = nullptr;
};
